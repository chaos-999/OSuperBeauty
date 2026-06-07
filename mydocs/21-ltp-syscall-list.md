# 📋 LTP 测试集 syscall 完整清单

> 📅 生成时间：`2026-06-07 11:25 (GMT+8)`
>
> 目标：列出 LTP（Linux Test Project）`syscalls` 测试组中所有测试 syscall、对应测试目录、测试用例数量，以及与 SpringOS 当前实现的对照关系。

---

## 零、LTP 测试集概况

LTP 是 Linux 内核的标准回归测试套件。OSCOMP 比赛从 LTP 20240524 版本中选取了 `syscalls` 组（含 1411 个测试用例），覆盖 **365 个测试目录**，其中约 **360 个目录** 各自对应一个 Linux syscall 或 syscall 族。

```
testsuits-for-oskernel/
└── ltp-full-20240524/
    ├── runtest/syscalls          ← 1411 行测试条目（决定哪些跑）
    ├── testcases/kernel/syscalls/ ← 365 个测试目录（源码）
    │   ├── open/                  ← open 测试源码（13 个用例）
    │   ├── write/                 ← write 测试源码（6 个用例）
    │   └── ... 共 360+ 个
    └── build-oscomp.sh           ← OSCOMP 定制构建脚本
```

**OSCOMP 构建时排除：** `fmtmsg`、`rt_sigtimedwait`、`rt_tgsigqueueinfo`、`timer_create`（4 个目录在 Makefile 中 `FILTER_OUT_DIRS`）。

**非 syscall 的辅助目录（不测试具体 syscall）：** `utils`、`ipc`、`string`、`switch`、`cma`。

---

## 一、按类别分组的 LTP 测试 syscall 清单

> 每个 syscall 标注：**测试用例数**（runtest/syscalls 条目数）和 SpringOS 实现状态。

### 1.1 进程管理（Process）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 1 | `fork` | fork | 12 | ✅ SYS_fork | 创建子进程 |
| 2 | `vfork` | vfork | 2 | ❌ | fork 变体，父进程阻塞 |
| 3 | `clone` | clone | 12 | ✅ SYS_clone | 创建线程/进程 |
| 4 | `clone3` | clone3 | 0 | ❌ | clone 新版本，无 runtest 条目 |
| 5 | `execve` | execve | 6 | ✅ SYS_execve | 执行程序 |
| 6 | `execveat` | execveat | 3 | ❌ | 相对目录执行程序 |
| 7 | `execv` | execv | 1 | ❌ | libc 封装，底层走 execve |
| 8 | `execvp` | execvp | 1 | ❌ | libc 封装 |
| 9 | `execl` | execl | 1 | ❌ | libc 封装 |
| 10 | `execle` | execle | 1 | ❌ | libc 封装 |
| 11 | `execlp` | execlp | 1 | ❌ | libc 封装 |
| 12 | `exit` | exit | 2 | ✅ SYS_exit | 进程退出 |
| 13 | `exit_group` | exit_group | 1 | ✅ SYS_exit_group | 线程组退出 |
| 14 | `wait4` | wait4 | 3 | ✅ SYS_wait4 | 等待子进程（含 rusage） |
| 15 | `waitpid` | waitpid | 11 | ✅ SYS_waitpid | 等待指定子进程 |
| 16 | `waitid` | waitid | 11 | ❌ | 等待子进程（siginfo_t） |
| 17 | `wait` | wait | 5 | ❌ | libc 封装，底层走 wait4 |
| 18 | `getpid` | getpid | 2 | ✅ SYS_getpid | 获取进程 PID |
| 19 | `getppid` | getppid | 2 | ✅ SYS_getppid | 获取父进程 PID |
| 20 | `gettid` | gettid | 2 | ✅ SYS_gettid | 获取线程 TID |
| 21 | `getpgid` | getpgid | 2 | ❌ | 获取进程组 ID |
| 22 | `setpgid` | setpgid | 3 | ❌ | 设置进程组 ID |
| 23 | `getpgrp` | getpgrp | 1 | ❌ | 获取进程组 |
| 24 | `setpgrp` | setpgrp | 2 | ❌ | 设置进程组 |
| 25 | `getsid` | getsid | 2 | ❌ | 获取会话 ID |
| 26 | `setsid` | setsid | 1 | ❌ | 创建新会话 |
| 27 | `getpriority` | getpriority | 2 | ❌ | 获取进程优先级 |
| 28 | `setpriority` | setpriority | 2 | ❌ | 设置进程优先级 |
| 29 | `nice` | nice | 5 | ❌ | 调整 nice 值 |
| 30 | `sched_yield` | sched_yield | 1 | ✅ SYS_sched_yield | 主动让出 CPU |
| 31 | `sched_setaffinity` | sched_setaffinity | 1 | ❌ | 设置 CPU 亲和性 |
| 32 | `sched_getaffinity` | sched_getaffinity | 1 | ❌ | 获取 CPU 亲和性 |
| 33 | `sched_setparam` | sched_setparam | 5 | ❌ | 设置调度参数 |
| 34 | `sched_getparam` | sched_getparam | 2 | ❌ | 获取调度参数 |
| 35 | `sched_setscheduler` | sched_setscheduler | 4 | ❌ | 设置调度策略 |
| 36 | `sched_getscheduler` | sched_getscheduler | 2 | ❌ | 获取调度策略 |
| 37 | `sched_get_priority_max` | sched_get_priority_max | 2 | ❌ | 获取最高优先级 |
| 38 | `sched_get_priority_min` | sched_get_priority_min | 2 | ❌ | 获取最低优先级 |
| 39 | `sched_rr_get_interval` | sched_rr_get_interval | 3 | ❌ | 获取 RR 时间片 |
| 40 | `sched_setattr` | sched_setattr | 1 | ❌ | 设置调度属性（新 API） |
| 41 | `sched_getattr` | sched_getattr | 2 | ❌ | 获取调度属性（新 API） |
| 42 | `prctl` | prctl | 10 | ❌ | 进程控制操作 |
| 43 | `personality` | personality | 2 | ❌ | 设置进程执行域 |
| 44 | `arch_prctl` | arch_prctl | 1 | ❌ | x86 架构特定 prctl |
| 45 | `unshare` | unshare | 2 | ❌ | 取消共享命名空间 |
| 46 | `setns` | setns | 2 | ❌ | 加入命名空间 |
| 47 | `times` | times | 2 | ✅ SYS_times | 获取进程时间 |
| 48 | `getrusage` | getrusage | 4 | ❌ | 获取资源使用统计 |
| 49 | `ptrace` | ptrace | 11 | ❌ | 进程跟踪 |
| 50 | `kcmp` | kcmp | 3 | ❌ | 比较两个进程的内核对象 |

