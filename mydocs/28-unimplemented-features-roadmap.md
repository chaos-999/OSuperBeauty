# 未实现功能实现路线图：从当前 FAIL/HANG 到全量 PASS

> 基于当前 82/82 非 pthread 测试全部 PASS 的基线，列出剩余失败/卡死测试的根因分析和逐步实现方案。
>
> **本文档的目标：** 让任何接手这个项目的人——无论是否熟悉内核细节——都能理解当前卡在哪里、为什么卡、以及一步一个脚印地把剩余功能做出来。

---

## 快速导航

- [序言：我们站在哪里，要往哪去](#序言我们站在哪里要往哪去)
- [预备知识：你需要弄懂的几个概念](#预备知识你需要弄懂的几个概念)
- [全景：当前状态与剩余差距](#1-全景当前状态与剩余差距)
- [模块一：pthread 线程创建（8+ 个测试卡死/FAIL）](#2-模块一pthread-线程创建8-个测试卡死fail)
- [模块二：sigtimedwait + waitpid 链路（runtest.exe FAIL）](#3-模块二sigtimedwait--waitpid-链路runtestexe-fail)
- [模块三：VFS lseek panic（~10 个测试未跑到）](#4-模块三vfs-lseek-panic10-个测试未跑到)
- [模块四：网络栈（socket 测试 FAIL）](#5-模块四网络栈socket-测试-fail)
- [实现优先级建议](#6-实现优先级建议)
- [动手修改指南](#动手修改指南)
- [验收标准](#验收标准)

---

## 序言：我们站在哪里，要往哪去

### 当前成绩

非 pthread 测试 **82/82 全部通过**。这是一个很好的基线——意味着系统调用（read、write、fork、exec、mmap 等）的基本路径都是正确的。

### 剩下的差距

还有约 **21 个测试** 没有 PASS，分成四类问题：

| 问题 | 测试数 | 一句话描述 |
|------|--------|-----------|
| pthread 线程创建卡死 | 8+ | clone 缺少 `CLONE_SIGHAND`，线程创建后互相干扰 |
| runtest.exe 不工作 | ~100（潜在） | wait4 或 sigtimedwait 有 bug，测试运行器跑不起来 |
| lseek panic | ~12 | 对 pipe/console 调 lseek 时内核崩溃 |
| 网络未实现 | 1 | socket 系列 syscall 返回 ENOSYS |

### 为什么这份文档重要？

这不是一份"研究文档"——这是一份**施工计划**。每个模块都给出了：
- 根因（为什么失败）
- 改什么文件、改哪里
- 改完后怎么验证

你不需要从头摸索，跟着做就行。

---

## 预备知识：你需要弄懂的几个概念

> 如果你已经熟悉 clone/pthread/signal/futex 这些概念，可以直接跳到后面的实现章节。

### pthread 是什么？它跟进程是什么关系？

**进程** = 一个独立运行的程序实例。每个进程有自己独立的地址空间（内存）、文件描述符表等。

**线程（pthread）** = 进程内部的"轻量级执行单元"。多个线程共享同一个地址空间、同一个文件描述符表，但各有自己的栈和寄存器。

**为什么要有线程？** 一个简单的例子：

```
单进程方案：
  主程序 → 读文件 → 处理数据 → 写结果 → 读下一个文件 → ...
  问题：读文件时 CPU 在等磁盘，浪费了

多线程方案：
  线程 1 → 读文件（阻塞时让出 CPU）
  线程 2 → 处理数据（在 CPU 上跑）
  线程 3 → 写结果（等磁盘写入）
  优势：CPU 和磁盘并行工作，效率更高
```

在 Linux 中，线程是通过 `clone()` 系统调用创建的——传入不同的标志位来区分"创建进程"和"创建线程"。

### clone() 的标志位是什么意思？

`clone()` 是 Linux 中创建新执行单元的底层系统调用。`fork()` 和 `pthread_create()` 最终都调用它。

```c
// fork() 等价于：
clone(SIGCHLD, 0, ...);
// → 创建独立进程，子进程有自己的地址空间、文件表等

// pthread_create() 等价于：
clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
      CLONE_THREAD | CLONE_SETTLS | ...);
// → 创建线程，共享地址空间、文件表、信号处理等
```

关键标志位的日常比喻理解：

| 标志位 | 日常比喻 |
|--------|---------|
| `CLONE_VM` | 共享同一个办公桌（地址空间） |
| `CLONE_FS` | 共享同一个工位号（文件系统根目录/工作目录） |
| `CLONE_FILES` | 共享同一套文件柜钥匙（文件描述符） |
| `CLONE_SIGHAND` | 共享同一个对讲机频道（信号处理函数）← **当前缺失** |
| `CLONE_THREAD` | 同属一个部门（线程组），共享部门编号（PID） |
| `CLONE_SETTLS` | 给新来的人发一件工服（线程局部存储） |
| `CLONE_PARENT_SETTID` | 把新人编号告诉经理（父线程获取子线程 ID） |
| `CLONE_CHILD_CLEARTID` | 新人离职时清空他的门禁卡（退出通知） |

> 💡 **当前最关键的缺失是 `CLONE_SIGHAND`**。如果没有它，多个线程共享同一份信号处理表但内核不知道它们是共享的，导致信号投递混乱、线程间互相覆盖信号配置——这就是 pthread 测试卡死的根因。

### 什么是信号（Signal）和信号处理？

信号是 Linux 中操作系统向进程发通知的机制。

```c
// 进程告诉内核："如果有人发 SIGCHLD 给我，请调用这个函数"
signal(SIGCHLD, my_sigchld_handler);

// 当子进程退出时，内核自动调用 my_sigchld_handler()
// 这就是"信号处理"
```

在多线程进程中，信号处理变得更复杂：
- 如果线程共享信号处理表（`CLONE_SIGHAND`），一个线程改了 `signal(SIGINT, ...)`，其他线程也能看到
- 但某些信号（如 `SIGSEGV` 段错误）应该只投递给"惹事"的那个线程
- libc-test 的 pthread 测试恰好会测这些边界情况

### 什么是 futex？

**futex = Fast Userspace muTEX**，是 Linux 中实现线程同步的底层机制。

不用 futex 的"忙等"方案：
```c
// 线程 A：等 flag 变成 1
while (flag != 1) { /* 占满 CPU，愚蠢 */ }

// 线程 B：设 flag = 1
flag = 1;
```

用 futex 的正确方案：
```c
// 线程 A：等 flag 变成 1
while (1) {
    if (flag == 1) break;
    futex_wait(&flag, 0);  // 内核帮你挂起，不占 CPU
}

// 线程 B：设 flag = 1，唤醒等待者
flag = 1;
futex_wake(&flag, 1);  // 内核帮你唤醒等待的线程
```

pthread 的 `pthread_mutex_lock`、`pthread_cond_wait` 等最终都依赖 futex。没有 futex 实现的话，pthread 的高级同步（条件变量、互斥锁超时等）全部无法工作。

### 什么是 VFS（虚拟文件系统）和 lseek？

**VFS** 是内核中"文件操作的统一接口层"。不管是真实的 ext4 磁盘文件、pipe（管道）、还是 console（控制台），对上层来说都是"一个文件"——有统一的 open/read/write/lseek 操作。

**lseek** 是"移动文件读写指针"的操作：
```c
lseek(fd, 100, SEEK_SET);  // 把指针移到文件第 100 字节处
lseek(fd, 0, SEEK_END);    // 把指针移到文件末尾
```

但并不是所有"文件"都支持 seek：
- ext4 磁盘文件 ✅ 支持（文件有具体大小）
- pipe（管道） ❌ 不支持（数据流，没法"倒退"读）
- console（控制台） ❌ 不支持（键盘输入，没有"位置"概念）

当前的 bug：内核对任何文件都强行调 ext4 的 lseek，但 pipe/console 没有 ext4 的数据结构，所以 panic 了。

---

## 1. 全景：当前状态与剩余差距

```
总测试数:     103 (functional 50 + regression 53)
├── ✅ PASS:  82  (直接 exec 方案)
├── ❌ FAIL:   1  (socket → 网络未实现)
├── ⏸️ HANG:   8  (pthread_* → pthread_create 卡死)
├── 💥 PANIC: ~12  (rewind_clear_error 起触发 lseek panic)
└── 跳过:      0  (runtest.exe 8 个 pthread 测试在脚本里，但未直接跑)
```

**理解这张图：**

- **82 个 PASS** 说明大部分系统调用（读、写、进程创建、文件操作等）是正确的
- **8 个 HANG** 是所有 pthread 测试共用同一个根因（clone 标志不完整），修好一个全修好
- **~12 个 PANIC** 是 lseek 对不支持的文件类型崩溃，修复成本极低（加类型检查）
- **1 个 FAIL** 是网络栈，工作量最大但可以放最后

---

## 2. 模块一：pthread 线程创建（8+ 个测试卡死/FAIL）

### 2.1 受影响的测试

| 测试 | 来源 | 症状 |
|------|------|------|
| pthread_cancel_points | functional | pthread_create 返回 EAGAIN → 测试卡死 |
| pthread_cancel | functional | 同上 |
| pthread_cond | functional | 同上 |
| pthread_tsd | functional | 同上 |
| pthread_robust_detach | regression | 同上 |
| pthread_cancel_sem_wait | regression | 同上 |
| pthread_cond_smasher | regression | 同上 |
| pthread_condattr_setclock | regression | 同上 |
| pthread_exit_cancel | regression | 同上 |
| pthread_once_deadlock | regression | 同上 |
| pthread_rwlock_ebusy | regression | 同上 |

> **好消息：** 所有 pthread 测试卡在同一个位置——`pthread_create` 内部。修好线程创建的底层问题，这 11 个测试会一起解除阻塞。

### 2.2 根因：pthread_create 调用的 clone 标志不完整

glibc/musl 的 `pthread_create()` 到底给内核传了什么？展开来看：

```c
// pthread_create 实际使用的 clone 调用
clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
      CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS |
      CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
      child_stack, &ptid, tls, &ctid);
```

当前内核对这些标志位的支持情况：

| 标志 | 当前内核状态 | 说明 |
|------|-------------|------|
| CLONE_VM | ✅ 已实现 | 页表共享（pt_shared 标志） |
| CLONE_FS | ✅ 已实现 | 文件系统信息共享 |
| CLONE_FILES | ✅ 已实现 | 文件描述符表共享 |
| CLONE_SIGHAND | ❌ **未实现** | 信号处理表共享（**关键缺失！**） |
| CLONE_THREAD | 🔄 部分 | 共享 PID 已实现，但缺少线程组语义 |
| CLONE_SYSVSEM | ❌ 未实现 | System V 信号量撤销 |
| CLONE_SETTLS | ✅ 已实现 | 子进程 tp 设置 |
| CLONE_PARENT_SETTID | ✅ 已实现 | 写 ptid |
| CLONE_CHILD_CLEARTID | ✅ 已实现 | exit 时清零 ctid |

**最重要的发现：** 缺了 `CLONE_SIGHAND`。这意味着 glibc 的 pthread 库认为所有线程应该共享信号处理函数表，但内核实际给了每个线程独立的一份。

**导致的后果：**
- 线程 A 在创建线程 B 时希望它们共享信号配置
- 但实际上线程 A 和 B 有各自独立的信号处理表
- 当信号到达时，内核不知道该唤醒谁，信号可能丢失
- pthread 的内部同步（信号 + futex 配合）因信号丢失而卡死

### 2.3 实现方案：分 4 步

#### 步骤 1：CLONE_SIGHAND 信号表共享

**为什么这是第一步？** 因为它是当前 pthread 卡死的直接根因。修好它，pthread_create 至少能成功创建线程（虽然后续同步可能还需要 futex）。

**改什么文件：** `kernel/proc/proc.c`（clone 函数）、`include/proc/proc.h`（结构体定义）

```c
// proc.h struct proc 新增
struct sig_handlers *sig_handlers;  // 信号处理器表（已有字段，无需新增）
// 关键：在 clone 中共享而非复制

// clone() 中 —— 找到 CLONE_SIGHAND 判断位置（通常紧挨 CLONE_FILES）
if (flags & CLONE_SIGHAND) {
    // 共享信号处理器表（不复制，直接赋指针）
    // ⚠️ 注意：需要引用计数，避免一个线程退出时释放了其他线程还在用的表
    np->sig_handlers = p->sig_handlers;
    atomic_inc(&np->sig_handlers->refcount);  // 增加引用计数
} else {
    // 现有逻辑：sig_init 已分配新的，保持不变
}
```

**配套修改（重要！）：**

```c
// 在 sig_cleanup() 或 proc 的 freeproc 中添加引用计数逻辑
void sig_handlers_put(struct sig_handlers *sh) {
    if (atomic_dec_and_test(&sh->refcount)) {
        // 引用计数降为 0，才真正释放
        kfree(sh);
    }
}
```

> 💡 **为什么需要引用计数？** 如果线程 A 和 B 共享一份 sig_handlers，当 A 先退出时会尝试释放它。但没有引用计数的话，B 还在使用这块内存——这叫做 use-after-free，会导致随机崩溃。

#### 步骤 2：CLONE_THREAD 线程组完善

**当前状态：** 共享 PID 已经做了，但线程组的"生命周期管理"缺失。

**需要补充的内容：**

```c
// proc.h 确认或新增
struct proc {
    int tgid;           // 线程组 ID（用已有的 pid 充当 tgid）
    int tid;            // 线程 ID ✅ 已有
    struct proc *thread_group;  // 线程组链表头（可选，用于遍历所有线程）
};

// clone() 中 CLONE_THREAD 时
if (flags & CLONE_THREAD) {
    np->tgid = p->tgid;          // 共享线程组 ID
    np->tid = allocpid();        // 唯一线程 ID ✅ 已有
    // 建议：将 np 添加到线程组链表
    np->thread_group = p->thread_group;
    p->thread_group = np;
}
```

**线程退出的特殊处理：**

```c
// exit() 中区分"主线程退出"和"普通线程退出"
if (p->tid == p->tgid) {
    // 我是主线程 → 退出整个线程组
    // 遍历 thread_group，给每个线程发 SIGKILL
    // 然后走正常的进程退出流程
} else {
    // 我是普通线程 → 只退出自己
    // 从线程组链表中移除
    // 唤醒父线程的 waitpid（如果需要）
}
```

> 这一部分是后续 pthread_join 功能的基础，但优先度低于步骤 1 和步骤 3。

#### 步骤 3：futex 线程同步

**为什么需要 futex？** pthread 内部大量使用 futex 做线程间同步：
- `pthread_mutex_lock` → `futex(FUTEX_WAIT)`
- `pthread_mutex_unlock` → `futex(FUTEX_WAKE)`
- `pthread_cond_wait` / `pthread_cond_signal` → futex REQUEUE 操作
- 线程退出时通过 futex 通知 join 等待者

**改什么文件：** 新建 `kernel/util/futex.c`，或扩展现有声明

```c
// 需要实现的 futex 系统调用完整签名：
// long sys_futex(uint32 *uaddr, int op, uint32 val,
//                const struct timespec *timeout, uint32 *uaddr2, uint32 val3);

#define FUTEX_WAIT    0  // 等待 *uaddr == val，否则睡眠
#define FUTEX_WAKE    1  // 唤醒最多 val 个在 uaddr 上等待的线程
#define FUTEX_REQUEUE 3  // 将等待者从 uaddr 移到 uaddr2

// 数据结构：为每个 futex 地址维护等待队列
struct futex_waiter {
    uint64 uaddr;         // 等待的用户空间地址
    struct proc *proc;    // 等待的进程
    struct futex_waiter *next;
};
```

**核心实现思路：**

```c
uint64 sys_futex(void) {
    // ... 从寄存器获取参数 ...

    switch (op) {
    case FUTEX_WAIT: {
        // 1. 读取用户空间 *uaddr 的值
        // 2. 如果 *uaddr != val → 说明条件已满足，直接返回 0（不睡眠）
        // 3. 如果 *uaddr == val → 创建 waiter，sleep on &uaddr
        // 4. 被唤醒后返回 0
        uint32 cur_val;
        copyin(p->pagetable, (char*)&cur_val, uaddr, sizeof(cur_val));
        if (cur_val != val) return -EAGAIN;
        // 加入等待队列，睡眠
        futex_wait_add(uaddr, p);
        sleep(p, &futex_wait_queue);
        return 0;
    }
    case FUTEX_WAKE: {
        // 唤醒最多 val 个在 uaddr 上等待的线程
        int woken = 0;
        for (每个在 uaddr 等待的 waiter，最多 val 个) {
            wakeup(waiter->proc);
            woken++;
        }
        return woken;  // 返回实际唤醒的次数
    }
    }
}
```

> 📖 **参考：** `kernel/syscall/sysproc.c` 已有 `sys_futex` 的声明和入口框架，可以在此基础上填充。

#### 步骤 4：线程退出与 join

**前提：** 步骤 1-3 完成后，线程能创建、能同步。但线程退出后的 cleanup 还需要处理。

```c
// exit() 中对 CLONE_THREAD 线程的特殊处理
if (p->tgid == p->tid) {
    // 主线程退出 → 等所有线程退出或给它们发 SIGKILL
} else {
    // 普通线程退出：
    // - 不清除进程级资源（页表、文件表等由主线程释放）
    // - 设置自己的退出状态
    // - 如果设置了 clear_child_tid，清零并 futex_wake
    // - 变为 ZOMBIE 状态，等 waitpid 回收
}

// waitpid() 需要支持：
// - WNOHANG: 非阻塞查询
// - __WALL: 不只等子进程，也等线程组中的其他线程
// - 返回的 status 正确编码退出状态
```

---

## 3. 模块二：sigtimedwait + waitpid 链路（runtest.exe FAIL）

### 3.1 什么是 runtest.exe？为什么它不工作影响这么大？

`runtest.exe` 是 libc-test 的**测试运行器**。它的工作流程：

```
runtest.exe
  ├── 循环遍历所有子测试
  │   ├── fork 子进程
  │   ├── 子进程 exec entry-static.exe <测试名>
  │   ├── 父进程 sigtimedwait 等 SIGCHLD（子进程退出信号）
  │   ├── 收到信号后 wait4 回收子进程
  │   └── 检查退出码，打印结果和耗时
  └── 汇总所有测试结果
```

当前症状：runtest.exe 的 `waitpid` 返回 EAGAIN，导致测试框架无法正常工作。这意味着约 100 个本应通过 runtest.exe 跑的测试无法执行。

### 3.2 根因定位

完整调用链见文档 27。核心问题在 `wait4()` 中的一个微妙 bug——`xstate` 处理不符合 Linux 约定：

```c
// kernel/proc/proc.c wait4() —— 当前代码（疑似有 bug）
if (pp->state == ZOMBIE) {
    found_pid = pp->pid;
    pp->xstate = pp->xstate << 8;   // ← 这里直接修改了 pp->xstate！
    copyout(pagetable, wstatus_addr, (char *)&pp->xstate, sizeof(pp->xstate));
    freeproc(pp);
    return found_pid;
}
```

**为什么这是一个 bug？**

Linux 的 `wait4` 约定：
- 内核返回"原始退出状态"（不左移）
- 用户空间通过宏提取：`WEXITSTATUS(status) = (status >> 8) & 0xff`

当前代码做了 `xstate << 8`：
1. 第一次 wait4 时，移位后 copyout → 用户空间看到移位后的值（可能碰巧正确）
2. 但如果同一僵尸被 wait4 两次（或不同路径），第二次读到的 `pp->xstate` 已经是移位后的垃圾值
3. 更危险的是，修改 `pp->xstate` 可能影响其他检查僵尸的代码路径

**可能的触发链：**
```
子进程 exit(0) → xstate = 0
wait4() → xstate <<= 8 → xstate = 0 → copyout
                       → freeproc(pp)
别的代码路径检查到 EAGAIN...
```

### 3.3 修复方案

```c
// kernel/proc/proc.c wait4() —— 修复后
if (pp->state == ZOMBIE) {
    found_pid = pp->pid;

    // ✅ 不修改 pp->xstate，用局部变量打包退出状态
    int status_word = (pp->xstate & 0xff) << 8;

    if (wstatus_addr != 0) {
        copyout(p->pagetable, wstatus_addr,
                (char *)&status_word, sizeof(status_word));
    }

    freeproc(pp);
    release(&pp->lock);
    release(&wait_lock);
    return found_pid;
}
```

**关键改动：**
- `pp->xstate` 保持原样不动
- 用局部变量 `status_word` 做移位和输出
- 符合 Linux 的 wait4 约定，不会产生副作用

### 3.4 调试 EAGAIN 的来源

如果修复 xstate 后问题仍在，用以下方法缩小范围：

```bash
# 1. 在 wait4() 入口加 printf，打印 target_pid 和当前 ZOMBIE 列表
# 2. 在 sigtimedwait 返回前加 printf，打印返回的信号编号
# 3. 用 runtest.exe 跑单个测试，观察输出序列：
```

```c
// 在 wait4() 中添加调试输出
printf("[wait4] pid=%d want_pid=%d options=0x%x zombie_list: ",
       p->pid, target_pid, options);
// 遍历 proc 列表，打印所有 ZOMBIE 的 pid
for (pp = proc; pp < &proc[NPROC]; pp++) {
    if (pp->state == ZOMBIE)
        printf("%d(xstate=%d) ", pp->pid, pp->xstate);
}
printf("\n");

// 在 sigtimedwait 返回前
printf("[sigtimedwait] returning signal=%d\n", sig);
```

---

## 4. 模块三：VFS lseek panic（~10 个测试未跑到）

### 4.1 现象

```
panic: vfs_ext_lseek: cannot get ext4 file
```

在第 82 个测试通过后，`rewind_clear_error` 调用了 `lseek`——内核直接 panic 了，后面的 ~12 个测试根本没机会跑。

### 4.2 为什么会 panic？

内核的 VFS 层对所有文件类型统一调用 ext4 的 lseek，但有些文件根本不支持 seek：

```c
// 当前代码大致是这样的（简化）
int vfs_lseek(struct file *f, int64 offset, int whence) {
    struct ext4_file *ef = (struct ext4_file *)f->f_extfile;
    if (!ef) panic("cannot get ext4 file");   // ← 这里崩溃了
    // ext4 的 lseek 逻辑...
}

// 当 f 是 pipe 时，f->f_extfile == NULL → panic!
```

**日常比喻：** 这就像一个快递柜的管理员，对所有包裹都按"大型快递柜"的规格操作。来了一个信封（pipe），他也试图放到大型柜里扫描，发现没有条码（f_extfile == NULL），然后崩溃了。

### 4.3 受影响的具体测试

从 `rewind_clear_error` 到 `wcsstr_false_negative`，大约 12 个测试因为内核 panic 而未能执行。

### 4.4 修复方案（修复成本极低：约 10 行代码）

**改什么文件：** `kernel/fs/vfs/ops.c` 或 lseek 实现所在文件

```c
// 在 lseek 实现的最开头加类型检查
int64 vfs_lseek(struct file *f, int64 offset, int whence) {
    // 步骤 1：拒绝不支持 seek 的文件类型
    if (f->f_type == FD_PIPE || f->f_type == FD_CONSOLE) {
        return -ESPIPE;  // "Illegal seek" —— 告诉调用者这个文件不能 seek
    }

    // 步骤 2：确保有 ext4 文件结构
    if (!f->f_extfile) {
        return -EBADF;   // 坏的文件描述符
    }

    // 步骤 3：正常的 ext4 lseek 逻辑（原有代码保持不变）
    struct ext4_file *ef = (struct ext4_file *)f->f_extfile;
    // ...
}
```

**参考文件类型枚举：**

| f_type 值 | 含义 | 支持 lseek？ |
|-----------|------|-------------|
| FD_PIPE | 管道 | ❌ 不支持，返回 -ESPIPE |
| FD_CONSOLE | 控制台 | ❌ 不支持，返回 -ESPIPE |
| FD_INODE | 普通 ext4 文件 | ✅ 支持 |
| FD_DEVICE | 块设备 | 🔄 特殊处理 |

> ⚠️ **重要教训：** 永远不要在 VFS 层对 `f_extfile` 做无条件强转。总是先判断文件类型。这是 VFS 设计的基本安全规则。

### 4.5 验证方法

修复后运行 libc-test 的 regression 测试，确认不再出现 lseek panic，且 `rewind_clear_error` 及之后的测试能正常跑到。

---

## 5. 模块四：网络栈（socket 测试 FAIL）

### 5.1 受影响测试

| 测试 | 症状 | 缺失系统调用 |
|------|------|-------------|
| socket | FAIL (status=1) | socket(198) |
| socket（回归路径） | 同上 | bind(200), listen(201), accept(202), connect(203), getsockname(204), sendto(206), recvfrom(207), setsockopt(208) |

### 5.2 实现策略：只做本地回环

完整 TCP/IP 协议栈非常复杂（需要 IP 分片重组、TCP 拥塞控制、滑动窗口等），但对于 libc-test 的 socket 测试，只需要**本地回环（127.0.0.1）**——同一台机器上两个程序通过网络接口通信，数据根本不经过真实网卡。

**这意味着：**
- ✅ 不需要网卡驱动
- ✅ 不需要 ARP/IP/路由
- ✅ 不需要 TCP 复杂的状态机
- ✅ 只需要：创建 socket → 绑定地址 → listen/connect → 数据在内存中转移

### 5.3 实现方案：分 3 步

#### 步骤 1：socket 框架

```c
// 新建 kernel/fs/socket.c

// socket 类型（libc-test 只需要这两个）
enum socket_type {
    SOCK_STREAM = 1,  // TCP —— 可靠的、面向连接的字节流
    SOCK_DGRAM  = 2,  // UDP —— 数据报（无连接）
};

// socket 状态机
enum socket_state {
    SOCKET_FREE = 0,
    SOCKET_UNCONNECTED,
    SOCKET_LISTENING,
    SOCKET_CONNECTED,
};

// 单个 socket 的数据结构
struct socket {
    int type;            // SOCK_STREAM 或 SOCK_DGRAM
    int state;           // 当前状态
    uint16 port;         // 绑定的端口号
    struct socket *peer; // 对方的 socket（连接建立后）

    // 环形缓冲区（数据不经过网卡，直接在内存中传递）
    char rbuf[4096];     // 接收缓冲区
    int rhead, rtail;    // 读写指针
    char sbuf[4096];     // 发送缓冲区
    int shead, stail;
};
```

#### 步骤 2：实现核心系统调用

**socket() —— 创建 socket：**

```c
uint64 sys_socket(int domain, int type, int protocol) {
    // 分配 socket 结构
    struct socket *s = socket_alloc();
    if (!s) return -ENOMEM;

    s->type = type;
    s->state = SOCKET_UNCONNECTED;

    // 关联到一个文件描述符，返回给用户
    return fdalloc_with_socket(s);
}
```

**bind() —— 绑定端口：**

```c
uint64 sys_bind(int fd, struct sockaddr *addr, socklen_t len) {
    struct socket *s = fd2socket(fd);
    if (!s) return -EBADF;
    s->port = addr->sin_port;  // 记录绑定的端口号
    return 0;
}
```

**listen() —— 开始监听：**

```c
uint64 sys_listen(int fd, int backlog) {
    struct socket *s = fd2socket(fd);
    if (!s) return -EBADF;
    s->state = SOCKET_LISTENING;
    return 0;  // backlog 参数可忽略
}
```

**accept() —— 接受连接：**

```c
uint64 sys_accept(int fd, struct sockaddr *addr, socklen_t *addrlen) {
    struct socket *server = fd2socket(fd);
    if (!server || server->state != SOCKET_LISTENING) return -EINVAL;

    // 等待有客户端来 connect
    while (server->peer == NULL) {
        sleep(server, &socket_wait);  // 挂起，等 connect 来唤醒
    }

    // 创建新的 socket 代表这个连接
    struct socket *client_sock = socket_alloc();
    client_sock->peer = server->peer;
    server->peer->peer = client_sock;

    return fdalloc_with_socket(client_sock);
}
```

**connect() —— 发起连接：**

```c
uint64 sys_connect(int fd, struct sockaddr *addr, socklen_t len) {
    struct socket *client = fd2socket(fd);
    if (!client) return -EBADF;

    // 在所有 socket 中找到监听这个端口的 server
    struct socket *server = find_listener(addr->sin_port);
    if (!server) return -ECONNREFUSED;

    // 建立连接
    client->peer = server;
    server->peer = client;
    wakeup(server);    // 唤醒 accept 中等待的 server
    return 0;
}
```

**sendto / recvfrom —— 发数据和收数据：**

```c
uint64 sys_sendto(int fd, void *buf, size_t len, int flags,
                  struct sockaddr *addr, socklen_t addrlen) {
    struct socket *s = fd2socket(fd);
    // 把用户数据复制到 peer 的接收缓冲区
    copyin(p->pagetable, s->peer->rbuf + s->peer->rtail, buf, len);
    s->peer->rtail += len;
    wakeup(s->peer);  // 唤醒可能在 recvfrom 中等待的对方
    return len;
}

uint64 sys_recvfrom(int fd, void *buf, size_t len, int flags,
                    struct sockaddr *addr, socklen_t *addrlen) {
    struct socket *s = fd2socket(fd);
    // 等缓冲区有数据
    while (s->rhead == s->rtail) {
        sleep(s, &socket_wait);  // 没数据就挂起
    }
    // 复制到用户空间
    copyout(p->pagetable, buf, s->rbuf + s->rhead, len);
    s->rhead += len;
    return len;
}
```

#### 步骤 3：注册到 syscall 表

```c
// kernel/syscall/syscall.c 添加
[SYS_socket]        sys_socket,
[SYS_bind]          sys_bind,
[SYS_listen]        sys_listen,
[SYS_accept]        sys_accept,
[SYS_connect]       sys_connect,
[SYS_getsockname]   sys_getsockname,
[SYS_sendto]        sys_sendto,
[SYS_recvfrom]      sys_recvfrom,
[SYS_setsockopt]    sys_setsockopt,   // 可简单返回 0
```

> 💡 **简化技巧：** 以上所有代码只需要本地回环——数据在内核内存中直接转移，不经过任何实际网络设备。libc-test 的 socket 测试只验证"socket/bind/listen/accept/connect/send/recv 这个流程能否走通"，不涉及真实网络通信。

---

## 6. 实现优先级建议

### 推荐执行顺序

```
P0 (30分钟)  →  P1 (2小时)  →  P2+P3+P4 (4小时)  →  P5 (4-8小时)
     ↓              ↓                ↓                    ↓
 lseek panic   runtest.exe      pthread 8+           socket 1个
   修复          可用            不再挂起             不再 FAIL
```

| 优先级 | 模块 | 影响测试数 | 估计工作量 | 理由 |
|--------|------|-----------|-----------|------|
| **P0** | VFS lseek panic | ~12 | 30 分钟（10 行） | 阻塞后续所有测试，修复成本最低，应该第一个做 |
| **P1** | wait4 xstate 修复 + 调试 | ~100 | 2 小时 | 让 runtest.exe 可用，一次性覆盖所有依赖它的测试 |
| **P2** | pthread CLONE_SIGHAND | 8+ | 1-2 小时 | 线程创建的核心缺失，修了它 pthread_create 就能成功 |
| **P3** | futex 实现 | 8+ | 2 小时 | pthread 同步依赖，futex 框架本身不复杂 |
| **P4** | pthread 退出/join | 8+ | 1 小时 | 线程生命周期管理，依赖 P2+P3 |
| **P5** | 网络栈 | 1 | 4-8 小时 | 全套新代码，但只需本地回环 |

### 为什么这个顺序？

1. **P0 先修：** lseek panic 阻塞了后面的测试。修好它，至少能知道后面哪些测试能过、哪些不能过，信息量巨大。
2. **P1 接着：** runtest.exe 是测试基础设施。修好它，所有非 pthread 测试都能规范地跑起来（带超时、带计时、带 START/END 标记）。
3. **P2-P4 一起做：** 这三个是同一个功能的三个层面，分开做反而增加调试难度。
4. **P5 放最后：** 网络栈独立于其他模块，且只影响 1 个测试，收益比最低。

---

## 动手修改指南

### 文件地图速查

```
my-springos/
├── kernel/
│   ├── proc/
│   │   ├── proc.c          ← clone() 实现（P2 CLONE_SIGHAND）、wait4()（P1 xstate 修复）
│   │   └── exec.c          ← ELF 加载
│   ├── syscall/
│   │   ├── syssig.c        ← 信号系统调用
│   │   └── syscall.c       ← syscall 注册表（P5 网络 syscall 注册）
│   ├── fs/
│   │   └── vfs/ops.c       ← VFS 操作（P0 lseek 类型检查）
│   └── util/
│       └── futex.c         ← futex 实现（P3，可能需要新建）
├── include/
│   └── proc/
│       └── proc.h          ← proc 结构体（P2 tgid/tid/引用计数）
└── user/
    └── init-rv.c           ← 用户态入口/测试运行器
```

### 每个模块的修改清单

#### P0: lseek panic

- [ ] `kernel/fs/vfs/ops.c`（或 lseek 函数所在文件）：在函数开头加 `f_type` 类型检查，pipe/console 返回 `-ESPIPE`
- [ ] 编译运行 → regression 测试确认不再 panic
- [ ] 观察 `rewind_clear_error` 及之后的测试结果

#### P1: wait4 xstate

- [ ] `kernel/proc/proc.c` `wait4()` 函数：用局部变量 `status_word` 替代直接修改 `pp->xstate`
- [ ] 添加调试 printf（可选，帮助定位 EAGAIN 来源）
- [ ] 编译运行 → 用 `runtest.exe` 单测验证

#### P2: CLONE_SIGHAND

- [ ] `include/proc/proc.h`：`struct sig_handlers` 添加 `refcount` 字段
- [ ] `kernel/proc/proc.c` `clone()`：添加 `CLONE_SIGHAND` 处理（共享指针 + 引用计数）
- [ ] `kernel/proc/proc.c`：`freeproc()` 中添加 `sig_handlers_put()` 调用
- [ ] 编译运行 → 检查 pthread 测试是否不再卡死

#### P3: futex

- [ ] 新建 `kernel/util/futex.c`，实现 `FUTEX_WAIT` 和 `FUTEX_WAKE`
- [ ] `kernel/syscall/sysproc.c`：在已有的 `sys_futex` 入口中填充实现
- [ ] 编译运行 → pthread 同步测试是否通过

#### P4: 线程退出/join

- [ ] `kernel/proc/proc.c` `exit()`：添加 CLONE_THREAD vs 主线程的区分
- [ ] `kernel/proc/proc.c` `wait4()`：支持 `WNOHANG` 和 `__WALL`
- [ ] 编译运行 → pthread_join 相关测试

#### P5: 网络栈

- [ ] 新建 `kernel/fs/socket.c`，实现 socket 框架（见 5.3）
- [ ] `kernel/syscall/syscall.c`：注册 8 个网络 syscall
- [ ] 编译运行 → socket 测试 PASS

### 通用调试技巧

**1. printf 永远是最快的调试器：**
```c
printf("[DEBUG] function=%s pid=%d value=0x%lx\n", __func__, p->pid, value);
```

**2. 缩小测试范围：** 不用每次跑全部 libc-test，只跑一个：
```c
// user/init-rv.c 中
if (fork() == 0) {
    execve("/bin/entry-static.exe",
           (char*[]){"entry-static.exe", "pthread_cancel", NULL}, NULL);
}
wait(NULL);
```

**3. 用 GDB 设断点：**
```bash
qemu-system-riscv64 ... -s -S &      # -s: gdb端口, -S: 启动时暂停
riscv64-linux-gnu-gdb kernel
(gdb) target remote :1234
(gdb) b clone
(gdb) c
```

**4. 检查进程/线程状态：** 在 /proc 或调试输出中看 pid/tid/state 是否正确。

**5. 理解 errno：** `-EAGAIN`（再试一次）、`-EINVAL`（参数不对）、`-ENOSYS`（没实现）含义不同，别混用。

---

## 验收标准

完成所有模块后，预期结果：

| 测试组 | 目标 | 当前 |
|--------|------|------|
| non-pthread functional | 50/50 PASS | 82/82（含 regression）|
| pthread 系列 | 11/11 PASS | 0/11（全 HANG） |
| socket | 1/1 PASS | 0/1（FAIL） |
| libc-test 总计 | **103/103 PASS** 🎯 | 82/103 |

> **最终目标：** libc-test 全部 103 个测试（functional 50 + regression 53）全部 PASS，每个测试都有 runtest.exe 生成的 START/END 标记、计时、和正确的 PASS/FAIL 判定。

---

> **一点鼓励：** 从 82 到 103，差了 21 个测试。但这 21 个共享 3 个根因（pthread 缺失、wait4 bug、lseek panic）和 1 个独立功能（网络栈）。不是 21 个独立问题，是 3+1 个。修一个 → 解锁一片。别被数量吓到，盯住根因。
