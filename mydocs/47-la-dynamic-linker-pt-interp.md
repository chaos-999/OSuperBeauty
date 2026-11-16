# LA 动态链接支持：移植 RV 的 PT_INTERP 逻辑

> 记录 LA 上 `entry-dynamic.exe` 全部 `[status 255]` 的根因，以及把
> RISC-V 分支的 `PT_INTERP`/动态链接器加载逻辑移植到 LA 的过程。
>
> 结论：**不是超时，也不是动态测试本身失败；LA `execve()` 之前只处理
> `ELF_PROG_LOAD`，完全忽略 `PT_INTERP`，动态程序直接跳到自己的 entry
> 后在用户态崩溃。** 移植 RV 逻辑并补齐 LA auxv 后，musl 动态程序可以
> 正常启动并运行。

---

## 1. 问题现象

静态测试正常：

```text
========== START entry-static.exe argv ==========
Pass!
========== END entry-static.exe argv ==========
```

动态测试全部立即失败：

```text
========== START entry-dynamic.exe argv ==========
FAIL argv [status 255]
========== END entry-dynamic.exe argv ==========
========== START entry-dynamic.exe basename ==========
FAIL basename [status 255]
========== END entry-dynamic.exe basename ==========
```

特征：

- 每个 dynamic 测试都是**立刻 255**；
- 没有等 5 秒超时；
- 没有任何 loader/库文件错误输出。

所以不是 `runtest.exe` 超时，而是动态程序根本没被正确启动。

---

## 2. 证据：entry-dynamic.exe 需要解释器

```text
$ readelf -lW entry-dynamic.exe
INTERP 0x000238
    [Requesting program interpreter: /lib64/ld-musl-loongarch-lp64d.so.1]

LOAD ...
DYNAMIC ...
TLS ...
```

动态 ELF 的入口不是给应用程序直接执行的，内核必须先加载
`PT_INTERP` 指定的动态链接器，再把入口切到解释器。

修复前 LA `execve()` 只处理：

```c
if (ph.type == ELF_PROG_LOAD) {
    ...
}
```

没有 `ELF_PROG_INTERP`，也没有加载解释器、设置 `AT_BASE`，直接：

```c
p->trapframe->era = elf.entry;
```

于是动态程序在自己的 entry 上立刻崩溃，`runtest.exe` 看到 `status 255`。

---

## 3. 移植内容

### 3.1 动态链接 helper 改为 RV/LA 共用

`exec_open_interp()`、`exec_load_interp()` 原来只放在 `#ifdef RISCV` 里，
现在改成：

```c
#if defined(RISCV) || defined(LOONGARCH)
static struct inode *exec_open_interp(const char *path) { ... }
static int exec_load_interp(...) { ... }
#endif
```

### 3.2 解释器路径查找

保留原路径优先，再尝试镜像里的实际位置：

```c
const char *dirs[] = {"/musl/lib/", "/glibc/lib/"};
```

musl 的解释器名在镜像里不存在，统一回落到 libc.so：

```c
/* ld-musl-riscv64*.so.1 / ld-musl-loongarch-lp64d.so.1 都不在镜像中 */
if (strncmp(base, "ld-musl-", 8) == 0)
    return namei("/musl/lib/libc.so");
```

LA 实际路径：

```text
PT_INTERP: /lib64/ld-musl-loongarch-lp64d.so.1
实际加载: /musl/lib/libc.so
```

glibc 的 `/glibc/lib/ld-linux-loongarch-lp64d.so.1` 可以通过
`/glibc/lib/` fallback 找到。

### 3.3 按架构设置解释器页权限

`exec_load_interp()` 原来使用 RV 的 PTE 位：

```c
PTE_W | PTE_X | PTE_R | PTE_U
```

LA 改为：

```c
#ifdef RISCV
int perm = PTE_W | PTE_X | PTE_R | PTE_U;
#elif defined(LOONGARCH)
int perm = PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D;
#endif
```

### 3.4 LA execve 解析并加载 PT_INTERP

LA 分支新增：

```c
char interp_buf[MAXPATH] = {0};
uint64 interp_off = 0, interp_len = 0;
uint64 interp_base = 0, interp_entry = 0;
int has_interp = 0;
```

程序头循环：

```c
if (ph.type == ELF_PROG_INTERP) {
    if (ph.filesz < MAXPATH) {
        interp_off = ph.off;
        interp_len = ph.filesz;
    }
    continue;
}
```

加载解释器：