### 1.2 内存管理（Memory）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 51 | `brk` | brk | 2 | ✅ SYS_brk | 调整数据段大小 |
| 52 | `sbrk` | sbrk | 3 | ✅ SYS_sbrk | brk 变体（老接口） |
| 53 | `mmap` | mmap | 17 | ✅ SYS_mmap | 内存映射 |
| 54 | `munmap` | munmap | 3 | ✅ SYS_munmap | 解除内存映射 |
| 55 | `mprotect` | mprotect | 5 | ✅ SYS_mprotect | 设置内存保护 |
| 56 | `madvise` | madvise | 10 | ✅ SYS_madvise | 内存使用建议 |
| 57 | `mremap` | mremap | 6 | ✅ SYS_mremap | 重新映射 |
| 58 | `mlock` | mlock | 8 | ❌ | 锁定内存页 |
| 59 | `mlock2` | mlock2 | 0 | ❌ | mlock 新版本，无 runtest 条目 |
| 60 | `munlock` | munlock | 2 | ❌ | 解锁内存页 |
| 61 | `mlockall` | mlockall | 3 | ❌ | 锁定全部内存 |
| 62 | `munlockall` | munlockall | 1 | ❌ | 解锁全部内存 |
| 63 | `mincore` | mincore | 4 | ❌ | 检查页是否在内存 |
| 64 | `msync` | msync | 4 | ❌ | 同步 mmap 到文件 |
| 65 | `remap_file_pages` | remap_file_pages | 2 | ❌ | 非线性文件映射（已废弃） |
| 66 | `membarrier` | membarrier | 1 | ❌ | 内存屏障 |
| 67 | `process_madvise` | process_madvise | 1 | ❌ | 跨进程 madvise |
| 68 | `migrate_pages` | migrate_pages | 11 | ❌ | NUMA 页面迁移 |
| 69 | `move_pages` | move_pages | 11 | ❌ | NUMA 页面移动 |
| 70 | `mbind` | mbind | 4 | ❌ | NUMA 内存绑定 |
| 71 | `get_mempolicy` | get_mempolicy | 2 | ❌ | 获取 NUMA 策略 |
| 72 | `set_mempolicy` | set_mempolicy | 4 | ❌ | 设置 NUMA 策略 |
| 73 | `userfaultfd` | userfaultfd | 1 | ❌ | 用户态缺页处理 |
| 74 | `memfd_create` | memfd_create | 4 | ❌ | 创建匿名文件 |
| 75 | `mallinfo` | mallinfo | 2 | ❌ | glibc 内存信息（非 syscall） |
| 76 | `mallinfo2` | mallinfo2 | 0 | ❌ | mallinfo 新版本，无 runtest 条目 |
| 77 | `mallopt` | mallopt | 1 | ❌ | glibc 内存调优（非 syscall） |
| 78 | `pkeys` | pkeys | 0 | ❌ | 内存保护键，无 runtest 条目 |

### 1.3 文件系统 — 文件操作（File Operations）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 79 | `open` | open | 13 | ❌ | 打开文件（老接口） |
| 80 | `openat` | openat | 7 | ✅ SYS_openat | 相对目录打开文件 |
| 81 | `openat2` | openat2 | 0 | ❌ | openat 扩展，无 runtest 条目 |
| 82 | `creat` | creat | 8 | ❌ | 创建文件（老接口） |
| 83 | `close` | close | 2 | ✅ SYS_close | 关闭文件描述符 |
| 84 | `close_range` | close_range | 2 | ❌ | 批量关闭 fd |
| 85 | `read` | read | 4 | ✅ SYS_read | 读取文件 |
| 86 | `write` | write | 6 | ✅ SYS_write | 写入文件 |
| 87 | `readv` | readv | 2 | ✅ SYS_readv | 分散读 |
| 88 | `writev` | writev | 6 | ✅ SYS_writev | 分散写 |
| 89 | `pread` | pread | 4 | ✅ SYS_pread64 | 指定偏移读 |
| 90 | `pwrite` | pwrite | 8 | ❌ | 指定偏移写 |
| 91 | `preadv` | preadv | 12 | ❌ | 指定偏移分散读 |
| 92 | `preadv2` | preadv2 | 0 | ❌ | preadv 扩展，无 runtest 条目 |
| 93 | `pwritev` | pwritev | 10 | ❌ | 指定偏移分散写 |
| 94 | `pwritev2` | pwritev2 | 0 | ❌ | pwritev 扩展，无 runtest 条目 |
| 95 | `lseek` | lseek | 4 | ✅ SYS_lseek | 移动文件偏移 |
| 96 | `llseek` | llseek | 3 | ❌ | 大文件 lseek |
| 97 | `dup` | dup | 16 | ✅ SYS_dup | 复制文件描述符 |
| 98 | `dup2` | dup2 | 0 | ❌ | 老接口，无独立 runtest 条目 |
| 99 | `dup3` | dup3 | 0 | ✅ SYS_dup3 | 无独立 runtest 条目，合并到 dup |
| 100 | `sync` | sync | 1 | ❌ | 同步所有文件系统 |
| 101 | `syncfs` | syncfs | 1 | ❌ | 同步单个文件系统 |
| 102 | `sync_file_range` | sync_file_range | 2 | ❌ | 范围同步 |
| 103 | `fsync` | fsync | 4 | ❌ | 同步单个文件 |
| 104 | `fdatasync` | fdatasync | 3 | ❌ | 同步文件数据 |
| 105 | `fcntl` | fcntl | 74 | ✅ SYS_fcntl | 文件控制操作（LTP 最大用例数） |
| 106 | `flock` | flock | 5 | ❌ | 文件锁 |
| 107 | `readahead` | readahead | 2 | ❌ | 预读文件 |
| 108 | `fadvise` | fadvise | 0 | ❌ | 文件访问建议，无 runtest 条目 |
| 109 | `splice` | splice | 9 | ❌ | 管道间零拷贝传输 |
| 110 | `tee` | tee | 2 | ❌ | 管道数据复制 |
| 111 | `vmsplice` | vmsplice | 4 | ❌ | 用户空间到管道 |
| 112 | `sendfile` | sendfile | 16 | ✅ SYS_sendfile | 文件到 socket 零拷贝 |
| 113 | `copy_file_range` | copy_file_range | 3 | ❌ | 文件间零拷贝复制 |
| 114 | `readdir` | readdir | 2 | ❌ | 读取目录（老接口） |
| 115 | `getdents` | getdents | 2 | ✅ SYS_getdents64 | 读取目录项 |
| 116 | `ioctl` | ioctl | 9 | ❌ | 设备控制 |
| 117 | `readlink` | readlink | 2 | ❌ | 读取符号链接 |
| 118 | `readlinkat` | readlinkat | 2 | ❌ | 相对目录读取符号链接 |
| 119 | `truncate` | truncate | 4 | ❌ | 截断文件（按路径） |
| 120 | `ftruncate` | ftruncate | 6 | ❌ | 截断文件（按 fd） |

