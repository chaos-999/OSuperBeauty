# 操作系统大赛：从 xv6 出发的参赛入门指南

> 写给刚刚学完 xv6、想参加操作系统大赛但不知从何下手的你

> 📅 文档生成时间：`2026-05-25 18:29 (GMT+8)` — 原始版本
> 📅 文档更新时间：`2026-05-25 18:41 (GMT+8)` — 补充

## 一、比赛到底要你做什么？

**一句话：基于 xv6 增加系统调用，让你的内核能运行比赛提供的测试程序。**

比赛评测时，会给你的内核挂载一个 ext4 格式的磁盘镜像，里面有预先编译好的测试程序（叫 `busybox` 或 `test_brk`、`test_fork` 等）。你的内核要能：

1. 成功引导（从 OpenSBI 跳转到内核入口）
2. 挂载 ext4 文件系统
3. 读取测试程序文件 → 加载到内存 → 执行
4. 测试程序通过系统调用请求内核服务（读写文件、创建进程等）
5. 内核正确实现这些 syscall，测试程序打印出预期结果

**评测机抓取你的终端输出**，用正则表达式匹配关键字来打分。比如：

```
========== START test_brk ==========
Before alloc,heap pos: 0
After alloc,heap pos: 64
Alloc again,heap pos: 128
========== END test_brk ==========
```

评测脚本看到这些输出，就给你 brk 测试的 3 分。

---

## 二、basic 测试涉及 30 个系统调用

比赛有 12 组测试题（basic、busybox、lua、libctest 等），其中 **basic 是最基础的**，只测 syscall，共 30 个：

| 编号 | 名字 | xv6 有吗 | 难度 | 说明 |
|------|------|----------|------|------|
| 17 | getcwd | ✅ 有 | ★ | 获取当前工作目录 |
| 59 | pipe2 | ✅ 有 pipe | ★★ | 创建管道，flags 参数 |
| 23 | dup | ✅ 有 | ★ | 复制文件描述符 |
| 24 | dup3 | ❌ 没有 | ★★ | 复制到指定 fd |
| 49 | chdir | ✅ 有 | ★ | 切换工作目录 |
| 56 | openat | ❌ 没有 | ★★ | 相对路径打开文件 |
| 57 | close | ✅ 有 | ★ | 关闭文件 |
| 61 | getdents64 | ❌ 没有 | ★★★ | 读取目录条目 |
| 63 | read | ✅ 有 | ★ | 读文件 |
| 64 | write | ✅ 有 | ★ | 写文件 |
| 37 | linkat | ❌ 没有 | ★★ | 创建硬链接 |
| 35 | unlinkat | ❌ 没有 | ★★ | 删除文件/目录 |
| 34 | mkdirat | ❌ 没有 | ★★ | 创建目录 |
| 39 | umount2 | ❌ 没有 | ★★★ | 卸载文件系统 |
| 40 | mount | ❌ 没有 | ★★★ | 挂载文件系统 |
| 80 | fstat | ✅ 有 | ★ | 获取文件信息 |
| 220 | clone | ❌ 没有 | ★★★★ | 创建线程/进程 |
| 221 | execve | ✅ 有 exec | ★★ | 执行程序 |
| 260 | wait4 | ❌ 没有 | ★★ | 等待子进程 |
| 93 | exit | ✅ 有 | ★ | 退出进程 |
| 173 | getppid | ✅ 有 | ★ | 获取父进程 PID |
| 172 | getpid | ✅ 有 | ★ | 获取进程 PID |
| 214 | brk | ❌ 没有 | ★★ | 调整堆大小 |
| 215 | munmap | ❌ 没有 | ★★★ | 解除内存映射 |
| 222 | mmap | ❌ 没有 | ★★★★ | 内存映射文件 |
| 153 | times | ❌ 没有 | ★★ | 获取进程时间 |
| 160 | uname | ❌ 没有 | ★ | 获取系统信息 |
| 124 | sched_yield | ✅ 有 yield | ★ | 主动让出 CPU |
| 169 | gettimeofday | ❌ 没有 | ★★ | 获取时间 |
| 101 | nanosleep | ❌ 没有 | ★★ | 高精度睡眠 |

---

## 三、你的工作其实就是这四步

