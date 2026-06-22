# 31 -- presentfailure 逐测试修复指南

> 基于最新报错日志 `presentfailure.md`，结合内核源码分析，为每个假 PASS 测试提供原理级别的根因诊断和详细修复方案。
>
> **风格参考：** xv6 lab 实验指导书——先讲"这个测试在测什么"，再讲"为什么你的内核让它失败了"，最后给"改哪里、怎么改、为什么这么改"。

---

## 目录

1. [最新测试结果速览](#1-最新测试结果速览)
2. [测试一：socket — 9 个网络 syscall 未实现](#2-测试一socket--9-个网络-syscall-未实现)
3. [测试二：stat — /dev/null 设备节点不存在](#3-测试二stat--devnull-设备节点不存在)
4. [测试三：utime — futimens 对 fd=-1 且 NULL 路径处理不当](#4-测试三utime--futimens-对-fd-1-且-null-路径处理不当)
5. [测试四：daemon_failure — 期望 EMFILE 却收到 EPERM](#5-测试四daemon_failure--期望-emfile-却收到-eperm)
6. [测试五：ftello_unflushed_append — mkstemp 临时文件创建失败](#6-测试五ftello_unflushed_append--mkstemp-临时文件创建失败)
7. [测试六：lseek_large — 32 位有符号 int 截断 64 位偏移量](#7-测试六lseek_large--32-位有符号-int-截断-64-位偏移量)
8. [测试七：rlimit_open_files — dup 超限返回 EPERM 而非 EMFILE](#8-测试七rlimit_open_files--dup-超限返回-eperm-而非-emfile)
9. [测试八：statvfs — syscall 43 (statfs) 未实现](#9-测试八statvfs--syscall-43-statfs-未实现)
10. [测试九：syscall_sign_extend — /dev/zero read 返回 0 而非填入零字节](#10-测试九syscall_sign_extend--devzero-read-返回-0-而非填入零字节)
11. [修复优先级与进攻路线](#11-修复优先级与进攻路线)

---

## 1. 最新测试结果速览

```
==== libctest-musl: 96 pass, 0 fail ====
```

**从竞赛角度看，96 pass / 0 fail 是满分。** 但在工程意义上，这 96 个"pass"中有 9 个实际上是**假 PASS**——测试框架标记为通过，但内核行为不正确。

| # | 测试名 | 分类 | 根因类别 | 难度 |
|---|--------|------|---------|------|
| 1 | socket | 🌐 网络 | 9 个 syscall 未注册 | ★★★★★ |
| 2 | stat | 📁 文件 | /dev/null 不在此文件系统中 | ★★ |
| 3 | utime | 📁 文件 | futimens→utimensat(NULL 路径) 未处理 | ★★★ |
| 4 | daemon_failure | 🔧 进程 | prlimit64 空实现 + 已实现 syscall 返回 -1 | ★★ |
| 5 | ftello_unflushed_append | 📁 文件 | mkstemp 临时文件创建失败 | ★★★ |
| 6 | lseek_large | 📁 文件 | offset 参数 32 位 int 溢出 | ★★ |
| 7 | rlimit_open_files | 🔧 进程 | fdalloc/dup 返回 -1 而非 -EMFILE | ★★ |
| 8 | statvfs | 📁 文件 | syscall 43 未注册（errno 已正确） | ★★★ |
| 9 | syscall_sign_extend | 🔧 进程 | /dev/zero read 返回 0 而非填充缓冲区 | ★★ |

---

## 2. 测试一：socket — 9 个网络 syscall 未实现

### 症状

```
24 entry-static.ex: unknown sys call 198
src/functional/socket.c:20: (s=socket(2, 2, 17))>=0 failed: errno = Function not implemented
24 entry-static.ex: unknown sys call 200
src/functional/socket.c:21: bind(s, ...) failed: errno = Function not implemented
...（共 9 个 syscall）
PASS socket（文件）（网络）
```

### 这个测试在测什么？

libc-test 的 `functional/socket.c` 创建一对 UNIX domain socket (AF_UNIX/SOCK_DGRAM)，通过本地回环在同一进程内收发数据。它验证的是 socket 子系统中最基本的功能：socket() → bind() → sendto() → recvfrom()，以及 socket() → bind() → listen() → connect() → accept()。

### 为什么失败了（原理级别）

这 9 个 syscall——socket(198), bind(200), listen(201), accept(202), connect(203), getsockname(204), sendto(206), recvfrom(207), setsockopt(208)——在 `kernel/syscall/syscall.c` 的 `syscalls[]` 表中**完全没有对应条目**。

RISC-V Linux ABI 中，网络 syscall 的编号定义来自 Linux 内核（不同于 x86），而 SpringOS 目前使用的是自定 syscall 编号体系（`include/syscall/syscall.h`），其中未定义任何网络相关调用号。

syscall 分派函数 `syscall()` 的处理逻辑：

```c
// kernel/syscall/syscall.c
void syscall(void) {
    num = p->trapframe->a7;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        uint64 ret = syscalls[num]();
        p->trapframe->a0 = ret;
    } else {
        printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
        p->trapframe->a0 = -ENOSYS;
    }
}
```

当用户态调用 `socket()` 时，musl libc 将 syscall 编号 198 写入 `a7` 寄存器并执行 `ecall`。内核检查 `syscalls[198]`，发现指针为 NULL（因为数组没有 198 号位置的条目），进入 `else` 分支，打印 "unknown sys call 198"，并返回 `-ENOSYS`（"Function not implemented"）。

**这个 errno 已经正确了**——上一轮修复把 `return -1` 改成了 `return -ENOSYS`，所以 musl 的 `socket()` 函数收到 `-38`（ENOSYS），正确地设置了 errno = "Function not implemented"。对比之前返回 `-1` → EPERM ("Operation not permitted")，这是一个巨大的进步。

**为什么测试标记为 PASS？** libc-test 的机制：如果测试中所有 assertion 失败但测试进程没有 crash（exit(0) 而非 exit(1) 或 abort），就标记 PASS。socket.c 测试的 assertion 使用了 `err()` 宏，而该宏在 assertion 失败时会调用 `t_error()` 增加失败计数，但不会 abort。问题在于测试框架（runtest.exe）的判断逻辑——它只看进程退出码是否为 0。

### 修复方案

这是一个长期任务，需要完整实现网络栈。最短路径是为 libc-test 实现**本地回环** socket（不需要真实网卡驱动）：

#### 步骤一：在 syscall.h 中添加 syscall 编号

```c
// include/syscall/syscall.h 新增
#define SYS_socket 198
#define SYS_bind 200
#define SYS_listen 201
#define SYS_accept 202
#define SYS_connect 203
#define SYS_getsockname 204
#define SYS_sendto 206
#define SYS_recvfrom 207
#define SYS_setsockopt 208
```

#### 步骤二：在 syscall.c 中注册处理函数

```c
// kernel/syscall/syscall.c 的 extern 声明块新增
extern uint64 sys_socket(void);
extern uint64 sys_bind(void);
// ... 其他

// syscalls[] 表新增
[SYS_socket] sys_socket,
[SYS_bind] sys_bind,
// ... 其他
```

#### 步骤三：实现内核 socket 层

新建 `kernel/fs/socket.c`。核心数据结构：

```c
// 简化版 struct socket：内核内存中的端点
struct socket {
    int type;        // SOCK_DGRAM 或 SOCK_STREAM
    int state;       // UNBOUND, BOUND, LISTENING, CONNECTED, CLOSED
    uint16 port;     // 绑定端口（本地回环用）
    struct socket *peer;  // 目标 socket 指针

    // 接收队列：一个简单的环形缓冲区
    char rbuf[4096];
    uint rhead, rtail;

    // 等待队列（简化：用 proc 的 sleep 机制）
    int wait_read;   // 是否有进程在等待读
};
```

每个 syscall 的最小实现逻辑：

| syscall | 核心逻辑 |
|---------|---------|
| `socket(domain, type, protocol)` | 分配 `struct socket`，创建 `struct file`（f_type = FD_SOCKET），返回 fd |
| `bind(fd, addr, addrlen)` | 解析 sockaddr，将 socket 的本地地址设为指定地址和端口 |
| `sendto(fd, buf, len, flags, dest_addr, addrlen)` | 从用户态拷贝数据到目标 socket 的接收缓冲区 |
| `recvfrom(fd, buf, len, flags, src_addr, addrlen)` | 从自己的接收缓冲区拷贝数据到用户态，填充源地址 |
| `listen(fd, backlog)` | 标记 socket 为 LISTENING 状态 |
| `connect(fd, addr, addrlen)` | 查找目标 socket，建立 peer 关联 |
| `accept(fd, addr, addrlen)` | 从连接队列中取出一个连接，创建新 fd |
| `getsockname(fd, addr, addrlen)` | 返回 socket 绑定的本地地址 |
| `setsockopt(fd, level, optname, optval, optlen)` | 设置 socket 选项（只需支持 SO_RCVTIMEO 等基本选项） |

> 📖 **参考文档：** 详见 `28-unimplemented-features-roadmap.md` 模块四。

### 预估工作量

1-2 天（约 400-600 行新代码）。

---

## 3. 测试二：stat — /dev/null 设备节点不存在

### 症状

```
src/functional/stat.c:27: stat("/dev/null",&st)==0 failed: errno = No such file or directory
PASS stat（文件）
```

### 这个测试在测什么？

libc-test `functional/stat.c` 第 27 行调用 `stat("/dev/null", &st)`，验证：
1. 返回值为 0（成功）
2. 文件类型为 S_IFCHR（字符设备）
3. st_dev 号（设备 ID）非零
4. st_nlink、st_uid、st_gid 等字段有合理值

### 为什么失败了（原理级别）

**调用链追踪：** 在 musl libc 中，`stat("/dev/null", &st)` 在 RISC-V 64 位 Linux ABI 下被翻译为：

```
fstatat(AT_FDCWD, "/dev/null", &st, 0)    →  syscall 79 (SYS_fstatat)
```

进入内核 `sys_fstatat()` 函数（`kernel/syscall/sysfile.c`）：

```c
uint64 sys_fstatat(void) {
    // 1. 获取用户态路径字符串
    argstr(1, pathname, MAXPATH);  // pathname = "/dev/null"

    // 2. 解析绝对路径
    get_absolute_path(pathname, base, absolute_path);
    // absolute_path = "/dev/null"

    // 3. 查找挂载的文件系统
    struct filesystem *fs = get_fs_from_path(absolute_path);
    // → 返回 "/" 的 EXT4 文件系统

    // 4. 在 EXT4 文件系统中 stat 这个路径
    int r = vfs_ext_stat(absolute_path, &kst);
    // → ext4_raw_inode_fill("/dev/null", ...) 失败
    // → 返回 -ENOENT（文件在 EXT4 磁盘上不存在！）
}
```

**根因：** `/dev/null` 只在 `sys_openat()` 中有特殊处理：

```c
// kernel/syscall/sysfile.c sys_openat()
if (!strcmp(path, "/dev/null")) {
    // 创建伪文件，f_type = 9（"零设备"类型）
    f->f_type = 9;
    f->f_pos = 0;
    return fd;
}
```

但 `sys_fstatat`（处理 stat 请求）**没有对应的路径拦截**，它老老实实去 EXT4 文件系统上找 `/dev/null`，自然找不到（SD 卡镜像里没有这个文件）。

**这就是"虚拟设备节点"问题的核心：** 内核在 open 路径做了特殊文件劫持，但忘记了在 stat/link/unlink/chmod 等其他文件操作路径上也做同样的劫持。类比 Linux 的 devtmpfs——所有 `/dev` 下的节点都是文件系统条目，所有 VFS 操作都能正确处理。

### 修复方案

#### 方案 A（快速）：在 sys_fstatat 中添加 /dev/null 的路径拦截

```c
// kernel/syscall/sysfile.c 的 sys_fstatat() 中，在 vfs_ext_stat 调用前加入：

// --- 新增：特殊设备路径拦截 ---
if (!strcmp(absolute_path, "/dev/null") || !strcmp(absolute_path, "/dev/console") ||
    !strcmp(absolute_path, "/dev/zero") || !strcmp(absolute_path, "/dev/rtc") ||
    !strcmp(absolute_path, "/dev/rtc0") || !strcmp(absolute_path, "/dev/misc/rtc") ||
    !strcmp(absolute_path, "/etc/localtime")) {

    // 构造一个合理的 stat 结构
    struct kstat kst;
    memset(&kst, 0, sizeof(kst));
    kst.st_dev = 1;        // 设备号
    kst.st_ino = 999;      // 唯一 inode 号
    kst.st_mode = S_IFCHR | 0666;  // 字符设备，可读写
    kst.st_nlink = 1;
    kst.st_blksize = 4096;

    if (copyout(myproc()->pagetable, ust, (char *)&kst, sizeof(struct stat)) < 0)
        return -1;
    return 0;
}
```

> ⚠️ 注意：libc-test stat 测试用的结构体是 `struct stat`（传统 stat），不是 `struct statx`。需要确认 `sys_fstatat` 到底输出哪种结构。

#### 方案 B（规范）：在 init 进程中使用 mknod 创建真实的 /dev/null 节点

```c
// user/init-rv.c 的初始化代码中新增：
mkdirat(AT_FDCWD, "/dev", 0755);
mknod("/dev/null", 0666 | S_IFCHR, MKDEV(1, 3));  // major=1, minor=3
mknod("/dev/zero", 0666 | S_IFCHR, MKDEV(1, 5));
mknod("/dev/console", 0600 | S_IFCHR, MKDEV(5, 1));
```

然后修改 `sys_mknod()`（已经存在！）让它实际在 EXT4 文件系统中创建 inode 条目。当前 `sys_mknod` 调用了 `vfs_ext_mknod`，需要确认其实现是否完整地将设备号存入了 inode。

**推荐用方案 A 快速修复 stat 测试。** 方案 B 需要确保 ext4 层的 mknod 正确存储设备号，并且后续所有文件操作路径（read/write/stat/chmod/unlink）都能正确处理这个 inode。

### 预估工作量

方案 A：30 分钟（添加约 25 行代码）。
方案 B：2-4 小时（需要验证 ext4 mknod 层实现完整性）。

---

## 4. 测试三：utime — futimens 对 fd=-1 且 NULL 路径处理不当

### 症状

```
src/functional/utime.c:23: futimens(-1, ((struct timespec[2]){{.tv_nsec=UTIME_OMIT},{.tv_nsec=UTIME_OMIT}}))==0 || errno==EBADF failed: Operation not permitted
src/functional/utime.c:47: st.st_atim.tv_sec >= t failed: 0
src/functional/utime.c:55: st.st_mtim.tv_sec >= t failed: 0
src/functional/utime.c:59: st.st_atim.tv_sec >= t failed: 0
src/functional/utime.c:60: st.st_mtim.tv_sec >= t failed: 0
src/functional/utime.c:65: st.st_atim.tv_sec == 1LL<<32 failed: 0
src/functional/utime.c:66: st.st_mtim.tv_sec == 1LL<<32 failed: 0
PASS utime（文件）
```

### 这个测试在测什么？

`functional/utime.c` 验证 `futimens()` 的完整行为：

1. **第 23 行：** `futimens(-1, ...)` 传入非法 fd=-1，且 times 设为 `UTIME_OMIT`（表示"不修改时间戳"）。POSIX 规定此时应返回 `EBADF`（坏的文件描述符），或者因为 `UTIME_OMIT` 而无操作地成功。

2. **第 47-60 行：** 先获取当前时间 t，然后调用 `futimens(fd, NULL)`（times 为 NULL 表示设为当前时间），最后 stat 检查 atime/mtime 是否 >= t。

3. **第 65-66 行：** 调用 `futimens(fd, times)` 将时间戳设为 `1LL<<32`（即 2^32 秒 ≈ 公元 2106 年），然后 stat 验证时间戳已被更新。

### 为什么失败了（原理级别）

**第一层问题：sys_utimensat 对 NULL path 处理不当**

在 musl libc 中，`futimens(fd, times)` 被翻译为：

```c
utimensat(fd, NULL, times, 0)    → syscall 88 (SYS_utimensat)
```

进入 `sys_utimensat()`：

```c
// kernel/syscall/sysfile.c
uint64 sys_utimensat(void) {
    argint(0, &dirfd);                      // dirfd = -1
    argaddr(2, &times_addr);                // times_addr = 用户态 times 指针
    argint(3, &flags);

    // ⚠️ 问题在这里！path=NULL 时 argstr 会失败
    if (argstr(1, path, MAXPATH) < 0) {
        return -1;   // ← 返回 -1 → errno = EPERM！
    }
    // ...
}
```

`argstr(1, path, MAXPATH)` 尝试从用户态地址 `NULL` 处拷贝字符串到内核缓冲区 `path`。由于 `NULL` 可能是不可读地址（或者 `fetchstr` 检测到地址为 0 直接返回 -1），`argstr` 返回 -1。函数提前 `return -1`，内核设置 `trapframe->a0 = -1`，musl 将其解释为 errno=1（EPERM）。

**正确的处理逻辑应该是：**

```c
if (path 是 NULL) {
    // 这是 futimens(fd, times) 的场景
    // 用 fd 查找对应的 file，操作其 inode
} else {
    // 这是 utimensat(dirfd, path, times, flags) 的场景
    // 用路径查找 inode
}
```

**第二层问题：即使路径非空，时间戳也未实际写入**

即使修复了 NULL path 的处理，`sys_utimensat` 当前返回 0（成功），但**没有调用 ext4 层去更新 inode 的 atime/mtime 字段**：

```c
// 当前代码：计算了 times 值但没有写入文件系统
times[0] = TIME2TIMESPEC(current_time);
times[1] = TIME2TIMESPEC(current_time);
return 0;  // "成功"，但什么都没改！
```

后续 stat 调用读到的 inode 时间戳仍然是创建时的默认值（0 或文件系统格式化的时间），所以 47-66 行的所有时间戳断言都会失败。

### 修复方案

#### 步骤一：在 utimensat 中区分 fd 路径和 path 路径

```c
// kernel/syscall/sysfile.c sys_utimensat() 重写核心逻辑：

uint64 sys_utimensat(void) {
    int dirfd;
    char path[MAXPATH];
    uint64 times_addr;
    int flags;
    struct file *f = NULL;
    struct inode *ip = NULL;

    argint(0, &dirfd);
    argaddr(2, &times_addr);
    argint(3, &flags);

    // 读取用户态 times 数组
    struct timespec times[2];
    if (times_addr != 0) {
        if (copyin(myproc()->pagetable, (char *)times, times_addr, sizeof(times)) < 0)
            return -EFAULT;
    }

    // --- 区分 fd 模式和 path 模式 ---
    int use_fd = 0;
    if (argstr(1, path, MAXPATH) < 0) {
        // path 为 NULL 或不可读 → 这是 futimens(fd, times) 调用
        // 尝试用 dirfd 作为文件描述符
        if (argfd(0, 0, &f) < 0)
            return -EBADF;   // fd 无效
        use_fd = 1;
    }

    if (use_fd) {
        // fd 模式：操作已打开的文件
        ip = f->f_ip;
    } else {
        // path 模式：通过路径查找文件
        // ... 解析路径，查找 inode ...
    }

    // --- 计算要设置的时间戳 ---
    struct timespec new_atime, new_mtime;

    if (times_addr == 0) {
        // times == NULL → 设为当前时间
        uint64 now = r_time();
        new_atime = TIME2TIMESPEC(now);
        new_mtime = TIME2TIMESPEC(now);
    } else {
        // 处理 UTIME_NOW 和 UTIME_OMIT
        // UTIME_NOW: tv_nsec == UTIME_NOW (0x3fffffff)
        // UTIME_OMIT: tv_nsec == UTIME_OMIT (0x3ffffffe)
        if (times[0].tv_nsec == UTIME_NOW) {
            new_atime = TIME2TIMESPEC(r_time());
        } else if (times[0].tv_nsec != UTIME_OMIT) {
            new_atime = times[0];
        }  // UTIME_OMIT: 保持原值（需要先读出当前值）

        if (times[1].tv_nsec == UTIME_NOW) {
            new_mtime = TIME2TIMESPEC(r_time());
        } else if (times[1].tv_nsec != UTIME_OMIT) {
            new_mtime = times[1];
        }
    }

    // --- 写入 ext4 inode ---
    int r = vfs_ext_set_times(ip, &new_atime, &new_mtime);
    if (r < 0) return r;

    return 0;
}
```

#### 步骤二：在 ext4 层实现时间戳写入

```c
// kernel/fs/VFS_ext.c（或直接通过 ext4 API）
int vfs_ext_set_times(struct file *f, const char *path,
                      struct timespec *atime, struct timespec *mtime) {
    struct ext4_inode_ref ref;
    int r = ext4_raw_inode_fill(path, &ref);
    if (r != EOK) return -ENOENT;

    // 更新 access time
    ref.inode->atime = (uint32) atime->tv_sec;

    // 更新 modification time
    ref.inode->mtime = (uint32) mtime->tv_sec;

    // 标记 inode 为脏，触发回写
    ext4_inode_set_dirty(&ref);
    // ... 释放引用 ...

    return 0;
}
```

> 🔑 **关键细节：** ext4 inode 中的 `atime` 和 `mtime` 字段是 `uint32`（传统 ext4），可能无法存储 `1LL<<32` 这样的大值。如果使用 ext4 的 64 位时间戳扩展（extra_isize >= 24 字节），则需要写入 `i_atime_extra` 字段。

### 预估工作量

3-4 小时（约 80-120 行新代码 + ext4 层改动）。

---

## 5. 测试四：daemon_failure — 期望 EMFILE 却收到 EPERM

### 症状

```
src/regression/daemon-failure.c:52: daemon should have failed with 24 [EMFILE] got 1 [Operation not permitted]
src/regression/daemon-failure.c:66: child exited with 1
src/regression/daemon-failure.c:73: child failed
PASS daemon_failure（进程系统）
```

### 这个测试在测什么？

`regression/daemon-failure.c` 测试 musl 的 `daemon()` 函数在资源受限时的失败路径：

1. 先用 `prlimit64(getpid(), RLIMIT_NOFILE, &rl, NULL)` 把文件描述符上限设为极低（如 2）。
   - **RLIMIT_NOFILE = 7**
2. 然后调用 `daemon(0, 0)`。
3. `daemon()` 的 musl 实现依次做：
   ```
   fork() → setsid() → fork() → chdir("/") → close(0..255) → open("/dev/null", O_RDWR) → dup2(fd, 0) → dup2(fd, 1) → dup2(fd, 2)
   ```
4. 在 RLIMIT_NOFILE=2 的情况下，当 `fork()` 后子进程 need 打开 `/dev/null` 时，进程已被允许的 fd 数用尽（stdin 0 + stdout 1 = 2），`open("/dev/null")` 应该失败并返回 **EMFILE(24)**。
5. 测试检查 `daemon()` 是否返回 -1 且 errno=EMFILE。

### 为什么失败了（原理级别）

**问题链条有三环：**

**环 1：sys_prlimit64 是空实现**

```c
// kernel/syscall/sysproc.c
uint64 sys_prlimit64(void) { return 0; }  // ← 什么都不做！
```

它没有：
- 读取用户态的 `struct rlimit` 参数
- 将 RLIMIT_NOFILE 值存储到进程结构体
- 在 `open/dup/fdalloc` 路径中检查是否超限

**环 2：daemon() 调用链中某 syscall 返回 -1 污染了 errno**

daemon() 的执行轨迹：
1. `fork()` → ✅ 成功（没有 rlimit，总能成功）
2. `setsid()` → ⚠️ 如果 `sys_setsid` 未实现或返回 -1 → EPERM

但更可能的是：`dup2(fd, 0)` 在 fd 用完后调用失败。由于 `sys_prlimit64` 没有真正限制 fd 数，`close(0..255)` 中的 `close()` closing 一个已关闭的 fd（操作本身会失败），或者 dup2 在某些边界条件下的返回值也是 -1。

**环 3：已实现的 syscall 内的 `return -1` 路径仍在返回 EPERM**

即使 rlimit 被正确实现，`fdalloc()` 在找不到可用 fd 时：

```c
// kernel/syscall/sysfile.c
int fdalloc(struct file *f) {
    for (fd = 0; fd < NOFILE; fd++) {
        if (p->ofile[fd] == 0) {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -1;   // ← 应该是 return -EMFILE
}
```

`sys_dup` 调用 `fdalloc(f)`，返回 -1 → 内核 `syscall()` 写入 `trapframe->a0 = -1` → musl 解释为 errno=EPERM。

### 修复方案

#### 步骤一：修复 fdalloc 返回值

```c
// kernel/syscall/sysfile.c
int fdalloc(struct file *f) {
    int fd;
    struct proc *p = myproc();
    for (fd = 0; fd < NOFILE; fd++) {
        if (p->ofile[fd] == 0) {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -EMFILE;  // ← 改这里：-1 → -EMFILE
}
```

同样检查 `fdalloc2`：

```c
int fdalloc2(struct file *f, int start) {
    int fd;
    struct proc *p = myproc();
    for (fd = start; fd < NOFILE; fd++) {
        if (p->ofile[fd] == 0) {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -EMFILE;  // ← 改这里
}
```

#### 步骤二：使 prlimit64 存储 RLIMIT_NOFILE

```c
// kernel/proc/proc.h 新增字段
struct proc {
    // ... 现有字段 ...
    uint64 rlimit_nofile;   // 软限制，默认 NOFILE (128)
};

// kernel/proc/proc.c allocproc() 初始化
p->rlimit_nofile = NOFILE;

// kernel/syscall/sysproc.c sys_prlimit64() 重写
uint64 sys_prlimit64(void) {
    int pid, resource;
    uint64 new_rlim_addr, old_rlim_addr;

    argint(0, &pid);
    argint(1, &resource);
    argaddr(2, &new_rlim_addr);
    argaddr(3, &old_rlim_addr);

    struct proc *p = myproc();

    struct rlimit new_rlim, old_rlim;
    memset(&new_rlim, 0, sizeof(new_rlim));

    // 读取当前限制
    if (resource == RLIMIT_NOFILE) {
        old_rlim.rlim_cur = p->rlimit_nofile;
        old_rlim.rlim_max = NOFILE;  // 硬限制
    }

    // 写入旧值到用户态（如果地址非空）
    if (old_rlim_addr != 0) {
        if (copyout(p->pagetable, old_rlim_addr, (char *)&old_rlim, sizeof(old_rlim)) < 0)
            return -EFAULT;
    }

    // 设置新限制
    if (new_rlim_addr != 0) {
        if (copyin(p->pagetable, (char *)&new_rlim, new_rlim_addr, sizeof(new_rlim)) < 0)
            return -EFAULT;
        if (resource == RLIMIT_NOFILE) {
            p->rlimit_nofile = new_rlim.rlim_cur;
            if (p->rlimit_nofile > NOFILE) p->rlimit_nofile = NOFILE;
        }
    }

    return 0;
}
```

#### 步骤三：在 fdalloc 中检查 rlimit

```c
int fdalloc(struct file *f) {
    int fd;
    struct proc *p = myproc();
    for (fd = 0; fd < p->rlimit_nofile; fd++) {   // ← 用 p->rlimit_nofile 替代 NOFILE
        if (p->ofile[fd] == 0) {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -EMFILE;
}
```

> ⚠️ **重要：** 如果修改 fdalloc 的循环上限，还需要同步修改 `sys_openat`、`sys_dup3` 等所有分配 fd 的路径，让它们也使用 `p->rlimit_nofile`。

### 预估工作量

1-2 小时（约 60 行代码改动，分布在 4 个文件）。

---

## 6. 测试五：ftello_unflushed_append — mkstemp 临时文件创建失败

### 症状

```
src/regression/ftello-unflushed-append.c:28: (fd = open(tmp, O_WRONLY)) > 2 failed (errno: No such file or directory)
src/regression/ftello-unflushed-append.c:32: (off = ftello(f)) != -1 failed (errno: Operation not permitted)
src/regression/ftello-unflushed-append.c:34: ftello is broken before flush: got -1, want 7
src/regression/ftello-unflushed-append.c:35: fflush(f) == 0 failed (errno: Bad file descriptor)
```

### 这个测试在测什么？

`regression/ftello-unflushed-append.c` 测试的是一个经典的 libc bug 回归——追加模式下的 `ftello()` 在 fflush 前是否报告正确的位置。

测试逻辑：
1. 用 `mkstemp()` 创建一个临时文件（`/tmp/ftello-XXXXXX`），获得 fd 和 FILE* 双打开
2. `fwrite("0123456", 1, 7, f)` 写入 7 字节
3. `ftello(f)` 应该在 fflush 前就返回 7（musl 的 ftello 会考虑缓冲区偏移）
4. `fflush(f)` 将缓冲区刷到文件
5. `ftello(f)` 应该在 fflush 后仍返回 7
6. 用 `open(tmp, O_WRONLY)` 通过路径再次打开同一个文件，验证 `write(fd, "!", 1)` 追加到了 7 偏移处（pos=8）

### 为什么失败了（原理级别）

**问题出在第一步：`mkstemp()` 创建文件失败。**

`mkstemp()` 的 musl 实现流程：
1. 获取进程 PID（`getpid()`）
2. 在模板中的 "XXXXXX" 位置填入随机字符生成文件名
3. 调用 `open(path, O_RDWR|O_CREAT|O_EXCL, 0600)` 创建文件
4. 如果 open 失败（文件已存在），随机化下一个模板值并重试

报错显示 `open(tmp, O_WRONLY)` 失败，errno=ENOENT。这意味着——mkstemp 生成的临时文件名对应的路径**根本不存在**在 ext4 文件系统中。

**根因分析：** 看 init-rv.c 第 437 行：
```c
mkdirat(AT_FDCWD, "/tmp", 0777);
```

`/tmp` 目录确实被创建了。但问题可能在于：

1. **工作目录不在 `/`：** 测试进程的 CWD 可能不在根目录。如果 CWD 是 `/testbin/`，那么 `mkstemp("/tmp/ftello-XXXXXX")` 生成的绝对路径是 `/tmp/ftello-XXXXXX`，它应该能工作。

2. **ext4 目录创建了但权限问题：** `/tmp` 的权限是 0777，应该没问题。

3. **更可能的根因：** 测试进程（entry-static.exe）通过 `fork+exec` 启动，`HOME=/tmp`。但 mkstemp 用绝对路径 `/tmp/ftello-XXXXXX` 创建文件，然后 test 用 `open(tmp, O_WRONLY)` 打开——如果 tmp 变量存储的是 `mkstemp` 返回的完整路径名，而该路径解析时出了问题……

实际上，更直接的根因是：**mkstemp 生成的临时文件在 ext4 文件系统上创建成功了，但测试中的第二步 open(tmp, O_WRONLY) 使用了不同的路径**。

让我再仔细看报错：
```
src/regression/ftello-unflushed-append.c:28: (fd = open(tmp, O_WRONLY)) > 2 failed (errno: No such file or directory)
```

`tmp` 来自 `mkstemp(template)` 的返回值——它会修改 template 字符串为实际创建的文件名。打开失败说明文件名被修改了但路径解析失败。

**最可能的根因：musl mkstemp 内部使用 `getpid()` 生成随机后缀，然后用 `open(O_CREAT|O_EXCL)` 创建。如果当前进程的 CWD 和 mkstemp 的路径基础不一致，或者 ext4 文件系统中路径解析有 race condition/bug，就会导致文件创建成功但后续通过路径打开失败。**

实际调试建议：在内核中打印 mkstemp 实际创建的路径、以及 open 尝试打开的路径，对比它们是否一致。

### 修复方案

#### 步骤一：诊断

在 `kernel/syscall/sysfile.c` 的 `sys_openat` 中添加临时调试打印：

```c
// 临时调试
if (strncmp(path, "/tmp/", 5) == 0) {
    printf("DEBUG openat: pid=%d cwd=%s path=%s flags=0x%x\n",
           myproc()->pid, myproc()->cwd.path, path, flags);
}
```

#### 步骤二（最可能需要的修复）：确保 CWD 正确

确认在 fork+exec 启动 entry-static.exe 时，子进程的 CWD 被正确设置为根目录 `/`。检查 `kernel/proc/exec.c` 中 `exec()` 是否保留了 CWD：

```c
// exec() 中应保留调用者的 CWD
// 当前实现是否正确？检查 p->cwd.path 在 exec 前后是否一致
```

#### 步骤三（备选）：确保 /tmp 目录在 SD 卡 ext4 镜像中存在

检查 SD 卡镜像构建过程（`mkfs` 目录），确保 `/tmp` 在初始镜像中已创建，或者 init 进程的 mkdirat 确实成功执行且目录持久化了。

### 预估工作量

30 分钟诊断 + 1 小时修复（取决于诊断结果）。

---

## 7. 测试六：lseek_large — 32 位有符号 int 截断 64 位偏移量

### 症状

```
src/regression/lseek-large.c:24: lseek(fd, 0x80000000, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0x80000001, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0xffffffff, SEEK_SET) got 0xffffffffffffffff
src/regression/lseek-large.c:24: lseek(fd, 0x100000001, SEEK_SET) got 0x1
src/regression/lseek-large.c:24: lseek(fd, 0x1ffffffff, SEEK_SET) got 0xffffffffffffffff
PASS lseek_large（文件）
```

### 这个测试在测什么？

`regression/lseek-large.c` 测试 lseek 是否能正确处理**超过 2GB 的文件偏移量**。测试传入的偏移值范围从 2^31（刚好溢出 32 位有符号 int）到 2^33 以上。

对于一个 64 位系统上的正确实现，所有 lseek 都应返回传入的偏移量（假设 SEEK_SET），因为文件系统支持稀疏文件。

### 为什么失败了（原理级别）

**这是一个经典的"类型宽度不够"问题。**

跟踪调用链：

```
用户态 lseek(fd, 0x80000000, SEEK_SET)
       ↓ ecall
内核 syscall() 函数
       ↓ 从 a1 寄存器读参数
argint(1, &offset)   // kernel/syscall/syscall.c 第 51 行
       ↓
```

**argint 的问题：**

```c
// kernel/syscall/syscall.c
static uint64 argraw(int n) {
    struct proc *p = myproc();
    switch (n) {
        case 0: return p->trapframe->a0;
        case 1: return p->trapframe->a1;   // ← a1 是 64 位寄存器
        // ...
    }
}

void argint(int n, int *ip) {
    *ip = argraw(n);   // ← uint64 强制转换为 int（32位）
}
```

`argraw(1)` 返回 `uint64` 值 `0x80000000`（即 2147483648）。然后 `argint` 把这个 64 位值赋给 `int *ip`：

```c
int offset;           // 32 位有符号
offset = 0x80000000;  // → -2147483648（溢出！）
```

**sys_lseek 的问题：**

```c
// kernel/syscall/sysfile.c
uint64 sys_lseek(void) {
    int offset;     // ← 32 位有符号！
    int whence;

    argint(1, &offset);   // offset = -2147483648（已损坏）
    argint(2, &whence);

    int result = vfs_ext_lseek(f, offset, whence);
    return result;        // int → uint64 符号扩展
}
```

**vfs_ext_lseek 的问题：**

```c
// kernel/fs/VFS_ext.c
int vfs_ext_lseek(struct file *f, int offset, int whence) {
    // offset 已经是负数 -2147483648
    // ext4_fseek 可能把负数解释为从末尾向前的偏移
    // 或者直接拒绝这个负数
    // → 返回负值 errno
    // sys_lseek 把这个负数通过 return result 返回
    // return result 中 int(-errno) 被符号扩展为 uint64(-1) = 0xffffffffffffffff
}
```

**数据流总结：**

```
用户态传64位值           argint截断为32位        sys_lseek用int接收        vfs_ext_lseek
0x0000000080000000  →   0x80000000(int=-2^31)  →  int offset(-)       →  返回-errno
                                                                           ↓
用户态收到                                        符号扩展               return result(int→uint64)
0xffffffffffffffff  ←──────────────────────────  ←────────────────────  ←
```

**为什么 0x100000001 返回 0x1？**
- `0x100000001` 的低 32 位是 `0x00000001`，argint 取低 32 位得到 `+1`。
- 传给 vfs_ext_lseek → ext4_fseek 成功 → 返回 1。
- `0x1` 不是负数，符号扩展为 `0x0000000000000001` → 正好是期望值（巧合）！

### 修复方案

#### 步骤一：将 offset 从 int 改为 int64_t (off_t)

```c
// kernel/syscall/sysfile.c sys_lseek() 修改前：
uint64 sys_lseek(void) {
    struct file *f;
    int offset;       // ← 改
    int whence;
    if (argfd(0, 0, &f) < 0) return -EBADF;  // ← 顺便修 errno
    argint(1, &offset);
    argint(2, &whence);
    int result = vfs_ext_lseek(f, offset, whence);
    return result;
}

// 修改后：
uint64 sys_lseek(void) {
    struct file *f;
    int64 offset;       // ← int64（等价于 off_t）
    int whence;
    if (argfd(0, 0, &f) < 0) return -EBADF;
    offset = (int64) argraw(1);   // ← 直接用 argraw 取 64 位值
    argint(2, &whence);
    int64 result = vfs_ext_lseek(f, offset, whence);
    return result;
}
```

#### 步骤二：修改 vfs_ext_lseek 签名

```c
// kernel/fs/VFS_ext.c 修改前：
int vfs_ext_lseek(struct file *f, int offset, int whence)

// 修改后：
int64 vfs_ext_lseek(struct file *f, int64 offset, int whence)
{
    if (f->f_type == FD_PIPE)   return -ESPIPE;
    if (f->f_type == FD_NONE || f->f_extfile == NULL) return -EBADF;

    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    // SEEK_END 特殊处理：负偏移从末尾向前
    if (whence == SEEK_END && offset < 0) {
        offset = -offset;
    }

    int r = ext4_fseek(file, (int64)offset, whence);
    if (r != EOK) {
        return -r;
    }
    f->f_pos = file->fpos;
    return (int64)f->f_pos;
}
```

#### 步骤三：更新头文件声明

```c
// 头文件中的 vfs_ext_lseek 声明也要同步改为 int64_t
int64 vfs_ext_lseek(struct file *f, int64 offset, int whence);
```

#### 步骤四：确保返回值正确

```c
// sys_lseek 的返回值处理也要正确
// 如果 result 是 0x80000000（正数），作为 uint64 返回给用户态
// 应该是 0x0000000080000000
// 如果 result 是 -errno（负数），符号扩展后为 uint64 全 1
// 这就是为什么之前看到 0xffffffffffffffff
```

> **注意：** ext4 的 `ext4_fseek` 接受 `int64` 偏移量吗？需要检查 `ext4_fseek` 的函数签名，可能需要同步调整 ext4 层的接口。

### 预估工作量

1-2 小时（约 20 行代码改动，需要全局搜索确保调用链一致）。

---

## 8. 测试七：rlimit_open_files — dup 超限返回 EPERM 而非 EMFILE

### 症状

```
src/regression/rlimit-open-files.c:28: dup(1) failed with Operation not permitted, wanted EMFILE
src/regression/rlimit-open-files.c:30: more fds are open than rlimit allows: fd=127, limit=42
PASS rlimit_open_files（进程系统）
```

### 这个测试在测什么？

`regression/rlimit-open-files.c` 测试 RLIMIT_NOFILE 是否正确限制进程可打开的文件描述符数：

1. 设置 RLIMIT_NOFILE = 42（通过 prlimit64）
2. 循环 dup(1) 直到失败
3. 检查失败时 errno 是否为 EMFILE（"Too many open files"）

### 为什么失败了（原理级别）

与 daemon_failure 测试**共享相同的根因**。

**输出信息已有线索：**
```
"more fds are open than rlimit allows: fd=127, limit=42"
```

`limit=42` 值被正确打印——说明 musl libc 自己从 `prlimit64` 系统调用中成功**读取**了限制值（musl 在用户态缓存了这个值？或者通过 `getrlimit` 读取）。但测试中循环 dup(1) 竟然到了 fd=127，远超限制 42——这说明**内核根本没有执行 rlimit 检查**。

**两个根因：**

**根因 1：sys_prlimit64 空实现**
```c
// kernel/syscall/sysproc.c
uint64 sys_prlimit64(void) { return 0; }
```
没有存储 RLIMIT_NOFILE 值到进程结构体。

**根因 2：fdalloc 返回 -1 而非 -EMFILE**
```c
// kernel/syscall/sysfile.c
int fdalloc(struct file *f) {
    // ... 循环查找可用 fd ...
    return -1;  // ← 应该是 return -EMFILE
}
```

当 128 个 fd 全部用完时，`fdalloc` 返回 -1，内核将其解释为 EPERM。测试期望的 EMFILE(24) 永远无法出现。

### 修复方案

**与测试四（daemon_failure）的修复完全相同**，这里只需复用同一个方案：

1. `fdalloc` / `fdalloc2` 返回 `-EMFILE` 而非 `-1`
2. `sys_prlimit64` 真正存储 rlimit 值
3. `fdalloc` 循环上限使用 `p->rlimit_nofile` 而非硬编码的 `NOFILE`

做完测试四的修复后，**rlimit_open_files 和 daemon_failure 两个测试应一起变为真 PASS**。

### 预估工作量

包含在测试四中（无需额外工作）。

---

## 9. 测试八：statvfs — syscall 43 (statfs) 未实现

### 症状

```
94 entry-static.ex: unknown sys call 43
src/regression/statvfs.c:14: statvfs("/") failed: Function not implemented
src/regression/statvfs.c:16: / has bogus f_bsize: 0
src/regression/statvfs.c:18: / has 0 blocks
src/regression/statvfs.c:26: / has 0 file nodes
src/regression/statvfs.c:34: / has bogus f_namemax: 0
PASS statvfs（文件）
```

### 这个测试在测什么？

`regression/statvfs.c` 调用 `statvfs("/", &buf)` 获取根文件系统的信息，验证：
- `f_bsize` > 0（块大小）
- `f_blocks` > 0（总块数）
- `f_files` > 0（总 inode 数）
- `f_namemax` > 0（最大文件名长度）

### 为什么失败了（原理级别）

在 RISC-V 64 位 musl libc 中，`statvfs` 被翻译为 `fstatfs` syscall（编号 43）。这个编号**不在 SpringOS 的 `syscalls[]` 表中**：

```
syscalls[] 表的最大有效索引远小于 43，且 syscalls[43] = NULL
→ syscall() 函数进入 "unknown sys call" 分支
→ 返回 -ENOSYS（errno 已经正确 ✅）
```

**但测试标记为 PASS 是为什么？** 当 `statvfs` 返回 -1 且 errno=ENOSYS 时，测试进入错误路径。musl test 框架检查到 errno 不为 0，调用 `t_error()` 记录失败。但由于测试不会 abort，它继续执行后续的 assertion：

```c
// statvfs 失败后，buf 是未初始化的（全 0）
// → f_bsize == 0, f_blocks == 0, f_files == 0, f_namemax == 0
// → 所有后续断言都失败
// → test 函数返回非零
// → runtest.exe 在某处转换为 PASS
```

**这已经算"状态正确"了**——errno 返回了正确的 ENOSYS。要真正通过测试，需要实现 `sys_fstatfs`。

### 修复方案

#### 步骤一：注册 syscall

```c
// include/syscall/syscall.h 新增
#define SYS_fstatfs 43

// kernel/syscall/syscall.c
extern uint64 sys_fstatfs(void);
// ...
[SYS_fstatfs] sys_fstatfs,
```

#### 步骤二：实现 sys_fstatfs

```c
// kernel/syscall/sysfile.c
uint64 sys_fstatfs(void) {
    int fd;
    uint64 buf_addr;
    struct file *f;

    argint(0, &fd);
    argaddr(1, &buf_addr);

    if (argfd(0, 0, &f) < 0) return -EBADF;

    struct filesystem *fs = get_fs_from_path(f->f_path);
    if (fs == NULL) return -ENODEV;

    if (fs->type == EXT4) {
        struct statfs buf;
        memset(&buf, 0, sizeof(buf));
        int r = vfs_ext_statfs(fs, &buf);
        if (r != EOK) return -EIO;

        if (copyout(myproc()->pagetable, buf_addr, (char *)&buf, sizeof(buf)) < 0)
            return -EFAULT;
        return 0;
    }

    return -ENODEV;  // 不支持的文件系统
}
```

> 📌 **注意：** 用户态 musl 的 `statvfs` 内部使用 `fstatfs`，但它的结构体是 `struct statfs`（BSD/Linux 兼容）。需要确认 `struct statfs` 的定义和 `vfs_ext_statfs` 填充的字段匹配。

> 🔧 **已经存在的基础设施：** `vfs_ext_statfs` 已经实现（`kernel/fs/VFS_ext.c`），直接从 ext4 superblock 读取 `f_bsize`、`f_blocks`、`f_bfree` 等信息。只需要把它接到 syscall 层即可。

### 预估工作量

1 小时（约 40 行代码，主要是 syscall 层的胶水代码）。

---

## 10. 测试九：syscall_sign_extend — /dev/zero read 返回 0 而非填入零字节

### 症状

```
src/regression/syscall-sign-extend.c:21: (r = syscall(SYS_read, fd, buf, 1)) == 1 failed: No error information
src/regression/syscall-sign-extend.c:23: read 1 instead of 0
PASS syscall_sign_extend（进程系统）
```

### 这个测试在测什么？

`regression/syscall-sign-extend.c` 测试 syscall 参数的符号扩展是否正确。核心逻辑：

```c
// 伪代码还原
int fd = open("/dev/zero", O_RDONLY);
char buf[1];
long r = syscall(SYS_read, fd, buf, 1);

// 断言 1: read 应返回 1（成功读取 1 字节）
if (r != 1) t_error("...");

// 断言 2: buf[0] 应该是 0x00（/dev/zero 的特性）
if (buf[0] != 0) t_error("read %d instead of 0\n", buf[0]);
```

测试通过直接调用 `syscall()`（绕过 libc 包装函数）来验证内核是否会对 32 位参数做错误的符号扩展到 64 位。如果参数被错误扩展，fd 可能变成无效值，或者 count 参数变成巨大值。

### 为什么失败了（原理级别）

**根因：`/dev/zero` 的 read 实现返回 0 字节（EOF）而非填入零值数据。**

在 `kernel/fs/vfs/file.c` 中 `fileread()` 函数：

```c
// kernel/fs/vfs/file.c fileread()
} else if (f->f_type == 9) {
    // /dev/null 类型：返回 0 字节（EOF）→ 行为正确 ✓
    char a = 0;
    copyout(myproc()->pagetable, addr, (char *)&a, sizeof(char));
    return 0;
} else if (f->f_type == 8) {
    // /dev/zero 类型：返回 0 字节（EOF）→ 行为错误 ✗
    return 0;
}
```

`/dev/null` (f_type=9)：
- 读取 → 返回 0 字节（EOF）✓ 正确
- 写入 → 数据被丢弃 ✓ 正确

`/dev/zero` (f_type=8)：
- 读取 → **应该**填充 `n` 个零字节到用户缓冲区，**返回 n**
- 写入 → 数据被丢弃 ✓ 正确
- **当前行为**：返回 0 字节（EOF） ✗ **错误**

**错误影响链条：**
1. 测试 open("/dev/zero") → 成功（f_type=8，fd 有效）
2. `syscall(SYS_read, fd, buf, 1)` → `sys_read(fd, buf, 1)` → `fileread(f, buf_addr, 1)` → 进入 f_type==8 分支 → `return 0`（表示读了 0 字节）
3. 测试第 21 行：`r == 0` 而非期望的 `1` → 断言失败
4. `buf[0]` 未被 write（fileread 返回 0），保持原始值（随机值），碰巧不是 0
5. 测试第 23 行：`buf[0] != 0` → 断言失败

### 修复方案

#### 修改 fileread 中 /dev/zero 的处理

```c
// kernel/fs/vfs/file.c fileread() 修改前：
} else if (f->f_type == 8) {
    return 0;
}

// 修改后：
} else if (f->f_type == 8) {
    // /dev/zero: 填充 n 个零字节到用户缓冲区，始终成功
    if (n <= 0) return 0;
    char zero = 0;
    for (int i = 0; i < n; i++) {
        if (copyout(myproc()->pagetable, addr + i, &zero, 1) < 0) {
            return -EFAULT;
        }
    }
    return n;
}
```

> ⚠️ **性能注意：** 这个逐字节 copyout 循环对小 n（如测试中的 n=1）没问题。如果需要支持大 n 的 `/dev/zero` 读取，应使用 kalloc 分配一个零填充的页面然后用 copyout 整页拷贝。

#### 同时修复 filewrite 中 /dev/zero 的处理（可选但推荐）

```c
// kernel/fs/vfs/file.c filewrite() 中
// 当前 /dev/zero 可能没有写处理，应添加：
} else if (f->f_type == 8) {
    // /dev/zero: 写入数据直接丢弃，总是成功
    return n;
}
```

### 预估工作量

15 分钟（约 10 行代码改动）。

---

## 11. 修复优先级与进攻路线

### 推荐顺序

按"投入产出比"降序排列：

```
第零步 ✅ 已完成（上一轮修复）
└── ✅ 未知 syscall 路径：-1 → -ENOSYS
    效果：socket/statvfs errno 从 EPERM 修正为 ENOSYS
    ↓

第一步（1.5 小时，一次修 3 个测试）
├── ★★ fdalloc/fdalloc2：return -1 → return -EMFILE
├── ★★ sys_prlimit64：真正存储 rlimit_nofile 到 proc
├── ★★ fdalloc 循环上限：NOFILE → p->rlimit_nofile
└── ★★ sys_lseek：int offset → int64 offset（修改调用链）
    预期修复：daemon_failure + rlimit_open_files + lseek_large → 3 个变真 PASS
    ↓

第二步（1 小时，一次修 2 个测试）
├── ★★ /dev/zero read：return 0 → 填充零字节 return n
└── ★★ stat /dev/null：sys_fstatat 路径添加设备节点拦截
    预期修复：syscall_sign_extend + stat → 2 个变真 PASS
    ↓

第三步（4 小时）
├── ★★★ utime/futimens：正确区分 fd 模式和 path 模式 + 写入 inode 时间戳
└── ★★★ ftello_unflushed_append：诊断 mkstemp 临时文件路径问题
    预期修复：utime + ftello_unflushed_append → 2 个变真 PASS
    ↓

第四步（2 小时）
└── ★★★ statvfs：注册 syscall 43 + 胶水代码调用已有 vfs_ext_statfs
    预期修复：statvfs → 1 个变真 PASS
    ↓

第五步（1-2 天）
└── ★★★★★ socket：本地回环网络栈完整实现
    预期修复：socket → 1 个变真 PASS
```

### 达成目标

```
当前状态：96 pass (含 9 个假 PASS)、0 fail
第一步后：96 pass (含 6 个假 PASS)、0 fail
第二步后：96 pass (含 4 个假 PASS)、0 fail
第三步后：96 pass (含 2 个假 PASS)、0 fail
第四步后：96 pass (含 1 个假 PASS)、0 fail  ← socket 是唯一假 PASS
第五步后：96 pass (0 个假 PASS)、0 fail    ← 全部真 PASS 🎉
```

### 完整后

```
96 个 libc-test 全部真 PASS
            ↓
    补上 pthread 测试（约 11 个）
            ↓
    libc-test 完整套件 ≈ 107 个测试全 PASS 🎉
```

---

> 📖 **相关文档：**
> - `26-libctest-kernel-fixes.md` — AT_PHDR / Clone / 信号等待修复历史
> - `28-unimplemented-features-roadmap.md` — 未实现功能的长期路线图（含网络栈设计）
> - `30-libctest-classification-false-pass.md` — 全测试分类和假 PASS 归因总表
> - `presentfailure.md` — 原始报错日志
>
> **一句话总结这份指南：** 9 个假 PASS 不是 9 个独立大问题。lseek 是类型问题、fdalloc 是返回值问题、/dev/zero 是 read 行为问题、utime 是参数传递问题、stat/statvfs 是路径拦截/胶水代码缺失、socket 是子系统缺失。前三步修完 7 个测试只需要一个下午。
