# SpringOS 完整文件/目录一览表

> 📅 生成时间：`2026-05-31 18:22 (GMT+8)`
>
> 目标：列出 `my-springos/` 下所有文件及文件夹的内容概要，方便快速查阅定位

---

## 📂 顶层目录总览

```
my-springos/                        ← 项目根目录
├── boot/                           ← 启动代码（多架构入口）
├── kernel/                         ← 内核源码（按模块组织）
├── include/                        ← 头文件（按模块组织）
├── user/                           ← 用户态程序
├── mkfs/                           ← 文件系统镜像制作工具
├── tools/                          ← 辅助工具脚本
├── site/                           ← 网页端测试脚本
├── .judge_tmp/                     ← 评测临时文件
├── mydocs/                         ← 学习文档
│
├── Makefile                        ← 主构建文件（~620行）
├── README.md                       ← SpringOS 项目说明
├── README                          ← xv6 原始英文 README
├── LICENSE                         ← GPL-2.0 许可证
│
├── kernel-rv                       ← RISC-V 编译产物：ELF
├── kernel-rv.asm                   ← RISC-V 编译产物：反汇编
├── kernel-rv.sym                   ← RISC-V 编译产物：符号表
│
├── .clang-format                   ← 代码格式化配置
├── .editorconfig                   ← 编辑器配置
├── .dir-locals.el                  ← Emacs 编辑器配置
├── .gitignore                      ← Git 忽略规则
├── .gdbinit.tmpl-riscv             ← GDB 调试模板
│
├── analyze_log.sh                  ← 日志分析脚本
├── judge_all.sh                    ← 综合评测脚本（全测试组）
├── judge.sh                        ← 评测脚本（基础版）
├── judge-offline.sh                ← 离线评测脚本
├── judge-site.sh                   ← 现场赛评测脚本
├── judge-la.sh                     ← LoongArch 评测脚本
├── busybox_testcode.sh             ← busybox 测试启动脚本
├── basic.md                        ← basic 测试说明文档
├── busybox.md                      ← busybox 测试说明文档
├── busybox_cmd.txt                 ← busybox 执行命令列表
├── oscomp_syscalls.md              ← 30+ syscall 完整说明文档
├── loongarch_ABI.txt               ← LoongArch 汇编 ABI
├── loongson2K1000LA.md             ← 龙芯 2K1000LA 处理器手册
├── mmexport.md                     ← 微信导出资料（龙芯异常表）
├── op.txt                          ← 操作记录（内存地址列表）
```

### 根目录文件详情

| 路径 | 类型 | 大小 | 用途概述 |
|------|------|------|---------|
| `Makefile` | 构建文件 | ~620行 | 支持 `make qemu/clean/all/qemu-la/vf2/2k1000/qemu-sh/qemu-gdb` 等目标；按架构（RV/LA/VF2）分别编译链接 |
| `README.md` | 说明文档 | ~4KB | SpringOS 项目介绍：队伍「静春山」、特性、构建方式、参考项目、参考文献 |
| `README` | 说明文档 | ~2KB | xv6 原始英文 README（MIT 6.1810），保留未修改 |
| `LICENSE` | 许可证 | — | GPL-2.0（因 lwext4/ext4 组件而采用） |
| `kernel-rv` | 二进制 | ~1.5MB | RISC-V QEMU 编译生成的最终内核 ELF，`make qemu` 产物 |
| `kernel-rv.asm` | 文本 | — | 内核反汇编，`objdump` 自动生成，调试用 |
| `kernel-rv.sym` | 文本 | — | 内核符号表，调试用 |
| `.clang-format` | 配置 | — | C/C++ 代码格式化规则 |
| `.editorconfig` | 配置 | — | 跨编辑器缩进/编码统一配置 |
| `.dir-locals.el` | 配置 | — | Emacs 项目级本地变量 |
| `.gitignore` | 配置 | — | Git 忽略 `*.o` `*.d` `kernel-rv*` 等编译产物 |
| `.gdbinit.tmpl-riscv` | 配置 | — | RISC-V GDB 连接 QEMU 调试模板 |
| `analyze_log.sh` | Shell 脚本 | ~270行 | 分析 `qemu_all.log` 日志，调用 `judge_basic.py` 等评测脚本输出成绩 |
| `judge_all.sh` | Shell 脚本 | ~280行 | 一次性运行并评测 basic/busybox 全组（basic-glibc, basic-musl, busybox-glibc, busybox-musl） |
| `judge.sh` | Shell 脚本 | — | 基础评测脚本 |
| `judge-offline.sh` | Shell 脚本 | — | 离线环境评测（无网络） |
| `judge-site.sh` | Shell 脚本 | — | 现场赛环境评测 |
| `judge-la.sh` | Shell 脚本 | — | LoongArch 架构评测 |
| `busybox_testcode.sh` | Shell 脚本 | ~30行 | 在 QEMU 内执行的 busybox 批量测试脚本，逐个执行 `busybox_cmd.txt` 中的命令 |
| `basic.md` | 文档 | ~3KB | basic 测试样例说明：测试范围、编译方法、样例输出 |
| `busybox.md` | 文档 | ~6KB | busybox 测试样例说明：测试范围、编译方法、评测方法 |
| `busybox_cmd.txt` | 文本 | — | busybox 要执行的命令列表（如 `ls /`, `cat README.md`, `sh` 等） |
| `oscomp_syscalls.md` | 文档 | ~15KB | 30+ 系统调用的完整说明（功能、参数、返回值、C 调用示例），对标 Linux 5.10 |
| `loongarch_ABI.txt` | 文档 | — | LoongArch 汇编调用约定（寄存器用途、ABI 规则） |
| `loongson2K1000LA.md` | 文档 | ~0.5MB | 龙芯 2K1000LA 处理器用户手册 V1.0（2022.5） |
| `mmexport.md` | 文档 | — | 微信导出：龙芯异常编码表（Ecode/EsubCode 对照表） |
| `op.txt` | 文本 | — | 内存地址列表（可能是调试 dump） |

---

## 📂 boot/ — 启动代码

### 目录结构

