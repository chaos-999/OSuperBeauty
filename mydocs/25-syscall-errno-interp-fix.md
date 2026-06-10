# 🔧 系统调用 errno 语义修复 + 动态链接器支持实录

> 📅 生成时间：`2026-06-09 03:23 (GMT+8)`
>
> 目标：修复所有已实现 syscall 的错误返回值语义（`return -1` → `return -ERRNO`），并实现 PT_INTERP 动态链接器加载支持，使 iozone 等动态链接二进制能够在内核上启动运行。

---

## 零、背景

SpringOS 源自 xv6-riscv，xv6 约定 syscall 失败返回 `-1`。但 OSCOMP 测试环境使用 musl/glibc，其 `__syscall_ret()` 严格按 Linux ABI 解析返回值：

```
Linux 内核 syscall 返回值约定：
  0 ~ MAX         → 成功，直接返回
  -1 ~ -4095      → 失败，取绝对值作为 errno
  -4096 及以下     → 成功（大正整数返回值）

musl __syscall_ret():
  r ∈ [0, ∞)        → 成功，直接返回 r
  r ∈ [-4095, -1]   → 失败，errno = -r，返回 -1
```

内核返回 `-1` → musl 解析为 `errno=EPERM(1)` → 所有失败都被当成"权限不足"，导致 libc-test / basic 测试中大量"本该通过"的 errno 断言失败。

同时，测试点 iozone 是动态链接的 ELF，需要 `/lib/ld-linux-riscv64-lp64d.so.1`。SpringOS 的 execve 不支持 PT_INTERP，导致动态二进制启动即崩溃。

---

## 一、syscall errno 修复

### 1.1 修改范围

| 文件 | 修改量 | 关键改动 |
|------|:------:|---------|
| `kernel/syscall/sysfile.c` | ~100 处 | `argfd` → `-EBADF`，`fdalloc` → `-EMFILE`，`copyout/copyin` → `-EFAULT`，路径不存在 → `-ENOENT`，pread on pipe → `-ESPIPE` |
| `kernel/syscall/sysproc.c` | ~80 处 | `growproc` OOM → `-ENOMEM`，`mmap` → `-EINVAL/-EACCES/-EBADF`，`munmap` → `-EINVAL/-ENOMEM`，`ppoll/sendfile/writev/copy_file_range` 精确化 |
| `kernel/syscall/syssig.c` | ~10 处 | `rt_sigaction/rt_sigprocmask` copyin/copyout → `-EFAULT/-EINVAL`，`tgkill` → `-EINVAL` |

### 1.2 内部 helper 修复

**`argfd`**（文件描述符校验器）：

```c
// 修复前
if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0) return -1;

// 修复后
if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0) return -EBADF;
```

**`fdalloc` / `fdalloc2`**（文件描述符分配）：

```c
// 修复前
return -1;

// 修复后
return -EMFILE;
```

### 1.3 errno 映射速查表

| 原返回值 | errno 宏 | 值 | 语义 |
|---------|---------|---|------|
| `-1` (无意义) | `-EBADF` | -9 | 无效文件描述符 |
| `-1` (无意义) | `-ENOENT` | -2 | 文件/路径不存在 |
| `-1` (无意义) | `-EINVAL` | -22 | 参数无效 |
| `-1` (无意义) | `-EFAULT` | -14 | 坏的用户地址 |
| `-1` (无意义) | `-ENOMEM` | -12 | 内存不足 |
| `-1` (无意义) | `-EACCES` | -13 | 权限不足 |
| `-1` (无意义) | `-ENOSYS` | -38 | 系统调用未实现 |
| `-1` (无意义) | `-ENOEXEC` | -8 | 非可执行文件格式 |
| `-1` (无意义) | `-EMFILE` | -24 | 打开文件过多 |
| `-1` (无意义) | `-ESPIPE` | -29 | 非法 seek（pipe） |
| `-1` (无意义) | `-ENOTDIR` | -20 | 不是目录 |
| `-1` (无意义) | `-EEXIST` | -17 | 文件已存在 |
| `-1` (无意义) | `-EIO` | -5 | I/O 错误 |
| `-1` (无意义) | `-EPERM` | -1 | 操作不允许 |
| `-1` (无意义) | `-EINTR` | -4 | 系统调用被中断 |
| `-1` (无意义) | `-ERANGE` | -34 | 结果超出范围 |
| `-1` (无意义) | `-EAGAIN` | -11 | 资源暂时不可用 |