### 1.4 文件系统 — 目录/路径操作（Directory & Path）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 121 | `chdir` | chdir | 2 | ✅ SYS_chdir | 切换工作目录 |
| 122 | `fchdir` | fchdir | 3 | ❌ | 按 fd 切换工作目录 |
| 123 | `getcwd` | getcwd | 4 | ✅ SYS_getcwd | 获取当前目录 |
| 124 | `mkdir` | mkdir | 5 | ❌ | 创建目录（老接口） |
| 125 | `mkdirat` | mkdirat | 2 | ✅ SYS_mkdirat | 相对目录创建 |
| 126 | `rmdir` | rmdir | 3 | ❌ | 删除空目录 |
| 127 | `link` | link | 5 | ❌ | 创建硬链接（老接口） |
| 128 | `linkat` | linkat | 2 | ✅ SYS_linkat | 相对目录创建硬链接 |
| 129 | `unlink` | unlink | 5 | ❌ | 删除文件（老接口） |
| 130 | `unlinkat` | unlinkat | 1 | ✅ SYS_unlinkat | 相对目录删除 |
| 131 | `symlink` | symlink | 4 | ❌ | 创建符号链接 |
| 132 | `symlinkat` | symlinkat | 1 | ❌ | 相对目录创建符号链接 |
| 133 | `rename` | rename | 13 | ❌ | 重命名（老接口） |
| 134 | `renameat` | renameat | 3 | ❌ | 相对目录重命名 |
| 135 | `renameat2` | renameat2 | 0 | ✅ SYS_renameat2 | 无 runtest 条目 |
| 136 | `mknod` | mknod | 9 | ✅ SYS_mknod | 创建设备节点 |
| 137 | `mknodat` | mknodat | 2 | ❌ | 相对目录创建设备节点 |
| 138 | `mount` | mount | 7 | ✅ SYS_mount | 挂载文件系统 |
| 139 | `umount` | umount | 5 | ❌ | 卸载文件系统（含 umount2） |
| 140 | `umount2` | umount2 | 0 | ✅ SYS_umount2 | 无独立 runtest 条目，合并到 umount |
| 141 | `mount_setattr` | mount_setattr | 1 | ❌ | 修改挂载属性 |
| 142 | `move_mount` | move_mount | 2 | ❌ | 移动挂载点 |
| 143 | `open_tree` | open_tree | 2 | ❌ | 以 fd 形式打开挂载树 |
| 144 | `fspick` | fspick | 2 | ❌ | 选择文件系统配置 |
| 145 | `fsopen` | fsopen | 2 | ❌ | 打开文件系统上下文 |
| 146 | `fsconfig` | fsconfig | 3 | ❌ | 设置文件系统参数 |
| 147 | `fsmount` | fsmount | 2 | ❌ | 挂载文件系统上下文 |
| 148 | `pivot_root` | pivot_root | 1 | ❌ | 切换根文件系统 |
| 149 | `chroot` | chroot | 4 | ❌ | 改变根目录 |
| 150 | `realpath` | realpath | 1 | ❌ | 解析真实路径（glibc） |
| 151 | `nftw` | nftw | 2 | ❌ | 文件树遍历（glibc） |
| 152 | `name_to_handle_at` | name_to_handle_at | 2 | ❌ | 获取文件句柄 |
| 153 | `open_by_handle_at` | open_by_handle_at | 2 | ❌ | 按句柄打开文件 |

### 1.5 文件系统 — 属性/元数据（File Attributes & Metadata）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 154 | `stat` | stat | 8 | ❌ | 获取文件信息（老接口） |
| 155 | `fstat` | fstat | 4 | ✅ SYS_fstat | 按 fd 获取文件信息 |
| 156 | `lstat` | lstat | 4 | ❌ | 不跟随符号链接的 stat |
| 157 | `fstatat` | fstatat | 1 | ❌ | 相对目录 stat |
| 158 | `statfs` | statfs | 6 | ❌ | 获取文件系统信息 |
| 159 | `fstatfs` | fstatfs | 4 | ❌ | 按 fd 获取文件系统信息 |
| 160 | `statvfs` | statvfs | 2 | ❌ | 获取文件系统信息（POSIX） |
| 161 | `statx` | statx | 12 | ✅ SYS_statx | 扩展文件信息（新 API） |
| 162 | `access` | access | 4 | ❌ | 检查文件访问权限 |
| 163 | `faccessat` | faccessat | 4 | ❌ | 相对目录检查权限 |
| 164 | `faccessat2` | faccessat2 | 0 | ❌ | faccessat 扩展，无 runtest 条目 |
| 165 | `chmod` | chmod | 5 | ❌ | 修改文件权限（老接口） |
| 166 | `fchmod` | fchmod | 6 | ❌ | 按 fd 修改权限 |
| 167 | `fchmodat` | fchmodat | 2 | ✅ SYS_fchmodat | 相对目录修改权限 |
| 168 | `chown` | chown | 10 | ❌ | 修改文件所有者（老接口） |
| 169 | `fchown` | fchown | 10 | ❌ | 按 fd 修改所有者 |
| 170 | `lchown` | lchown | 6 | ❌ | 不跟随符号链接的 chown |
| 171 | `fchownat` | fchownat | 2 | ❌ | 相对目录 chown |
| 172 | `umask` | umask | 1 | ❌ | 设置文件创建掩码 |
| 173 | `utime` | utime | 7 | ❌ | 修改文件时间（老接口） |
| 174 | `utimes` | utimes | 1 | ❌ | 修改文件时间 |
| 175 | `utimensat` | utimensat | 1 | ✅ SYS_utimensat | 修改文件时间（纳秒精度） |
| 176 | `futimesat` | futimesat | 1 | ❌ | 相对目录 utimes |
| 177 | `fallocate` | fallocate | 6 | ❌ | 预分配文件空间 |
| 178 | `pathconf` | pathconf | 2 | ❌ | 获取路径配置 |
| 179 | `fpathconf` | fpathconf | 1 | ❌ | 获取文件配置 |
| 180 | `quotactl` | quotactl | 9 | ❌ | 磁盘配额控制 |