```
boot/
├── main.c              ← ★ 主启动逻辑（多核初始化 + SpringOS Banner）
├── main-rv.d           ← RISC-V 编译依赖文件（自动生成）
├── main-rv.o           ← RISC-V 编译目标文件（自动生成）
├── rv/                 ← RISC-V 架构启动
│   ├── entry.S         ← RISC-V 内核入口汇编（从 OpenSBI 跳转）
│   ├── entry.d         ← 编译依赖
│   ├── entry.o         ← 编译目标
│   ├── start.c         ← RISC-V 平台初始化（uart、plic、virtio 等）
│   ├── start-rv.d      ← 编译依赖
│   ├── start-rv.o      ← 编译目标
│   ├── initcode.S      ← 第一个用户程序跳板（xv6 init）
│   ├── initcode.d      ← 编译依赖
│   ├── initcode.o      ← 编译目标
│   ├── initcode-sh.S   ← shell 版 initcode（直接进 sh）
│   └── initcode-sh.o   ← 编译目标
├── la/                 ← LoongArch 架构启动（可忽略）
│   ├── entry.S         ← LA 内核入口
│   ├── initcode.S      ← LA initcode
│   └── initcode-sh.S   ← LA shell 版 initcode
├── vf2/                ← VisionFive2 开发板启动（可忽略）
│   ├── entry.S         ← VF2 内核入口
│   └── main.c          ← VF2 主逻辑
└── 2k1000/             ← 龙芯 2K1000 星云板启动（可忽略）
    ├── entry.S         ← 2K1000 内核入口
    ├── early_uart.c    ← 早期串口输出
    └── main-2k1000.c   ← 2K1000 主逻辑
```

### 文件详情

| 路径 | 用途 |
|------|------|
| `boot/main.c` | **多核启动入口**。`start()` 函数：每个 CPU 从这里开始→初始化中断/内存→启动 scheduler。打印 SpringOS ASCII Art Banner |
| `boot/rv/entry.S` | RISC-V 内核入口汇编。设置栈→关分页→跳转 C 代码。等价于 xv6 原版 `entry.S` |
| `boot/rv/start.c` | RISC-V 平台初始化：设置 PLIC/UART/virtio 设备→设置页表→跳转 `boot/main.c` |
| `boot/rv/initcode.S` | xv6 第一个用户进程的汇编跳板。加载 `/init` 程序到用户空间 |
| `boot/rv/initcode-sh.S` | Shell 版 initcode，等价于直接进 sh（`make qemu-sh` 用） |
| `boot/la/entry.S` | LoongArch 内核入口，设置 LA 栈 |
| `boot/la/initcode.S` | LoongArch initcode |
| `boot/la/initcode-sh.S` | LoongArch shell 版 initcode |
| `boot/vf2/entry.S` | VisionFive2 开发板内核入口 |
| `boot/vf2/main.c` | VF2 板级初始化逻辑 |
| `boot/2k1000/entry.S` | 龙芯 2K1000 开发板内核入口 |
| `boot/2k1000/early_uart.c` | 2K1000 早期串口驱动（内核启动调试输出） |
| `boot/2k1000/main-2k1000.c` | 2K1000 板级初始化 |

### 启动顺序（RISC-V QEMU）

```
OpenSBI → boot/rv/entry.S → boot/rv/start.c → boot/main.c → scheduler
```

---

## 📂 kernel/ — 内核源码

### 目录结构

