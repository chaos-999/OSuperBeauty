# LA 内核栈从 1 页扩到 4 页

> 记录 LoongArch `kernelvec` 入口压栈触发 PIS、`panic: kerneltrap` 的根因，
> 以及把 LA 内核栈统一成和 RV 一致的 4 页布局的修改。

---

## 1. 问题现象

LA `basic-musl` 跑到 `dup` / `dup2` 附近时，串口日志突然中断：

```text
========== END test_dup ==========
========== START test_dup2 ==========
estat 20000
era=9000000080032014 eentry=9000000080032000
panic: kerneltrap
```

- `estat = 0x20000`：Ecode = `0x2`，即 **PIS（Page Invalid Store）**
- `eentry = 0x9000000080032000`：内核 trap 入口 `kernelvec`
- `era = 0x9000000080032014`：`kernelvec + 0x14`

反汇编 `kernelvec` 入口：

```asm
9000000080032000 <kernelvec>:
9000000080032000: addi.d  $sp, $sp, -256
9000000080032004: st.d    $ra, $sp, 0
9000000080032008: st.d    $tp, $sp, 8
900000008003200c: st.d    $sp, $sp, 16
9000000080032010: st.d    $a0, $sp, 24
9000000080032014: st.d    $a1, $sp, 32   ← PIS
```

`era` 正好是 `st.d $a1, $sp, 32`，说明异常发生在 **内核 trap 入口保存寄存器时**，而不是后面的 C 代码里。

---

## 2. 根因：内核栈只有 4 KiB，入口 256 字节压栈越过栈底

修改前 LA 的配置是：

```c
// include/memlayout.h
#define KSTACK(p) (TRAPFRAME - ((p) + 1) * 2 * PGSIZE)

// kernel/proc/proc.c
char *pa = kalloc();                 // 只分配 1 页
mappages(kpgtbl, va, PGSIZE, ...);   // 只映射 1 页

// 栈顶
p->context.sp           = p->kstack + PGSIZE;
p->trapframe->kernel_sp = p->kstack + PGSIZE;
```

看起来 `KSTACK` 间隔 2 页，但实际可用只有 **1 页 = 4 KiB**，下一块是未映射的页。

LA 的内核调用链较深：

```text
kernelvec
  → kerneltrap
  → usertrap
  → syscall
  → sys_execve
  → execve
  → namei / VFS
  → ext4
```

`execve()` 里还有 `ustack[]`、`estack[]`、`aux[]` 等栈上数组。栈接近底部时，timer 中断进入 `kernelvec`，第一条：

```asm
addi.d $sp, $sp, -256
```

把 `sp` 推过栈底，随后的 `st.d` 落入未映射页，触发 PIS，`kernelvec` 自己取异常，最终 `panic: kerneltrap`。

这也解释了为什么日志总停在进程切换 / `fork` / `exec` 附近，而不是固定某条业务指令：**触发点是中断时机，不是业务代码本身**。

---

## 3. 修复：LA 统一为 4 页内核栈

以 RV 为基准。RV 一直是 4 页，LA 也改成同样的布局。

### 3.1 `include/memlayout.h`

两个 LA 分支都改为 4 页间隔：

```c
#define KSTACK(p) (TRAPFRAME - ((p) + 1) * 4 * PGSIZE)
```

涉及：

- `LA2K1000` 分支
- 默认 QEMU / LS7A 分支

RV 保持：

```c
#define KSTACK(p) (TRAMPOLINE - ((p) + 1) * 4 * PGSIZE)
```

### 3.2 `kernel/proc/proc.c`

`proc_mapstacks()` 分配并映射 4 页：

```c
char *pa = kmalloc(4 * PGSIZE);
if (pa == 0) panic("kalloc");

uint64 va = KSTACK((int)(p - proc));

#ifdef LA2K1000
if (mappages(kpgtbl, va, 4 * PGSIZE, (uint64)pa,
             PTE_P | PTE_W | PTE_MAT | PTE_D) != 0)
#else
if (mappages(kpgtbl, va, 4 * PGSIZE, (uint64)pa,
             PTE_NX | PTE_P | PTE_W | PTE_MAT | PTE_D) != 0)
#endif
    panic("kvmmap");
```