### 1.6 文件系统 — 扩展属性（Extended Attributes / xattr）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 181 | `getxattr` | getxattr | 5 | ❌ | 获取扩展属性 |
| 182 | `fgetxattr` | fgetxattr | 3 | ❌ | 按 fd 获取 xattr |
| 183 | `lgetxattr` | lgetxattr | 2 | ❌ | 不跟随链接获取 xattr |
| 184 | `setxattr` | setxattr | 3 | ❌ | 设置扩展属性 |
| 185 | `fsetxattr` | fsetxattr | 2 | ❌ | 按 fd 设置 xattr |
| 186 | `listxattr` | listxattr | 3 | ❌ | 列出扩展属性 |
| 187 | `flistxattr` | flistxattr | 3 | ❌ | 按 fd 列出 xattr |
| 188 | `llistxattr` | llistxattr | 3 | ❌ | 不跟随链接列出 xattr |
| 189 | `removexattr` | removexattr | 2 | ❌ | 删除扩展属性 |
| 190 | `fremovexattr` | fremovexattr | 2 | ❌ | 按 fd 删除 xattr |
| 191 | `lremovexattr` | lremovexattr | 1 | ❌ | 不跟随链接删除 xattr |

### 1.7 epoll / 事件通知（Event Poll & Notification）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 192 | `epoll_create` | epoll_create | 4 | ❌ | 创建 epoll（老接口） |
| 193 | `epoll_create1` | epoll_create1 | 0 | ❌ | epoll_create 新接口，合并到上一层 |
| 194 | `epoll_ctl` | epoll_ctl | 5 | ❌ | 控制 epoll |
| 195 | `epoll_wait` | epoll_wait | 7 | ❌ | 等待 epoll 事件 |
| 196 | `epoll_pwait` | epoll_pwait | 5 | ❌ | 带信号屏蔽的 epoll_wait |
| 197 | `epoll` | epoll | 1 | ❌ | epoll 综合测试 |
| 198 | `poll` | poll | 2 | ❌ | 多路复用 |
| 199 | `ppoll` | ppoll | 1 | ✅ SYS_ppoll | 带信号屏蔽的 poll |
| 200 | `select` | select | 4 | ❌ | 多路复用（老接口） |
| 201 | `pselect` | pselect | 6 | ❌ | 带信号屏蔽的 select |
| 202 | `eventfd` | eventfd | 9 | ❌ | 事件通知 fd |
| 203 | `eventfd2` | eventfd2 | 0 | ❌ | eventfd 新版本，无 runtest 条目 |
| 204 | `inotify_init` | inotify_init | 2 | ❌ | 创建 inotify 实例 |
| 205 | `inotify` | inotify | 12 | ❌ | 文件系统事件监控 |
| 206 | `fanotify` | fanotify | 23 | ❌ | 文件系统访问监控 |
| 207 | `signalfd` | signalfd | 3 | ❌ | 信号 fd（含 signalfd4） |
| 208 | `signalfd4` | signalfd4 | 0 | ❌ | 无独立 runtest 条目，合并到 signalfd |
| 209 | `timerfd` | timerfd | 3 | ❌ | 定时器 fd |

### 1.8 信号（Signals）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 210 | `kill` | kill | 11 | ✅ SYS_kill | 发送信号 |
| 211 | `tkill` | tkill | 2 | ✅ SYS_tkill | 向线程发信号 |
| 212 | `tgkill` | tgkill | 3 | ✅ SYS_tgkill | 向线程组发信号 |
| 213 | `sigaction` | sigaction | 2 | ❌ | 设置信号处理（老接口） |
| 214 | `rt_sigaction` | rt_sigaction | 3 | ✅ SYS_rt_sigaction | 设置信号处理（实时） |
| 215 | `signal` | signal | 6 | ❌ | 信号处理（老接口） |
| 216 | `sigprocmask` | sigprocmask | 1 | ❌ | 阻塞/解阻塞信号（老） |
| 217 | `rt_sigprocmask` | rt_sigprocmask | 2 | ✅ SYS_rt_sigprocmask | 阻塞信号（实时） |
| 218 | `sigpending` | sigpending | 1 | ❌ | 检查待处理信号 |
| 219 | `sigsuspend` | sigsuspend | 1 | ❌ | 等待信号（老接口） |
| 220 | `rt_sigsuspend` | rt_sigsuspend | 1 | ❌ | 等待信号（实时） |
| 221 | `sigtimedwait` | sigtimedwait | 1 | ❌ | 等待信号（超时） |
| 222 | `rt_sigtimedwait` | rt_sigtimedwait | 1 | ⚠️ 构建排除 | 等待信号（实时+超时） |
| 223 | `sigwait` | sigwait | 1 | ❌ | 等待信号（线程） |
| 224 | `sigwaitinfo` | sigwaitinfo | 1 | ❌ | 等待信号并获取信息 |
| 225 | `sigqueue` / `rt_sigqueueinfo` | rt_sigqueueinfo | 1 | ❌ | 带数据发送信号 |
| 226 | `rt_tgsigqueueinfo` | rt_tgsigqueueinfo | 1 | ⚠️ 构建排除 | 向线程组发带数据信号 |
| 227 | `sighold` | sighold | 1 | ❌ | 阻塞信号（老接口） |
| 228 | `sigrelse` | sigrelse | 1 | ❌ | 解阻塞信号（老接口） |
| 229 | `sigaltstack` | sigaltstack | 2 | ❌ | 设置备用信号栈 |
| 230 | `sgetmask` | sgetmask | 1 | ❌ | 获取信号掩码（老接口） |
| 231 | `ssetmask` | ssetmask | 1 | ❌ | 设置信号掩码（老接口） |
| 232 | `abort` | abort | 1 | ❌ | 异常终止（glibc） |

### 1.9 用户/组/权限（User & Group）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 233 | `getuid` | getuid | 4 | ✅ SYS_getuid | 获取用户 ID |
| 234 | `setuid` | setuid | 6 | ✅ SYS_setuid | 设置用户 ID |
| 235 | `geteuid` | geteuid | 4 | ✅ SYS_geteuid | 获取有效用户 ID |
| 236 | `getgid` | getgid | 4 | ✅ SYS_getgid | 获取组 ID |
| 237 | `setgid` | setgid | 6 | ✅ SYS_setgid | 设置组 ID |
| 238 | `getegid` | getegid | 4 | ✅ SYS_getegid | 获取有效组 ID |
| 239 | `setegid` | setegid | 2 | ❌ | 设置有效组 ID |
| 240 | `getresuid` | getresuid | 6 | ❌ | 获取 real/effective/saved UID |
| 241 | `setresuid` | setresuid | 10 | ❌ | 设置 real/effective/saved UID |
| 242 | `getresgid` | getresgid | 6 | ❌ | 获取 real/effective/saved GID |
| 243 | `setresgid` | setresgid | 8 | ❌ | 设置 real/effective/saved GID |
| 244 | `setreuid` | setreuid | 14 | ✅ SYS_setreuid | 设置 real 和 effective UID |
| 245 | `setregid` | setregid | 8 | ✅ SYS_setregid | 设置 real 和 effective GID |
| 246 | `setfsuid` | setfsuid | 8 | ❌ | 设置文件系统 UID |
| 247 | `setfsgid` | setfsgid | 6 | ❌ | 设置文件系统 GID |
| 248 | `getgroups` | getgroups | 4 | ❌ | 获取附加组列表 |
| 249 | `setgroups` | setgroups | 6 | ❌ | 设置附加组列表 |
| 250 | `getdomainname` | getdomainname | 1 | ❌ | 获取域名 |
| 251 | `setdomainname` | setdomainname | 3 | ❌ | 设置域名 |