```
kernel/
├── rv/kernel.ld        ← RISC-V 链接脚本
├── la/kernel.ld        ← LoongArch 链接脚本
├── vf2/kernel.ld       ← VisionFive2 链接脚本
├── 2k1000/kernel.ld    ← 龙芯 2K1000 链接脚本
├── proc/               ← 进程管理
│   ├── proc.c          ★ 进程创建/调度/退出
│   ├── proc-rv.d/o     ← 编译产物
│   ├── exec.c          ★ 程序加载执行 (execve)
│   └── exec-rv.d/o     ← 编译产物
├── mm/                 ← 内存管理
│   ├── vm.c            ★ 页表/vma/mmap
│   ├── vm-rv.d/o       ← 编译产物
│   ├── kalloc.c        ★ 物理页分配器
│   ├── kalloc-rv.d/o   ← 编译产物
│   ├── buddy.c         ★ 伙伴分配器（新增）
│   ├── buddy-rv.d/o    ← 编译产物
│   └── khead_lwext4.c.disabled  ← 已禁用的 ext4 头集成
├── syscall/            ← 系统调用
│   ├── syscall.c       ★ syscall 入口分发
│   ├── syscall-rv.d/o  ← 编译产物
│   ├── sysproc.c       ★ 进程类 syscall（fork/exit/brk/clone/wait4/…）
│   ├── sysproc-rv.d/o  ← 编译产物
│   ├── sysfile.c       ★ 文件类 syscall（open/read/write/getdents64/…）
│   ├── sysfile-rv.d/o  ← 编译产物
│   ├── syssig.c        ★ 信号类 syscall（sigaction/sigprocmask/…）
│   └── syssig-rv.d/o   ← 编译产物
├── trap/               ← 中断/异常处理
│   ├── rv/             ← RISC-V 架构
│   │   ├── trap.c      ★ 中断/异常处理主逻辑
│   │   ├── kernelvec.S ★ 内核态 trap 入口
│   │   ├── trampoline.S ★ 用户态↔内核态跳板
│   │   ├── swtch.S     ★ 上下文切换
│   │   ├── sigtrampoline.S ★ 信号返回跳板
│   │   └── *.d/*.o     ← 编译产物
│   ├── la/             ← LoongArch 架构
│   │   ├── trap.c      ← LA 异常处理
│   │   ├── apic.c      ← LA 中断控制器
│   │   ├── extioi.c    ← LA 外部中断
│   │   ├── iointc-2k1000.c ← 2K1000 中断控制器
│   │   ├── kernelvec.S ← LA 内核态 trap 入口
│   │   ├── uservec.S   ← LA 用户态 trap 入口
│   │   ├── swtch.S     ← LA 上下文切换
│   │   ├── sigtrampoline.S ← LA 信号返回
│   │   ├── tlbrefill.S ← LA TLB 重填
│   │   └── merror.S    ← LA 机器错误
│   └── signal.c        ★ 信号递送逻辑（架构无关）
├── fs/                 ← 文件系统
│   ├── bio.c           ★ 块缓存（buffer cache）
│   ├── console.c       ★ 控制台/终端
│   ├── pipe.c          ★ 管道
│   ├── ramdisk.c       ★ 内存磁盘（VF2 用）
│   ├── VFS_block.c     ★ VFS 块设备桥接（lwext4 ↔ 内核）
│   ├── VFS_ext.c       ★ VFS ext4 操作层
│   ├── vfs/            ← 虚拟文件系统
│   │   ├── file.c      ★ 文件描述符管理
│   │   ├── fs.c        ★ 超级块/挂载/目录操作
│   │   ├── inode.c     ★ inode 管理
│   │   └── ops.c       ★ VFS 操作接口
│   └── lwext4/         ← lwext4 库（ext4 实现）
│       ├── CMakeLists.txt  ← lwext4 原始构建文件（不用）
│       ├── ext4.c          ★ lwext4 主入口
│       ├── ext4_balloc.c   ★ 块分配
│       ├── ext4_bcache.c   ★ 块缓存
│       ├── ext4_bitmap.c   ★ 位图管理
│       ├── ext4_blockdev.c ★ 块设备抽象
│       ├── ext4_block_group.c ★ 块组管理
│       ├── ext4_crc32.c    ★ CRC32 校验
│       ├── ext4_debug.c    ★ 调试输出
│       ├── ext4_dir.c      ★ 目录操作
│       ├── ext4_dir_idx.c  ★ 目录索引
│       ├── ext4_extent.c   ★ extent 树
│       ├── ext4_fs.c       ★ 文件系统操作
│       ├── ext4_hash.c     ★ 哈希函数
│       ├── ext4_ialloc.c   ★ inode 分配
│       ├── ext4_inode.c    ★ inode 读写
│       ├── ext4_journal.c  ★ 日志（journal）
│       ├── ext4_mbr.c      ★ MBR 分区表
│       ├── ext4_mkfs.c     ★ 格式化工具
│       ├── ext4_super.c    ★ 超级块
│       ├── ext4_trans.c    ★ 事务管理
│       ├── ext4_xattr.c    ★ 扩展属性
│       └── *.d/*.o         ← 编译产物
├── drive/              ← 设备驱动
│   ├── uart.c          ★ 串口驱动（QEMU 终端的 I/O）
│   ├── rv/
│   │   ├── virtio_disk.c ★ virtio 磁盘驱动（QEMU 块设备）
│   │   └── plic.c        ★ PLIC 中断控制器
│   └── la/
│       ├── pci.c         ← LA PCI 总线
│       ├── virtio_disk.c ← LA virtio 磁盘
│       ├── virtio_pci.c  ← LA virtio PCI 桥接
│       └── virtio_ring.c ← LA virtio 环形队列
├── lock/               ← 同步原语
│   ├── spinlock.c      ★ 自旋锁
│   ├── sleeplock.c     ★ 睡眠锁
│   └── *.d/*.o         ← 编译产物
└── util/               ← 工具函数
    ├── printf.c        ★ 格式化输出
    ├── string.c        ★ 字符串操作（memset/memcpy/strncpy…）
    ├── qsort.c         ★ 快速排序
    ├── futex.c         ★ futex 同步（用户态锁）
    └── *.d/*.o         ← 编译产物
```

### 文件详情

#### proc/ — 进程管理

| 文件 | 行数 | 功能概述 |
|------|------|---------|
| `kernel/proc/proc.c` | — | **核心进程管理**：`allocproc()`、`fork()`、`exit()`、`wait()`、`scheduler()`、`clone()`、`growproc()`、`brk()`。对比 xv6 增加了 clone/futex/mmap 相关字段 |
| `kernel/proc/exec.c` | — | **execve 实现**：ELF 解析→加载段→设置栈→跳转。支持 RISC-V 和 LoongArch ELF |

#### mm/ — 内存管理

| 文件 | 功能概述 |
|------|---------|
| `kernel/mm/vm.c` | **虚拟内存**：页表创建/复制/释放、`mmap`/`munmap`、`copyin`/`copyout`、虚拟地址映射 |
| `kernel/mm/kalloc.c` | **物理页分配**：`kalloc()`/`kfree()`，空闲链表管理。基本同 xv6 |
| `kernel/mm/buddy.c` | **伙伴分配器**（新增）：更大的连续物理页分配，为了 ext4 的块缓存等大块内存需求 |
| `kernel/mm/khead_lwext4.c.disabled` | 已禁用。原本用于将 ext4 相关头文件静态分配到内核镜像中 |

#### syscall/ — 系统调用

| 文件 | 功能概述 |
|------|---------|
| `kernel/syscall/syscall.c` | **syscall 总入口**：从 `a7` 读调用号→查 `syscalls[]` 表→调处理函数。约 90 个注册项 |
| `kernel/syscall/sysproc.c` | **进程类系统调用**：`sys_fork`、`sys_exit`、`sys_wait4`、`sys_brk`、`sys_clone`、`sys_getpid`、`sys_getppid`、`sys_times`、`sys_uname`、`sys_nanosleep` 等 |
| `kernel/syscall/sysfile.c` | **文件类系统调用**：`sys_read`、`sys_write`、`sys_openat`、`sys_close`、`sys_dup`、`sys_dup3`、`sys_pipe2`、`sys_getcwd`、`sys_chdir`、`sys_mkdirat`、`sys_linkat`、`sys_unlinkat`、`sys_mount`、`sys_umount2`、`sys_getdents64`、`sys_fstat`、`sys_mmap` 等 |
| `kernel/syscall/syssig.c` | **信号类系统调用**：`sys_rt_sigaction`、`sys_rt_sigprocmask`、`sys_rt_sigreturn`、`sys_tkill`、`sys_tgkill` |

#### trap/ — 中断异常处理