### 第 1 步：让内核识别 Linux 编号的 syscall

xv6 原本的 syscall 编号是自己随便定的（比如 `SYS_fork = 1`），但比赛的测试程序用的是 **Linux 的编号**（比如 `SYS_exit = 93`、`SYS_write = 64`）。

**改什么：** `kernel/syscall.h` 和 `kernel/syscall.c`

把 xv6 自带的编号全部改成比赛要求的 Linux 编号。比如：

```c
// xv6 原来的：
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4

// 比赛要求的 Linux 编号：
#define SYS_fork    1       // 同名
#define SYS_exit    93      // 变了！
#define SYS_read    63      // xv6 是 5
#define SYS_write   64      // xv6 是 16
// ... 全部 30 个按 oscomp_syscalls.md 来
```

然后在 `syscall.c` 的 `syscalls[]` 数组里注册对应的处理函数：

```c
static uint64 (*syscalls[])(void) = {
    [SYS_fork]     sys_fork,
    [SYS_exit]     sys_exit,
    [SYS_read]     sys_read,
    [SYS_write]    sys_write,
    // ... 全部
};
```

> ⚠️ 最关键的一点：**编号对标 Linux** 才能让测试程序找到你的 syscall。

### 第 2 步：xv6 已有的 syscall 微调适配

约 15 个 syscall xv6 本身已经实现，但接口需要微调：

**例如 `exec` → `execve`：**
- xv6 有 `sys_exec(void)`，功能就是执行程序
- 比赛测试程序调的是 `SYS_execve (221)`，功能完全一样
- 只需在 `syscall.h` 加 `#define SYS_execve 221`，然后在 `syscall.c` 注册 `sys_exec` 即可

**例如 `sleep` → `nanosleep`：**
- xv6 的 `sys_sleep` 参数是：`sleep(int ticks)`
- 比赛测试调 `SYS_nanosleep (101)`，参数是 `timespec` 结构体
- 需要封装一层：从 `timespec` 算出 ticks，再调用 xv6 的 sleep 逻辑

**例如 `sbrk` → `brk`：**
- xv6 的 `sys_sbrk` 参数是增量：`sbrk(int increment)`
- 比赛测试调 `SYS_brk (214)`，参数是绝对地址：`brk(void *addr)`
- 需要 xv6 记录当前堆顶，根据绝对地址计算增量 → 调用 growproc

### 第 3 步：没有的 syscall 要新增实现

约 15 个是 xv6 没有的，需要你写代码。按难度排序：

**简单新实现（1-2天）：**
- `gettimeofday` — 从时钟中断计数器读出时间，转换
- `uname` — 返回固定的系统信息字符串
- `times` — 统计进程在用户态/内核态的运行时间
- `dup3` — 在 `dup` 的基础上指定新 fd 编号

**中等难度（1周）：**
- `openat` — xv6 的 `open` 只支持绝对路径改造成支持相对路径
- `linkat` / `unlinkat` / `mkdirat` — xv6 已有类似功能，加 `at` 系列参数解析
- `getdents64` — 读取目录内容，返回 `struct dirent` 结构体
- `mount` / `umount2` — 文件系统挂载/卸载（需要 VFS 或至少支持 ext4）
- `wait4` / `waitpid` — xv6 只有 `wait`，需要扩展

**困难（需要重新理解 xv6 架构）：**
- `clone` — 创建线程，支持 `CLONE_VM` 等 flag
- `mmap` / `munmap` — 内存映射文件，需要 VMA (Virtual Memory Area)
- `brk` — 其实就是 sbrk 的另一种形式，但需要处理好

### 第 4 步：文件系统（最大的坎）

测试程序以 **ext4 格式的磁盘镜像**提供。但 xv6 自带的是自己头号的文件系统（叫 logging FS）。

你有两个选择：

**选择 A：集成 lwext4（推荐，参考 SpringOS）**
把 lwext4 库移植到 xv6 中，替换掉 xv6 的原始文件系统。这是大赛大多数队伍的做法。

需要写的代码：
- `bio.c` 适配：把 xv6 的块缓存桥接到 lwext4 的块设备接口
- `fs.c` 替换：用 ext4 的读写函数替代 xv6 的 `readi`/`writei`
- `VFS` 层：至少让 `open`、`read`、`write`、`getdents` 这几个 syscall 能通过 ext4 操作文件

