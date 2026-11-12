# RV 动态链接缺失支持修复：PT_INTERP 解释器加载

> 记录 libc-test `run-dynamic.sh` 中所有动态测试统一
> `scause=0xc sepc=0xfffffffffffffffe` 的根因与修复。
>
> 结论：**RISC-V 的 `execve()` 只加载了主 ELF 的 `PT_LOAD`，从未处理
> `PT_INTERP`。动态程序没有动态链接器做 GOT/PLT 重定位，跳到非法地址 -2。
> 修复方式是解析 `PT_INTERP`、在独立基址加载解释器、把入口切换到解释器，
> 并补齐 `AT_BASE`；同时兼容镜像里没有 `/lib`、解释器实际放在 `/musl/lib`
> 或 `/glibc/lib` 的布局。**

---

## 1. 问题描述

静态测试正常，但执行：

```text
libctest_testcode.sh -> run-dynamic.sh -> runtest.exe -w entry-dynamic.exe ...
```

所有动态测试统一失败：

```text
========== START entry-dynamic.exe argv ==========
usertrap(): unexpected scause 0xc pid=4
            sepc=0xfffffffffffffffe stval=0xfffffffffffffffe
FAIL argv [status 255]
========== END entry-dynamic.exe argv ==========
```

关键信息：

- `scause=0xc`：Instruction Page Fault
- `sepc = 0xfffffffffffffffe`：程序计数器跑到了非法地址
- `stval = 0xfffffffffffffffe`：访问的也是非法地址
- 每个动态测试都是同样的 `sepc=-2`

这说明不是某个 libc-test 用例逻辑失败，而是**所有动态二进制在启动阶段就崩溃了**。

---

## 2. 动态链接的基本流程

静态 ELF 只需要内核把 `PT_LOAD` 段映射到内存，然后跳到 `elf.entry`。

动态 ELF 多了一步：

```text
内核加载主程序 PT_LOAD
  → 读取 PT_INTERP 指定的解释器路径
  → 加载解释器（musl 的 libc.so / glibc 的 ld-linux）
  → 把入口设置为解释器入口，不是主程序入口
  → 通过 auxv 告诉解释器：
        AT_PHDR  = 主程序 program headers 地址
        AT_ENTRY = 主程序入口
        AT_BASE  = 解释器加载基址
  → 解释器自己完成：
        重定位主程序 GOT/PLT
        加载 DT_NEEDED 依赖库
        最后跳转到 AT_ENTRY 执行主程序
```

测试镜像中的实际文件布局：

```text
/musl/entry-dynamic.exe                 // 动态主程序
/musl/lib/libc.so                       // musl libc，同时是 musl 动态链接器
/glibc/lib/ld-linux-riscv64-lp64d.so.1  // glibc 动态链接器
/glibc/lib/libc.so.6                    // glibc libc
```

但动态主程序的 `PT_INTERP` 通常写的是：

```text
/lib/ld-musl-riscv64-sf.so.1
```

镜像里没有 `/lib` 目录，所以还需要做路径映射。

---

## 3. 根因

### 3.1 RISC-V `execve()` 只处理 `PT_LOAD`

修复前 `kernel/proc/exec.c` 的主循环：

```c
for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
    ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph));
    if (ph.type != ELF_PROG_LOAD)
        continue;
    ...
}
```

`PT_INTERP` 被直接忽略了。

### 3.2 入口仍然是主程序 `elf.entry`

```c
p->trapframe->epc = elf.entry;
```

动态主程序的 `_start` 会调用 PLT：

```asm
_start:
    ...
    j  __libc_start_main@plt
```

但 GOT/PLT 还没有被动态链接器重定位，PLT 最终跳到一个非法地址，
于是 `sepc = 0xfffffffffffffffe`，触发 `scause=0xc`。

### 3.3 `AT_BASE` 是 0

```c
AUXV(AT_BASE, 0);
```

即使内核强行跳到某个解释器入口，解释器也不知道自己加载在哪里，
无法完成自重定位。

### 3.4 解释器路径 `/lib/...` 在镜像里不存在

镜像只有：

```text
/musl/lib/libc.so
/glibc/lib/ld-linux-riscv64-lp64d.so.1
```

所以直接 `namei("/lib/ld-musl-riscv64-sf.so.1")` 会失败，需要 fallback。

### 3.5 附带发现：init 栈第 3 页未映射

