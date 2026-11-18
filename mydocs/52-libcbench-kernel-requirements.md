# libcbench 内核需求对照：测试需要什么、原来怎样、本次改了什么

> 本文按「功能域」逐项对照 libcbench 对内核的要求：
> **① 测试需要的功能 → ② 原本的实现 → ③ 本次的修改 → ④ 对应代码**。
>
> 结论：27 项所需功能已全部具备并验证通过；另有 1 项非 libcbench 必需的功能
> （`/dev/zero` read）来自 PR #3，已按本仓库情况有区分地合入。

---

## 0. 测试全景

`libc-bench` 的 27 项分为 6 组：

| 组 | 数量 | 内核相关度 |
|---|---|---|
| `b_malloc_*` | 8（含 2 个线程版） | 高：brk/mmap/munmap/线程/同步 |
| `b_string_*` | 8 | 低：纯用户态 |
| `b_pthread_*` | 4 | 最高：clone/页表/退出/同步 |
| `b_utf8_*` | 2 | 低：纯用户态 |
| `b_stdio_*` | 2 | 中：tmpfile、读写、缓冲 |
| `b_regex_*` | 3 | 低：纯用户态 + 大用户栈 |

框架层（`main.c`）：每个测例 `fork()` 一个子进程 → 子进程跑 benchmark →
`print_stats()` → `exit(0)` → 父进程 `wait()`。计时用 `CLOCK_REALTIME`。

---

## 1. `clone`：进程/线程统一创建入口

### ① 测试需要的功能

- RV 版 `createjoin_serial1/2`、`create_serial1` 各创建 **2500 个线程**；
- `malloc_thread_*`、`uselesslock` 也依赖线程创建；
- 需要支持：
  `CLONE_VM | CLONE_THREAD | CLONE_SIGHAND | CLONE_SETTLS |
   CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_FS | CLONE_FILES`
- **ABI 架构相关**：
  - RISC-V：`clone(flags, stack, ptid, tls, ctid)`
  - LoongArch：`clone(flags, stack, ptid, ctid, tls)`

### ② 原本的实现

`sys_clone()` 只按一种顺序取参（LA 顺序），RV 下 `tls/ctid` 互换。

### ③ 本次修改

按 `RISCV` / `LOONGARCH` 分别 `argaddr`。

### ④ 代码

`kernel/syscall/sysproc.c`：

```c
int sys_clone(void) {
    int flags;
    uint64 stack, tls, ctid, ptid;

    argint(0, &flags);
    argaddr(1, &stack);
    argaddr(2, &ptid);
#ifdef RISCV
    /* RISC-V musl/glibc: clone(flags, stack, ptid, tls, ctid) */
    argaddr(3, &tls);
    argaddr(4, &ctid);
#elif defined(LOONGARCH)
    /* LoongArch musl: clone(flags, stack, ptid, ctid, tls) */
    argaddr(3, &ctid);
    argaddr(4, &tls);
#endif
    return clone(flags, (void *)stack, ptid, tls, ctid);
}
```

---

## 2. 共享页表下的 `TRAPFRAME`（本轮最核心）

### ① 测试需要的功能

线程第一次从内核返回用户态时，必须拿到：

- `a0 = 0`（clone 子分支）
- 自己的 `sp`（线程栈顶）
- 自己的 `epc`（ecall 之后）

### ② 原本的实现

`CLONE_VM` 时 `np->pagetable = p->pagetable`，但用户页表里的
`TRAPFRAME`（`MAXVA - 3*PGSIZE`）仍映射**父进程**的 trapframe。
子线程 `userret` 恢复出父线程的 `a0/sp/epc`，于是“以为自己是父线程”，
继续执行 `pthread_create` 父分支（`sched_setscheduler` → `unknown sys call 119`），
最终父子状态互相踩踏，出现 `panic: release`。

### ③ 本次修改

- `struct proc` 新增 `int tg_shared`；创建线程时 **leader 和线程都置 1**；
- `usertrapret()` 中若 `tg_shared`，把本进程页表的 `TRAPFRAME` PTE
  重指向 `p->trapframe`。

