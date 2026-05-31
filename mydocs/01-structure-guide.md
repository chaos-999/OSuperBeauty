# SpringOS 文件结构与 xv6 对比分析

> 📅 生成时间：`2026-05-25 18:57 (GMT+8)`
>
> 目标：从一个刚学完 xv6 的学生的视角，解释 SpringOS 每个文件/文件夹是干嘛的

---

## 顶层目录对比

先摆标准 xv6 vs SpringOS 的顶层：

```
xv6-riscv                  SpringOS (my-springos)
━━━━━━━━━                  ━━━━━━━━━━━━━━━━━━━
boot/          ← 启动       boot/           ✓ 同名但内容大不同
kernel/        ← 内核       kernel/         ✓ 同名但目录结构改了
include/       ← 头文件     include/        ✓ 同名但目录结构改了
user/          ← 用户程序   user/           ✓ 同名但多了编译脚本

                            分析用脚本    analyze_log.sh
                            说明文档      basic.md, busybox.md, README.md
                            评测系统      judge_all.sh, judge.sh 等
                            硬件文档      loongson2K1000LA.md
                            syscall 清单  oscomp_syscalls.md
                            内核 ELF      kernel-rv + kernel-rv.asm + kernel-rv.sym
                            磁盘工具      mkfs/
                            启动脚本      tools/
                            网页端        site/
                            许可文件      LICENSE
```

你只要关心 **`boot/`、`kernel/`、`include/`、`user/`** 这四个 + `Makefile`。其他的都是比赛相关的附属品，暂时不用管。

---

## 每个额外文件是干嘛的

### 🗂 根目录的杂项文件

| 文件 | 干什么用 | 你要管吗 |
|------|---------|---------|
| `kernel-rv` | 编译好的 RISC-V 内核 ELF（1.5MB） | ✅ 最终产物 |
| `kernel-rv.asm` | 内核的反汇编，调试用的 | ❌ 不用管，自动生成 |
| `kernel-rv.sym` | 内核符号表，调试用的 | ❌ 不用管，自动生成 |
| `Makefile` | xv6 也有，但这里**巨大**（24856行） | ✅ 你要修改 |
| `README` | xv6 自带的英文 README，没改过 | ❌ |
| `README.md` | SpringOS 的说明 | ⭐ 建议读一遍 |
| `LICENSE` | GPL-2.0 许可 | ❌ |
| `.clang-format` | 代码格式化配置 | ❌ VSCode 用的 |
| `.editorconfig` | 编辑器配置 | ❌ |
| `.dir-locals.el` | Emacs 编辑器配置 | ❌ |
| `.gitignore` | Git 过滤规则 | ❌ |
| `.gdbinit.tmpl-riscv` | GDB 调试模板 | ⭐ 调试时有用 |

### 🏁 评测相关（比赛用的，你暂时不用管）

| 文件 | 干什么 |
|------|--------|
| `analyze_log.sh` | 分析评测日志 |
| `judge.sh` / `judge_all.sh` / `judge-offline.sh` / `judge-site.sh` / `judge-la.sh` | 各种评测脚本 |
| `.judge_tmp/` | 评测临时文件暂存 |
| `basic.md` | basic 测试说明文档 |
| `busybox.md` | busybox 测试说明文档 |
| `busybox_cmd.txt` | busybox 要执行的命令列表 |
| `busybox_testcode.sh` | busybox 的评测启动脚本 |
| `oscomp_syscalls.md` | **30个 syscall 的完整说明文档** ⭐ 建议读 |
| `op.txt` | 可能是操作记录 |

### 📚 硬件文档（龙芯相关，RISC-V 可以忽略）

| 文件 | 说明 |
|------|------|
| `loongarch_ABI.txt` | LoongArch 汇编 ABI 文档 |
| `loongson2K1000LA.md` | 龙芯 2K1000LA 处理器文档（0.5MB）|
| `mmexport.md` | 微信导出的资料 |

### 🗂 site/ — 网页版展示

```
site/
└── git_testcode.sh    # 网站相关的测试脚本
```

比赛时团队可以用来展示网页版介绍。**用不到。**

### 🗂 tools/ — 工具脚本

```
tools/
└── create_ramdisk.sh  # 创建 RAM 磁盘镜像的脚本
```

给 VF2 开发板用的，QEMU 用不上。**可以忽略。**

### 🗂 mkfs/ — 磁盘镜像工具

```
mkfs/
└── mkfs.c             # 制作 xv6 原始文件系统镜像的工具
```

这是 **xv6 自带的**。但 SpringOS 主要用 ext4 格式了，所以这个 `mkfs` 基本不用（比赛镜像用 `dd + mkfs.ext4` 做）。

---

## 核心目录对比（你要关注的部分）

### boot/ — 启动代码