| 文件 | 功能概述 |
|------|---------|
| `kernel/trap/rv/trap.c` | RISC-V trap 处理：`usertrap()`（用户态中断/异常）和 `kerneltrap()`（内核态中断/异常）。处理 syscall/timer/device 中断 |
| `kernel/trap/rv/kernelvec.S` | 内核 trap 向量：保存/恢复内核寄存器 |
| `kernel/trap/rv/trampoline.S` | **用户态↔内核态跳板**：保存/恢复用户寄存器，切换页表。对应 xv6 的 `trampoline.S` |
| `kernel/trap/rv/swtch.S` | **上下文切换汇编**：保存 callee-saved 寄存器，切换栈指针。对应 xv6 的 `swtch.S` |
| `kernel/trap/rv/sigtrampoline.S` | 信号返回跳板：从信号处理函数返回内核 |
| `kernel/trap/signal.c` | **信号递送**：检查进程挂起信号→在返回用户态前递送→设置 trampoline |
| `kernel/trap/la/trap.c` | LoongArch trap 处理 |
| `kernel/trap/la/kernelvec.S` | LA 内核 trap 向量 |
| `kernel/trap/la/uservec.S` | LA 用户 trap 向量 |
| `kernel/trap/la/swtch.S` | LA 上下文切换 |
| `kernel/trap/la/sigtrampoline.S` | LA 信号返回 |
| `kernel/trap/la/tlbrefill.S` | LA TLB 重填处理 |
| `kernel/trap/la/merror.S` | LA 机器错误处理 |
| `kernel/trap/la/apic.c` | LA APIC 中断控制器 |
| `kernel/trap/la/extioi.c` | LA 外部中断接口 |
| `kernel/trap/la/iointc-2k1000.c` | 2K1000 中断控制器 |

#### fs/ — 文件系统

| 文件 | 功能概述 |
|------|---------|
| `kernel/fs/bio.c` | **块缓存**（buffer cache）：LRU 链表管理磁盘块缓存。基本同 xv6 的逻辑，适配了 lwext4 |
| `kernel/fs/console.c` | **控制台驱动**：键盘输入、回显、退格、Ctrl+C/D 处理 |
| `kernel/fs/pipe.c` | **管道**：`pipealloc()`、`pipewrite()`、`piperead()`、`pipeclose()` |
| `kernel/fs/ramdisk.c` | **内存磁盘**：将 ext4 镜像数据嵌入内核，VF2 开发板从内存加载根文件系统 |
| `kernel/fs/VFS_block.c` | **VFS 块设备层**：将 lwext4 的块设备操作翻译为内核的 `bio.c` `bread()`/`bwrite()` 调用 |
| `kernel/fs/VFS_ext.c` | **VFS ext4 操作层**：将内核的文件系统调用（`readi`/`writei`/`namei` 等）翻译为 lwext4 的 ext4 操作 |
| `kernel/fs/vfs/file.c` | **文件描述符管理**：`ftable`、`filealloc()`、`fileclose()`、`filedup()`、`fileread()`、`filewrite()` |
| `kernel/fs/vfs/fs.c` | **文件系统核心**：超级块 `sb`、`fsinit()`、`namei()`（路径解析）、`dirlink()`、`dirlookup()` |
| `kernel/fs/vfs/inode.c` | **inode 管理**：`ialloc()`、`ilock()`/`iunlock()`、`iput()`、`readi()`、`writei()`、`itrunc()` |
| `kernel/fs/vfs/ops.c` | **VFS 操作接口**：统一的文件操作向量 |

#### lwext4/ — ext4 文件系统库

| 文件 | 功能概述 |
|------|---------|
| `kernel/fs/lwext4/ext4.c` | lwext4 主入口，初始化/挂载 |
| `kernel/fs/lwext4/ext4_balloc.c` | 数据块分配/释放（位图管理） |
| `kernel/fs/lwext4/ext4_bcache.c` | ext4 块缓存层 |
| `kernel/fs/lwext4/ext4_bitmap.c` | 块/inode 位图操作 |
| `kernel/fs/lwext4/ext4_blockdev.c` | 块设备抽象层 |
| `kernel/fs/lwext4/ext4_block_group.c` | 块组描述符管理 |
| `kernel/fs/lwext4/ext4_crc32.c` | CRC32 校验和（元数据一致） |
| `kernel/fs/lwext4/ext4_debug.c` | 调试输出宏 |
| `kernel/fs/lwext4/ext4_dir.c` | 目录操作（创建/删除/遍历条目） |
| `kernel/fs/lwext4/ext4_dir_idx.c` | 目录索引（HTree） |
| `kernel/fs/lwext4/ext4_extent.c` | extent 树操作（大文件块映射） |
| `kernel/fs/lwext4/ext4_fs.c` | 文件系统读写操作 |
| `kernel/fs/lwext4/ext4_hash.c` | 目录哈希函数 |
| `kernel/fs/lwext4/ext4_ialloc.c` | inode 分配/释放 |
| `kernel/fs/lwext4/ext4_inode.c` | inode 读写操作 |
| `kernel/fs/lwext4/ext4_journal.c` | 日志（jbd2 简化版） |
| `kernel/fs/lwext4/ext4_mbr.c` | MBR 分区表解析 |
| `kernel/fs/lwext4/ext4_mkfs.c` | 格式化 ext4（创建文件系统） |
| `kernel/fs/lwext4/ext4_super.c` | 超级块管理（挂载/卸载/统计） |
| `kernel/fs/lwext4/ext4_trans.c` | 事务管理 |
| `kernel/fs/lwext4/ext4_xattr.c` | 扩展属性（xattr） |

#### drive/ — 设备驱动

| 文件 | 功能概述 |
|------|---------|
| `kernel/drive/uart.c` | **16550 UART 串口驱动**：`uartinit()`、`uartputc()`、`uartgetc()`（QEMU 终端 I/O） |
| `kernel/drive/rv/virtio_disk.c` | **RISC-V virtio 块设备驱动**：`virtio_disk_rw()`，通过 MMIO 读写 virtio 磁盘 |
| `kernel/drive/rv/plic.c` | **RISC-V PLIC 中断控制器**：`plicinit()`、`plicinithart()`、`plic_claim()`、`plic_complete()` |
| `kernel/drive/la/pci.c` | LoongArch PCI 总线枚举 |
| `kernel/drive/la/virtio_disk.c` | LoongArch virtio 磁盘驱动 |
| `kernel/drive/la/virtio_pci.c` | LoongArch virtio PCI 桥接 |
| `kernel/drive/la/virtio_ring.c` | LoongArch virtio 环形队列 |