### 1.10 时间（Time）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 252 | `time` | time | 1 | ❌ | 获取时间（老接口） |
| 253 | `gettimeofday` | gettimeofday | 2 | ✅ SYS_gettimeofday | 获取时间（微秒） |
| 254 | `settimeofday` | settimeofday | 2 | ❌ | 设置时间 |
| 255 | `nanosleep` | nanosleep | 3 | ✅ SYS_nanosleep | 纳秒级睡眠 |
| 256 | `clock_gettime` | clock_gettime | 4 | ❌ | 获取时钟时间 |
| 257 | `clock_settime` | clock_settime | 3 | ❌ | 设置时钟时间 |
| 258 | `clock_getres` | clock_getres | 1 | ❌ | 获取时钟分辨率 |
| 259 | `clock_nanosleep` | clock_nanosleep | 4 | ❌ | 按指定时钟睡眠 |
| 260 | `clock_adjtime` | clock_adjtime | 2 | ❌ | 调整时钟 |
| 261 | `getitimer` | getitimer | 2 | ❌ | 获取间隔定时器 |
| 262 | `setitimer` | setitimer | 2 | ❌ | 设置间隔定时器 |
| 263 | `timer_create` | timer_create | 3 | ⚠️ 构建排除 | 创建 POSIX 定时器 |
| 264 | `timer_delete` | timer_delete | 2 | ❌ | 删除 POSIX 定时器 |
| 265 | `timer_settime` | timer_settime | 3 | ❌ | 设置定时器时间 |
| 266 | `timer_gettime` | timer_gettime | 1 | ❌ | 获取定时器时间 |
| 267 | `timer_getoverrun` | timer_getoverrun | 1 | ❌ | 获取定时器超限次数 |
| 268 | `alarm` | alarm | 5 | ❌ | 设置闹钟 |
| 269 | `stime` | stime | 2 | ❌ | 设置时间（老接口） |

### 1.11 网络（Network / Sockets）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 270 | `socket` | socket | 2 | ❌ | 创建 socket |
| 271 | `socketpair` | socketpair | 2 | ❌ | 创建 socket 对 |
| 272 | `socketcall` | socketcall | 3 | ❌ | socket 调用入口（老） |
| 273 | `bind` | bind | 6 | ❌ | 绑定地址 |
| 274 | `listen` | listen | 1 | ❌ | 监听连接 |
| 275 | `accept` | accept | 4 | ❌ | 接受连接（含 accept4） |
| 276 | `accept4` | accept4 | 0 | ❌ | 无独立 runtest 条目，合并到 accept |
| 277 | `connect` | connect | 2 | ❌ | 发起连接 |
| 278 | `send` | send | 2 | ❌ | 发送数据 |
| 279 | `sendto` | sendto | 3 | ❌ | 发送到指定地址 |
| 280 | `sendmsg` | sendmsg | 3 | ❌ | 发送消息 |
| 281 | `sendmmsg` | sendmmsg | 2 | ❌ | 批量发送消息 |
| 282 | `recv` | recv | 1 | ❌ | 接收数据 |
| 283 | `recvfrom` | recvfrom | 1 | ❌ | 从指定地址接收 |
| 284 | `recvmsg` | recvmsg | 3 | ❌ | 接收消息 |
| 285 | `recvmmsg` | recvmmsg | 1 | ❌ | 批量接收消息 |
| 286 | `getsockname` | getsockname | 1 | ❌ | 获取 socket 本地名 |
| 287 | `getpeername` | getpeername | 1 | ❌ | 获取 socket 对端名 |
| 288 | `getsockopt` | getsockopt | 2 | ❌ | 获取 socket 选项 |
| 289 | `setsockopt` | setsockopt | 10 | ❌ | 设置 socket 选项 |
| 290 | `sockioctl` | sockioctl | 1 | ❌ | socket ioctl |
| 291 | `gethostname` | gethostname | 2 | ❌ | 获取主机名 |
| 292 | `sethostname` | sethostname | 3 | ❌ | 设置主机名 |
| 293 | `gethostid` | gethostid | 1 | ❌ | 获取主机 ID |
| 294 | `gethostbyname_r` | gethostbyname_r | 1 | ❌ | DNS 解析（glibc） |

### 1.12 管道（Pipes）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 295 | `pipe` | pipe | 18 | ❌ | 创建管道（含 pipe2 用例） |
| 296 | `pipe2` | pipe2 | 0 | ✅ SYS_pipe2 | 无独立 runtest 条目，合并到 pipe |

### 1.13 IPC（进程间通信）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 297 | `ipc` | ipc | 0 | ❌ | IPC 入口（老），无 runtest 条目 |
| 298 | `mq_open` | mq_open | 1 | ❌ | 创建 POSIX 消息队列 |
| 299 | `mq_unlink` | mq_unlink | 1 | ❌ | 删除消息队列 |
| 300 | `mq_timedsend` | mq_timedsend | 1 | ❌ | 向消息队列发送 |
| 301 | `mq_timedreceive` | mq_timedreceive | 1 | ❌ | 从消息队列接收 |
| 302 | `mq_notify` | mq_notify | 3 | ❌ | 消息队列通知 |