```
xv6-riscv                      SpringOS
━━━━━━━━━                      ━━━━━━━━
boot/                           boot/
└── start.c                     ├── main.c         ← 重写了！多核启动 + 双架构
                                ├── rv/            ← RISC-V 专用
                                │   ├── entry.S    ← 入口，和 xv6 的 start.c 类似
                                │   ├── start.c    ← RISC-V 初始化
                                │   ├── initcode.S ← 第一个用户程序跳板
                                │   └── initcode-sh.S ← shell 版 initcode
                                ├── la/            ← LoongArch 专用（可忽略）
                                ├── vf2/           ← VisionFive2 专用（可忽略）
                                └── 2k1000/        ← 龙芯板子专用（可忽略）
```

**区别**：xv6 就一个 `start.c`，SpringOS 拆成了 `main.c` + 按架构分的子目录。你在 RISC-V QEMU 上跑，内核入口是 `boot/rv/entry.S`，初始化在 `boot/rv/start.c`，然后是 `boot/main.c`。**有兴趣可以看看 main.c，它完整画出了 RISC-V 的启动顺序**。

### kernel/ — 核心改动最大的地方

```
xv6-riscv                      SpringOS
━━━━━━━━━                      ━━━━━━━━
kernel/                         kernel/
├── console.c                   ├── console.c  ← 搬到 kernel/fs/ 了
├── exec.c                      ├── exec.c     ← 搬到 kernel/proc/ 了
├── file.c                      ├── …… 所有 .c 都按功能分了类
├── fs.c                        
├── kalloc.c                    
├── pipe.c                      
├── proc.c                      
├── spinlock.c                  
├── string.c                    ├── util/string.c  ← 搬到 util 子目录
├── swtch.S                     ├── (在 hal/riscv/ 里)
├── syscall.c                   ├── syscall/syscall.c
├── syscall.h                   ├── (在 include/syscall/syscall.h)
├── trap.c                      ├── trap/rv/trap.c  ← 搬到子目录
├── uart.c                      ├── drive/uart.c  ← 搬到子目录
├── virtio_disk.c               ├── drive/rv/virtio_disk.c  ← 搬到子目录
├── vm.c                        ├── mm/vm.c  ← 搬到子目录
                                │
                                ├── proc/    ← 进程管理
                                │   ├── proc.c   ← 你熟悉的，但加了 clone
                                │   └── exec.c
                                ├── mm/      ← 内存管理
                                │   ├── vm.c         ← 页表管理
                                │   ├── kalloc.c     ← 物理页分配
                                │   └── buddy.c      ← ★ 新东西！伙伴分配器
                                ├── syscall/ ← 系统调用
                                │   ├── syscall.c    ← syscall 分发
                                │   ├── sysproc.c    ← 进程类 syscall（fork/exit/brk...）
                                │   ├── sysfile.c    ← 文件类 syscall（open/read/write...）
                                │   └── syssig.c    ← ★ 新！信号系统调用
                                ├── trap/    ← 陷入处理
                                │   ├── rv/trap.c    ← RISC-V 的 trap
                                │   ├── signal.c     ← ★ 新！信号递送
                                │   └── la/trap.c    ← LoongArch 忽略
                                ├── fs/      ← ★ 新！文件系统子系统
                                │   ├── bio.c        ← 块缓存（xv6 也有）
                                │   ├── console.c    ← 控制台
                                │   ├── pipe.c       ← 管道
                                │   ├── VFS_block.c  ← ★ 新！VFS 块设备桥梁
                                │   ├── VFS_ext.c    ← ★ 新！ext4 文件系统操作
                                │   └── ramdisk.c    ← ★ 新！内存磁盘（VF2 用）
                                ├── drive/   ← ★ 新！设备驱动
                                │   ├── uart.c
                                │   ├── rv/virtio_disk.c  ← QEMU 磁盘
                                │   ├── rv/plic.c         ← 中断控制器
                                │   ├── la/*              ← LoongArch 忽略
                                ├── lock/    ← 锁
                                │   ├── spinlock.c
                                │   └── sleeplock.c
                                ├── util/    ← ★ 新！工具函数集
                                │   ├── printf.c
                                │   ├── string.c
                                │   ├── qsort.c  ← 快速排序
                                │   └── futex.c   ← ★ 新！futex 同步
                                ├── rv/      ← RISC-V 链接脚本 kernel.ld
                                ├── la/      ← LoongArch 链接脚本（忽略）
                                ├── vf2/     ← VF2 链接脚本（忽略）
                                └── 2k1000/  ← 龙芯板链接脚本（忽略）
```

**一句话：** xv6 所有 .c 文件平铺在 kernel/ 里；SpringOS 把它们按功能扔进了子目录（`proc/`、`mm/`、`syscall/`、`trap/`、`fs/`、`drive/`、`util/`）。

### include/ — 头文件也按模块分了