#### lock/ — 同步原语

| 文件 | 功能概述 |
|------|---------|
| `kernel/lock/spinlock.c` | **自旋锁**：`initlock()`、`acquire()`、`release()`。支持 `push_off()`/`pop_off()` 中断管理 |
| `kernel/lock/sleeplock.c` | **睡眠锁**：`initsleeplock()`、`acquiresleep()`、`releasesleep()`。阻塞时让出 CPU |

#### util/ — 工具函数

| 文件 | 功能概述 |
|------|---------|
| `kernel/util/printf.c` | **格式化输出**：`printf()`、`panic()`、`printf_color()`、`printf_bold()`、`printf_highlight()`。支持 ANSI 颜色 |
| `kernel/util/string.c` | **字符串操作**：`memset()`、`memcpy()`、`memmove()`、`strncpy()`、`strlen()`、`strcmp()`、`safestrcpy()` |
| `kernel/util/qsort.c` | **快速排序**（qsort 实现） |
| `kernel/util/futex.c` | **Futex**：`futex_wait()`、`futex_wake()`。快速用户态互斥锁，用于 libc 的 pthread_mutex |

#### 链接脚本

| 文件 | 功能概述 |
|------|---------|
| `kernel/rv/kernel.ld` | RISC-V QEMU 内核链接脚本（内存布局、段定义） |
| `kernel/la/kernel.ld` | LoongArch QEMU 内核链接脚本 |
| `kernel/vf2/kernel.ld` | VisionFive2 开发板链接脚本 |
| `kernel/2k1000/kernel.ld` | 龙芯 2K1000 开发板链接脚本 |

---

## 📂 include/ — 头文件

### 目录结构

```
include/
├── defs.h             ★ 全局函数声明（~300行）
├── param.h            ★ 内核常量参数
├── memlayout.h        ★ 内存布局定义
├── types.h            ★ 基础类型定义
├── platform.h         ★ 平台选择宏
├── asm.h              ★ 内联汇编工具宏
├── elf.h              ★ ELF 格式定义
├── errno.h            ★ 错误码（新增）
├── signal.h           ★ 信号定义（新增，~150行）
├── futex.h            ★ futex 常量（新增）
├── mem.h              ★ mmap/mremap 相关宏
├── poll.h             ★ poll/ppoll 结构
├── pipe.h             ★ 管道结构
├── time.h             ★ 时间结构体
├── kalloc.h           ★ kalloc 声明
├── ioctl.h            ★ ioctl 命令常量
├── ctype.h            ★ 字符分类
├── buddy.h            ★ 伙伴分配器声明
├── extioi.h           ★ 外部中断接口声明
├── apic.h             ★ APIC 声明
│
├── board/             ← 板级支持
│   └── 2k1000.h       ← 龙芯 2K1000 寄存器定义
├── proc/              ← 进程定义
│   └── proc.h         ★ 进程结构体（新增 vma/sig/futex 等字段）
├── syscall/           ← 系统调用
│   └── syscall.h      ★ syscall 编号定义（~90个，对标 Linux）
├── lock/              ← 锁定义
│   ├── lock.h         ← 锁接口声明
│   └── semaphore.h    ← 信号量
├── lib/               ← 库函数
│   ├── elf.h          ← ELF 辅助定义
│   ├── list.h         ← 链表宏
│   └── sbi.h          ← SBI 调用封装
├── user/              ← 用户态
│   └── user.h         ★ 用户态库函数声明
├── dev/               ← 设备驱动
│   ├── virtio.h       ← virtio 通用定义
│   └── pci/           ← PCI 相关
│       ├── pci.h
│       ├── virtio_pci.h
│       └── virtio_ring.h
└── fs/                ← 文件系统
    ├── buf.h          ★ 块缓存结构
    ├── stat.h         ★ stat 结构体
    ├── fcntl.h        ★ fcntl/open 标志
    ├── ioctl.h        ← 文件系统 ioctl
    ├── vfs/           ← VFS 接口
    │   ├── file.h     ★ 文件描述符结构
    │   ├── fs.h       ★ 超级块/Dinode/Dirent 结构
    │   ├── inode.h    ★ inode 结构
    │   └── ops.h      ★ VFS 操作接口
    ├── vfs_ext4_blockdev_ext.h ← lwext4 块设备桥接声明
    ├── vfs_ext4_ext.h          ← ext4 VFS 操作声明
    ├── vfs_ext4_inode_ext.h    ← ext4 inode 扩展声明
    └── ext4/          ← ext4 库头文件
        ├── ext4.h     ★ ext4 主头
        ├── ext4_config.h      ★ ext4 配置
        ├── ext4_types.h       ★ ext4 类型定义
        ├── ext4_fs.h          ★ 文件系统常量和结构
        ├── ext4_super.h       ★ 超级块定义
        ├── ext4_inode.h       ★ inode 结构
        ├── ext4_blockdev.h    ★ 块设备接口
        ├── ext4_block_group.h ★ 块组描述符
        ├── ext4_balloc.h      ★ 块分配器
        ├── ext4_bcache.h      ★ 块缓存
        ├── ext4_bitmap.h      ★ 位图
        ├── ext4_dir.h         ★ 目录结构
        ├── ext4_dir_idx.h     ★ 目录索引
        ├── ext4_extent.h      ★ extent 树
        ├── ext4_crc32.h       ★ CRC32
        ├── ext4_debug.h       ★ 调试宏
        ├── ext4_errno.h       ★ 错误码
        ├── ext4_hash.h        ★ 哈希
        ├── ext4_ialloc.h      ★ inode 分配器
        ├── ext4_journal.h     ★ 日志
        ├── ext4_mbr.h         ★ MBR
        ├── ext4_mkfs.h        ★ 格式化
        ├── ext4_misc.h        ★ 杂项
        ├── ext4_oflags.h      ★ 打开标志
        ├── ext4_trans.h       ★ 事务
        ├── ext4_xattr.h       ★ 扩展属性
        └── misc/              ← 辅助结构
            ├── queue.h        ← 队列（sys/queue.h）
            └── tree.h         ← 树（sys/tree.h）
```