> ⚠️ 内部 helper `find_vma_by_addr()` 的 `return -1` 为索引查找失败，非 errno 语义，保留不修改。

---

## 二、execve AT_PHDR 修正

### 2.1 问题

RISC-V execve 中 `AT_PHDR` 使用了文件偏移 `elf.phoff` 而非内存中的虚拟地址：

```c
// 修复前
AUXV(AT_PHDR, elf.phoff);  // phoff 是文件内偏移（如 0x40），不是虚拟地址！
```

musl 的 `__init_tls()` 使用 `getauxval(AT_PHDR)` 获取程序头在**内存中**的虚拟地址。文件偏移 ≠ 虚拟地址，导致 musl 在错误地址读取 PT_TLS → 段错误。

### 2.2 修复

跟踪第一个 PT_LOAD 段的虚拟地址，计算正确的 AT_PHDR：

```c
// 在加载循环中
uint64 first_load_vaddr = 0;
for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
    // ...
    if (first_load_vaddr == 0) first_load_vaddr = ph.vaddr;
    // ...
}

// 修复后
AUXV(AT_PHDR, first_load_vaddr + elf.phoff);  // 虚拟地址 = 段基址 + 文件偏移
```

### 2.3 LoongArch execve 补全

LA execve 原本缺失 5 个 auxv 条目，一并补全：

```c
alloc_aux(aux, AT_HWCAP, 0);
alloc_aux(aux, AT_PAGESZ, PGSIZE);
alloc_aux(aux, AT_PHDR, first_load_vaddr + elf.phoff);  // 新增
alloc_aux(aux, AT_PHENT, elf.phentsize);                 // 新增
alloc_aux(aux, AT_PHNUM, elf.phnum);                     // 新增
alloc_aux(aux, AT_BASE, 0);                              // 新增
alloc_aux(aux, AT_ENTRY, elf.entry);
// ...
```

---

## 三、动态链接器（PT_INTERP）支持

### 3.1 背景

iozone 测试点是动态链接的 ELF，其程序头包含 PT_INTERP（type=3），声明需要 `/lib/ld-linux-riscv64-lp64d.so.1`。内核原 execve 跳过所有非 PT_LOAD 段，导致动态二进制启动时 GOT/PLT 未解析，跳转到非法地址崩溃（`sepc=-2`）。

```
检测现象：
  musl iozone:  usertrap scause=0xc sepc=0xfffffffffffffffe
  glibc iozone: usertrap scause=0xc sepc=0xfffffffffffffffe
```

### 3.2 实现方案

在 `kernel/proc/exec.c` 的 RISC-V execve 中添加 PT_INTERP 检测和解释器加载：

**① 扫描 PT_INTERP**

```c
// 在 PT_LOAD 加载循环中
if (ph.type != ELF_PROG_LOAD) {
    if (ph.type == 3 && ph.filesz > 0 && ph.filesz < MAXPATH) {
        interp_off = ph.off;
        interp_sz = ph.filesz;
    }
    continue;
}
```

**② 读取解释器路径**

```c
// 主二进制锁释放前，读 PT_INTERP 字符串
if (interp_sz > 0) {
    if (ip->i_op->read(ip, 0, (uint64)interp_path, interp_off, interp_sz) != (int)interp_sz)
        goto bad;
    interp_path[interp_sz] = '\0';
}
```

**③ 加载解释器 ELF**