```
xv6-riscv                      SpringOS
━━━━━━━━━                      ━━━━━━━━
kernel/syscall.h   →            include/syscall/syscall.h
param.h            →            include/param.h      ← 基本一样
memlayout.h        →            include/memlayout.h
types.h            →            include/types.h
defs.h             →            include/defs.h        ← ★ 变大了很多
                                
                                ├── include/fs/       ← 文件系统头文件
                                │   ├── buf.h
                                │   ├── fcntl.h
                                │   ├── stat.h
                                │   └── vfs_*.h       ← VFS 接口
                                ├── include/proc/     ← 进程结构定义
                                │   └── proc.h        ← ★ 你的 proc 加了新字段
                                ├── include/lock/     ← 锁的定义
                                ├── include/syscall/  ← syscall 编号
                                ├── include/board/    ← 板级支持
                                ├── include/dev/      ← 设备驱动
                                ├── include/lib/      ← 库函数（sbi等）
                                ├── signal.h          ← ★ 全新！信号系统
                                ├── futex.h           ← ★ 全新！futex
                                ├── errno.h           ← ★ 全新！错误码
                                ├── mem.h             ← ★ 全新！mmap 相关宏
                                ├── poll.h            ← ★ 全新！poll
                                └── time.h            ← ★ 全新！时间结构体
```

**你主要看的头文件：**
- `include/defs.h` — 函数声明大全，对应 xv6 的 `defs.h`
- `include/param.h` — 常量参数，和 xv6 一样
- `include/syscall/syscall.h` — syscall 编号
- `include/proc/proc.h` — 进程结构体（比 xv6 多了一大堆字段）
- `include/memlayout.h` — 内存布局

### user/ — 用户程序

和 xv6 一样，多了几个用于测试的程序：
- `sigtest.c` — 信号测试
- `pptest.c` — 优先级测试
- `futex.c` — futex 测试
- `simple.c` — 简单测试
- `new.c` — 新测试
- `sendtest.c` — 发送测试
- `usys-rv.pl` / `usys-la.pl` — **perl 脚本**，自动生成 syscall 的跳转汇编

---

## 你真正要读的文件（优先级排序）

从 xv6 出发，我建议你只看这几个文件就能理解 80%：

```
⭐⭐⭐ MUST READ
├── boot/main.c           ← 启动顺序，看懂了就知道内核怎么开始跑的
├── include/defs.h        ← 所有函数的声明，地图
├── include/proc/proc.h   ← 进程结构体（对比 xv6 的 proc.h 看差异）
├── include/syscall/syscall.h  ← syscall 编号
├── kernel/syscall/syscall.c   ← syscall 分发
├── kernel/proc/proc.c         ← 进程管理（fork/clone/exit）
├── kernel/mm/vm.c             ← 内存管理（mmap 相关）
├── Makefile              ← 看 qemu 和 kernel-rv 目标怎么工作

⭐⭐ READ IF INTERESTED
├── kernel/syscall/sysproc.c   ← 进程类 syscall
├── kernel/syscall/sysfile.c   ← 文件类 syscall
├── kernel/fs/VFS_ext.c        ← ext4 文件系统操作
├── boot/rv/entry.S            ← 入口汇编
├── kernel/trap/rv/trap.c      ← 异常处理

⭐ REFERENCE
├── kernel/mm/buddy.c          ← 伙伴分配器
├── kernel/trap/signal.c       ← 信号系统
├── kernel/util/futex.c        ← futex
├── user/*                     ← 用户程序
```

---

## 快速定位 xv6 → SpringOS

| xv6 文件 | SpringOS 位置 |
|---------|--------------|
| `kernel/start.c` | `boot/main.c` |
| `kernel/syscall.c` | `kernel/syscall/syscall.c` |
| `kernel/syscall.h` | `include/syscall/syscall.h` |
| `kernel/trap.c` | `kernel/trap/rv/trap.c` |
| `kernel/proc.c` | `kernel/proc/proc.c` |
| `kernel/exec.c` | `kernel/proc/exec.c` |
| `kernel/kalloc.c` | `kernel/mm/kalloc.c` |
| `kernel/vm.c` | `kernel/mm/vm.c` |
| `kernel/uart.c` | `kernel/drive/uart.c` |
| `kernel/virtio_disk.c` | `kernel/drive/rv/virtio_disk.c` |
| `kernel/file.c` | 被 ext4 VFS 替代，在 `kernel/fs/VFS_*.c` + `include/fs/` |
| `kernel/fs.c` | 被 ext4 替代 |
| `kernel/bio.c` | `kernel/fs/bio.c`（基本一样） |
| `kernel/console.c` | `kernel/fs/console.c` |
| `kernel/pipe.c` | `kernel/fs/pipe.c` |
| `kernel/printf.c` | `kernel/util/printf.c` |
| `kernel/string.c` | `kernel/util/string.c` |
| `kernel/spinlock.c` | `kernel/lock/spinlock.c` |
| `kernel/swtch.S` | `hal/riscv/swtch.S`（不在 kernel/ 下） |