两个易错点：

1. **不能只判断 `pt_shared`**：leader 自己 `pt_shared=0`，但它和线程共用页表，
   也必须重映射（否则 `b_malloc_thread_stress` 卡死）。
2. **LA 的 `PTE_V`（bit0）与 `PTE_P`（bit7）是两个独立位**，重写时漏掉
   `PTE_V` 会让 LA 的 `pthread_create` 无声卡死。

### ④ 代码

`kernel/proc/proc.c`（clone 线程分支）：

```c
if (flags & CLONE_THREAD) {
    np->tgid = p->tgid;
    np->tid = allocpid();
    np->leader = p->leader ? p->leader : p;
    /* 整个线程组共用一张页表，所有成员返回用户态时都要重映射 TRAPFRAME */
    np->tg_shared = 1;
    p->tg_shared = 1;
}
```

`kernel/trap/rv/trap.c`（`usertrapret`）：

```c
if (p->tg_shared) {
    pte_t *tfpte = walk(p->pagetable, TRAPFRAME, 0);
    if (tfpte && PTE2PA(*tfpte) != (uint64)p->trapframe) {
        *tfpte = PA2PTE((uint64)p->trapframe) | PTE_R | PTE_W | PTE_V;
        sfence_vma();
    }
}
```

`kernel/trap/la/trap.c`（`usertrapret`）：

```c
if (p->tg_shared) {
    pte_t *tfpte = walk(p->pagetable, TRAPFRAME, 0);
    if (tfpte && PTE2PA(*tfpte) != (uint64)p->trapframe) {
#ifdef LA2K1000
        *tfpte = PA2PTE((uint64)p->trapframe)
               | PTE_P | PTE_W | PTE_MAT | PTE_D | PTE_U | PTE_V;
#else
        *tfpte = PA2PTE((uint64)p->trapframe)
               | PTE_NX | PTE_P | PTE_W | PTE_MAT | PTE_D | PTE_V;
#endif
    }
}
```

> LA 不需要额外 `invtlb`：`uservec.S` 的 `userret` 在切换 `PGDL` 后已做全量刷新。

---

## 3. `mprotect`：允许尚未缺页的 VMA 页

### ① 测试需要的功能

musl `pthread_create`：

```c
map = mmap(NULL, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
mprotect(map + guard, size - guard, PROT_READ|PROT_WRITE);
```

mmap 只登记 VMA，页是惰性分配的，`mprotect` 时页**还不存在**。

### ② 原本的实现

`sys_mprotect` 对区间内每个页要求 `PTE_V`，否则返回 `-ENOMEM`。

### ③ 本次修改

- 未缺页页直接跳过（不报错）；
- 已存在页更新权限；
- 遍历 VMA，更新覆盖区间的 `v->prot`，让后续缺页按新权限映射。

### ④ 代码

`kernel/syscall/sysproc.c`：

```c
for (uint64 va = addr; va < addr + len; va += PGSIZE) {
    pte_t *pte = walk(p->pagetable, va, 0);
    if (pte == 0 || (*pte & PTE_V) == 0) continue;   /* 惰性页：先跳过 */

    *pte &= ~(PTE_R | PTE_W | PTE_X);
    if (prot & PROT_READ)  *pte |= PTE_R;
    if (prot & PROT_WRITE) *pte |= PTE_W;
    if (prot & PROT_EXEC)  *pte |= PTE_X;
    *pte |= PTE_U;
}

for (int i = 0; i < NVMA; i++) {
    struct vma *v = &p->vmas[i];
    if (!v->valid) continue;
    if (addr >= v->addr && addr + len <= v->addr + v->length) {
        v->prot = prot;
        break;
    }
}
```

---

## 4. `set_tid_address`：只记录地址，不写 TID

### ① 测试需要的功能

Linux 语义：`set_tid_address(addr)` **只记录 addr**；线程退出时内核把 `*addr`
清 0 并 `futex_wake`。

musl 启动时调用 `set_tid_address(&__thread_list_lock)`，并把该地址当线程链表锁。

### ② 原本的实现