当前 `init-rv.c` 编译出来的 initcode 小于 8 KiB，但初始 `sp = 4 * PGSIZE`。
`uvmfirst()` 原来只在 `sz > 2 * PGSIZE` 时才映射第 3 页。
initcode 小于 8 KiB 时第一次压栈访问 `0x3ff8` 就会 page fault：

```text
vma lazy touch failed
usertrap(): unexpected scause 0xf
            sepc=0x49e stval=0x3ff8
```

这是动态测试之外顺带暴露的独立问题，修复见 4.5。

---

## 4. 代码修改

### 4.1 `include/elf.h`：补上 `PT_INTERP` 常量

```c
// Values for Proghdr type
#define ELF_PROG_LOAD   1
#define ELF_PROG_INTERP 3
```

---

### 4.2 `exec_open_interp()`：查找解释器

文件：`kernel/proc/exec.c`

```c
static struct inode *exec_open_interp(const char *path) {
    /* 先按 ELF 中写的原始路径找，例如 /lib/ld-musl-riscv64-sf.so.1 */
    struct inode *ip = namei((char *)path);
    if (ip)
        return ip;

    /* 取 basename，用于在镜像实际目录中查找 */
    const char *base = path;
    for (const char *p = path; *p; p++)
        if (*p == '/')
            base = p + 1;

    char alt[MAXPATH];
    const char *dirs[] = {"/musl/lib/", "/glibc/lib/"};

    for (int d = 0; d < (int)(sizeof(dirs) / sizeof(dirs[0])); d++) {
        int n = strlen(dirs[d]);
        safestrcpy(alt, dirs[d], MAXPATH);
        safestrcpy(alt + n, base, MAXPATH - n);
        ip = namei(alt);
        if (ip)
            return ip;
    }

    /*
     * musl 的 libc.so 同时就是动态链接器本身。
     * 镜像里没有 /lib/ld-musl-*.so.1，只有 /musl/lib/libc.so。
     */
    if (strncmp(base, "ld-musl-riscv64", 15) == 0)
        return namei("/musl/lib/libc.so");

    return 0;
}
```

查找顺序：

```text
1. /lib/ld-musl-riscv64-sf.so.1          （原路径）
2. /musl/lib/ld-musl-riscv64-sf.so.1     （镜像实际目录）
3. /glibc/lib/ld-musl-riscv64-sf.so.1
4. /musl/lib/libc.so                     （musl fallback）
```

glibc 动态程序则会在第 3 步找到：

```text
/glibc/lib/ld-linux-riscv64-lp64d.so.1
```

---

### 4.3 `exec_load_interp()`：加载解释器

文件：`kernel/proc/exec.c`

解释器是一个 ET_DYN 共享对象，不能直接放到主程序的地址空间里，
必须选一个独立基址。这里使用 `0x40000000`。

```c
static int exec_load_interp(pagetable_t pagetable, struct inode *ip,
                            uint64 base, uint64 *entry_out) {
    struct elfhdr lelf;
    struct proghdr ph;
    uint64 off;
    int i;

    /* 读取解释器 ELF 头 */
    if (ip->i_op->read(ip, 0, (uint64)&lelf, 0, sizeof(lelf)) != sizeof(lelf))
        return -1;
    if (lelf.magic != ELF_MAGIC)
        return -1;

    for (i = 0, off = lelf.phoff; i < lelf.phnum; i++, off += sizeof(ph)) {
        if (ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
            return -1;
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz)
            return -1;
        if (ph.vaddr + ph.memsz < ph.vaddr)
            return -1;

        uint64 va = base + ph.vaddr;
        uint64 vend = va + ph.memsz;
        if (va < base || vend < va)
            return -1;

        /*
         * 解释器不进入 p->sz，直接映射到 base 以上的固定区域。
         * 这样不会破坏主程序 brk/heap 的起点。
         */
        for (uint64 pg = PGROUNDDOWN(va); pg < PGROUNDUP(vend); pg += PGSIZE) {
            char *mem = kalloc();
            if (mem == 0)
                return -1;
            memset(mem, 0, PGSIZE);
            if (mappages(pagetable, pg, PGSIZE, (uint64)mem,
                         PTE_W | PTE_X | PTE_R | PTE_U) < 0) {
                kfree(mem);
                return -1;
            }
        }

        /* 拷贝文件内容；va 可能不是页对齐的 */
        uint64 fend = va + ph.filesz;
        for (uint64 pg = PGROUNDDOWN(va); pg < PGROUNDUP(fend); pg += PGSIZE) {
            uint64 pa = walkaddr(pagetable, pg);
            if (pa == 0)
                return -1;

            uint64 skip = (pg < va) ? (va - pg) : 0;
            uint64 foff = ph.off + (pg + skip - va);
            uint64 n = PGSIZE - skip;
            if (pg + skip + n > fend)
                n = fend - (pg + skip);

            if (n != 0 && ip->i_op->read(ip, 0, pa + skip, foff, n) != (int)n)
                return -1;
        }

        /* 清零 BSS */
        if (ph.memsz > ph.filesz) {
            uint64 bss = va + ph.filesz;
            for (uint64 pg = PGROUNDDOWN(bss); pg < vend; pg += PGSIZE) {
                uint64 pa = walkaddr(pagetable, pg);
                if (pa == 0)
                    continue;
                uint64 skip = (pg < bss) ? (bss - pg) : 0;
                uint64 len = PGSIZE - skip;
                if (pg + len > vend)
                    len = vend - pg;
                memset((void *)(pa + skip), 0, len);
            }
        }
    }

    /* 解释器入口 = 加载基址 + ELF e_entry */
    *entry_out = base + lelf.entry;
    return 0;
}
```

