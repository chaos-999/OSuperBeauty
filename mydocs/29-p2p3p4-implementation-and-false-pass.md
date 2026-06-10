# 29 —— 内核 panic 修复 & 假 PASS 诊断

> 这是一份"动手施工记录 + 诊断报告"。
> 基于文档 28 的路线图，实施了 P0～P4 的代码修改，并发现了测试通过但实际有问题的"假 PASS"。
>
> **本文档的目标读者：** 正在学习 xv6/SpringOS 内核开发，想理解"测试通过了 ≠ 代码没问题"这个道理的你。

---

## 快速导航

- [序言：从 panic 说起](#序言从-panic-说起)
- [预备知识](#预备知识)
- [修复一：VFS 的 panic 炸弹](#修复一vfs-的-panic-炸弹)
- [修复二：线程的信号处理表共享](#修复二线程的信号处理表共享)
- [修复三：线程退出要跟进程不一样](#修复三线程退出要跟进程不一样)
- [修复四：wait4 的一个微小 bug](#修复四wait4-的一个微小-bug)
- [意外发现：那些已经存在的代码](#意外发现那些已经存在的代码)
- [测试结果：96 个 PASS，但……](#测试结果96-个-pass但)
- [假 PASS 诊断：谁在说谎](#假-pass-诊断谁在说谎)
- [待修复清单](#待修复清单)
- [动手指南：重现这些修改](#动手指南重现这些修改)
- [排查技巧](#排查技巧)

---

## 序言：从 panic 说起

### 发生了什么？

在上一轮测试中，内核跑到第 83 个测试时就崩溃了：

```
panic: vfs_ext_lseek: cannot get ext4 file
```

内核崩溃（panic）意味着整个系统挂掉——后面的 14 个测试根本没机会执行。这就像你正在批改 100 份试卷，改到第 83 份时红笔断了，剩下 17 份直接跳过。

### 我们做了什么？

修了 **4 个 bug**，改了 **4 个文件**：

| Bug | 日常比喻 | 严重程度 |
|-----|---------|---------|
| lseek panic | 快递柜管理员对信封也扫快递柜条码，没扫到就砸柜子 | 💥 内核崩溃 |
| clone 缺少 SIGHAND 共享 | 招了新员工但没给他办公室钥匙 | ⚠️ 功能缺失 |
| 线程退出处理不当 | 室友搬走时把共享客厅也打包带走了 | ⚠️ 资源泄漏 |
| wait4 破坏僵尸状态 | 医生检查病人时顺便把病历改了 | 🐛 逻辑错误 |

### 测试结果变化

```
修复前：82 PASS, ~14 个被 panic 阻塞
修复后：96 PASS, 0 个 panic
```

但 96 个 PASS 里有 **9 个是假 PASS**——测试输出里明明报了错，但 exit code 是 0，被测试框架误判为通过。后面的章节会逐一分析。

---

## 预备知识

> 如果你已经熟悉 VFS、panic vs errno、文件描述符类型这些概念，可以跳到修复章节。

### 什么是 VFS？什么叫"文件类型"？

**VFS（Virtual File System，虚拟文件系统）** 是内核中"文件操作的统一接口层"。

在 Linux（以及 SpringOS）中，不管底层是真实的磁盘文件、管道（pipe）、还是控制台（console），对上层应用程序来说，它们都是"一个文件"——你可以对它们调 `read`、`write`、`lseek` 等操作。

但并不是所有"文件"都支持所有操作：

| 文件类型 | 底层存储 | read | write | lseek |
|---------|---------|------|-------|-------|
| ext4 磁盘文件 | ext4 文件系统 | ✅ | ✅ | ✅ |
| pipe（管道） | 内存环形缓冲 | ✅ | ✅ | ❌ 不支持 |
| console（控制台） | 键盘/串口 | ✅ | ✅ | ❌ 不支持 |

**日常比喻：** VFS 就像快递柜的管理系统。所有包裹（文件）统一登记，但信封和纸箱的存取方式不同——你不能把信封塞进大型快递柜的最深处再试图"定位"它（lseek）。

### 什么是 panic？它和返回 error 有什么区别？

**panic** = 内核遇到无法恢复的错误，主动停止一切运行。相当于"系统崩溃"，所有正在运行的进程都完蛋。

**返回 error（如 -EBADF）** = 告诉调用者"这个操作我做不到"，但系统继续运行。

```c
// panic 的后果
panic("vfs_ext_lseek: cannot get ext4 file");
// → 系统崩溃，所有进程死亡，QEMU 卡住

// 返回 error 的后果
return -EBADF;
// → 当前进程收到"坏文件描述符"错误，系统继续运行
// → 进程可以选择处理这个错误，或者退出
```

**日常比喻：** panic 就像是程序对你说"这道菜做不了，我把厨房炸了"。返回 error 是"这道菜做不了，请问要换一道吗？"

### 什么是 errno？-1 和 -EBADF 有什么区别？

**errno** 是 Linux 中表示"操作失败原因"的编号系统。每个 syscall 的返回值如果 < 0，用户态 C 库就会把这个负数转成对应的 errno 值。

```c
// 内核返回不同值，用户态看到不同 errno：
return -1;      // → errno = 1  = EPERM    "Operation not permitted"（操作不被允许）
return -9;      // → errno = 9  = EBADF    "Bad file descriptor"（坏的文件描述符）
return -29;     // → errno = 29 = ESPIPE   "Illegal seek"（非法 seek）
return -38;     // → errno = 38 = ENOSYS   "Function not implemented"（功能未实现）
```

**关键区别：**
- `return -1` → 用户看到的 errno 永远是 `EPERM`（操作不被允许），不管你实际想表达什么错误
- `return -EBADF` → 用户看到 `EBADF`（坏的文件描述符），知道是文件描述符的问题
- `return -ENOSYS` → 用户看到 `ENOSYS`（功能未实现），知道这个 syscall 还没做

**日常比喻：** 去医院挂号，护士问"你哪里不舒服"。
- `return -1` ＝ "我不舒服"（等于没说）
- `return -EBADF` ＝ "我脚疼"（给了有用信息）
- `return -ENOSYS` ＝ "这个科室还没开"（准确描述了问题）

### 进程和线程在退出时有什么区别？

| | 进程退出 | 线程退出 |
|------|---------|---------|
| 打开的文件 | 全部关闭 | 不动（其他线程还在用） |
| 内存映射（VMA） | 全部释放 | 不动（共享页表） |
| 当前工作目录 | 清空 | 不动（共享） |
| 子进程 | 转给 init | 不动（线程不该有子进程） |

如果线程退出时错误地关闭了文件——其他线程还在读写这个文件——就会出问题。

---

## 修复一：VFS 的 panic 炸弹

### 问题：快递柜管理员砸柜子

在 `kernel/fs/VFS_ext.c` 中，8 个 VFS 函数（lseek、read、write、fstat、ioctl 等）在遇到 `f_extfile == NULL` 时直接 `panic()`。

```c
// 修复前 —— 8 处这样的代码
int vfs_ext_lseek(struct file *f, int offset, int whence) {
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        panic("vfs_ext_lseek: cannot get ext4 file\n");  // 💥
    }
    // ...
}
```

`f_extfile` 只在 ext4 磁盘文件中有效。pipe 和 console 这种非 ext4 文件，`f_extfile` 就是 NULL。

**触发场景：** libc-test 的 `rewind_clear_error` 测试调用了 `lseek` —— 内核在 VFS 层直接 panic。后面的 14 个测试全没跑到。

### 修复

```c
// 修复后 —— 先判断文件类型，不支持的操作优雅返回错误

// 以 lseek 为例：
int vfs_ext_lseek(struct file *f, int offset, int whence) {
    // 对不支持 seek 的文件类型返回错误
    if (f->f_type == FD_PIPE)
        return -ESPIPE;  // "Illegal seek"——管道不能 seek
    if (f->f_type == FD_NONE || f->f_extfile == NULL)
        return -EBADF;   // 无效文件或缺少 ext4 底层结构

    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    // ... 正常 ext4 lseek 逻辑
}

// read / write / fstat / ioctl / statx 也做了同样处理：
// if (file == NULL) return -EBADF;
```

**涉及函数清单：** `vfs_ext_lseek`、`vfs_ext_read`、`vfs_ext_readat`、`vfs_ext_write`、`vfs_ext_writeat`、`vfs_ext_ioctl`、`vfs_ext_fstat`、`vfs_ext_statx`

### 为什么能这么改？

内核的调用路径保证了：pipe 文件应该走 pipe 专用的 read/write 路径，不会走到 ext4 的 VFS 层。但如果代码有路由 bug，或者某个测试程序走了一条不寻常的路径，这个 NULL 检查就能兜底——报错但不会崩溃。

> 📝 **教训：** VFS 层对 `f_extfile` 永远不要做无条件强转。先判断文件类型，不支持的操作返回错误。这是 VFS 设计的基本安全规则。

---

## 修复二：线程的信号处理表共享

### 问题：新员工没有办公室钥匙

当 `pthread_create()` 创建新线程时，它会调用：

```c
clone(CLONE_VM | ... | CLONE_SIGHAND | ...);
```

`CLONE_SIGHAND` 表示"新线程和父线程共享同一份信号处理函数表"。在 SpringOS 中，这个标志位的定义在 `include/defs.h` 里已经有了，但 clone() 函数没有处理它。

**后果：** 每个线程有各自独立的信号处理表。线程 A 注册了 `signal(SIGINT, handler_A)`，但线程 B 看不到这个设置。当信号到达时，内核不知道该按谁的配置来投递信号，pthread 内部的同步机制因此卡死。

### 前置修复：信号表引用计数初始化

`kernel/trap/signal.c` 中的 `sig_init()` 负责为新进程分配信号处理器表。结构体定义里已经有了 `refcount` 字段用于引用计数，但初始化时没有设值：

```c
// sig_init() —— 修复前
memset(p->sig_handlers, 0, sizeof(struct sig_handlers));
initlock(&p->sig_handlers->lock, "sighandlers");
// ❌ 缺少 refcount = 1 的初始化

// 修复后
memset(p->sig_handlers, 0, sizeof(struct sig_handlers));
initlock(&p->sig_handlers->lock, "sighandlers");
p->sig_handlers->refcount = 1;  // ✅ 初始引用计数为 1
```

**为什么需要引用计数？** 当多个线程共享同一份 sig_handlers 时，你不能在第一个线程退出时就释放它——因为其他线程还在用。引用计数确保只有当最后一个使用者退出时才真正释放内存。

### 主修复：clone() 中共享信号表

```c
// kernel/proc/proc.c clone() —— 在 CLONE_SETTLS 和 CLONE_CHILD_CLEARTID 之间新增

// CLONE_SIGHAND: 共享信号处理器表（递增引用计数）
if (flags & CLONE_SIGHAND) {
    acquire(&p->sig_handlers->lock);   // 加锁保护 refcount 修改
    p->sig_handlers->refcount++;
    release(&p->sig_handlers->lock);
    np->sig_handlers = p->sig_handlers;  // 共享指针，不复制
}
```

**关键细节：**
- 不是复制一份新的信号表，而是直接赋指针（`np->sig_handlers = p->sig_handlers`）
- 引用计数加 1，表示多了一个使用者
- 当线程退出时，`sig_cleanup()` 会自动减引用计数，减到 0 才释放

### 配套修复：CLONE_PARENT_SETTID 写对 ID

顺便修了一个小问题：`CLONE_PARENT_SETTID` 应该把新线程的 **TID**（线程 ID）写回父线程的 `ptid` 地址，而不是 PID（进程 ID）：

```c
// 修复前
if (flags & CLONE_PARENT_SETTID)
    copyout(p->pagetable, ptid, (char *)&np->pid, sizeof(int));
    // ❌ 写的是 np->pid，对线程来说是 tgid，不是唯一定位符

// 修复后
if (flags & CLONE_PARENT_SETTID) {
    int child_id = (flags & CLONE_THREAD) ? np->tid : np->pid;
    copyout(p->pagetable, ptid, (char *)&child_id, sizeof(int));
    // ✅ 线程写 tid，进程写 pid
}
```

### 代码清理

删除 clone() 末尾的重复逻辑（原 561-573 行）——这段代码和上面的 `CLONE_PARENT_SETTID` 做了重复的 ptid/ctid copyout，且没有线程/进程的区分。

---

## 修复三：线程退出要跟进程不一样

### 问题：室友搬走时把共享客厅也打包了

进程退出时要做的事：关闭所有文件、释放内存映射、清空工作目录、把子进程转给 init。

但线程退出时，**文件、内存映射、工作目录都是和其他线程共享的**——如果线程退出时关了文件，其他线程就读不到数据了。

### 第一步：补上 tgid 初始化

`include/proc/proc.h` 中 `struct proc` 早就有 `tgid` 字段，但在 `allocproc()` 中从未初始化它：

```c
// kernel/proc/proc.c allocproc() —— 修复前
p->pid = allocpid();
p->tid = p->pid;  // Default: tid == pid
// ❌ tgid 没有设值，默认为 0

// 修复后
p->pid = allocpid();
p->tgid = p->pid; // ✅ 单线程进程：tgid == pid
p->tid = p->pid;  // 单线程进程：tid == pid
```

**为什么这很重要？** `exit()` 中用 `p->tid != p->tgid` 来判断"这是线程还是主线程"。如果 tgid 一直是 0，每个进程都会被误判为线程，导致退出时跳过文件关闭等关键清理步骤。

### 第二步：exit() 中区分线程和进程

```c
// kernel/proc/proc.c exit() —— 核心改动

void exit(int status) {
    struct proc *p = myproc();
    if (p == initproc) panic("init exiting");

    // 🔑 关键判断：tid != tgid → 这是一个线程（不是主线程）
    int is_thread = (p->tid != p->tgid);

    if (!is_thread) {
        // === 主线程/进程退出：清理全部资源 ===
        // 关闭所有文件
        for (int fd = 0; fd < NOFILE; fd++) {
            if (p->ofile[fd]) { get_fops()->close(p->ofile[fd]); p->ofile[fd] = 0; }
        }
        // 如果页表不共享（非 CLONE_VM），清理 VMA
        if (!p->pt_shared) { /* ... 释放 VMA ... */ }
        // 清空当前工作目录
        memset(&(p->cwd), 0, sizeof(p->cwd));
        // 把子进程转给 init
        reparent(p);
    }
    // 线程退出：跳过以上共享资源的清理

    // 通知父进程、发 SIGCHLD、设置 ZOMBIE（线程和进程都要做）
    wakeup(p->parent);
    // ...
}
```

### 第三步：线程退出时通知 pthread_join

pthread 的线程 join 机制依赖一个 futex：父线程在 `ctid` 地址上调 `futex_wait` 睡眠，子线程退出时清零 `ctid` 并 `futex_wake` 唤醒父线程。

```c
// exit() 中 —— 修复前
if (p->clear_child_tid) {
    int clear = 0;
    copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
    p->clear_child_tid = 0;
    // ❌ 清零了地址，但没有唤醒 pthread_join 的等待者
}

// 修复后
if (p->clear_child_tid) {
    int clear = 0;
    copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
    futex_wake(p->clear_child_tid, 1);  // ✅ 唤醒等待在此地址上的线程
    p->clear_child_tid = 0;
}
```

---

## 修复四：wait4 的一个微小 bug

### 问题：医生检查病人时顺便把病历改了

`wait4()` 函数在回收僵尸子进程的退出状态时，有一个微妙的 bug：

```c
// kernel/proc/proc.c wait4() —— 修复前
if (pp->state == ZOMBIE) {
    found_pid = pp->pid;
    pp->xstate = pp->xstate << 8;   // ❌ 直接修改了僵尸进程的退出状态！
    if (wstatus_addr != 0)
        copyout(p->pagetable, wstatus_addr, (char *)&pp->xstate, sizeof(pp->xstate));
    freeproc(pp);
    // ...
}
```

**为什么这是 bug？**

Linux 的 wait4 约定是：内核返回原始退出状态，用户空间用 `WEXITSTATUS(status) = (status >> 8) & 0xff` 来提取。当前代码在内核里提前做了移位，然后直接修改了 `pp->xstate`。

问题在于：如果同一个僵尸进程被 wait4 查看了两次（在竞争条件或不同代码路径下），第二次读到的 `pp->xstate` 已经是移位后的垃圾值了。更危险的是，内核中可能有其他代码路径依赖 `xstate` 的原始值。

```c
// 修复后
if (pp->state == ZOMBIE) {
    found_pid = pp->pid;
    // ✅ 用局部变量打包退出状态，不动 pp->xstate
    int status_word = (pp->xstate & 0xff) << 8;
    if (wstatus_addr != 0)
        copyout(p->pagetable, wstatus_addr, (char *)&status_word, sizeof(status_word));
    freeproc(pp);
    // ...
}
```

**改动要点：**
- `pp->xstate` 保持原样不修改
- 用栈上的局部变量 `status_word` 做移位并 copyout
- 符合 Linux 的 wait4 约定，不产生副作用

---

## 意外发现：那些已经存在的代码

翻阅文档 28 的路线图时，发现有好几个"需要实现"的模块**其实已经有了**：

| 功能 | 文档 28 状态 | 实际状态 | 位置 |
|------|------------|---------|------|
| futex_wait / futex_wake | 需要实现 | ✅ **已有完整实现** | `kernel/util/futex.c` |
| CLONE_VM / CLONE_FILES | 需要确认 | ✅ 已实现 | clone() |
| CLONE_THREAD (tgid/tid) | 部分 | ✅ 已实现 | clone() + allocproc |
| CLONE_SETTLS / CLONE_CHILD_CLEARTID | 需要确认 | ✅ 已实现 | clone() |
| sig_handlers refcount 字段 | 需要新增 | ✅ **结构体已有** | `include/signal.h` |
| sig_cleanup 引用计数逻辑 | 需要实现 | ✅ **已有** | `kernel/trap/signal.c` |
| freeproc pt_shared 判断 | 需要确认 | ✅ 已有 | `kernel/proc/proc.c` |

这意味着 **P2（CLONE_SIGHAND）和 P4（线程退出）的工作量比预期小得多**——基础设施就在那里，只需要补上缺失的几个连接点。

> 💡 **经验：** 实现新功能之前，先仔细翻一翻代码。你想要的，可能别人已经写了一半。

---

## 测试结果：96 个 PASS，但……

```
libctest-musl: 96 pass, 0 fail
```

| 指标 | 修复前 | 修复后 |
|------|--------|--------|
| 非 pthread 测试 | 82 PASS | 96 PASS |
| 卡死/panic | `panic: vfs_ext_lseek` 在 rewind_clear_error | 无 |
| 被阻塞测试 | ~14 个未执行 | 全部跑到 |

看起来是完胜？不——仔细看测试输出，有些"PASS"很可疑。

---

## 假 PASS 诊断：谁在说谎

### 测试框架怎么判断 PASS/FAIL？

```c
// user/init-rv.c 中的判断逻辑
wait(&st);                              // 等子进程退出
int exit_code = (st >> 8) & 0xff;       // 取 exit code
if (exit_code == 0) {
    printf("PASS %s\n", ...);           // exit(0) → PASS
} else {
    printf("FAIL %s [status %d]\n", ...);
}
```

**问题：它只看 exit code！** libc-test 的很多测试函数会用 `t_error()` 把失败信息打印到 stderr，但 `main()` 函数不一定会把这些内部失败汇总为非零 exit code。

结果：stderr 满天飞错误，但 exit(0)，框架照样判 PASS。

### 9 个假 PASS 逐一诊断

#### 🔴 类型 A：未知 syscall 返回 -1 导致 errno 混乱（影响 4 个测试）

这是本轮发现的最重要的 Bug。

**根因：** `kernel/syscall/syscall.c` 中，当用户程序调了还没实现的 syscall 时：

```c
// kernel/syscall/syscall.c
} else {
    printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;       // ❌ -1 → musl 解读为 errno=1=EPERM
}
```

**到用户态发生了什么？**
```
内核返回 -1
    ↓
musl 的 syscall 分发代码：if (a0 < 0) { errno = -a0; return -1; }
    ↓
errno = -(-1) = 1 = EPERM
    ↓
perror() 打印："Operation not permitted"
```

**应该是：**
```
内核返回 -38 (ENOSYS)
    ↓
errno = 38 = ENOSYS
    ↓
perror() 打印："Function not implemented"
```

**受影响的测试：**

| 测试 | 缺失的 syscall | 当前看到 | 应该看到 |
|------|--------------|---------|---------|
| **socket** | 198,200,201,202,203,204,206,207,208（8 个网络 syscall） | "Operation not permitted" (EPERM) | "Function not implemented" (ENOSYS) |
| **statvfs** | 43 (statfs) | "Operation not permitted" | "Function not implemented" |
| **daemon_failure** | 调用链中某处触发未实现 syscall | "got 1 [EPERM]" | "got 24 [EMFILE]" |
| **rlimit_open_files** | dup 耗尽或 rlimit syscall | "Operation not permitted" | "EMFILE" (Too many open files) |

**为什么 EPERM 比 ENOSYS 更糟糕？** 测试程序看到 EPERM（"操作不被允许"）可能会认为"权限不足，换另一种方式试试"，而看到 ENOSYS（"功能未实现"）就会直接跳过并报告 UNSUPPORTED。错误的 errno 导致测试程序走错了分支，产生混乱的结果。

> 📝 **这个 bug 说明：** 哪怕是最简单的"这个 syscall 还没实现"的场景，返回值的语义也极其重要。`-1` 不是一个中性的"出错啦"返回值——它是 errno=1=EPERM，有明确的语义。

#### 🟠 类型 B：内核功能缺失（5 个测试）

| 测试 | 症状 | 缺失的功能 |
|------|------|-----------|
| **stat** | `stat("/dev/null")` 找不到文件 | 文件系统里没有 `/dev/null` 设备节点 |
| **utime** | `futimens` 调用失败，文件时间戳都是 0 | `futimens`/`utimensat` syscall 未完整实现 |
| **ftello_unflushed_append** | 临时文件打不开，ftello 返回 -1 | `mkstemp` 创建临时文件的流程不完整 |
| **lseek_large** | `lseek(0x80000000)` 返回 -1 | lseek 返回值类型是 `int`（32位有符号），超过 2GB 的偏移量被当作负数处理 |
| **syscall_sign_extend** | `read` 返回了错误数量 | 系统调用参数传递过程中有符号扩展 bug |

### 真 vs 假 PASS 汇总

| 状态 | 数量 | 说明 |
|------|------|------|
| ✅ 真 PASS | 87 | 功能正确实现，测试干净通过 |
| ⚠️ 假 PASS（类型 A） | 4 | socket, statvfs, daemon_failure, rlimit_open_files —— 被 -1→EPERM 污染 |
| ⚠️ 假 PASS（类型 B） | 5 | stat, utime, ftello_unflushed_append, lseek_large, syscall_sign_extend —— 功能缺失 |
| ❌ 真 FAIL | 0 | exit code 全部为 0 |

---

## 待修复清单

### 🔴 优先级最高：`-1 → -ENOSYS`（一行改动）

**位置：** `kernel/syscall/syscall.c` 第 272 行

```c
// 修复前
p->trapframe->a0 = -1;
// 修复后
p->trapframe->a0 = -ENOSYS;
```

**预期效果：**
- 所有未实现 syscall 正确返回 ENOSYS
- socket 和 statvfs 不再看到 EPERM
- daemon_failure 和 rlimit_open_files 可能变成真 PASS（不再被 -1 污染 errno）

### 🟡 测试框架：不只看出门码

**方案 A（推荐）：** 用 libc-test 自带的 `runtest.exe` 替代当前的 fork/exec 循环。runtest.exe 会：
- 给每个子测试打印 `START`/`END` 标记
- 自动计时
- 带超时保护
- 更准确地判断 PASS/FAIL

**方案 B（快速）：** 在现有 `test_libctest()` 中同时检查 exit code 和 stderr 输出：
```c
// 捕获子进程的 stderr，如果有 "failed" 关键字就算失败
```

### 🟠 功能缺失（按难度排列）

| 功能 | 测试 | 估计难度 |
|------|------|---------|
| lseek 64-bit 偏移 | lseek_large | ★ |
| /dev/null 设备节点 | stat | ★★ |
| mkstemp 临时文件 | ftello_unflushed_append | ★★ |
| syscall 参数符号扩展 | syscall_sign_extend | ★★ |
| futimens/utimensat | utime | ★★★ |
| 网络栈 | socket | ★★★★★ |

---

## 动手指南：重现这些修改

### 文件地图

```
my-springos/
├── kernel/
│   ├── proc/
│   │   └── proc.c          ← allocproc/clone/exit/wait4 四个函数
│   ├── syscall/
│   │   └── syscall.c       ← 未知 syscall 的 -1 → -ENOSYS
│   ├── fs/
│   │   └── VFS_ext.c       ← lseek/read/write/fstat/ioctl panic → error
│   └── trap/
│       └── signal.c        ← sig_init refcount 初始化
├── include/
│   └── proc/proc.h         ← struct proc（tgid/tid 字段）
└── user/
    └── init-rv.c           ← test_libctest() 测试运行器
```

### 每处修改的验证方法

**VFS panic：** 运行 libc-test，确认 `rewind_clear_error` 不再 crash。

**CLONE_SIGHAND：** 在 clone() 中加 printf 打印 flags & CLONE_SIGHAND 的值：
```c
printf("[clone] CLONE_SIGHAND=%d refcount=%d\n",
       (flags & CLONE_SIGHAND) ? 1 : 0,
       p->sig_handlers->refcount);
```

**线程退出：** 在 exit() 中加 printf 打印 is_thread 和清理路径：
```c
printf("[exit] pid=%d tid=%d tgid=%d is_thread=%d\n",
       p->pid, p->tid, p->tgid, is_thread);
```

**wait4 xstate：** 在 wait4() 中加 printf 打印修改前后的 xstate：
```c
printf("[wait4] found pid=%d xstate=%d status_word=%d\n",
       found_pid, pp->xstate, status_word);
```

### 编译和运行

```bash
cd /home/my-springos
make -j4                         # 编译内核
# 然后启动 QEMU 跑测试（具体命令取决于你的测试脚本）
```

---

## 排查技巧

### 技巧 1：区分"内核 bug"和"测试框架 bug"

当测试输出出现矛盾（stderr 报错但框架判 PASS）时，先问自己：
1. 子进程的 **exit code** 是多少？（用 `wait(&st); printf("exit=%d\n", st>>8 & 0xff);`）
2. stderr 的错误信息是什么？
3. 这两者一致吗？

如果不一致，问题可能在测试框架，而不是内核。

### 技巧 2：errno 总是从内核返回值来

看到 `"Operation not permitted"` → errno=1=EPERM → 说明内核某处返回了 `-1`。
看到 `"Bad file descriptor"` → errno=9=EBADF → 说明内核返回了 `-9`。
看到 `"Function not implemented"` → errno=38=ENOSYS → 说明内核返回了 `-38`。

**根据 errno 反推内核返回值的技巧很实用。**

### 技巧 3：最小化测试

不用每次都跑全部 96 个测试。只跑一个：

```c
// user/init-rv.c 中
if (fork() == 0) {
    execve("/musl/entry-static.exe",
           (char*[]){"entry-static.exe", "lseek_large", NULL}, NULL);
}
wait(NULL);
```

### 技巧 4：用 GDB 设断点

```bash
# 终端 1: 启动 QEMU 带 GDB 支持
qemu-system-riscv64 ... -s -S &

# 终端 2: 连接 GDB
riscv64-linux-gnu-gdb kernel
(gdb) target remote :1234
(gdb) b vfs_ext_lseek     # 在 lseek 函数设断点
(gdb) b exit              # 在 exit 设断点
(gdb) c
```

---

> **这份文档的核心信息：**
>
> 1. **panic 永远可以避免** —— 总是优先考虑返回 error 而非崩溃
> 2. **测试通过了 ≠ 代码没问题** —— 假 PASS 比真 FAIL 更危险，因为它让你以为万事大吉
> 3. **一行错误的返回值可以污染整个系统** —— `-1 ≠ "出错了"`，它有明确语义（EPERM）
> 4. **先翻代码，再动手写** —— 你想要的，可能别人已经写了一半