### 关键头文件详情

| 文件 | 概述 |
|------|------|
| `include/defs.h` | **函数声明总目录**。包含所有内核函数的声明（bio、console、exec、file、fs、kalloc、pipe、printf、proc、spinlock、string、syscall、trap、uart、virtio、vm、signal、futex、buddy、lwext4 等）。约 300+ 行 |
| `include/param.h` | 内核常量：`NCPU=8`、`NPROC=64`、`NOFILE=16`、`BSIZE=1024`、`MAXPATH=128` 等 |
| `include/memlayout.h` | 虚拟内存布局：`UART0`、`VIRTIO0`、`PLIC`、`CLINT`、`KERNBASE`、`TRAMPOLINE` 地址 |
| `include/types.h` | 基础类型：`uint64`、`pagetable_t`、`pte_t` 等 |
| `include/platform.h` | 平台选择：`#ifdef QEMU` / `#ifdef VF2` / `#ifdef 2K1000` 等条件编译宏 |
| `include/elf.h` | ELF 格式定义：`struct elfhdr`、`struct proghdr`、`ELF_MAGIC` 等 |
| `include/errno.h` | POSIX 标准错误码：`EINVAL`、`ENOMEM`、`EACCES`、`ENOENT` 等（新增） |
| `include/signal.h` | 信号定义：信号编号（`SIGKILL`、`SIGTERM`…）、`sigaction` 结构体、`siginfo` 等（~150行） |
| `include/futex.h` | Futex 常量：`FUTEX_WAIT`、`FUTEX_WAKE` 等 |
| `include/syscall/syscall.h` | **系统调用编号**：~90 个 syscall（包括 `SYS_read=63`、`SYS_write=64`、`SYS_openat=56`、`SYS_clone=220`、`SYS_mmap=222` 等），对标 Linux 5.10 |
| `include/proc/proc.h` | **进程结构体**：`struct proc`（pid、pagetable、kstack、trapframe、context、vma 链表、信号挂起队列、futex 等待队列、cwd、ofile 等） |
| `include/fs/buf.h` | 块缓存结构：`struct buf`（device、blockno、data、flags、LRU 链表） |
| `include/fs/stat.h` | 文件状态结构：`struct stat`（dev、ino、type、nlink、size）和 `struct kstat` |
| `include/fs/fcntl.h` | 文件控制标志：`O_RDONLY`、`O_WRONLY`、`O_CREAT`、`O_DIRECTORY` 等 |
| `include/fs/vfs/file.h` | 文件结构：`struct file`（type、ref、readable、writable、pipe/inode 指针、offset） |
| `include/fs/vfs/fs.h` | 文件系统核心：`struct superblock`、`BSIZE=1024`、`NDIRECT=12`、`MAXFILE`、`struct dinode`、`struct dirent` |
| `include/fs/vfs/inode.h` | inode 结构：`struct inode`（dev、inum、ref、flags、type、size、addrs、ext4 inode 引用） |
| `include/fs/vfs/ops.h` | VFS 操作接口函数声明 |
| `include/fs/ext4/ext4.h` | lwext4 主头文件：包含所有 ext4 子模块的头文件 |
| `include/fs/ext4/ext4_config.h` | ext4 编译配置：块大小、inode 大小、支持特性 |
| `include/fs/ext4/ext4_types.h` | ext4 类型定义：`ext4_fsblk_t`、`ext4_lblk_t` 等 |
| `include/fs/ext4/ext4_fs.h` | ext4 文件系统常量和结构：magic、特性标志、inode 标志 |
| `include/fs/ext4/ext4_super.h` | 超级块定义：`ext4_sblock`、`ext4_super_block` |
| `include/fs/ext4/ext4_block_group.h` | 块组描述符结构 |
| `include/dev/virtio.h` | virtio 通用定义：寄存器偏移、状态位 |
| `include/dev/pci/virtio_pci.h` | virtio PCI 能力结构 |
| `include/dev/pci/pci.h` | PCI 配置空间定义 |
| `include/dev/pci/virtio_ring.h` | virtio vring（环形队列）定义 |
| `include/lock/lock.h` | 锁接口声明 |
| `include/lock/semaphore.h` | 信号量结构 |
| `include/lib/elf.h` | ELF 辅助宏 |
| `include/lib/list.h` | 双向链表宏 |
| `include/lib/sbi.h` | SBI 调用封装（`sbi_console_putchar`、`sbi_shutdown` 等） |
| `include/user/user.h` | 用户态库函数声明：`open`、`read`、`write`、`fork`、`exec` 等包装 |

---

## 📂 user/ — 用户程序

### 目录结构

```
user/
├── init-rv.c            ★ RISC-V init 程序（启动 shell）
├── init-rv.d/o          ← 编译产物
├── init-la.c            ← LoongArch init 程序
├── init-sh.c            ★ shell 版 init（直接进 sh）
├── initcode.S           ★ 用户态入口汇编跳板（同 xv6）
├── initcode-rv          ← 编译产物：二进制
├── initcode-rv.asm      ← 编译产物：反汇编
├── initcode-rv.d/o      ← 编译产物
├── initcode-la.S        ← LoongArch initcode
├── sh.c                 ★ Shell 程序（命令解析执行）
├── cat.c                ★ cat 工具
├── echo.c               ★ echo 工具
├── grep.c               ★ grep 工具
├── kill.c               ★ kill 命令
├── ln.c                 ★ ln 命令
├── ls.c                 ★ ls 命令
├── mkdir.c              ★ mkdir 命令
├── rm.c                 ★ rm 命令
├── wc.c                 ★ wc 命令
├── printf.c             ★ printf 工具
├── ulib.c               ★ 用户库（系统调用包装函数）
├── ulib-rv.d/o          ← 编译产物
├── umalloc.c            ★ 用户态 malloc
├── umalloc-rv.d/o       ← 编译产物
├── usys.pl              ★ Perl 脚本：生成 syscall 跳转存根
├── usys-rv.pl           ★ RISC-V 版 syscall 生成脚本
├── usys-rv.S            ← 生成的汇编存根
├── usys-rv.d/o          ← 编译产物
├── usys-la.pl           ★ LoongArch 版 syscall 生成脚本
├── user.ld              ★ RISC-V 用户程序链接脚本
├── user-la.ld           ← LoongArch 用户程序链接脚本
├── forktest.c           ← fork 压力测试
├── usertests.c          ← xv6 自带综合测试
├── grind.c              ← 随机 syscall 压力测试
├── stresstest.c         ← 文件系统压力测试
├── functest.c           ← 功能测试（已删除？不在列表中）
├── sigtest.c            ★ 信号测试
├── pptest.c             ★ 管道压力测试
├── futex.c              ★ futex 测试
├── sendtest.c           ★ 发送测试
├── new.c                ★ 新测试
├── simple.c             ★ 简单测试
├── zombietest.c         ← 僵尸进程测试
└── stresfs.c            ← 文件系统压力测试
```