这里没有用 `uvmalloc()`，因为 `uvmalloc()` 会推进 `p->sz`，
把解释器地址算进主程序堆区，影响 `sbrk` 的起点。
所以解释器采用手工 `kalloc + mappages`。

---

### 4.4 `execve()`：接入 `PT_INTERP`

文件：`kernel/proc/exec.c`

新增变量：

```c
char interp_buf[MAXPATH] = {0};
uint64 interp_off = 0, interp_len = 0;
uint64 interp_base = 0, interp_entry = 0;
int has_interp = 0;
```

扫描主程序程序头时记录 `PT_INTERP`：

```c
for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
    if (ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
        goto bad;

    if (ph.type == ELF_PROG_INTERP) {
        /* 先记住路径在文件中的位置，等主程序加载完再读 */
        if (ph.filesz < MAXPATH) {
            interp_off = ph.off;
            interp_len = ph.filesz;
        }
        continue;
    }

    if (ph.type != ELF_PROG_LOAD)
        continue;

    /* 主程序 PT_LOAD 加载逻辑保持不变 */
    ...
}
```

读取解释器路径：

```c
if (interp_len > 0) {
    if (ip->i_op->read(ip, 0, (uint64)interp_buf, interp_off, interp_len) !=
        (int)interp_len)
        goto bad;
    interp_buf[interp_len] = '\0';
}
```

加载解释器：

```c
if (interp_len > 0) {
    struct inode *lip = exec_open_interp(interp_buf);
    if (lip == 0) {
        printf("execve: interpreter not found: %s\n", interp_buf);
        goto bad;
    }

    lip->i_op->lock(lip);
    interp_base = 0x40000000;
    if (exec_load_interp(pagetable, lip, interp_base, &interp_entry) < 0) {
        lip->i_op->unlock(lip);
        goto bad;
    }
    lip->i_op->unlock(lip);
    has_interp = 1;
}
```

设置 auxv：

```c
AUXV(AT_PHDR, first_load_vaddr + elf.phoff);  // 主程序 phdr
AUXV(AT_PHENT, elf.phentsize);
AUXV(AT_PHNUM, elf.phnum);
AUXV(AT_BASE, has_interp ? interp_base : 0); // 解释器基址
AUXV(AT_ENTRY, elf.entry);                   // 主程序入口
```

设置入口寄存器：

```c
p->trapframe->epc = has_interp ? interp_entry : elf.entry;
```

关键点：

- 有解释器时，CPU 先执行 `interp_entry`；
- 解释器通过 `AT_PHDR`、`AT_ENTRY`、`AT_BASE` 完成主程序重定位；
- 最后解释器跳到 `AT_ENTRY`，也就是主程序的 `elf.entry`。

---

### 4.5 `uvmfirst()`：无条件映射 init 栈第 3 页

文件：`kernel/mm/vm.c`