**选择 B：只做 syscall 兼容层（适合临时测试）**
用 xv6 的块设备驱动直接读取 ext4 镜像。但 ext4 格式复杂，这基本等于自己实现一个 ext4 解析器——比选择 A 更累。

> **建议：** 直接参考 SpringOS 的 lwext4 集成方式。代码在上一轮分析的 `kernel/fs/lwext4/` 和 `kernel/fs/VFS_*.c` 里。

---

## 四、比赛测试怎么跑？（本地运行）

### 你需要准备的

```
你的 xv6 项目/
├── kernel/            ← 你修改后的内核代码
├── Makefile
├── kernel-rv          ← 编译生成的内核 ELF
├── sdcard.img         ← ext4 格式磁盘镜像（放测试程序）
└── hal/riscv/SBI/     ← OpenSBI 或其他 SBI 实现
```

### 测试全流程

```
┌────────────────────────────────────────────────────────┐
│                                                        │
│   1. dd if=/dev/zero of=sdcard.img bs=1M count=256     │
│      mkfs.ext4 sdcard.img                              │
│      → 创建空的 ext4 磁盘镜像                           │
│                                                        │
│   2. sudo mount sdcard.img /mnt                        │
│      sudo cp -r test_programs/* /mnt/                  │
│      sudo umount /mnt                                  │
│      → 把测试程序复制进镜像                              │
│                                                        │
│   3. qemu-system-riscv64 ... -drive file=sdcard.img    │
│      → 启动 QEMU，挂载镜像                               │
│                                                        │
│   4. 内核启动 → 挂载 ext4 → 找到 /mnt/busybox           │
│      执行 busybox → busybox 调用你的 syscall            │
│      → 你处理 syscall → 测试程序打印结果 ✅              │
│                                                        │
│   5. 评测机抓取串口输出 → judge_basic.py 比对           │
│      → 输出 JSON 评分结果                               │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### 跑 basic 测试的具体命令

```bash
# 1. 创建空镜像
dd if=/dev/zero of=test.img bs=1M count=128
mkfs.ext4 test.img