### 文件详情

| 文件 | 功能概述 |
|------|---------|
| `user/init-rv.c` | RISC-V 启动的第一个用户进程：打开 console，创建 shell 进程 |
| `user/init-la.c` | LoongArch 版 init |
| `user/init-sh.c` | 直接启动 shell（`make qemu-sh` 用） |
| `user/initcode.S` | 用户态入口跳板：设置参数→`exec("/init")` |
| `user/sh.c` | **Shell**：命令解析、管道、重定向、后台执行 |
| `user/cat.c` | cat：读取并输出文件内容 |
| `user/echo.c` | echo：回显参数 |
| `user/grep.c` | grep：正则搜索文本 |
| `user/kill.c` | kill：向进程发送信号 |
| `user/ln.c` | ln：创建硬链接 |
| `user/ls.c` | ls：列出目录内容 |
| `user/mkdir.c` | mkdir：创建目录 |
| `user/rm.c` | rm：删除文件 |
| `user/wc.c` | wc：统计行/词/字符数 |
| `user/printf.c` | printf：格式化输出 |
| `user/ulib.c` | **用户库**：`open()`、`read()`、`write()`、`close()`、`dup()`、`fork()`、`exit()` 等系统调用包装 |
| `user/umalloc.c` | **用户态 malloc**：`malloc()`/`free()` 基于 sbrk |
| `user/usys.pl` | 自动生成 syscall 的汇编跳转存根（RISC-V） |
| `user/usys-rv.pl` | RISC-V 专用 syscall 生成脚本（通过 `ecall` → a7 传调用号） |
| `user/usys-la.pl` | LoongArch 专用 syscall 生成脚本 |
| `user/user.ld` | 用户程序链接脚本（RISC-V） |
| `user/user-la.ld` | LoongArch 用户程序链接脚本 |
| `user/sigtest.c` | 信号功能测试 |
| `user/pptest.c` | 管道/进程间通信压力测试 |
| `user/futex.c` | futex 同步机制测试 |
| `user/sendtest.c` | IPC 发送测试 |
| `user/new.c` | 新功能测试 |
| `user/simple.c` | 简单功能测试 |
| `user/forktest.c` | fork 压力测试（大量 fork/exit） |
| `user/usertests.c` | xv6 自带综合测试（文件、进程、管道等） |
| `user/grind.c` | 随机 syscall 压力测试 |
| `user/stressfs.c` | 文件系统压力测试 |
| `user/zombie.c` | 僵尸进程测试 |

---

## 📂 其他目录

### mkfs/ — 文件系统镜像工具

| 文件 | 功能概述 |
|------|---------|
| `mkfs/mkfs.c` | xv6 自带的文件系统制作工具。读取文件列表→写入 xv6 原始 FS 格式磁盘镜像。**SpringOS 主要用 ext4，此工具基本不用** |

### tools/ — 辅助工具

| 文件 | 功能概述 |
|------|---------|
| `tools/create_ramdisk.sh` | 创建 ext4 RAMDisk 镜像并转为 C 头文件。VF2 上板时将 ext4 镜像嵌入内核 |

### site/ — 网页端

| 文件 | 功能概述 |
|------|---------|
| `site/git_testcode.sh` | Git 功能测试脚本（在 QEMU 内执行 git init/commit/log 等） |

### .judge_tmp/ — 评测临时文件

| 文件 | 功能概述 |
|------|---------|
| `.judge_tmp/temp_judge_result_git-glibc.json` | Git 测试（glibc）评测结果暂存 |
| `.judge_tmp/temp_judge_result_git-musl.json` | Git 测试（musl）评测结果暂存 |

### mydocs/ — 学习文档

| 文件 | 功能概述 |
|------|---------|
| `mydocs/README.md` | mydocs 目录说明和索引 |
| `mydocs/01-structure-guide.md` | SpringOS 文件结构与 xv6 对比分析 |
| `mydocs/02-beginners-guide.md` | 操作系统大赛参赛入门完整教程 |
| `mydocs/03-file-catalog.md` | 本文档：完整文件/目录一览表 |

---

## 📐 系统调用一览（include/syscall/syscall.h）

SpringOS 定义了约 **90 个系统调用**，对标 Linux 5.10。以下是完整列表：

### 进程管理类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 1 | `SYS_fork` | fork |
| 3 | `SYS_wait` | wait（兼容） |
| 93 | `SYS_exit` | exit |
| 94 | `SYS_exit_group` | exit_group |
| 96 | `SYS_set_tid_address` | set_tid_address |
| 172 | `SYS_getpid` | getpid |
| 173 | `SYS_getppid` | getppid |
| 178 | `SYS_gettid` | gettid |
| 214 | `SYS_brk` | brk |
| 220 | `SYS_clone` | clone |
| 221 | `SYS_execve` | execve |
| 247 | `SYS_waitpid` | waitpid |
| 260 | `SYS_wait4` | wait4 |