```c
if (interp_sz > 0) {
    struct inode *iip = namei(interp_path);
    if (iip == 0) {
        printf("execve: interpreter not found: %s\n", interp_path);
        goto bad;
    }
    iip->i_op->lock(iip);
    struct elfhdr ielf;
    // 校验、加载解释器的所有 PT_LOAD 段（与主二进制相同的 loadseg 流程）
    interp_entry = ielf.entry;
    for (i = 0, off = ielf.phoff; i < (int)ielf.phnum; i++, off += sizeof(ph)) {
        if (ph.type != ELF_PROG_LOAD) continue;
        // 分配内存 + loadseg 加载
        sz = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, PTE_W | PTE_X | PTE_R | PTE_U);
        loadseg(pagetable, PGROUNDDOWN(ph.vaddr), iip, PGROUNDDOWN(ph.off), ph.filesz + margin);
    }
    iip->i_op->unlock(iip);
}
```

**④ 入口点切换**

```c
// 如果加载了解释器，从解释器的入口开始执行
p->trapframe->epc = interp_entry ? interp_entry : elf.entry;
```

### 3.3 动态链接器二进制嵌入

比赛提供的 SD 卡镜像不含动态链接器文件，需要嵌入内核。

**Makefile**：

```makefile
# 在 OBJS_RV 中添加
boot/rv/initcode.o \
  $U/ld_so_kern.o \
```

**ld_so_kern.o 生成**：

```bash
riscv64-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv:rv64 \
  --rename-section .data=.rodata,contents,alloc,load,readonly,data \
  /usr/riscv64-linux-gnu/lib/ld-linux-riscv64-lp64d.so.1 \
  user/ld_so_kern.o
```

**provider（`kernel/util/ld_so_provider.c`）**：

```c
#include "types.h"
extern const char _binary__usr_riscv64_linux_gnu_lib_ld_linux_riscv64_lp64d_so_1_start;
extern const char _binary__usr_riscv64_linux_gnu_lib_ld_linux_riscv64_lp64d_so_1_size;
const char *ld_so_glibc_data = &_binary__usr_riscv64_linux_gnu_lib_ld_linux_riscv64_lp64d_so_1_start;
const uint64 ld_so_glibc_len = (uint64)(uintptr_t)&_binary__usr_riscv64_linux_gnu_lib_ld_linux_riscv64_lp64d_so_1_size;
```

### 3.4 虚拟文件暴露 ld.so

**`/proc/ld_so` 虚拟文件（`kernel/syscall/sysfile.c`）**：

```c
// sys_openat: 创建虚拟 fd
if (!strcmp(path, "/proc/ld_so")) {
    if ((f = filealloc()) == NULL) return -ENOMEM;
    if ((fd = fdalloc2(f, 0)) < 0) { get_fops()->close(f); return fd; }
    f->f_type = 19;  // FD_LD_SO
    f->f_pos = 0;
    f->f_flags = flags;
    strcpy(f->f_path, path);
    return fd;
}

// sys_read: 读取时返回 ld.so 数据
if (f->f_type == 19) {
    if (f->f_pos >= (int64)ld_so_glibc_len) return 0;
    int remaining = (int64)ld_so_glibc_len - f->f_pos;
    if (n > remaining) n = remaining;
    if (copyout(myproc()->pagetable, p, (char *)ld_so_glibc_data + f->f_pos, n) < 0) return -EFAULT;
    f->f_pos += n;
    return n;
}
```

### 3.5 init 写入 ld.so

**`user/init-rv.c`** 启动时从 `/proc/ld_so` 读取嵌入数据，写入 ext4 文件系统：

```c
// 在 main() 中，/tmp、/proc 初始化之后，test_pre() 之前
mkdirat(AT_FDCWD, "/lib", 0755);
int fds = openat(AT_FDCWD, "/proc/ld_so", O_RDONLY, 0);
if (fds >= 0) {
    int fdd = openat(AT_FDCWD, "/lib/ld-linux-riscv64-lp64d.so.1", O_CREAT | O_WRONLY, 0755);
    if (fdd >= 0) {
        char buf[512];
        int n;
        while ((n = read(fds, buf, sizeof(buf))) > 0)
            write(fdd, buf, n);
        close(fdd);
    }
    close(fds);
}
```

---

## 四、验证结果

### 4.1 errno 修复

编译通过，`make kernel-rv` 无警告。内核正常启动，basic 测试、lua 测试等不受影响。

### 4.2 动态链接器