内核把 `p->pid` `copyout` 到 `addr`。于是锁值变成主线程 tid，
`__tl_lock()` 以为锁被主线程永久持有，`pthread_join` / `__pthread_exit`
互相等待 → 永久睡眠。

### ③ 本次修改

只保存 `p->clear_child_tid`，返回 `p->tid`。

### ④ 代码

`kernel/syscall/sysproc.c`：

```c
uint64 sys_set_tid_address(void) {
    struct proc *p = myproc();
    uint64 tidptr;
    argaddr(0, &tidptr);

    /*
     * 只记录“退出时要清零并 futex 唤醒”的地址；
     * 不能写 TID：musl 传的是 &__thread_list_lock。
     */
    p->clear_child_tid = tidptr;
    return p->tid;
}
```

`kernel/proc/proc.c`（exit 中清地址 + 唤醒）：

```c
if (p->clear_child_tid) {
    int clear = 0;
    copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
    futex_wake(p->clear_child_tid, 1);
    p->clear_child_tid = 0;
}
```

---

## 5. `brk`：线程组共享堆顶

### ① 测试需要的功能

`malloc` 主 arena 走 `brk`；`malloc_thread_*` 两个线程同时 malloc。
多线程共享地址空间，必须共享同一个堆顶。

### ② 原本的实现

每个任务用自己的 `p->sz` 扩堆。两个线程会映射重叠虚拟地址 →
`panic("mappages: remap")`。

### ③ 本次修改

- `struct proc` 新增 `struct proc *leader`；
- 线程继承 leader；
- `growproc()` 与 `sys_brk()` 统一使用 `leader->sz`。

### ④ 代码

`kernel/proc/proc.c`：

```c
int growproc(int n) {
    struct proc *p = myproc();
    struct proc *owner = (p->leader && p->leader->state != UNUSED) ? p->leader : p;

    uint64 sz = owner->sz;
    ...
    owner->sz = sz;
    p->sz = sz;
    return 0;
}
```

`kernel/syscall/sysproc.c`：

```c
uint64 sys_brk(void) {
    uint64 addr, n;
    argaddr(0, &n);
    struct proc *mp = myproc();
    struct proc *owner = (mp->leader && mp->leader->state != UNUSED) ? mp->leader : mp;
    addr = owner->sz;
    if (n == 0) return addr;
    if (growproc(n - addr) < 0) return -ENOMEM;
    return n;
}
```

---

## 6. `exit` / `exit_group`：等兄弟线程退出再释放共享 mm

### ① 测试需要的功能

- 线程 `exit` 只结束自己；
- 进程主线程 `exit_group` 结束整个线程组；
- 最后一个线程消失前不能释放共享地址空间。

`create_serial1` 的 leader 结束时仍有兄弟线程在跑。

### ② 原本的实现

leader 立即 `vmaunmap`（释放线程栈）+ `proc_freepagetable`（释放共享页表），
兄弟线程在已释放内存上运行 → 系统永久卡住（`wait4` 已回收子进程也不返回）。

### ③ 本次修改

leader 退出时：

1. 给同组线程 `killed=1` 并 `wakeup(chan)`；
2. `yield()` 等待它们变成 `ZOMBIE/UNUSED`；
3. 再做文件/VMA/页表清理。

`sys_exit_group` 复用同一路径。

### ④ 代码

`kernel/proc/proc.c`：

```c
int is_thread = (p->tid != p->tgid);

if (!is_thread) {
    for (;;) {
        struct proc *q;
        int alive = 0;
        for (q = proc; q < &proc[NPROC]; q++) {
            if (q == p || q->state == UNUSED) continue;
            if (q->tgid != p->tgid || q->tid == q->tgid) continue;
            acquire(&q->lock);
            if (q->state != UNUSED && q->state != ZOMBIE) {
                alive = 1;
                q->killed = 1;
                void *chan = q->chan;
                release(&q->lock);
                if (chan) wakeup(chan);
            } else {
                release(&q->lock);
            }
        }
        if (!alive) break;
        yield();
    }
}
```

---