### 1.14 内核/系统信息（Kernel & System Info）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 303 | `uname` | uname | 3 | ✅ SYS_uname | 获取系统信息 |
| 304 | `newuname` | newuname | 1 | ❌ | uname（新 struct） |
| 305 | `sysinfo` | sysinfo | 3 | ❌ | 获取系统统计 |
| 306 | `syslog` | syslog | 2 | ❌ | 内核日志读取 |
| 307 | `sysctl` | sysctl | 3 | ❌ | 系统控制（已废弃） |
| 308 | `sysfs` | sysfs | 5 | ❌ | 获取文件系统类型 |
| 309 | `sysconf` | sysconf | 1 | ❌ | 获取系统配置（glibc） |
| 310 | `syscall` | syscall | 1 | ❌ | 间接 syscall 调用（glibc） |
| 311 | `confstr` | confstr | 1 | ❌ | 获取配置字符串 |
| 312 | `getpagesize` | getpagesize | 1 | ❌ | 获取页面大小（glibc） |
| 313 | `reboot` | reboot | 2 | ❌ | 重启系统 |
| 314 | `vhangup` | vhangup | 2 | ❌ | 虚拟挂断终端 |
| 315 | `getrandom` | getrandom | 5 | ❌ | 获取随机数 |
| 316 | `set_tid_address` | set_tid_address | 1 | ✅ SYS_set_tid_address | 设置线程清理地址 |
| 317 | `set_robust_list` | set_robust_list | 1 | ✅ SYS_set_robust_list | 设置 robust futex 列表 |
| 318 | `get_robust_list` | get_robust_list | 1 | ❌ | 获取 robust futex 列表 |
| 319 | `set_thread_area` | set_thread_area | 1 | ❌ | 设置线程局部存储 |
| 320 | `futex` | futex | 0 | ✅ SYS_futex | 快速用户态锁（无 runtest 条目） |

### 1.15 能力/安全（Capabilities & Security）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 321 | `capget` | capget | 2 | ❌ | 获取进程能力 |
| 322 | `capset` | capset | 4 | ❌ | 设置进程能力 |
| 323 | `keyctl` | keyctl | 9 | ❌ | 内核密钥管理 |
| 324 | `add_key` | add_key | 5 | ❌ | 添加内核密钥 |
| 325 | `request_key` | request_key | 5 | ❌ | 请求内核密钥 |
| 326 | `init_module` | init_module | 2 | ❌ | 加载内核模块 |
| 327 | `finit_module` | finit_module | 2 | ❌ | 按 fd 加载内核模块 |
| 328 | `delete_module` | delete_module | 3 | ❌ | 卸载内核模块 |

### 1.16 AIO（异步 I/O）

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 329 | `io_setup` | io_setup | 2 | ❌ | 创建 AIO 上下文 |
| 330 | `io_destroy` | io_destroy | 2 | ❌ | 销毁 AIO 上下文 |
| 331 | `io_submit` | io_submit | 3 | ❌ | 提交 AIO 请求 |
| 332 | `io_cancel` | io_cancel | 2 | ❌ | 取消 AIO 请求 |
| 333 | `io_getevents` | io_getevents | 2 | ❌ | 获取 AIO 完成事件 |
| 334 | `io_pgetevents` | io_pgetevents | 2 | ❌ | 获取 AIO 完成事件（信号屏蔽） |

### 1.17 io_uring

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 335 | `io_uring` | io_uring | 2 | ❌ | 高性能异步 I/O |

### 1.18 其他杂项

| # | Syscall | LTP 目录 | 用例数 | SpringOS | 说明 |
|---|---------|---------|:-----:|:--------:|------|
| 336 | `iopl` | iopl | 2 | ❌ | 设置 I/O 特权级别（x86） |
| 337 | `ioperm` | ioperm | 2 | ❌ | 设置端口权限（x86） |
| 338 | `ioprio` | ioprio | 0 | ❌ | I/O 优先级，无 runtest 条目 |
| 339 | `swapoff` | swapoff | 2 | ❌ | 关闭交换 |
| 340 | `swapon` | swapon | 3 | ❌ | 开启交换 |
| 341 | `bpf` | bpf | 0 | ❌ | BPF 系统调用，无 runtest 条目 |
| 342 | `modify_ldt` | modify_ldt | 3 | ❌ | 修改 LDT（x86） |
| 343 | `getcpu` | getcpu | 1 | ❌ | 获取当前 CPU |
| 344 | `getcontext` | getcontext | 1 | ❌ | 获取用户上下文（glibc） |
| 345 | `profil` | profil | 1 | ❌ | 执行时间统计 |
| 346 | `cacheflush` | cacheflush | 1 | ❌ | 刷新缓存 |
| 347 | `perf_event_open` | perf_event_open | 3 | ❌ | 性能事件监控 |
| 348 | `pidfd_open` | pidfd_open | 4 | ❌ | 以 fd 引用进程 |
| 349 | `pidfd_getfd` | pidfd_getfd | 2 | ❌ | 从另一进程获取 fd |
| 350 | `pidfd_send_signal` | pidfd_send_signal | 3 | ❌ | 向 pidfd 发送信号 |
| 351 | `fmtmsg` | fmtmsg | 1 | ⚠️ 构建排除 | 格式化消息（glibc） |
| 352 | `ustat` | ustat | 2 | ❌ | 获取文件系统统计（已废弃） |
| 353 | `ulimit` | ulimit | 1 | ❌ | 资源限制（已废弃） |
| 354 | `acct` | acct | 2 | ❌ | 进程记账 |
| 355 | `adjtimex` | adjtimex | 3 | ❌ | 调整内核时钟 |
| 356 | `rlimit` (`getrlimit`/`setrlimit`) | getrlimit | 3 | ❌ | 资源限制 |
| 357 | — | setrlimit | 6 | ❌ | 资源限制 |
| 358 | `pause` | pause | 3 | ❌ | 等待信号 |
| 359 | `ustring` / `string` | string | 1 | ❌ | 字符串操作测试 |

---

## 二、OSCOMP 构建特殊处理

### 2.1 被排除的 4 个 syscall 测试目录

在 `build-oscomp.sh` 的 Makefile 修改中，以下 4 个目录从编译中过滤掉：

| Syscall | 排除原因推测 |
|---------|------------|
| `fmtmsg` | glibc 库函数，非真正 syscall |
| `rt_sigtimedwait` | 需要完整实时信号支持 |
| `rt_tgsigqueueinfo` | 需要线程组信号队列支持 |
| `timer_create` | 需要 POSIX 定时器完整实现 |

### 2.2 源码修改

OSCOMP 构建脚本还对 LTP 源码做了以下修改：
- `#include <sys/sysinfo.h>` → `#include <linux/sysinfo.h>`（全局替换，因交叉编译工具链头文件差异）

---

## 三、SpringOS 实现状态汇总

### 3.1 已实现的 syscall（37 个 LTP 覆盖 + 若干未覆盖）

| 状态 | 数量 | syscall |
|:----:|:----:|--------|
| ✅ 已实现 | 37 | fork, wait4, waitpid, kill, tkill, tgkill, sbrk, getcwd, dup, dup3, fcntl, mkdirat, unlinkat, linkat, mount, umount2, chdir, fchmodat, openat, close, pipe2, getdents64, lseek, read, write, readv, writev, pread64, fstat, utimensat, exit, exit_group, set_tid_address, getuid/setuid/getgid/setgid/geteuid/getegid/setreuid/setregid, nanosleep, sched_yield, times, uname, gettimeofday, getpid, getppid, brk, munmap, mremap, clone, execve, mmap, mprotect, madvise, statx, renameat2, sendfile, ppoll, gettid, set_robust_list, futex |
| ❌ 未实现 | ~319 | 其余所有 |
| ⚠️ 构建排除 | 4 | rt_sigtimedwait, rt_tgsigqueueinfo, timer_create, fmtmsg |