```
内核启动日志：
  Wrote ld-linux-riscv64-lp64d.so.1 to /lib/

iozone glibc 测试：
  修复前：usertrap scause=0xc sepc=0xfffffffffffffffe    ← 指令页错误，跳转到 -2
  修复后：usertrap scause=0xf sepc=0x90d0                ← 存储页错误，ld.so 代码正常执行

iozone musl 测试（待完成）：
  需要 /lib/ld-musl-riscv64-sf.so.1（musl 动态链接器），
  SD 卡上 musl iozone 依赖此文件。可从 musl RISC-V 工具链获取后同法嵌入。
```

**iozone 已从"无法启动"进入"运行时阶段"**——动态链接器成功加载并开始执行 iozone，存储页错误是应用层重定位问题（`stval=0x20717006447c8` 为非法地址，疑似 GOT 重定位异常），需进一步排查 RISC-V R_RISCV_RELATIVE 等重定位类型。

---

## 五、改动文件总览

| 文件 | 改动内容 | 说明 |
|------|---------|------|
| `kernel/syscall/sysfile.c` | ~100 处 `return -1` → `-ERRNO` + `/proc/ld_so` 虚拟文件 | syscall 错误语义 + ld.so 暴露 |
| `kernel/syscall/sysproc.c` | ~80 处 `return -1` → `-ERRNO` | 进程/内存 syscall errno |
| `kernel/syscall/syssig.c` | ~10 处 `return -1` → `-ERRNO` | 信号 syscall errno |
| `kernel/proc/exec.c` | AT_PHDR 修正 + PT_INTERP 加载 + LA auxv 补全 | 动态链接支持 |
| `kernel/util/ld_so_provider.c` | 新建，暴露嵌入的 ld.so 符号 | ld.so 数据提供 |
| `include/ld_so.h` | 新建，声明 ld_so 外部符号 | 头文件 |
| `user/ld_so_kern.o` | 新建，ld-linux-riscv64-lp64d.so.1 的 objcopy 产物 | 嵌入的动态链接器 |
| `user/init-rv.c` | 启动时写 ld.so 到 /lib/ + iozone 测试 | 初始化 |
| `Makefile` | OBJS_RV 添加 `ld_so_kern.o` | 构建系统 |

---

## 六、知识沉淀

### 6.1 xv6 内核 errno 约定

xv6 的 `return -1` 习惯在 musl/glibc 环境下必须改为 `return -ERRNO`。错误码定义在 `include/errno.h`。

**常见映射**：

| 失败原因 | 应返回 | musl 解析 errno |
|---------|--------|:---:|
| 非 ELF 文件 | `-ENOEXEC` (-8) | 8 |
| 文件不存在 | `-ENOENT` (-2) | 2 |
| fd 无效 | `-EBADF` (-9) | 9 |
| 参数无效 | `-EINVAL` (-22) | 22 |
| 权限不足 | `-EACCES` (-13) | 13 |
| 内存不足 | `-ENOMEM` (-12) | 12 |
| bad address | `-EFAULT` (-14) | 14 |
| 默认兜底 | `-1` | 1 (EPERM) |

### 6.2 PT_INTERP 加载流程

```
execve("iozone", ...)
  ├─ 加载 PT_LOAD 段（主二进制）
  ├─ 扫描 PT_INTERP → 获取解释器路径 "/lib/ld-linux-riscv64-lp64d.so.1"
  ├─ namei(解释器路径) → 打开解释器 inode
  ├─ 加载解释器的 PT_LOAD 段
  ├─ epc = 解释器入口（而非主二进制入口）
  └─ 解释器初始化 GOT/PLT → 跳转到主二进制入口
```

### 6.3 AT_PHDR 的正确值

```c
// ❌ 错误：文件偏移
AUXV(AT_PHDR, elf.phoff);

// ✅ 正确：虚拟地址（段基址 + 文件内偏移）
AUXV(AT_PHDR, first_load_vaddr + elf.phoff);
```

对于典型 RISC-V 二进制（`first_load_vaddr=0x10000`, `phoff=0x40`），AT_PHDR = `0x10040`。