## 7. 线程 ZOMBIE 回收 + `wait4`/`reparent` 跳过线程

### ① 测试需要的功能

`pthread_join()` 不走 `wait4()`；线程退出后必须有人回收，否则 `NPROC=128`
被 2500 个线程耗尽。

### ② 原本的实现

线程退出后永久 `ZOMBIE`；且 leader 退出时 `reparent()` 可能把线程 zombie
交给 init，`init` 的 `wait()` 会提前返回（我们实测会导致测试套件提前关机）。

### ③ 本次修改

- 调度器回收 `ZOMBIE && tid != tgid` 的线程；
- `wait4()` 与 `reparent()` 跳过线程。

### ④ 代码

`kernel/proc/proc.c`（调度器）：

```c
if (p->state == ZOMBIE && p->tid != p->tgid) {
    freeproc(p);
    release(&p->lock);
    continue;
}
```

`kernel/proc/proc.c`（wait4 / reparent）：

```c
/* wait4 */
if (pp->tid != pp->tgid) continue;   /* 线程由调度器回收 */

/* reparent */
if (pp->tid != pp->tgid) continue;   /* 线程没有 wait4 父进程 */
```

---

## 8. 用户栈：容量与布局

### ① 测试需要的功能

- `b_regex_search`：`char buf[260000]`；
- 2500 个线程栈从高地址向下增长约 60MB。

### ② 原本的实现

`USTACK_PAGE=32`（128KB），且用户栈距离 `MMAPEND` 只有约 20MB。

### ③ 本次修改

- `USTACK_PAGE = 256`（1MB）；
- 用户栈整体下移 512MB，给 mmap 留空间。

### ④ 代码

`include/memlayout.h`：

```c
#define USTACK_PAGE 256
#define USTACK (MAXVA - 512 * 1024 * 1024 - USTACK_PAGE * PGSIZE)
#define USTACK_TOP (USTACK + USTACK_PAGE * PGSIZE)
```

---

## 9. VMA 容量

### ① 测试需要的功能

不 join 的线程栈不会被 `munmap`，每个线程占一个 VMA；`serial2` 同时 50 个。

### ② 原本的实现

`NVMA=16` → 第 17 个线程 `panic("mmap: no free vma")`。

### ③ 本次修改

`NVMA = 3072`（2500 + 余量）。

### ④ 代码

`include/param.h`：

```c
#define NVMA 3072   // maximum VMA regions per process
```

---

## 10. `fstat`：基于打开文件 inode

### ① 测试需要的功能

对已 `unlink` 但仍打开的文件执行 `fstat` 应成功（`tmpfile()` 就是这种文件）。

### ② 原本的实现

`filestat/filestatx` 用 `f->f_path` 做路径 `stat`；`tmpfile()` 已 unlink，
路径不存在 → 失败。

### ③ 本次修改

改用 `vfs_ext_fstat(f, &st)`（按打开文件 inode）。

### ④ 代码

`kernel/fs/vfs/file.c`：

```c
/* filestat() */
if (vfs_ext_fstat(f, &st) < 0) return -1;

/* filestatx() */
if (vfs_ext_fstat(f, &ks) < 0) return -1;
```

> 说明：这是一项**正确性修复**，但它**不是** stdio 200 秒的原因——
> musl 的 `__fdopen()` 固定使用 `BUFSIZ`，并不依赖 `st_blksize`。
> stdio 慢的根因是 `BSIZE=512` 的块设备层 + lwext4 日志写路径（见
> `51-libcbench-all-pass.md` §7）。

---

## 11. 无需修改但必须存在的功能

| 功能 | 用途 | 现状 |
|---|---|---|
| `mmap/munmap/madvise` | 线程栈、malloc 大块、trim | ✅ 支持（madvise 为空实现） |
| `futex` WAIT/WAKE | mutex、join、线程链表锁 | ✅ 支持（private 位忽略） |
| `set_robust_list` | musl 线程初始化 | ✅ 记录 |
| `wait4` | 测例隔离、回收子进程 | ✅ 支持 |
| `read/write/readv/writev` | stdio | ✅ 支持 |
| `openat/unlinkat/lseek/close` | tmpfile | ✅ 支持 |
| `ioctl(TIOCGWINSZ)/fcntl` | `fdopen` 探测 | ✅ 支持 |
| `rt_sigprocmask` | 线程启动屏蔽信号 | ✅ 支持 |
| `clock_gettime(CLOCK_REALTIME)` | 计时 | ✅ 支持 |
| `gettid` | 线程自识别 | ✅ 支持 |