### 3.2 LTP 测试用例数 Top 20

| 排名 | Syscall | 用例数 | SpringOS |
|:----:|---------|:-----:|:--------:|
| 1 | fcntl | 74 | ✅ |
| 2 | fanotify | 23 | ❌ |
| 3 | pipe | 18 | ✅（pipe2） |
| 4 | mmap | 17 | ✅ |
| 5 | dup | 16 | ✅ |
| 6 | sendfile | 16 | ✅ |
| 7 | setreuid | 14 | ✅ |
| 8 | rename | 13 | ❌ |
| 9 | open | 13 | ❌ |
| 10 | execve | 12 | ✅ |
| 11 | fork | 12 | ✅ |
| 12 | clone | 12 | ✅ |
| 13 | inotify | 12 | ❌ |
| 14 | statx | 12 | ✅ |
| 15 | preadv | 12 | ❌ |
| 16 | waitid | 11 | ❌ |
| 17 | waitpid | 11 | ✅ |
| 18 | kill | 11 | ✅ |
| 19 | move_pages | 11 | ❌ |
| 20 | migrate_pages | 11 | ❌ |

---

## 四、完整按字母排序的 LTP syscall 目录索引

> 含测试目录名、runtest 用例数、SpringOS 状态。可用 Ctrl+F 快速查找。