```c
struct inode *lip = exec_open_interp(interp_buf);
lip->i_op->lock(lip);
interp_base = 0x40000000;
exec_load_interp(pagetable, lip, interp_base, &interp_entry);
lip->i_op->unlock(lip);
has_interp = 1;
```

提交时切换入口：

```c
p->trapframe->era = has_interp ? interp_entry : elf.entry;
```

### 3.5 补齐 LA auxv

动态链接器依赖完整的 auxv。LA 分支新增：

```c
alloc_aux(aux, AT_HWCAP, 0);
alloc_aux(aux, AT_PAGESZ, PGSIZE);
alloc_aux(aux, AT_PHDR, first_load_vaddr + elf.phoff);
alloc_aux(aux, AT_PHENT, elf.phentsize);
alloc_aux(aux, AT_PHNUM, elf.phnum);
alloc_aux(aux, AT_BASE, has_interp ? interp_base : 0);
alloc_aux(aux, AT_ENTRY, elf.entry);
```

其中：

- `AT_PHDR` 必须是**虚拟地址**，即 `first_load_vaddr + e_phoff`；
- `AT_BASE` 是解释器加载基址；
- `AT_ENTRY` 是主程序入口，不是解释器入口。

### 3.6 PIE 的 first_load 判断

glibc 的 `entry-dynamic.exe` 是 `ET_DYN`（PIE），第一个 LOAD 的
`p_vaddr = 0`。原来的写法：

```c
if (first_load_vaddr == 0 || ph.vaddr < first_load_vaddr)
    first_load_vaddr = ph.vaddr;
```

会把 0 当成“未初始化”，第二个 LOAD 的 `0x53000` 反而覆盖了它，导致
`AT_PHDR` 错误。现在改成显式标志：

```c
int first_load_set = 0;
int start_set = 0;

if (!first_load_set || ph.vaddr < first_load_vaddr) {
    first_load_vaddr = ph.vaddr;
    first_load_set = 1;
}
if (!start_set) {
    start_vaddr = ph.vaddr;
    start_set = 1;
}
```

这同时修复了 PIE 第一段 `p_vaddr=0` 时 `uvmalloc` 起点被第二段覆盖的问题。

---

## 4. 修改文件

| 文件 | 修改 |
|------|------|
| `kernel/proc/exec.c` | 动态链接 helper 改为 RV/LA 共用；LA 分支解析/加载 `PT_INTERP`、补齐 auxv、修复 PIE 的 first_load/start_vaddr 判断 |
| `mydocs/47-la-dynamic-linker-pt-interp.md` | 本文 |

---

## 5. 验证

### 5.1 musl 动态程序

构造最小镜像：

```text
/musl/busybox
/musl/runtest.exe
/musl/entry-dynamic.exe
/musl/lib/libc.so
```

执行：

```text
/musl/libctest_testcode.sh
  └── ./runtest.exe -w entry-dynamic.exe argv
```

结果：

```text
========== START entry-dynamic.exe argv ==========
Pass!
========== END entry-dynamic.exe argv ==========
```

说明：

- `PT_INTERP` 已被加载；
- `/musl/lib/libc.so` 作为解释器正常工作；
- `AT_PHDR` / `AT_BASE` / `AT_ENTRY` 正确；
- musl 动态程序可以运行。

### 5.2 glibc 动态程序

glibc 的 `entry-dynamic.exe` 也是 PIE，移植后可以越过
`rtld_setup_main_map` 断言，但后续加载 `libm.so.6` 仍失败：

```text
libm.so.6: failed to map segment from shared object
```

这是**另一个独立问题**：glibc loader 需要 `MAP_FIXED` 映射共享库，而当前
`sys_mmap()` 直接拒绝 `MAP_FIXED`。musl 动态程序不使用这条路径，所以
musl 已经通过。

后续如果需要 glibc 动态测试，要继续实现：

- `sys_mmap()` 对 `MAP_FIXED` 的支持；
- glibc 共享库搜索路径（`LD_LIBRARY_PATH=/glibc/lib` 或 `/etc/ld.so.cache`）；
- 共享库的段映射/权限处理。

---

## 6. 一句话总结

LA 动态测试全 255 不是超时，而是 **LA `execve()` 没实现 `PT_INTERP`**。
把 RV 的解释器查找、加载、auxv 和入口切换逻辑移植过来，并修正 PIE 的
`first_load_vaddr=0` 判断后，musl 动态程序已经可以正常运行。