```c
/*
 * The initial user stack top is 4 * PGSIZE, so page 3 is always used as
 * stack even when initcode itself is smaller than 8 KiB.  Map it
 * unconditionally; otherwise the first push at sp-8 faults.
 */
mem = kalloc();
memset(mem, 0, PGSIZE);
mappages(pagetable, 3 * PGSIZE, PGSIZE, (uint64)mem,
         PTE_W | PTE_R | PTE_X | PTE_U);

if (sz > 3 * PGSIZE) {
    memmove(mem, (uchar *)((uint64)src + 3 * PGSIZE),
            MIN(sz - 3 * PGSIZE, PGSIZE));
}
```

原来第 3 页映射被包在 `if (sz > 2 * PGSIZE)` 里；
当前 initcode 只有 7335 字节（< 8 KiB），第 3 页不会映射，
但 `sp = 4 * PGSIZE` 第一次压栈就访问 `0x3ff8`，直接 page fault。

---

## 5. 修复后的执行流程

以 musl 动态程序为例：

```text
execve("/musl/entry-dynamic.exe")
  ├─ 读取 ELF 头
  ├─ 扫描程序头
  │    ├─ PT_LOAD   : 加载主程序到 0x10000 等地址
  │    └─ PT_INTERP : 记录 /lib/ld-musl-riscv64-sf.so.1
  ├─ 读取 PT_INTERP 字符串
  ├─ exec_open_interp()
  │    └─ /lib/... 找不到
  │       → /musl/lib/... 找不到
  │       → /glibc/lib/... 找不到
  │       → 命中 /musl/lib/libc.so
  ├─ exec_load_interp(base = 0x40000000)
  │    ├─ 映射 libc.so 的 PT_LOAD
  │    ├─ 拷贝文件内容
  │    └─ 清零 BSS
  ├─ 建立用户栈 / argv / envp
  ├─ auxv:
  │    AT_PHDR  = 主程序 phdr
  │    AT_ENTRY = 0x2e2e8
  │    AT_BASE  = 0x40000000
  └─ trapframe->epc = 0x40000000 + libc.e_entry
       → 先执行 musl _dlstart
       → 完成主程序 GOT/PLT 重定位
       → 跳转 AT_ENTRY 执行 entry-dynamic 的 _start
```

---

## 6. 验证结果

使用 oscomp 的 RISC-V 镜像运行 `run-dynamic.sh`：

```text
========== START entry-dynamic.exe argv ==========
Pass!
========== START entry-dynamic.exe basename ==========
Pass!
========== START entry-dynamic.exe clocale_mbfuncs ==========
Pass!
========== START entry-dynamic.exe clock_gettime ==========
Pass!
========== START entry-dynamic.exe dirname ==========
Pass!
========== START entry-dynamic.exe env ==========
Pass!
...
```

修复前所有动态测试都是：

```text
scause=0xc sepc=0xfffffffffffffffe
FAIL xxx [status 255]
```

修复后不再出现该错误，动态链接器能够正常执行，普通动态测试可以 PASS。

剩余失败是独立功能问题：

- `pthread_*`：内核 pthread/clone 线程语义不完整
- `dlopen`：动态库加载/符号解析尚未完整实现
- `tls_init` / `tls_local_exec`：TLS/线程支持不完整
- 其他 libc-test 自身功能失败

这些不是 `PT_INTERP` 缺失导致的。

---

## 7. 已知限制

当前实现把解释器映射在 `0x40000000`，但没有把这段范围计入 `p->sz`。
因此：

- `execve` 本身正常；
- 进程退出时 `freewalk()` 会释放残留的解释器页；
- 但 `fork()` 的 `uvmcopy()` 只复制 `0..p->sz`，不会复制解释器页。

如果后续动态测试需要在动态进程里 `fork()`，需要额外记录解释器范围
（例如在 `struct proc` 中保存 `interp_base/interp_end`），并在 `uvmcopy()` 中复制。
当前动态测试的启动、普通函数测试和执行链路已经可以工作。

---

## 8. 一句话总结

动态链接失败不是 `PT_LOAD` 的问题，而是 RISC-V `execve()` 完全没有处理
`PT_INTERP`：没有加载解释器、没有设置 `AT_BASE`、入口仍然指向主程序，
所以 PLT 未重定位，跳到 `-2`。修复后先加载 `/musl/lib/libc.so`（或 glibc 解释器），
把入口切到解释器，再由解释器完成重定位并跳回主程序。