### 文件系统类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 17 | `SYS_getcwd` | getcwd |
| 23 | `SYS_dup` | dup |
| 24 | `SYS_dup3` | dup3 |
| 25 | `SYS_fcntl` | fcntl |
| 29 | `SYS_ioctl` | ioctl |
| 34 | `SYS_mkdirat` | mkdirat |
| 35 | `SYS_unlinkat` | unlinkat |
| 36 | `SYS_symlinkat` | symlinkat |
| 37 | `SYS_linkat` | linkat |
| 39 | `SYS_umount2` | umount2 |
| 40 | `SYS_mount` | mount |
| 46 | `SYS_ftruncate` | ftruncate |
| 48 | `SYS_faccessat` | faccessat |
| 49 | `SYS_chdir` | chdir |
| 53 | `SYS_fchmodat` | fchmodat |
| 54 | `SYS_fchownat` | fchownat |
| 56 | `SYS_openat` | openat |
| 57 | `SYS_close` | close |
| 59 | `SYS_pipe2` | pipe2 |
| 61 | `SYS_getdents64` | getdents64 |
| 62 | `SYS_lseek` | lseek |
| 63 | `SYS_read` | read |
| 64 | `SYS_write` | write |
| 65 | `SYS_readv` | readv |
| 66 | `SYS_writev` | writev |
| 67 | `SYS_pread64` | pread64 |
| 71 | `SYS_sendfile` | sendfile |
| 76 | `SYS_splice` | splice |
| 78 | `SYS_readlinkat` | readlinkat |
| 79 | `SYS_fstatat` | fstatat |
| 80 | `SYS_fstat` | fstat |
| 82 | `SYS_fsync` | fsync |
| 83 | `SYS_fdatasync` | fdatasync |
| 276 | `SYS_renameat2` | renameat2 |
| 285 | `SYS_copy_file_range` | copy_file_range |
| 291 | `SYS_statx` | statx |

### 内存管理类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 215 | `SYS_munmap` | munmap |
| 216 | `SYS_mremap` | mremap |
| 222 | `SYS_mmap` | mmap |
| 226 | `SYS_mprotect` | mprotect |
| 233 | `SYS_madvise` | madvise |

### 信号类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 129 | `SYS_kills` | kill（信号） |
| 130 | `SYS_tkill` | tkill |
| 131 | `SYS_tgkill` | tgkill |
| 134 | `SYS_rt_sigaction` | rt_sigaction |
| 135 | `SYS_rt_sigprocmask` | rt_sigprocmask |
| 137 | `SYS_rt_sigtimedwait` | rt_sigtimedwait |
| 139 | `SYS_rt_sigreturn` | rt_sigreturn |

### 时间类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 101 | `SYS_nanosleep` | nanosleep |
| 113 | `SYS_clock_gettime` | clock_gettime |
| 115 | `SYS_clock_nanosleep` | clock_nanosleep |
| 153 | `SYS_times` | times |
| 169 | `SYS_gettimeofday` | gettimeofday |

### 同步/调度类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 73 | `SYS_ppoll` | ppoll |
| 98 | `SYS_futex` | futex |
| 99 | `SYS_set_robust_list` | set_robust_list |
| 124 | `SYS_sched_yield` | sched_yield |

### 系统信息类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 116 | `SYS_syslog` | syslog |
| 160 | `SYS_uname` | uname |
| 179 | `SYS_sysinfo` | sysinfo |
| 261 | `SYS_prlimit64` | prlimit64 |
| 278 | `SYS_getrandom` | getrandom |

### 用户/组管理类

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 143 | `SYS_setregid` | setregid |
| 144 | `SYS_setgid` | setgid |
| 145 | `SYS_setreuid` | setreuid |
| 146 | `SYS_setuid` | setuid |
| 174 | `SYS_getuid` | getuid |
| 175 | `SYS_geteuid` | geteuid |
| 176 | `SYS_getgid` | getgid |
| 177 | `SYS_getegid` | getegid |

### 网络/其他

| 编号 | 宏 | 对应 Linux 系统调用 |
|------|-----|-------------------|
| 300 | `SYS_shutdown` | shutdown（网络） |
| 666 | `SYS_fchown` | fchown |

---

## 🗂 按架构/平台分布

### RISC-V QEMU 路径（主开发平台）

```
boot/rv/entry.S → boot/rv/start.c → boot/main.c
    ↓
kernel/trap/rv/trap.c          ← 中断/异常
kernel/trap/rv/kernelvec.S     ← 内核 trap 向量
kernel/trap/rv/trampoline.S    ← 用户态跳板
kernel/trap/rv/swtch.S         ← 上下文切换
kernel/trap/rv/sigtrampoline.S ← 信号返回
kernel/drive/rv/virtio_disk.c  ← 磁盘驱动
kernel/drive/rv/plic.c         ← 中断控制器
kernel/rv/kernel.ld            ← 链接脚本
user/user.ld                   ← 用户程序链接脚本
user/usys-rv.pl                ← syscall 生成脚本
```

### LoongArch 路径

```
boot/la/entry.S → boot/main.c
    ↓
kernel/trap/la/trap.c / kernelvec.S / uservec.S / swtch.S / sigtrampoline.S
kernel/trap/la/apic.c / extioi.c / iointc-2k1000.c
kernel/drive/la/pci.c / virtio_disk.c / virtio_pci.c / virtio_ring.c
kernel/la/kernel.ld
user/user-la.ld / usys-la.pl
```

### VisionFive2 路径

```
boot/vf2/entry.S → boot/vf2/main.c
    ↓
kernel/vf2/kernel.ld
```

### 龙芯 2K1000 路径

```
boot/2k1000/entry.S → boot/2k1000/main-2k1000.c
    ↓
boot/2k1000/early_uart.c
kernel/2k1000/kernel.ld
```

---

## 📊 统计概览

| 项目 | 数量 |
|------|------|
| 总文件数 | ~280 个（含编译产物） |
| 源码文件（.c） | ~68 个 |
| 汇编文件（.S） | ~22 个 |
| 头文件（.h） | ~60 个 |
| Shell 脚本 | 8 个 |
| 文档（.md/.txt） | 8 个 |
| 系统调用数 | ~90 个 |
| 支持架构 | 3（RISC-V / LoongArch / VisionFive2） |
| 支持开发板 | 2（VF2 / 2K1000） |
| 文件系统 | 2（xv6 原生 FS / ext4 via lwext4） |
| 代码许可证 | GPL-2.0 |