```
abort           1   ❌        alarm           5   ❌
accept          4   ❌        arch_prctl      1   ❌
accept4         0   ❌        bind            6   ❌
access          4   ❌        bpf             0   ❌
acct            2   ❌        brk             2   ✅
add_key         5   ❌        cacheflush      1   ❌
adjtimex        3   ❌        capget          2   ❌
capset          4   ❌        fchmodat        2   ✅
chdir           2   ✅        fchown         10   ❌
chmod           5   ❌        fchownat        2   ❌
chown          10   ❌        fcntl          74   ✅
chroot          4   ❌        fdatasync       3   ❌
clock_adjtime   2   ❌        fgetxattr       3   ❌
clock_getres    1   ❌        finit_module    2   ❌
clock_gettime   4   ❌        flistxattr      3   ❌
clock_nanosleep 4   ❌        flock           5   ❌
clock_settime   3   ❌        fmtmsg          1   ⚠️
clone          12   ✅        fork           12   ✅
clone3          0   ❌        fpathconf       1   ❌
close           2   ✅        fremovexattr    2   ❌
close_range     2   ❌        fsconfig        3   ❌
confstr         1   ❌        fsetxattr       2   ❌
connect         2   ❌        fsmount         2   ❌
copy_file_range 3   ❌        fsopen          2   ❌
creat           8   ❌        fspick          2   ❌
delete_module   3   ❌        fstat           4   ✅
dup            16   ✅        fstatat         1   ❌
dup2            0   ❌        fstatfs         4   ❌
dup3            0   ✅        fsync           4   ❌
epoll           1   ❌        ftruncate       6   ❌
epoll_create    4   ❌        futex           0   ✅
epoll_create1   0   ❌        futimesat       1   ❌
epoll_ctl       5   ❌        getcontext      1   ❌
epoll_pwait     5   ❌        getcpu          1   ❌
epoll_wait      7   ❌        getcwd          4   ✅
eventfd         9   ❌        getdents        2   ✅
eventfd2        0   ❌        getdomainname   1   ❌
execl           1   ❌        getegid         4   ✅
execle          1   ❌        geteuid         4   ✅
execlp          1   ❌        getgid          4   ✅
execv           1   ❌        getgroups       4   ❌
execve          6   ✅        gethostbyname_r 1   ❌
execveat        3   ❌        gethostid       1   ❌
execvp          1   ❌        gethostname     2   ❌
exit            2   ✅        getitimer       2   ❌
exit_group      1   ✅        get_mempolicy   2   ❌
faccessat       4   ❌        getpagesize     1   ❌
faccessat2      0   ❌        getpeername     1   ❌
fadvise         0   ❌        getpgid         2   ❌
fallocate       6   ❌        getpgrp         1   ❌
fanotify       23   ❌        getpid          2   ✅
fchdir          3   ❌        getppid         2   ✅
fchmod          6   ❌        getpriority     2   ❌
getrandom       5   ❌        mlock2          0   ❌
getresgid       6   ❌        mlockall        3   ❌
getresuid       6   ❌        mmap           17   ✅
getrlimit       3   ❌        modify_ldt      3   ❌
get_robust_list 1   ❌        mount           7   ✅
getrusage       4   ❌        mount_setattr   1   ❌
getsid          2   ❌        move_mount      2   ❌
getsockname     1   ❌        move_pages     11   ❌
getsockopt      2   ❌        mprotect        5   ✅
gettid          2   ✅        mq_notify       3   ❌
gettimeofday    2   ✅        mq_open         1   ❌
getuid          4   ✅        mq_timedreceive 1   ❌
getxattr        5   ❌        mq_timedsend    1   ❌
init_module     2   ❌        mq_unlink       1   ❌
inotify        12   ❌        mremap          6   ✅
inotify_init    2   ❌        msync           4   ❌
io_cancel       2   ❌        munlock         2   ❌
ioctl           9   ❌        munlockall      1   ❌
io_destroy      2   ❌        munmap          3   ✅
io_getevents    2   ❌        name_to_handle_at 2 ❌
ioperm          2   ❌        nanosleep       3   ✅
io_pgetevents   2   ❌        newuname        1   ❌
iopl            2   ❌        nftw            2   ❌
ioprio          0   ❌        nice            5   ❌
io_setup        2   ❌        open           13   ❌
io_submit       3   ❌        openat          7   ✅
io_uring        2   ❌        openat2         0   ❌
ipc             0   ❌        open_by_handle_at 2 ❌
kcmp            3   ❌        open_tree       2   ❌
keyctl          9   ❌        pathconf        2   ❌
kill           11   ✅        pause           3   ❌
lchown          6   ❌        perf_event_open 3   ❌
lgetxattr       2   ❌        personality     2   ❌
link            5   ❌        pidfd_getfd     2   ❌
linkat          2   ✅        pidfd_open      4   ❌
listen          1   ❌        pidfd_send_signal 3 ❌
listxattr       3   ❌        pipe           18   ❌
llistxattr      3   ❌        pipe2           0   ✅
llseek          3   ❌        pivot_root      1   ❌
lremovexattr    1   ❌        pkeys           0   ❌
lseek           4   ✅        poll            2   ❌
lstat           4   ❌        ppoll           1   ✅
madvise        10   ✅        prctl          10   ❌
mallinfo        2   ❌        pread           4   ✅
mallinfo2       0   ❌        preadv         12   ❌
mallopt         1   ❌        preadv2         0   ❌
mbind           4   ❌        process_madvise 1   ❌
membarrier      1   ❌        profil          1   ❌
memcmp          1   ❌        pselect         6   ❌
memcpy          1   ❌        ptrace         11   ❌
memfd_create    4   ❌        pwrite          8   ❌
memset          1   ❌        pwritev        10   ❌
migrate_pages  11   ❌        pwritev2        0   ❌
mincore         4   ❌        quotactl        9   ❌
mkdir           5   ❌        read            4   ✅
mkdirat         2   ✅        readahead       2   ❌
mknod           9   ✅        readdir         2   ❌
mknodat         2   ❌        readlink        2   ❌
mlock           8   ❌        readlinkat      2   ❌
readv           2   ✅        set_mempolicy   4   ❌
realpath        1   ❌        setns           2   ❌
reboot          2   ❌        setpgid         3   ❌
recv            1   ❌        setpgrp         2   ❌
recvfrom        1   ❌        setpriority     2   ❌
recvmmsg        1   ❌        setregid        8   ✅
recvmsg         3   ❌        setresgid       8   ❌
remap_file_pages 2 ❌        setresuid      10   ❌
removexattr     2   ❌        setreuid       14   ✅
rename         13   ❌        setrlimit       6   ❌
renameat        3   ❌        set_robust_list 1   ✅
renameat2       0   ✅        setsid          1   ❌
request_key     5   ❌        setsockopt     10   ❌
rmdir           3   ❌        set_thread_area 1   ❌
rt_sigaction    3   ✅        set_tid_address 1   ✅
rt_sigprocmask  2   ✅        settimeofday    2   ❌
rt_sigqueueinfo 1   ❌        setuid          6   ✅
rt_sigsuspend   1   ❌        setxattr        3   ❌
rt_sigtimedwait 1   ⚠️        sgetmask        1   ❌
rt_tgsigqueueinfo 1 ⚠️        sigaction       2   ❌
sbrk            3   ✅        sigaltstack     2   ❌
sched_getaffinity 1 ❌        sighold         1   ❌
sched_getattr   2   ❌        signal          6   ❌
sched_getparam  2   ❌        signalfd        3   ❌
sched_get_priority_max 2 ❌    signalfd4       0   ❌
sched_get_priority_min 2 ❌    sigpending      1   ❌
sched_getscheduler 2 ❌       sigprocmask     1   ❌
sched_rr_get_interval 3 ❌     sigrelse        1   ❌
sched_setaffinity 1 ❌         sigsuspend      1   ❌
sched_setattr   1   ❌        sigtimedwait    1   ❌
sched_setparam  5   ❌        sigwait         1   ❌
sched_setscheduler 4 ❌       sigwaitinfo     1   ❌
sched_yield     1   ✅        socket          2   ❌
select          4   ❌        socketcall      3   ❌
send            2   ❌        socketpair      2   ❌
sendfile       16   ✅        sockioctl       1   ❌
sendmmsg        2   ❌        splice          9   ❌
sendmsg         3   ❌        ssetmask        1   ❌
sendto          3   ❌        stat            8   ❌
setdomainname   3   ❌        statfs          6   ❌
setegid         2   ❌        statvfs         2   ❌
setfsgid        6   ❌        statx          12   ✅
setfsuid        8   ❌        stime           2   ❌
setgid          6   ✅        string          1   ❌
setgroups       6   ❌        swapoff         2   ❌
sethostname     3   ❌        swapon          3   ❌
setitimer       2   ❌        switch          1   ❌
symlink         4   ❌        truncate        4   ❌
symlinkat       1   ❌        ulimit          1   ❌
sync            1   ❌        umask           1   ❌
sync_file_range 2   ❌        umount          5   ❌
syncfs          1   ❌        umount2         0   ✅
syscall         1   ❌        uname           3   ✅
sysconf         1   ❌        unlink          5   ❌
sysctl          3   ❌        unlinkat        1   ✅
sysfs           5   ❌        unshare         2   ❌
sysinfo         3   ❌        userfaultfd     1   ❌
syslog          2   ❌        ustat           2   ❌
tee             2   ❌        utime           7   ❌
tgkill          3   ✅        utimensat       1   ✅
time            1   ❌        utimes          1   ❌
timer_create    3   ⚠️        vfork           2   ❌
timer_delete    2   ❌        vhangup         2   ❌
timerfd         3   ❌        vmsplice        4   ❌
timer_getoverrun 1  ❌        wait            5   ❌
timer_gettime   1   ❌        wait4           3   ✅
timer_settime   3   ❌        waitid         11   ❌
times           2   ✅        waitpid        11   ✅
tkill           2   ✅        write           6   ✅
writev          6   ✅
```

---

## 五、快速索引：按功能分类统计

| 类别 | 目录数 | SpringOS 已实现 |
|------|:-----:|:--------------:|
| 进程管理 | 50 | 14 |
| 内存管理 | 28 | 7 |
| 文件操作 | 42 | 10 |
| 目录/路径操作 | 33 | 8 |
| 文件属性/元数据 | 27 | 2 |
| 扩展属性 (xattr) | 11 | 0 |
| epoll / 事件通知 | 18 | 1 |
| 信号 | 23 | 6 |
| 用户/组/权限 | 19 | 7 |
| 时间 | 18 | 2 |
| 网络 (Socket) | 25 | 0 |
| 管道 | 2 | 1 |
| IPC | 6 | 0 |
| 内核/系统信息 | 18 | 4 |
| 能力/安全 | 8 | 0 |
| AIO | 6 | 0 |
| io_uring | 1 | 0 |
| 其他杂项 | 21 | 0 |
| **总计** | **~356** | **37（+ 构建排除 4）** |

---

## 六、参考

- LTP 源码：`/home/chaos/testsuits-for-oskernel/ltp-full-20240524/`
- 测试用例清单：`runtest/syscalls`（1411 行）
- 测试源码目录：`testcases/kernel/syscalls/`（365 个目录）
- OSCOMP 构建脚本：`build-oscomb.sh`
- 评测脚本：`/home/chaos/autotest-for-oskernel/kernel/judge/judge_ltp-{glibc,musl}.py`
- SpringOS syscall 定义：`include/syscall/syscall.h`