---

## 12. 已知部分支持 / 未实现（不影响 27 项）

| 项 | 现状 | 影响 |
|---|---|---|
| `/proc/self/smaps` | 未实现 | `virt/res/dirty=0`；judge 不解析 |
| `clock_gettime(CLOCK_MONOTONIC)` | 只处理 REALTIME | libcbench 不用；通用缺口 |
| `mremap` | 仅支持缩小，且残留 `printf("here\n")` | 本测例不依赖；建议清理 |
| `madvise` | 空实现返回 0 | 只影响统计 |
| `sched_setscheduler` | 未实现 | 默认 attr 不触发 |
| `riscv_flush_icache` | 未注册 | 静态程序不触发 |

---

## 13. 非 libcbench 需求但本次合入：`/dev/zero`（PR #3）

### 需求来源

libc-test 的 `syscall_sign_extend`：`read(/dev/zero, buf, 1)` 必须返回 1 且
`buf[0]==0`。原实现 `f_type==8` 直接 `return 0`，被当成 EOF。

### 本次处理（有区分地接受 PR）

| PR 内容 | 处理 |
|---|---|
| `kernel/fs/vfs/file.c` 源码修复 | ✅ 采纳，并改为 `static char zeros[512]`（避免每次读都初始化 512B 栈） |
| `mydocs/50-dev-zero-...md` | ✅ 采纳，文首加「合并说明」 |
| `mydocs/README.md` 索引 | ✅ 采纳，并补充 51/52 两行 |
| `kernel-rv` 二进制 | ❌ **不采纳**（基于旧 main，缺少本仓库线程/VM 修复），本地重新编译 |
| 编号 50 | 保留给该 PR；本文档为 52，libcbench 总文档为 51 |

### 代码

`kernel/fs/vfs/file.c`：

```c
} else if (f->f_type == 8) {
    /* /dev/zero：写入 n 个零字节并返回 n，永不为 EOF */
    if (n <= 0) return 0;
    static char zeros[512];
    int remain = n;
    while (remain > 0) {
        int chunk = remain < (int)sizeof(zeros) ? remain : (int)sizeof(zeros);
        if (copyout(myproc()->pagetable, addr, zeros, chunk) < 0) return -1;
        addr += chunk;
        remain -= chunk;
    }
    return n;
}
```

### 独立验证（非 libcbench）

32MB 最小镜像 + 静态测试程序：

```text
zero r=3 bytes=000000
zero big r=1000 nonzero=0
null r=0
done
```

---

## 14. 验证矩阵

| 验证项 | 环境 | 结果 |
|---|---|---|
| libcbench 27 项 | RV 真实内核 + 真实 sdcard | 27/27 `time:`，`GROUP END`，正常关机 |
| `/dev/zero` read | RV 最小镜像 | 3/1000 字节全零，返回请求长度；`/dev/null` 返回 0 |
| LA 启动 | LA 内核 + `sdcard-la.img` | basic-glibc / busybox-musl 正常 |
| LA 线程 | LA 最小镜像 | create → worker → join → malloc 全通过 |
| LA libcbench | LA | 未跑完（LA malloc/缺页性能极低，见 51 文档 §8） |

---

## 15. 一句话总结

libcbench 考验的不是“系统调用数量”，而是 **clone/线程语义、共享页表的用户态返回、
退出与回收、futex+cleartid 同步、brk/VMA/用户栈容量、tmpfile 文件路径** 这些
**语义正确性**。本文列出的 10 项修改正是把这些语义补齐；`/dev/zero` 是附带合入的
libc-test 修复，与 libcbench 判分无关。