# 2. 复制测试程序（需要从测试套件仓库获取编译好的二进制）
sudo mount test.img /mnt
sudo cp -r oskernel-testsuits-cooperation/basic/build/riscv64/* /mnt/
sudo cp oskernel-testsuits-cooperation/basic/basic_testcode.sh /mnt/
sudo umount /mnt

# 3. 运行 QEMU
qemu-system-riscv64 -machine virt -kernel kernel-rv -m 1G -nographic \
  -bios path/to/opensbi \
  -drive file=test.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
```

### 看懂测试输出

测试程序会输出**带标记的文本**：

```
#### OS COMP TEST GROUP START basic-musl ####   ← 测试开始标记
========== START test_brk ==========              ← 单个测试点开始
Before alloc,heap pos: 0
After alloc,heap pos: 64
Alloc again,heap pos: 128
========== END test_brk ==========                ← 单个测试点结束
========== START test_fork ==========
  child process
  parent process. wstatus:0
========== END test_fork ==========
#### OS COMP TEST GROUP END basic-musl ####     ← 测试结束标记
```

评测脚本 `judge_basic.py` 会：
1. 找到 `START test_xxx` 和 `END test_xxx` 之间的输出
2. 用正则表达式匹配关键字（比如 `heap pos: 64`、`wstatus:0`）
3. 匹配到了就给分，匹配不到就 0 分

---

## 五、从 xv6 出发的具体改法（手把手）

### 5.1 先把你已有的 xv6 跑起来

```bash
# 克隆原始 xv6
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv

# 编译运行
make qemu   # 如果能进 shell，说明基础环境没问题
```

### 5.2 改造 syscall 编号（推荐第一步做）

打开 `kernel/syscall.h`，把 xv6 的编号全部改成 Linux 编号：

```c
// kernel/syscall.h — 全部替换
#define SYS_getcwd      17
#define SYS_dup          23
#define SYS_dup3         24
#define SYS_chdir        49
#define SYS_openat       56
#define SYS_close        57
#define SYS_getdents64   61
#define SYS_read         63
#define SYS_write        64
#define SYS_linkat       37
#define SYS_unlinkat     35
#define SYS_mkdirat      34
#define SYS_fstat        80
#define SYS_clone       220
#define SYS_execve      221
#define SYS_wait4       260
#define SYS_exit         93
#define SYS_getppid     173
#define SYS_getpid      172
#define SYS_brk         214
#define SYS_munmap      215
#define SYS_mmap        222
#define SYS_times       153
#define SYS_uname       160
#define SYS_sched_yield 124
#define SYS_gettimeofday 169
#define SYS_nanosleep   101
#define SYS_mount        40
#define SYS_umount2      39
#define SYS_pipe2        59

// 这些 xv6 有的但编号不同
#define SYS_fork          1
#define SYS_wait          3   // 兼容，但比赛主要用 wait4
#define SYS_kill          6   // 比赛没要求但留着
#define SYS_sbrk         12   // 留着兼容
#define SYS_sleep        13   // khs留着
#define SYS_uptime       14   // 留着
#define SYS_mknod        16   // 留着
```

### 5.3 改造 syscall.c

打开 `kernel/syscall.c`，注册新的 syscall 函数：

```c
static uint64 (*syscalls[])(void) = {
    [SYS_fork]      sys_fork,
    [SYS_exit]      sys_exit,
    [SYS_wait]      sys_wait,
    [SYS_pipe2]     sys_pipe2,       // 把 xv6 的 pipe 函数绑到新编号
    [SYS_read]      sys_read,
    [SYS_kill]      sys_kill,
    [SYS_execve]    sys_execve,      // 复用 xv6 的 sys_exec
    [SYS_fstat]     sys_fstat,
    [SYS_chdir]     sys_chdir,
    [SYS_dup]       sys_dup,
    [SYS_getpid]    sys_getpid,
    [SYS_getppid]   sys_getppid,     // 新加
    [SYS_sbrk]      sys_sbrk,        // 保留，brk 也用到
    [SYS_sleep]     sys_sleep,
    [SYS_uptime]    sys_uptime,
    [SYS_openat]    sys_openat,      // 新实现
    [SYS_write]     sys_write,
    [SYS_close]     sys_close,
    [SYS_pipe2]     sys_pipe2,
    [SYS_getcwd]    sys_getcwd,
    [SYS_getdents64] sys_getdents64, // 新实现
    [SYS_brk]       sys_brk,         // 新实现
    [SYS_clone]     sys_clone,       // 新实现
    [SYS_mmap]      sys_mmap,        // 新实现
    [SYS_munmap]    sys_munmap,      // 新实现
    [SYS_times]     sys_times,       // 新实现
    [SYS_uname]     sys_uname,       // 新实现
    [SYS_sched_yield] sys_yield,
    [SYS_gettimeofday] sys_gettimeofday,  // 新实现
    [SYS_nanosleep] sys_nanosleep,        // 新实现
    [SYS_mount]     sys_mount,            // 新实现
    [SYS_umount2]   sys_umount2,          // 新实现
    [SYS_wait4]     sys_wait4,            // 新实现
    [SYS_linkat]    sys_linkat,           // 新实现
    [SYS_unlinkat]  sys_unlinkat,         // 新实现
    [SYS_mkdirat]   sys_mkdirat,          // 新实现
    [SYS_dup3]      sys_dup3,             // 新实现
};
```

### 5.4 按需要实现新 syscall

**示例：sys_uname（实现最简单，适合入门练习）**

```c
// kernel/sysproc.c 中新增

struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
};

uint64 sys_uname(void) {
    uint64 buf;
    struct utsname uts;
    char *p = (char*)&uts;

    argaddr(0, &buf);

    memset(&uts, 0, sizeof(uts));
    safestrcpy(uts.sysname, "xv6", 65);
    safestrcpy(uts.nodename, "localhost", 65);
    safestrcpy(uts.release, "6.0.0", 65);
    safestrcpy(uts.version, "#1 SMP", 65);
    safestrcpy(uts.machine, "riscv64", 65);

    if (copyout(myproc()->pagetable, buf, (char*)&uts, sizeof(uts)) < 0)
        return -1;
    return 0;
}
```

**示例：sys_brk（堆内存管理）**

```c
// xv6 的 sbrk 是按增量调整堆
// 但 brk 是按绝对地址调整

extern uint64 curr_break;  // 当前堆顶

uint64 sys_brk(void) {
    uint64 addr;
    argaddr(0, &addr);

    struct proc *p = myproc();

    if (addr == 0) {
        // 查询当前堆位置
        return curr_break;
    }

    if (addr >= p->sz) {
        return -1;  // 越界
    }

    if (addr > curr_break) {
        // 扩大堆
        int diff = addr - curr_break;
        if (growproc(diff) < 0)
            return -1;
    } else if (addr < curr_break) {
        // 缩小堆（可选）
        int diff = curr_break - addr;
        // growproc 负值会释放内存
        if (growproc(-diff) < 0)
            return -1;
    }

    curr_break = addr;
    return 0;
}
```

### 5.5 文件系统是最大工程

**关键在于 ext4 的读写。**你有两种办法：

**方式 1：加入 lwext4（推荐）**

把 SpringOS 的 `kernel/fs/lwext4/` 目录和 `include/fs/ext4/` 头文件复制到你的 xv6 中，然后写一个桥接层：

```c
// 新建 kernel/ext4_bridge.c
// 作用：让 xv6 的读盘函数能喂给 lwext4

int ext4_block_read(uint64 sector, void *buffer) {
    // 直接调用 xv6 的 virtio_disk_rw
    struct buf b;
    memset(&b, 0, sizeof(b));
    b.dev = ROOTDEV;
    b.blockno = sector / (BSIZE / 512);  // 转换
    virtio_disk_rw(&b, 0);
    memmove(buffer, b.data, BSIZE);
    return 0;
}
```

**方式 2：不走 ext4，只做 syscall 兼容**
如果你暂时只在 QEMU 里用自己的文件系统测试，那可以先用 xv6 自己的文件系统把 syscall 逻辑调通，ext4 留到后面做。

---

## 六、常见问题（FAQ）

### Q：我不知道哪些 syscall 要改，怎么办？

**A：**先把 `basic_testcode.sh` 下载到本地，看看它调用了什么测试程序。然后看 `judge_basic.py` 里的正则表达式——它搜什么关键字，你的内核就要输出什么。这是最直观的需求列表。

### Q：我改了 syscall.h，但程序没有进入我的新 syscall？

**A：**检查两件事：① `syscall.c` 中的 `syscalls[]` 数组有没有注册；② 系统调用编号是否和 `a7` 寄存器传进来的数值一致。可以在 `syscall()` 函数里加一行 `printf("syscall num=%d\n", num)` 来调试。

### Q：测试程序说 `execve: cannot find file`？

**A：**说明你的内核成功启动了，挂载了 ext4，但在指定的路径（比如 `/mnt/busybox`）找不到文件。排查：① 磁盘镜像是否真的有 busybox 二进制？② 你的 ext4 驱动能否正确读取目录？

### Q：我改完 syscall 后 xv6 编译不过了？

**A：**常见原因：① `syscall.h` 的编号被 xv6 自己的 syscall 函数内部用了（比如 `SYS_link` 原本是 12，你改成 37 了，但代码里其他地方用到了）；② 有些 syscall 名字变了（比如 xv6 的 `argint` 和比赛的不兼容）。

### Q：所以比赛其实就是像 xv6 前几个实验一样实现一些 syscall 就行？

**对，但不全对。** 看阶段：

**basic 测试确实就是** —— 30 个 syscall 的实现，跟 xv6 Lab 2-4 差不多。区别只是 xv6 实验有自己的测试框架（`usertests`），比赛用的是 Linux 编号的 syscall + 外部测试程序。

**但后面还有 11 组测试**，难度逐步上升：

| 测试组 | 考什么 | 难度 | 需要什么 |
|--------|--------|------|----------|
| basic | 30 个基础 syscall | ★★ | 基本 syscall 实现 |
| busybox | 运行 busybox（ls/cat/sh/echo 等） | ★★★ | 完整的文件、进程、信号、终端支持 |
| libctest | C 标准库兼容性 | ★★★ | mmap、brk、完整文件 syscall |
| lua | 运行 Lua 解释器 | ★★★★ | syscall 基本全覆盖 + mmap 稳定 |
| lmbench | 系统性能基准 | ★★★★ | 高效 syscall + 精确计时 |
| Unixbench | 综合性能测试 | ★★★★ | 完整 POSIX 环境 |
| libcbench | libc 函数基准 | ★★★ | 同上 |
| cyclictest | 实时性/中断延迟 | ★★★★ | 精确时钟 + 中断处理 |
| iozone | 文件系统性能 | ★★★★★ | 完善的 ext4 驱动 |
| iperf | 网络性能 | ★★★★★★ | **网络栈**（最难） |
| netperf | 网络性能 | ★★★★★★ | **网络栈** |
| LTP | Linux Test Project 全量测试 | ★★★★★ | 几百个 syscall 全覆盖 |

**建议的路线：**

1. ✅ **先通过 basic** —— 这是入场券，证明你的内核能跑 syscall
2. ✅ **再通过 busybox** —— 证明你能跑实际应用程序（ls、cat、sh）
3. ✅ **然后 lua + libctest** —— 证明你的 syscall 覆盖足够完整
4. ❌ **网络组先不管** —— iperf/netperf 需要完整 TCP/IP 协议栈，是最难的两组

**一句话：** basic 就是 xv6 Lab 的升级版，后面的才是比赛真正的区分度所在。

### Q：每次都重新编内核和做镜像太慢了？

**A：**写一个脚本：

```bash
#!/bin/bash
make -j4 && \
dd if=/dev/zero of=test.img bs=1M count=128 2>/dev/null && \
mkfs.ext4 -F test.img 2>/dev/null && \
sudo mount test.img /mnt && \
sudo cp -r build/* /mnt/ && \
sudo umount /mnt && \
qemu-system-riscv64 -machine virt -kernel kernel -m 1G -nographic \
  -bios path/to/opensbi \
  -drive file=test.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0
```

---

## 七、推荐学习路线

```
第 1 周：环境搭建
├── xv6-riscv 编译通过，make qemu 能进 shell
├── 安装 riscv64-linux-gnu-gcc + g++ 交叉工具链
├── 学会用 QEMU 调试（-s -S + gdb）
└── 理解 xv6 的 Makefile 结构

第 2 周：syscall 编号改造
├── 把 syscall.h 全部改成 Linux 编号
├── 修改 syscall.c 注册新函数（先用 stub 返回 0）
├── 测试：加 printf 看测试程序是否调用了你的 syscall
└── 实现 uname、getpid、getppid、times（最简单的4个）

第 3-4 周：文件系统
├── 理解 xv6 的 virtio_disk 读写
├── 集成 lwext4（复制 SpringOS 的 ext4 代码）
├── 桥接层：virtio_disk → lwext4
└── 验证：能挂载 ext4 镜像，读取目录

第 5-6 周：基本 syscall 实现
├── 文件类：openat, read, write, close, getdents64
├── 进程类：fork, execve, exit, wait4, clone
├── 内存类：brk, mmap, munmap
├── 目录类：chdir, getcwd, mkdirat, linkat, unlinkat
└── 时间类：gettimeofday, nanosleep

第 7-8 周：调试 & 通过 basic 测试
├── 反复运行 basic 测试，看哪个测试点失败
├── 根据 judge_basic.py 的正则改代码
├── 通过 basic 全部测试点
└── 开始下一个测试组（busybox 或 libctest）
```

---

## 八、参考项目

比赛结束后，可以借鉴这两个项目的代码（他们都通过了大部分测试）：

- **SpringOS**（静春山·中山大学）— `/home/chaos/T202510558995330-264-main/`
  - C 语言编写，代码结构清晰
  - 有完善的 ext4 + VFS 实现
  - 约 90 个 syscall

- **RuOK**（RuOK队）— `/home/chaos/T202510486995232-2402/`
  - C++ 语言，面向对象设计
  - 模块化的硬件抽象层（HSAI）
  - 微内核风格架构

---

> **最后的话：** 别被 30 个 syscall 吓到。其中一半 xv6 已经有或略作修改就行。真正新的也就 mmap、clone、ext4 这三个难点。按顺序一个一个来，每一步都能在 QEMU 上看到效果——这是 xv6 最大的好处。加油 💪
