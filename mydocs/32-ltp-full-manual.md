# 32 -- LTP 测试套件完整手册

> 详细介绍 LTP (Linux Test Project) v20240524 测试套件的全部内容：分类、数量、测试目标、
> SpringOS 实现状态映射、以及竞赛评测体系中的角色。
>
> **数据来源：** `testsuits-for-oskernel/ltp-full-20240524/` + 当前 SpringOS 内核源码

---

## 目录

1. [LTP 概述](#1-ltp-概述)
2. [总体规模](#2-总体规模)
3. [SpringOS 实现状态总览](#3-springos-实现状态总览)
4. [测试分类详解](#4-测试分类详解)
5. [重点加分项：Syscalls 逐项映射](#5-重点加分项syscalls-逐项映射)
6. [竞赛环境下 LTP 如何评分](#6-竞赛环境下-ltp-如何评分)
7. [当前 SpringOS 对 LTP 的准备度](#7-当前-springos-对-ltp-的准备度)
8. [建议优先级策略](#8-建议优先级策略)

---

## 1. LTP 概述

Linux Test Project (LTP) 是 Linux 内核最权威的功能测试套件，由 SGI 发起，IBM、Red Hat、Oracle、SUSE 等共同维护。它包含数千个自动化测试用例，用于验证内核的可靠性、健壮性、稳定性。

### 在 OS 内核竞赛中的角色

在 OS 内核竞赛 (`oscomp`) 中，LTP 是**最高权重**的评测项之一。评判标准：

- **PASS**：测试通过（系统调用行为与 Linux 一致）→ 得分
- **FAIL**：测试失败（行为不一致或崩溃）→ 不得分
- **TCONF**：功能未实现（测试跳过）→ 不计分（不扣分也不得分）
- **BROK**：测试中断（资源不足等）→ 根据情况分析

### 编译方式

与其他竞赛套件不同，LTP 测试**不在内核编译时静态链接**。它们需要：
1. 用交叉工具链（riscv64-linux-gnu-gcc）在宿主机编译为独立 ELF
2. 存入 SD 卡 ext4 镜像
3. QEMU 中逐个启动执行，通过串口输出匹配 PASS/FAIL

---

## 2. 总体规模

```
LTP 全量统计
├── 总测试目录数：364（仅 syscalls 子类）
├── runtest/syscalls 条目：1411 个
├── 全部 runtest 类别：59 个
│   ├── syscalls      1,411  ← 核心，占全部测试 80%+
│   ├── controllers     347
│   ├── scsi_debug.part1 140
│   ├── ltp-aiodio.*    443  （4 个部分）
│   ├── net.*           731  （ipsec/tcp/udp/sctp/nfs...）
│   ├── mm               77
│   ├── fs               68
│   ├── syscalls-ipc     57
│   ├── ... 及其他
└── 物理文件：全部 testcases/kernel/syscalls/*.c
```

### 全部 runtest 类别（按测试数量降序）

| 类别 | 数量 | 涉及领域 | 竞赛相关性 |
|------|------|---------|-----------|
| **syscalls** | 1,411 | 系统调用功能验证 | ⭐⭐⭐ 核心 |
| controllers | 347 | cgroup 控制器 | ⭐ 低（单核无需） |
| scsi_debug.part1 | 140 | SCSI 设备模拟 | ❌ 无关 |
| ltp-aiodio.part1-4 | 303 | 异步 IO + Direct IO | ⭐ 低 |
| net.nfs | 113 | NFS 网络文件系统 | ❌ 无关 |
| net_stress.* | 484 | 网络压力测试 | ❌ 无关 |
| fs_bind | 95 | mount bind 语义 | ⭐ 低 |
| cve | 91 | 已知 CVE 回归 | ❌ 无关 |
| containers | 84 | 容器功能 | ❌ 无关 |
| mm | 77 | 内存管理 | ⭐⭐ 中 |
| fs | 68 | 文件系统功能 | ⭐⭐⭐ 核心 |
| net.features | 62 | 网络功能 | ❌ 无关 |
| syscalls-ipc | 57 | SysV IPC（信号量/共享内存/消息队列） | ⭐⭐ 中 |
| fs_readonly | 55 | 只读文件系统 | ⭐ 低 |
| ltp-aio-stress | 54 | AIO 压力测试 | ⭐ 低 |
| commands | 37 | 常用命令功能 | ⭐ 低 |
| kernel_misc | 18 | 内核杂项 | ⭐ 低 |
| smoketest | 15 | 冒烟测试 | ⭐ 低 |
| sched | 13 | 调度器 | ⭐⭐ 中 |
| ipc | 6 | IPC 基础 | ⭐⭐ 中 |
| 其他 | ~50 | misc | ⭐ / ❌ |

### 竞赛重点是前两行

```
1. syscalls (1,411)  ← 核心得分项
2. fs (68)           ← 文件系统加分
3. mm (77)           ← 内存管理加分
4. syscalls-ipc (57) ← IPC 加分
```

---

## 3. SpringOS 实现状态总览

当前 SpringOS 已在 `kernel/syscall/syscall.c` 中注册了 **93 个 syscall**，其中 **87 个** 在 LTP syscalls 测试目录中有对应的独立测试目录。

```
SpringOS syscalls:      93  ← 已注册
LTP 匹配目录:            87  ← 有独立测试套件
LTP 总 syscall 目录:   364  ← 完整 ABI
覆盖率:                 24%  ← 87/364
```

### SpringOS 已实现且有 LTP 测试的 syscall （87 个）

| Syscall | LTP测试数 | Syscall | LTP测试数 | Syscall | LTP测试数 |
|---------|----------|---------|----------|---------|----------|
| fcntl | 37 | mmap | 20 | ioctl | 24 |
| futex | 15 | pipe | 15 | open | 14 |
| statx | 12 | fork | 11 | kill | 11 |
| waitpid | 11 | clone | 9 | execve | 9 |
| mknod | 9 | splice | 9 | mount | 8 |
| sendfile | 8 | mremap | 6 | writev | 6 |
| write | 6 | setreuid | 7 | openat | 5 |
| fchmod | 6 | dup | 7 | dup2 | 7 |
| .... | ... | .... | ... | .... | ... |

> 📊 完整 87 个 syscall 和其 LTP 测试文件数见本文末尾附录。

### SpringOS 已实现但缺少 LTP 测试的 syscall（6 个）

这些 syscall 在 LTP 中没有独立测试目录，但可能在其他复合测试中覆盖：

```
clock_gettime → 无独立目录（功能通过 LTP clock_gettime* 测试覆盖）
clock_nanosleep → 同上
copy_file_range → 有独立目录！
pread64 → 无独立目录（pread 目录会覆盖）
prlimit64 → 无独立目录（getrlimit/setrlimit 目录间接覆盖）
syslog → 有独立目录！
shutdown → 无独立目录
sbrk → 无独立目录（brk 目录覆盖）
```

### SpringOS 完全未实现的 LTP syscall（~277 个）

涵盖六大功能域：

| 功能域 | 代表 syscall | LTP 测试数 |
|--------|------------|-----------|
| 🌐 网络栈 | socket, bind, listen, accept, connect, sendto, recvfrom, getsockname, getpeername, setsockopt, sendmsg, recvmsg | ~50 |
| 📂 扩展文件操作 | chmod, chown, creat, flock, lstat, lchown, rmdir, truncate, utimes, access, umask, link, symlink | ~60 |
| ⏱️ 定时器/时钟 | timer_create, timer_delete, timer_settime, timer_gettime, timerfd, clock_settime, clock_getres, clock_adjtime | ~25 |
| 🧵 线程/调度 | sched_setparam, sched_getscheduler, sched_setaffinity, setpgid, getsid, setsid | ~20 |
| 📡 信号扩展 | sigaction, sigsuspend, sigaltstack, sigpending, sigwait, sigwaitinfo, signalfd, rt_sigsuspend | ~15 |
| 🔐 权限/属性 | setresuid, setresgid, setfsuid, getresuid, getxattr, setxattr, listxattr | ~25 |
| 💾 IPC | msgget, msgsnd, msgrcv, msgctl, semget, semop, semctl, shmget, shmat, shmdt, shmctl | ~15 |
| 🔔 inotify/epoll | inotify_init, inotify_add_watch, epoll_create, epoll_ctl, epoll_wait, eventfd | ~20 |
| 📊 其他 | prctl, getrusage, getrlimit, setrlimit, sched_yield_ext, mallopt, sysctl, reboot | ~47 |

---

## 4. 测试分类详解

### 4.1 Syscalls（系统调用，1411 测试）

这是 LTP 最核心、最大的分类。每个 syscall 有 1-37 个不等的测试用例，按功能分层：

```
单个 syscall 的典型测试结构（以 fork 为例）:
fork01 → 基本功能：fork 返回子进程 PID
fork03 → 边界条件：fork 在内存压力下是否成功
fork04 → 错误路径：fork 后子进程正常退出
fork05 → 资源限制：大量 fork 直到 RLIMIT_NPROC
fork06 → 并发：1000 个并发 fork
fork07 → 信号交互：fork 时 pending 信号
fork08 → 文件描述符：fork 后 fd 共享
fork09 → 地址空间：fork 后 COW 正确
fork10 → 线程组：fork vs clone 语义
fork11 → 100 个进程树 fork
fork13 → 竞争条件
fork14 → SIGCHLD 送达
```

每个测试源码使用统一的 LTP 框架宏：

```c
#include "tst_test.h"

static void verify_fork(void) {
    pid_t pid = SAFE_FORK();
    if (!pid) {
        /* 子进程 */
        exit(42);
    }
    /* 父进程 */
    tst_res(TPASS, "fork succeeded");
}

static struct tst_test test = {
    .test_all = verify_fork,
    .forks_child = 1,
};
```

**输出格式：**
```
fork01    1  TPASS  :  correct child status returned 42
fork01    2  TPASS  :  child pid matches
```

### 4.2 FS（文件系统，68 测试）

测试 ext4（或任何挂载的文件系统）的核心操作：

| 测试 | 内容 |
|------|------|
| openfile | 打开文件的各种方式 |
| ftest, fstest | 文件读写和 stat |
| link, symlink | 硬链接和软链接 |
| chmod, chown | 权限和所有者变更 |
| quota | 磁盘配额（ext4 相关） |
| writetest, rwtest | 并发读写 |
| fsstress | 随机文件操作压力测试 |
| fs_flood | 大量文件创建/删除 |

### 4.3 MM（内存管理，77 测试）

测试虚拟内存和物理内存管理：

| 测试 | 内容 |
|------|------|
| mmap / munmap | 内存映射基础 |
| mprotect | 内存保护 |
| mlock / munlock | 锁定页面 |
| mremap | 重新映射 |
| msync | 同步映射到文件 |
| hugepage | 大页支持 |
| oom | OOM Killer 行为 |
| overcommit | 内存超分配 |
| ksm | 内核同页合并 |

### 4.4 Syscalls-IPC（SysV IPC，57 测试）

| IPC 类型 | 测试数 | syscall |
|----------|--------|---------|
| 消息队列 (msg) | ~20 | msgget, msgsnd, msgrcv, msgctl |
| 信号量 (sem) | ~15 | semget, semop, semctl |
| 共享内存 (shm) | ~15 | shmget, shmat, shmdt, shmctl |

### 4.5 其他类别（竞赛关联低）

| 类别 | 说明 | 竞赛必要性 |
|------|------|-----------|
| controllers / containers | cgroup/namespace | ❌ 单核内核不需要 |
| net.* | 网络协议栈全量 | ❌ 无网卡驱动 |
| ltp-aiodio | 异步 IO + Direct IO | ❌ 块设备层要求高 |
| scsi_debug | SCSI 设备模拟 | ❌ 无关 |
| cve | 已知漏洞回归 | ❌ 安全修复 |
| sched | 调度器高级特性 | ⭐ 低优先级 |

---

## 5. 重点加分项：Syscalls 逐项映射

### 5.1 完全可用于得分的 LTP 测试（SpringOS 已实现 + LTP 有测试）

分类标准：syscall 已注册 + LTP 有独立测试目录 + 功能预期可工作。

#### 🟢 高优先级（核心 syscall，测试数量大，容易 PASS）

| Syscall | LTP 测试数 | 状态 | 备注 |
|---------|-----------|------|------|
| **fcntl** | 37 | ⚠️ 部分实现 | F_DUPFD/F_GETFD/F_SETFD/F_GETFL/F_SETFL 需完整 |
| **mmap** | 20 | ✅ 基本可用 | MAP_ANON/MAP_PRIVATE/MAP_SHARED 已有实现 |
| **open** | 14 | ✅ 基本可用 | open + openat，O_CREAT/O_EXCL/O_TRUNC/O_APPEND |
| **pipe** | 15 | ✅ 基本可用 | pipe + pipe2 |
| **fork** | 11 | ✅ 基本可用 | fork + COW，SIGCHLD |
| **kill** | 11 | ⚠️ 部分 | 信号发送 + 权限检查 |
| **clone** | 9 | ⚠️ 部分 | CLONE_VM/CLONE_VFORK/CLONE_THREAD 已修复 |
| **execve** | 9 | ✅ 基本可用 | AT_PHDR 已修复 |
| **write** | 6 | ✅ 基本可用 | 基本文件写 |
| **read** | 4 | ✅ 基本可用 | 基本文件读 |
| **close** | 2 | ✅ 基本可用 | fd 关闭 |
| **dup / dup2** | 14 | ⚠️ 部分 | dup2 语义需确认（已打开 fd 的处理） |
| **waitpid** | 11 | ✅ 基本可用 | WNOHANG/WUNTRACED |
| **wait4** | 3 | ✅ 基本可用 | status + rusage |
| **exit / exit_group** | 3 | ✅ 基本可用 | 进程退出 |
| **getpid / getppid** | 4 | ✅ 基本可用 | 身份查询 |
| **getuid / geteuid / getgid** | 6 | ✅ 基本可用 | 用户/组 ID |
| **uname** | 3 | ✅ 基本可用 | 系统信息 |
| **times** | 2 | ⚠️ 部分 | 需返回合理值 |
| **brk** | 2 | ✅ 基本可用 | sbrk 已实现 |
| **mkdir / mkdirat** | 7 | ✅ 基本可用 | 目录创建 |
| **unlinkat** | 1 | ✅ 基本可用 | 文件删除 |
| **getcwd** | 4 | ✅ 基本可用 | 当前工作目录 |
| **getdents** | 2 | ✅ 基本可用 | 目录遍历 |
| **gettimeofday** | 2 | ✅ 基本可用 | 时间查询 |
| **nanosleep** | 3 | ✅ 基本可用 | 高精度睡眠 |
| **chdir** | 2 | ✅ 基本可用 | 工作目录切换 |
| **writev / readv** | 8 | ✅ 基本可用 | 分散/聚集 IO |
| **lseek** | 4 | ⚠️ 需修复 | 32 位 → 64 位修复待做（见 doc 31） |
| **fstat** | 2 | ⚠️ 部分 | stat 结构填充需完整 |
| **fstatat** | 1 | ⚠️ 部分 | /dev/null 路径拦截缺失 |
| **statx** | 12 | ⚠️ 部分 | 新 statx 结构，基本字段已填 |
| **utimensat** | 1 | ⚠️ 不完整 | futimens fd 模式未处理 + 时间戳未写入 |
| **mknod** | 9 | ⚠️ 部分 | 需 ext4 层支持设备号存储 |
| **mount / umount2** | 10 | ⚠️ 部分 | 简单 mount 可用，option 不全 |
| **ioctl** | 24 | ⚠️ 极低 | 大部分 ioctl cmd 未实现 |
| **futex** | 15 | ⚠️ 基础 | 仅支持基本 FUTEX_WAIT/FUTEX_WAKE |
| **mprotect** | 5 | ⚠️ 部分 | 页保护标志需完整 |
| **madvise** | 10 | ⚠️ 低 | 大部分 advice 未实现 |

#### 🟡 中优先级（已注册但 LTP 测试要求较高）

| Syscall | LTP 测试数 | 状态 | 备注 |
|---------|-----------|------|------|
| sendfile | 8 | ⚠️ 部分 | 管道到文件/文件到管道 |
| splice | 9 | ⚠️ 部分 | 同 sendfile |
| ppoll | 1 | ⚠️ 部分 | poll + 信号掩码 |
| rt_sigaction | 3 | ⚠️ 部分 | SA_RESTART/SA_SIGINFO |
| rt_sigprocmask | 2 | ✅ 基本可用 | 信号掩码 |
| rt_sigtimedwait | 1 | ✅ 基本可用 | 信号等待（已实现） |
| setuid / setgid | 6 | ⚠️ 部分 | 权限模型需完整 |
| setreuid / setregid | 11 | ⚠️ 部分 | 同上 |
| faccessat | 2 | ⚠️ 部分 | 访问权限检查 |
| fchmod / fchmodat | 8 | ⚠️ 部分 | 需 ext4 层支持 mode 写入 |
| fchown / fchownat | 7 | ⚠️ 部分 | owner 变更（可 stub 返回 0） |
| fdatasync / fsync | 7 | ⚠️ 部分 | ext4 缓存刷新已接上 |
| ftruncate | 3 | ⚠️ 未完整 | 文件截断需 ext4 支持 |
| linkat | 2 | ⚠️ 部分 | 硬链接 |
| renameat2 | 2 | ⚠️ 部分 | 文件重命名 |
| symlinkat | 1 | ⚠️ 部分 | 软链接 |
| readlinkat | 2 | ⚠️ 部分 | 读软链接 |
| getrandom | 5 | ⚠️ 部分 | 随机数生成器 |
| set_tid_address | 1 | ✅ 基本可用 | 已实现 |
| set_robust_list | 1 | ✅ stub | 返回 0 即可 |
| tgkill / tkill | 5 | ⚠️ 部分 | 线程信号 |
| sysinfo | 3 | ⚠️ 部分 | 系统信息结构 |
| syslog | 2 | ⚠️ 未完整 | syslog 读写 |
| copy_file_range | 1 | ⚠️ 低 | 文件间复制 |
| sched_yield | 1 | ✅ 基本可用 | yield |
| getpgid | 0 | ⚠️ 无 LTP | 无独立测试目录 |

#### 🔴 已注册但质量不足以 PASS 的

| Syscall | 问题 |
|---------|------|
| ioctl | 24 个测试，绝大部分 ioctl cmd 返回 -1 → EPERM |
| futex | 15 个测试，仅 WAIT/WAKE，无 FUTEX_REQUEUE/FUTEX_CMP_REQUEUE |
| madvise | 10 个测试，MADV_DONTNEED/MADV_NORMAL 未实现 |
| mknod | 9 个测试，需要 ext4 正确存储设备号 |
| mmap | 20 个测试，MAP_SHARED 文件映射、MAP_GROWSDOWN 未实现 |
| splice/sendfile | 17 个测试，零拷贝逻辑可能有边界 bug |

### 5.2 当前无法得分的 LTP 测试（SpringOS 未实现 syscall）

这些测试会因 `unknown sys call` 返回 ENOSYS，测试框架判定为 TCONF 或不通过。简要按重要性分组：

| 重要性 | 功能域 | 典型 syscall | 估计测试数 |
|--------|--------|------------|-----------|
| 🔴 完全阻塞 | 网络栈全套 | socket, bind, listen, accept, connect, sendto, recvfrom, getsockname, getpeername, setsockopt, sendmsg, recvmsg, socketpair | ~50 |
| 🟠 中阻塞 | 扩展文件操作 | chmod, chown, creat, flock, lstat, lchown, rmdir, truncate, utimes, access, umask | ~60 |
| 🟠 中阻塞 | 信号扩展 | sigaction, sigsuspend, sigaltstack, sigpending, sigwait, sigwaitinfo, signalfd | ~15 |
| 🟠 中阻塞 | 调度/线程 | sched_setparam, sched_getscheduler, sched_setaffinity, setpgid, getsid, setsid | ~20 |
| 🟡 低阻塞 | SysV IPC | msgget, msgsnd, msgrcv, msgctl, semget, semop, semctl, shmget, shmat, shmdt, shmctl | ~15 |
| 🟡 低阻塞 | 权限/属性扩展 | setresuid, setresgid, getresuid, getxattr, setxattr, listxattr | ~25 |
| 🟡 低阻塞 | epoll/eventfd/inotify | epoll_create, epoll_ctl, epoll_wait, eventfd, inotify_init, inotify_add_watch | ~25 |
| 🟢 可跳过 | 高级特性 | bpf, ptrace, perf_event_open, kcmp, keyctl, cgroup, fanotify, io_uring, prctl, memfd_create | ~77 |

---

## 6. 竞赛环境下 LTP 如何评分

### 6.1 评分机制

竞赛使用 `scripts/ltp/ltp_testcode.sh` 脚本执行测试：

```bash
for file in ltp/testcases/bin/*; do
    echo "RUN LTP CASE $(basename "$file")"
    "$file"              # 直接执行二进制文件
    ret=$?
    echo "FAIL LTP CASE $(basename "$file") : $ret"
done
```

**关键事实：**
- 退出码 0 = PASS
- 退出码非 0 = FAIL
- judge 脚本会解析 QEMU 串口输出，匹配 `TPASS`/`TFAIL`/`TCONF` 标签
- 但实际的竞赛 judge 可能只看退出码

### 6.2 得分计算公式

```
得分 = (PASS 数 / (总测试数 - TCONF 数)) × 总分配分
```

where:
- TCONF = 测试因环境不支持而跳过（如缺少 root 权限、内核版本太旧）
- 对于 SpringOS，未实现 syscall 的测试如果能正确返回 ENOSYS → TCONF → **不计分也不扣分**
- 对于已实现 syscall 但行为不正确的测试 → FAIL → **扣分**

### 6.3 当前 SpringOS 的策略优势

SpringOS 的 `syscall()` 分派函数中未知 syscall 返回 `-ENOSYS` 而非 `-1`（EPERM），这使得：

- 用户态 LTP 测试能正确检测到 "功能未实现"
- musl/libc 包装函数设置 errno=ENOSYS
- LTP 测试框架将其标记为 TCONF（不适用）→ 不计入总分分母
- **不会**因为错误的 errno 而被判 FAIL

**这是 31.md 文档中 -ENOSYS 修复最重要的竞赛收益！**

---

## 7. 当前 SpringOS 对 LTP 的准备度

### 7.1 定量分析

```
SpringOS 已实现有 LTP 测试的 syscall:  87 个
SpringOS 实现的 syscall 对应的 LTP 测试:  ~450 个（估算）
LTP syscalls 总测试:                    1,411 个
预期能 PASS 的测试:                     ~150-200 个（初次运行）
预期 TCONF 的测试:                      ~1,200 个（未实现功能 → ENOSYS）
预期 FAIL 的测试:                       ~50-100 个（已实现但行为不正确）
```

### 7.2 预期得分估算

```
分母 = 1411 - TCONF数 = 1411 - 1200 = 211
分子 = PASS数 = 150
得分率 = 150/211 ≈ 71%
```

> ⚠️ **这个估算假设所有未实现 syscall 都返回 ENOSYS 并被判定为 TCONF。** 实际情况取决于：
> 1. LTP 框架是否将 ENOSYS 识别为 TCONF（需要看具体测试的 tst_test 配置）
> 2. 部分测试如果遇到 bug 导致 crash（SIGSEGV/SIGABRT）会被判 FAIL 而非 TCONF

### 7.3 最需要优先修复的已实现但会 FAIL 的 syscall

基于 31.md 的分析，这些已注册的 syscall 有已知 bug：

| Syscall | LTP 测试数 | 已知问题 | 修复难度 |
|---------|-----------|---------|---------|
| lseek | 4 | 32 位 offset 截断 | ★★ |
| fstat/fstatat | 3 | /dev/null 不存在于文件系统 | ★★ |
| utimensat | 1 | futimens 路径未处理 | ★★★ |
| fcntl | 37 | 部分 cmd 未实现（F_GETLK/F_SETLK/F_SETLKW） | ★★★ |
| ftruncate | 3 | 文件截断未完整实现 | ★★★ |
| dup/dup2 | 14 | fdalloc 返回 -1 而非 -EMFILE | ★★ |
| mmap | 20 | MAP_SHARED/MAP_GROWSDOWN 等模式 | ★★★ |
| madvise | 10 | 大部分 advice 返回 0 但不做操作 | ★★ |
| mknod | 9 | 需要 ext4 层存储 dev 号 | ★★★ |
| open/openat | 19 | O_SYNC/O_DSYNC/O_DIRECT 等 flag | ★★ |
| ioctl | 24 | 绝大部分 cmd 返回 -1 | ★★★ |
| futex | 15 | 缺少 requeue/优先级继承 | ★★★★ |

---

## 8. 建议优先级策略

### 阶段一：防扣分（1-2 天）

目标：已实现的 87 个 syscall 对应的 LTP 测试中，减少 FAIL 转为 PASS。

```
优先级 1：修 libc-test 中已知的 bug → LTP 也受益
  ├── lseek 64 位修复 (★★) → lseek 4 个 LTP 测试全部 PASS
  ├── fdalloc EMFILE 修复 (★★) → dup/dup2 14 个 LTP 测试受益
  ├── /dev/zero read 修复 (★★) → read/open 相关受益
  └── fstatat /dev/null 拦截 (★★) → fstatat 1 个 PASS

优先级 2：已注册 syscall 中返回 -1 的路径
  ├── ioctl：添加 FIONREAD/TCGETS 等常见 cmd 的 stub
  ├── fcntl：F_DUPFD/F_GETFD/F_SETFD/F_GETFL 的核心路径
  ├── futex：至少 FUTEX_WAIT/FUTEX_WAKE 正确
  └── madvise：MADV_NORMAL/MADV_DONTNEED 简单实现
```

### 阶段二：争取得分（3-5 天）

目标：实现新的 syscall 以增加 PASS 数（减小分母影响）。

```
优先级排序（按得分效益）：
1. rmdir, access, creat, truncate, umask, lstat（~30 个测试）→ ★★
   扩展文件操作，实现简单，测试通过率高
2. chmod, chown（~10 个测试）→ ★★
   权限变更，可 stub 为 "允许所有"
3. getrlimit, setrlimit（~9 个测试）→ ★★
   资源限制，配合 prlimit64 已有基础
4. sigaction, sigsuspend（~18 个测试）→ ★★★
   信号处理的完整实现
```

### 阶段三：冲击高分（1-2 周）

目标：实现复杂子系统和大量测试。

```
1. epoll 系列（~20 个测试）→ ★★★★   Linux 核心 IO 多路复用
2. SysV IPC 全套（~45 个测试）→ ★★★★ 消息队列/信号量/共享内存
3. 网络栈基础（~50 个测试）→ ★★★★★ 本地回环 socket
4. eventfd, signalfd, timerfd（~15 个测试）→ ★★★★ 新式 fd 通知机制
```

### 不推荐投入的

```
- bpf, perf_event_open, ptrace, kcmp → 内核调试/追踪功能
- cgroup, namespace, containers → 容器化
- io_uring → 高级异步 IO
- fanotify, inotify → 文件事件监控（epoll 之前不值得）
- keyctl, add_key, request_key → 内核密钥环
```

---

## 附录：SpringOS 已实现 syscall 与 LTP 测试数的完整映射

| # | Syscall | LTP C 文件数 | 预计可 PASS | 主要障碍 |
|---|---------|-------------|-----------|---------|
| 1 | brk | 2 | 2 | 无 |
| 2 | chdir | 2 | 2 | 无 |
| 3 | clone | 9 | 5 | CLONE_VM/CLONE_FILES 信号交互 |
| 4 | close | 2 | 2 | 无 |
| 5 | copy_file_range | 1 | 0 | 未完整实现 |
| 6 | dup | 7 | 7 | fdalloc EMFILE 修复后 |
| 7 | dup2 | 7 | 7 | 同上 |
| 8 | dup3 | 2 | 2 | 同上 |
| 9 | execve | 9 | 6 | ENOEXEC/脚本执行 |
| 10 | exit | 2 | 2 | 无 |
| 11 | exit_group | 1 | 1 | 无 |
| 12 | faccessat | 2 | 2 | stub 返回 0 |
| 13 | fchmod | 6 | 3 | ext4 mode_set 需完整 |
| 14 | fchmodat | 2 | 1 | 同上 |
| 15 | fchown | 5 | 3 | stub 返回 0 |
| 16 | fchownat | 2 | 1 | 同上 |
| 17 | fcntl | 37 | 10 | 只实现 F_DUPFD/F_GETFD/F_SETFD/F_GETFL/F_SETFL |
| 18 | fdatasync | 3 | 3 | ext4 flush 已接 |
| 19 | fork | 11 | 8 | 边界 case |
| 20 | fstat | 2 | 2 | 需确保 st_dev/st_ino/st_mode 填充正确 |
| 21 | fstatat | 1 | 1 | /dev/null 路径拦截后 |
| 22 | fsync | 4 | 4 | ext4 flush 已接 |
| 23 | ftruncate | 3 | 0 | ext4 截断未完整实现 |
| 24 | futex | 15 | 2 | 仅 FUTEX_WAIT/Wake |
| 25 | getcwd | 4 | 4 | 基本可用 |
| 26 | getdents | 2 | 2 | 基本可用 |
| 27 | getegid | 2 | 2 | 无 |
| 28 | geteuid | 2 | 2 | 无 |
| 29 | getgid | 2 | 2 | 无 |
| 30 | getpgid | 0 | 0 | 无独立 LTP 测试 |
| 31 | getpid | 2 | 2 | 无 |
| 32 | getppid | 2 | 2 | 无 |
| 33 | getrandom | 5 | 3 | 需真实随机源 |
| 34 | gettid | 2 | 2 | 无 |
| 35 | gettimeofday | 2 | 2 | 无 |
| 36 | getuid | 2 | 2 | 无 |
| 37 | ioctl | 24 | 2 | 大部分 cmd 未实现 |
| 38 | kill | 11 | 8 | 权限检查 |
| 39 | linkat | 2 | 2 | ext4 link 已接 |
| 40 | lseek | 4 | 4 | 64 位修复后 |
| 41 | madvise | 10 | 3 | 大部分 advice stub |
| 42 | mkdir | 5 | 5 | 基本可用 |
| 43 | mkdirat | 2 | 2 | 基本可用 |
| 44 | mknod | 9 | 2 | ext4 设备号存储 |
| 45 | mmap | 20 | 8 | MAP_SHARED/MAP_GROWSDOWN 等 |
| 46 | mount | 8 | 4 | 基本 mount 可用 |
| 47 | mprotect | 5 | 3 | 页保护标志 |
| 48 | mremap | 6 | 3 | 重映射 + 扩容 |
| 49 | munmap | 3 | 3 | 基本可用 |
| 50 | nanosleep | 3 | 3 | 基本可用 |
| 51 | open | 14 | 10 | 部分 flag 未实现 |
| 52 | openat | 5 | 4 | 同上 |
| 53 | pipe | 15 | 12 | 基本可用 |
| 54 | pipe2 | 4 | 4 | 基本可用 |
| 55 | ppoll | 1 | 1 | poll + sigmask |
| 56 | pread | 2 | 2 | pread64 已实现 |
| 57 | read | 4 | 3 | /dev/zero 修复后 |
| 58 | readlinkat | 2 | 1 | 软链接读取 |
| 59 | readv | 2 | 2 | 基本可用 |
| 60 | renameat2 | 2 | 2 | 基本可用 |
| 61 | rt_sigaction | 3 | 2 | SA_RESTART/SA_SIGINFO |
| 62 | rt_sigprocmask | 2 | 2 | 基本可用 |
| 63 | rt_sigtimedwait | 1 | 1 | 已实现 |
| 64 | sched_yield | 1 | 1 | 无 |
| 65 | sendfile | 8 | 4 | 管道与文件互传 |
| 66 | setgid | 3 | 2 | 权限模型 |
| 67 | setregid | 4 | 2 | 同上 |
| 68 | setreuid | 7 | 3 | 同上 |
| 69 | set_robust_list | 1 | 1 | stub 返回 0 |
| 70 | set_tid_address | 1 | 1 | 已实现 |
| 71 | setuid | 3 | 2 | 权限模型 |
| 72 | splice | 9 | 4 | 管道零拷贝 |
| 73 | statx | 12 | 6 | statx 结构字段完整性 |
| 74 | symlinkat | 1 | 0 | 软链接创建需 ext4 支持 |
| 75 | sysinfo | 3 | 2 | sysinfo 结构填充 |
| 76 | syslog | 2 | 1 | syslog 读写部分 |
| 77 | tgkill | 3 | 2 | 线程信号 |
| 78 | times | 2 | 2 | 返回合理值即可 |
| 79 | tkill | 2 | 2 | 线程信号 |
| 80 | umount2 | 2 | 2 | 基本卸载 |
| 81 | uname | 3 | 3 | 无 |
| 82 | unlinkat | 1 | 1 | 基本可用 |
| 83 | utimensat | 1 | 0 | futimens 路径 + 时间戳写入 |
| 84 | wait | 2 | 2 | 基本可用 |
| 85 | wait4 | 3 | 3 | 基本可用 |
| 86 | waitpid | 11 | 8 | WNOHANG/WUNTRACED |
| 87 | write | 6 | 5 | 基本可用 |
| 88 | writev | 6 | 5 | 基本可用 |

**预计初始 LTP 得分：约 200 / (450 - 150) ≈ 67%**

其中 450 是通过 LTP 测试的系统调用对应的测试数量，"约 150" 是其大部分测试将返回 TCONF 而非 FAIL。

---

> 📖 **相关文档：**
> - `31-presentfailure-fix-guide.md` — libc-test 当前假 PASS 的逐项修复指南
> - `30-libctest-classification-false-pass.md` — libc-test 全量分类
> - `28-unimplemented-features-roadmap.md` — 未实现功能长期路线图
> - `08-score-analysis.md` — 竞赛得分分析
> - `11-prelim-code-slim.md` — 初赛代码精简策略
>
> **核心结论：** LTP 是后 libc-test 阶段的重头戏。SpringOS 已有 87 个 syscall 实现，对应约 450 个 LTP 测试，预计初次运行可 PASS 约 200 个。后续通过修复已知 bug（len=64位、fdalloc EMFILE 等）和实现高收益的 syscall（rmdir/access/creat 等），可快速提升到 300+ 个 PASS。