`allocproc()` 中栈顶同步改成 4 页：

```c
p->context.sp = p->kstack + 4 * PGSIZE;
```

### 3.3 `kernel/trap/la/trap.c`

`usertrapret()` 写入的 `kernel_sp` 同步为 4 页栈顶：

```c
p->trapframe->kernel_sp = p->kstack + 4 * PGSIZE;
```

---

## 4. 修改前后对照

| 项目 | 修改前 | 修改后 |
|------|--------|--------|
| `KSTACK` 间隔 | 2 页 | 4 页 |
| 实际映射 | 1 页 | 4 页 |
| 可用内核栈 | 4 KiB | 16 KiB |
| `context.sp` | `kstack + 1P` | `kstack + 4P` |
| `trapframe->kernel_sp` | `kstack + 1P` | `kstack + 4P` |
| 与 RV | 不一致 | 一致 |

修改后四者必须同时成立：

```text
KSTACK(p) 起始地址
  → 映射 4 页
  → 栈顶 = kstack + 4 页
  → context.sp / kernel_sp 都指向栈顶
  → 下一个进程的 KSTACK 间隔也是 4 页
```

只改其中一部分会制造更隐蔽的问题，见第 6 节。

---

## 5. 验证

编译：

```bash
PATH=/opt/cross-tools/bin:$PATH make kernel-la -j$(nproc)
```

结果成功生成 `kernel-la`。

之后 LA 串口日志中：

```text
#### OS COMP TEST GROUP START basic-musl ####
...
========== START test_dup2 ==========
========== END test_dup2 ==========
...
#### OS COMP TEST GROUP END basic-musl ####
```

不再出现：

```text
estat 20000
panic: kerneltrap
```

原来固定停在 `dup2` 附近的现象消失，`basic-musl` 可以完整跑完。

---

## 6. 注意：半改状态会导致更严重的问题

修复过程中出现过一次“只改了一半”的状态，值得记录：

```c
char *pa = kalloc(4 * PGSIZE);   // 编译错误：kalloc 没有参数
```

以及：

```text
KSTACK 间隔仍是 2 页
mappages 仍只映射 1 页
但 context.sp / kernel_sp 已经写成 kstack + 4P
```

后者的后果是栈指针直接落在未映射区域：

```text
可用栈：[kstack, kstack + 1P)
实际 sp：kstack + 4P
```

任何一次 push 都会立刻 PIS。

所以这类修改必须四处一起检查：

```text
include/memlayout.h   KSTACK 间隔
kernel/proc/proc.c    分配大小 + mappages 大小
kernel/proc/proc.c    context.sp
kernel/trap/la/trap.c trapframe->kernel_sp
```

---

## 7. 遗留问题：4 页栈之间没有 guard page

当前：

```text
KSTACK 间隔 = 4 页
映射大小   = 4 页
```

相邻进程的内核栈是紧挨着的：

```text
[kstack p=0][kstack p=1][kstack p=2] ...
```

如果某个进程把 16 KiB 全部用完，继续向下写不会立刻 page fault，而是覆盖下一个进程的栈。RV 目前也是这个布局，所以先保持与 RV 一致。

若要更安全，可以改成：

```c
#define KSTACK(p) (TRAPFRAME - ((p) + 1) * 5 * PGSIZE)
```

映射仍为 4 页，多出 1 页作为真正的 guard page：

```text
[4 页栈][1 页 guard][4 页栈][1 页 guard] ...
```

代价是内核虚拟地址空间多占 `NPROC * PGSIZE`，对当前布局影响很小。

---

## 8. 小结

| 问题 | 结论 |
|------|------|
| `panic: kerneltrap` 直接原因 | `kernelvec` 入口 `addi.d $sp, -256` 越过栈底，`st.d` 触发 PIS |
| 根本原因 | LA 默认分支只映射 1 页内核栈，调用链深 + 中断压栈导致溢出 |
| 修复 | LA 内核栈统一为 4 页，与 RV 一致 |
| 验证 | `kernel-la` 编译通过，`basic-musl` 完整跑完，不再出现 `kernelvec+0x14` |
| 后续可选项 | 4 页映射配 5 页间隔，增加 guard page |
