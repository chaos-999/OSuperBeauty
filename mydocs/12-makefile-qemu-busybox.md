# 📖 Makefile + QEMU 入门 + busybox 问题调试实录

> 📅 生成时间：`2026-06-02 22:21 (GMT+8)`
>
> 目标：以本项目真实的 Makefile 和 QEMU 命令为教材，零基础讲清楚构建和调试流程，并记录 busybox 问题的发现过程和解决方案。

---

## 前置知识：这三样东西的关系

```
你写的 C 代码 (init-rv.c, proc.c, ...)
          │
          ▼
    Makefile 指挥编译器编译 → kernel-rv (可执行的内核)
          │
          ▼
    QEMU 加载 kernel-rv + SD 卡镜像 → 虚拟出一台 RISC-V 机器 → 启动你的内核
          │
          ▼
    你看到串口输出 → 判断你的代码有没有按预期运行
```

---

## 一、Makefile 速成：以本项目为例

### 1.1 最基本的 Makefile 长什么样

```makefile
# 这是一个最简单的 Makefile
hello: hello.c
	gcc -o hello hello.c
```

- `hello:` 是**目标**（你要做什么）
- `hello.c` 是**依赖**（目标需要什么原材料）
- `gcc -o hello hello.c` 是**配方**（怎么用原材料做出目标，前面必须用 Tab 缩进）

**规则：** 如果依赖比目标更新（修改过），就执行配方重新构建。

### 1.2 变量：避免重复写同样的东西

```makefile
# ❌ 不好的写法：每次都要写一长串
kernel-rv: file1.o file2.o file3.o
	riscv64-linux-gnu-gcc -o kernel-rv file1.o file2.o file3.o

# ✅ 好的写法：用变量
CC = riscv64-linux-gnu-gcc
OBJS = file1.o file2.o file3.o

kernel-rv: $(OBJS)
	$(CC) -o kernel-rv $(OBJS)
```

`$(OBJS)` 展开成 `file1.o file2.o file3.o`。改一次变量值，所有引用自动更新。

### 1.3 本项目的 Makefile 逐段讲解

#### ① 定义简称（第 1-2 行）

```makefile
K = kernel       # 以后写 $(K) 就等于写 kernel
U = user         # $(U) 就等于 user
```

#### ② 自动收集源文件（第 5-12 行）

```makefile
K_SRC_COMMON := $(filter-out ..., $(wildcard $K/*/*.c) ...)
K_SRC_RV     := $(wildcard $K/*/rv/*.c) $(wildcard $K/*/*/rv/*.c)
K_SRC_LA     := $(wildcard $K/*/la/*.c) $(wildcard $K/*/*/la/*.c)
```

逐词拆解：
- `$(wildcard $K/*/*.c)` → 等价于 `$(wildcard kernel/*/*.c)` → 列出 `kernel/` 下所有 `.c` 文件
- `$(filter-out ..., ...)` → 从列表中排除某些文件（排除架构专用文件，留下公共代码）
- `:=` vs `=` → `:=` 立即求值，`=` 延迟求值（用 `:=` 更安全）

**效果：** 你新加一个 `.c` 文件到 `kernel/` 下，Makefile 自动发现，不用手动加。

#### ③ 定义编译产物列表（第 17-30 行）

```makefile
OBJS_RV = \
  boot/rv/entry.o \              # 汇编入口
  boot/start-rv.o \              # 启动代码
  boot/main-rv.o \               # 主函数
  $(K_SRC_COMMON:.c=-rv.o) \     # 公共内核代码 → 变成 -rv.o
  $(K_SRC_RV:.c=-rv.o) \         # RV 专用代码 → 变成 -rv.o
  ...
```

`$(K_SRC_COMMON:.c=-rv.o)` 是 Makefile 的**字符串替换**语法：

```
kernel/fs/pipe.c  kernel/proc/proc.c  kernel/mm/vm.c
        ↓       替换 .c → -rv.o
kernel/fs/pipe-rv.o  kernel/proc/proc-rv.o  kernel/mm/vm-rv.o
```

**为什么每个 `.c` 编译成 `-rv.o`？** 因为同一份公共代码要给 RISC-V 和 LoongArch 各编译一遍（用不同编译器、不同宏定义），后缀区分避免覆盖。

#### ④ 编译器选择（第 60-78 行）

```makefile
TOOLPREFIX_RV = riscv64-linux-gnu-
CC_RV = $(TOOLPREFIX_RV)gcc    # → riscv64-linux-gnu-gcc
LD_RV = $(TOOLPREFIX_RV)ld     # → riscv64-linux-gnu-ld

TOOLPREFIX_LA = loongarch64-linux-gnu-
CC_LA = $(TOOLPREFIX_LA)gcc    # → loongarch64-linux-gnu-gcc
```

交叉编译器：你在 x86 机器上编译 RISC-V / LoongArch 的代码。

#### ⑤ 编译选项（第 80-120 行）

```makefile
CFLAGS_COMMON = -Wall -Werror -O ...   # 所有架构共用
CFLAGS_RV = $(CFLAGS_COMMON)           # 继承公共部分
CFLAGS_RV += -mcmodel=medany -mno-relax  # RISC-V 特有
CFLAGS_RV += -D RISCV                    # 定义宏 RISCV
```

- `-Wall -Werror`：开了所有警告 + 把警告当错误（代码质量要求高）
- `-D RISCV`：等价于在 C 代码里写 `#define RISCV`，控制 `#ifdef RISCV` 分支
- `-ffreestanding -nostdlib`：不链接标准 C 库（内核不能依赖 libc，得自己实现一切）

#### ⑥ 编译规则（第 165-180 行）

```makefile
kernel-rv: $U/initcode-rv $(OBJS_RV) $K/rv/kernel.ld
	$(LD_RV) $(LDFLAGS_RV) -T $K/rv/kernel.ld -o kernel-rv $(OBJS_RV)
```

读法：「要生成 `kernel-rv`，需要先准备好 initcode、所有 `.o` 文件、链接脚本；然后用 RISC-V 链接器按链接脚本把它们拼成一个 ELF。」

#### ⑦ 最终入口（第 490 行）

```makefile
all: kernel-rv kernel-la
```

`make` / `make all` → 构建 RISC-V + LoongArch 两个内核。

### 1.4 你经常用的几个命令

| 命令 | 干了什么 | 什么时候用 |
|------|---------|-----------|
| `make` 或 `make all` | 编译 kernel-rv + kernel-la | 最终测评前 |
| `make kernel-rv` | 只编 RISC-V | **调试时**（快，省掉 LA 编译） |
| `make kernel-la` | 只编 LoongArch | 只想测 LA 时 |
| `docker run ... os-contest ... make ...` | 用 Docker 编译（本机无 LA 编译器时） | LA 交叉编译器未装时 |
| `make clean` | 删除所有编译产物 | 编译出诡异错误时先清一遍 |
| `make clean-test` | 只删 SD 卡残留 img | 测评前清理 |
| `make test` | 清残留 → 编 RV → 启动 Docker 测评 | 一键测评 |
| `make -j$(nproc)` | 用所有 CPU 核并行编译 | 加速编译 |
| `make -n` | 只打印会执行什么命令，不真执行 | 调试 Makefile 本身 |

### 1.5 Makefile 报错怎么看

最常见的三种：

```
make: *** No rule to make target 'xxx'.  Stop.
→ 依赖的文件不存在，或者 Makefile 里没写对应的构建规则

make: *** [xxx.o] Error 1
→ 编译某个 .c 文件时 gcc 报错了，往上看 gcc 的具体错误

make: Nothing to be done for 'xxx'.
→ 目标已经是最新的，没有被修改的依赖，不需要重新编译
```

---

## 二、QEMU 命令速成：以本项目的调试命令为例

### 2.0 前置：解压 SD 卡镜像

QEMU 命令中的 `sdcard-rv.img` 不是凭空来的——测试套件仓库编译输出的是压缩包 `sdcard-rv.img.gz`，需要先解压才能用。

> 📌 **镜像存放位置：** 本项目仓库里**不包含** SD 卡镜像文件（太大，4GB）。压缩镜像统一放在 `/home/chaos/oscomp-testdata/` 目录下，命名为 `sdcard-rv.img.gz`（RISC-V）和 `sdcard-la.img.gz`（LoongArch）。

**解压并放置到项目目录：**

```bash
# 只需解压一次，镜像可以反复使用
# 在项目目录下操作
cd ~/my-springos

# 解压到当前目录（或者用 -c 管道 + 重定向）
gzip -dc /home/chaos/oscomp-testdata/sdcard-rv.img.gz > sdcard-rv.img
# 得到 sdcard-rv.img（约 4GB）

# 或者也可以先复制再解压（需要 2 倍空间）：
cp /home/chaos/oscomp-testdata/sdcard-rv.img.gz .
gzip -d sdcard-rv.img.gz
```

> ⚠️ 解压后镜像约 4GB，确保磁盘空间充足。如果你的镜像是 `.img.xz` 格式（如 `sdcard-rv.img.xz`），用 `xz -dc ...img.xz > sdcard-rv.img` 解压。

### 2.1 完整命令

```bash

make kernel-la -j$(nproc)

qemu-system-riscv64 \
  -machine virt \
  -kernel kernel-rv \
  -m 256M \
  -nographic \
  -smp 1 \
  -bios default \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -no-reboot

  qemu-system-loongarch64 \
    -machine virt \
    -kernel kernel-la \
    -m 1G \
    -nographic \
    -smp 1 \
    -drive file=sdcard-la.img,if=none,format=raw,id=x0 \
    -device virtio-blk-pci,drive=x0 \
    -no-reboot \
    -device virtio-net-pci,netdev=net0 \
    -netdev user,id=net0 \
    -rtc base=utc
```

### 2.2 逐参数讲解

| 参数 | 值 | 什么意思 | 为什么这样设 |
|------|-----|---------|------------|
| `qemu-system-riscv64` | — | 启动 RISC-V 64 位虚拟机 | 你的内核 target 是 riscv64 |
| `-machine virt` | — | 模拟 `virt` 虚拟主板 | 标准 QEMU RISC-V 板子，不需要模拟真实硬件 |
| `-kernel kernel-rv` | `kernel-rv` | 加载你的内核 ELF 文件 | 刚才 `make` 编译出来的 |
| `-m 256M` | 256MB | 给虚拟机分配 256MB 内存 | 测试用 256MB 够了，线上测评用 1G |
| `-nographic` | — | 不要图形窗口 | 在终端里看串口输出就行 |
| `-smp 1` | 1 核 | 单核 CPU | 先调试单核，复杂点后面再加 |
| `-bios default` | — | 用 QEMU 自带的 OpenSBI | OpenSBI 负责从机器模式切换到内核模式 |
| `-drive file=sdcard-rv.img,if=none,format=raw,id=x0` | SD 卡镜像 | 虚拟硬盘，指向解压后的 ext4 磁盘镜像 | `if=none` 先创建虚拟磁盘但不插总线上 |
| `-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0` | — | 把上面的磁盘接到 virtio-mmio 总线 | 你的内核通过 virtio 驱动读写这个磁盘 |
| `-no-reboot` | — | 内核关机后不重启，QEMU 直接退出 | 防止关机后反复重启 |

### 2.3 QEMU 参数的核心模式

QEMU 命令行有固定套路：**`-设备类型 具体设备,属性1=值1,属性2=值2`**

```
-drive file=镜像文件,if=none,format=raw,id=磁盘编号
         ↑         ↑        ↑         ↑
       文件路径  不插总线  原始格式  给这个磁盘起名叫 x0

-device virtio-blk-device,drive=磁盘编号,bus=总线名
        ↑                  ↑              ↑
     virtio 块设备       引用上面那个磁盘  插到这条总线上
```

**类比：** 你先买一个硬盘（`-drive`），再把它插到主板的 SATA 口上（`-device`）。两步拆开是为了灵活组合——同一个硬盘可以插到不同总线类型。

### 2.4 `-nographic` 下如何退出

QEMU 在 `-nographic` 模式下会把串口输出（也就是你的 `printf`）直接打印到终端。要退出：
- **正常情况：** 内核调用 `shutdown()` → QEMU 自动退出
- **卡住时：** 按 `Ctrl+A`，松开，再按 `X` → 强制退出
- **脚本中：** 用 `timeout 30 qemu-system-riscv64 ...` → 30 秒后自动杀死

### 2.5 网络参数（以后用到再看）

```bash
-device virtio-net-device,netdev=net0 \
-netdev user,id=net0
```

创建一个用户态虚拟网卡。初赛 basic 测试不需要网络，先不管。

### 2.6 LoongArch QEMU 启动命令

```bash
# 0. 编译 LoongArch 内核
#    本机有 LA 编译器：
make kernel-la -j$(nproc)

#    本机无 LA 编译器（用 Docker）：
docker run --rm -v /home/my-springos:/os zhouzhouyi/os-contest:20260510 \
  bash -c "cd /os && make kernel-la -j\$(nproc)"

# 1. 解压 LA SD 卡镜像（只需一次）
gzip -dc /home/chaos/oscomp-testdata/sdcard-la.img.gz > sdcard-la.img

# 2. 启动 LA QEMU
timeout 30 qemu-system-loongarch64 \
  -machine virt \
  -kernel kernel-la \
  -m 256M \
  -nographic \
  -smp 1 \
  -drive file=sdcard-la.img,if=none,format=raw,id=x0 \
  -device virtio-blk-pci,drive=x0 \
  -no-reboot
```

和 RISC-V 命令的主要区别：

| 参数 | RISC-V | LoongArch | 说明 |
|------|--------|-----------|------|
| QEMU 程序 | `qemu-system-riscv64` | `qemu-system-loongarch64` | 不同架构的虚拟机 |
| 内核文件 | `kernel-rv` | `kernel-la` | `make` 产物名不同 |
| SD 卡镜像 | `sdcard-rv.img` | `sdcard-la.img` | 镜像分别下载 |
| 磁盘设备 | `virtio-blk-device` | `virtio-blk-pci` | LA 用 PCI 总线，RV 用 MMIO |
| 总线名 | `virtio-mmio-bus.0` | 不需要指定 | LA 的 PCI 总线自动枚举 |
| BIOS | `-bios default`（OpenSBI） | 不需要 | LA QEMU 内置固件 |

> ⚠️ LoongArch 交叉编译器 `loongarch64-linux-gnu-gcc` 本机通常未装。如果 `make kernel-la` 报"没有那个文件或目录"，用 Docker 编译：
> ```bash
> docker run --rm -v /home/my-springos:/os zhouzhouyi/os-contest:20260510 \
>   bash -c "cd /os && make kernel-la -j\$(nproc)"
> ```

---

## 三、调试实录：busybox 死循环的发现过程

### 3.1 完整时间线

```
第 1 步：在 init-rv.c 的 test_pre() 之前插入测试代码
         ↓
第 2 步：make kernel-rv  ← 只编 RISC-V，3 秒完成
         ↓
第 3 步：gzip -dc /home/chaos/oscomp-testdata/sdcard-rv.img.gz > sdcard-rv.img  ← 解压 SD 卡镜像（4GB，略慢）
         ↓
第 4 步：timeout 30 qemu-system-riscv64 ...  ← 启动 QEMU，30 秒超时
         ↓
第 5 步：观察输出 ← 发现 busybox 死循环！
```

### 3.2 加的测试代码

在 `user/init-rv.c` 的 `main()` 中，`test_pre()` 之前：

```c
// 试试 busybox 能不能跑
chdir("/glibc/");
int pid = fork();
if (pid == 0) {
    char *argv[] = {"busybox", "sh", 0};
    execve("busybox", argv, NULL);
    printf("execve busybox sh FAILED\n");  // 如果 execve 失败会打这行
    exit(1);
}
wait(0);
```

**逻辑：** 如果 `execve` 成功，子进程被 busybox 替换，不会执行到 `printf("FAILED")`。如果失败了，父进程的 `wait(0)` 会收到退出信号。

### 3.3 实际输出

```
===== SpringOS Auto Test Runner =====
2 busybox: unknown sys call 155
2 busybox: unknown sys call 155
2 busybox: unknown sys call 155
2 busybox: unknown sys call 155
...（无限循环，直到 timeout 30 秒结束）
```

### 3.4 从输出读出的信息

| 现象 | 说明 |
|------|------|
| `2 busybox:` | `execve` **成功了**，busybox 的代码已经在运行（2=pid） |
| `unknown sys call 155` | busybox 初始化时调了一个系统调用，你的内核不认识 |
| 无限重复 | busybox 收到 `-ENOSYS` 错误后没有优雅退出，而是死循环重试 |

**结论：不是 `execve` 传参有问题，不是 "applet not found"——busybox 被成功加载了，卡在初始化阶段的 syscall 缺失。**

### 3.5 查 syscall 155 是什么

```bash
# RISC-V Linux 的 syscall 155
grep 155 /usr/include/asm-generic/unistd.h

# 输出：__NR_getpgid 155
```

**syscall 155 = `getpgid`**（获取进程组 ID）。你的内核有 92 个 syscall 但缺了这个。

去 `include/syscall/syscall.h` 确认：

```bash
grep getpgid include/syscall/syscall.h
# 无输出 — 确实没定义
```

---

## 四、解决方案

### 4.1 最简修复：加 `getpgid` 系统调用

只需改两个文件：

**① `include/syscall/syscall.h`** — 加宏定义：

```c
#define SYS_getpgid  155
```

**② `kernel/syscall/syscall.c`** — 加实现 + 注册：

```c
// 实现函数
uint64 sys_getpgid(void) {
    // 绝大多数情况下进程组 ID = 进程 ID
    // 安全的简化实现：直接返回当前进程的 pid
    struct proc *p = myproc();
    return p->pid;
}

// 在 syscalls[] 表里加一行：
[SYS_getpgid] sys_getpgid,
```

### 4.2 但这只是开始

修了 `getpgid`，busybox 会继续往下走初始化流程，大概率遇到下一个缺失的 syscall。busybox 是一个功能丰富的用户程序，它的启动路径会调用一长串系统调用，至少包括：

```text
getpgid → getuid → geteuid → getgid → getegid
→ sigaction → sigprocmask → ioctl → fcntl
→ readlink → statfs → ...
```

### 4.3 系统化的解决方法

不要一个一个猜。直接编译一个**最小 busybox** 测试找出缺失列表：

```bash
# 观察第一次跑时卡在哪个 syscall
make kernel-rv && timeout 5 qemu-system-riscv64 ... | grep "unknown sys"

# 实现它

# 再来一次
make kernel-rv && timeout 5 qemu-system-riscv64 ... | grep "unknown sys"

# 循环，直到 busybox 不再报 unknown sys call
```

参考策略见 `09-competition-strategy.md`：修好 busybox 是初赛拿高分的最大杠杆。

### 4.4 调试效率对比

| 方式 | 一轮耗时 | 什么时候用 |
|------|---------|-----------|
| Docker 评测 | ~5-10 分钟 | 最终确认分数 |
| `make kernel-rv` + 本地 QEMU | ~30 秒 | **日常调试**（快 20 倍） |

**强烈建议日常调试用本地 QEMU，只有要出分数报告时才跑 Docker。**

---

## 五、你的调试工具箱（速查卡）

```bash
# ═══ 日常编码-测试-修复循环 ═══

# 0. 首次使用：解压 SD 卡镜像到项目目录（只需做一次）
gzip -dc /home/chaos/oscomp-testdata/sdcard-rv.img.gz > sdcard-rv.img

# 1. 改代码
vim user/init-rv.c

# 2. 编译
cd ~/my-springos
make kernel-rv -j$(nproc)

# 3. 跑 QEMU（30 秒超时，防死循环）
timeout 30 qemu-system-riscv64 \
  -machine virt -kernel kernel-rv -m 256M \
  -nographic -smp 1 -bios default \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -no-reboot

#=>这个更好用
# 1. 编译内核（只编 RISC-V，不用等 LoongArch）
cd /home/my-springos
make kernel-rv -j$(nproc)

# 2. 解压 SD 卡镜像到项目目录（只需一次）
gzip -dc /home/chaos/oscomp-testdata/sdcard-rv.img.gz > sdcard-rv.img

# 3. 启动 QEMU，看输出
timeout 30 qemu-system-riscv64 \
  -machine virt \
  -kernel kernel-rv \
  -m 256M \
  -nographic \
  -smp 1 \
  -bios default \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -no-reboot

# 4. 看输出
#   - 看到 "unknown sys call N" → 去实现 syscall N
#   - 看到 "panic: xxx" → 去修对应 bug
#   - 看到 "All tests completed" → 正常退出了

# ═══ 快速过滤关键输出 ═══
# 只看 busybox 相关的行
timeout 30 qemu... 2>&1 | grep -E 'busybox|unknown sys|FAILED|##.*COMP'

# 只看 panic
timeout 30 qemu... 2>&1 | grep -E 'panic|unknown sys'

# ═══ 需要 clean 的时候 ═══
make clean && make kernel-rv -j$(nproc)
```

---

> **核心收获：** Makefile 就是一份"怎么用编译器把 C 代码变成内核"的说明书，QEMU 命令就是一份"怎么启动这台虚拟 RISC-V 电脑"的说明书。busybox 的问题不是 "sh applet 不存在"，而是一个接一个缺失的系统调用——修一个 getpgid 只是万里长征第一步。

---

## 六、如何在 init-rv.c 中加入 busybox 测试套件

> 📅 补充时间：`2026-06-03`
>
> 背景：syscall 155（getpgid）已修复，`busybox echo hello` 已验证通过。接下来需要让 init-rv.c 跑完整的 busybox 测试集（`busybox sh busybox_testcode.sh`）。

### 6.1 busybox 测试套件的工作机制

先理解 `busybox_testcode.sh` 做了什么（完整内容见镜像中的 `busybox_testcode.sh`）：

```
内核 init 进程
  └── fork + execve("busybox", ["busybox", "sh", "busybox_testcode.sh"], ...)
        └── busybox sh 逐行执行 busybox_cmd.txt:
              ├── eval "./busybox echo xxx"         ← 每次 fork+exec busybox
              ├── eval "./busybox ash -c exit"      ← 再次 fork+exec
              ├── eval "./busybox cal"              ← 再次...
              └── ...（共 ~55 条命令）
```

关键点：
- **CWD 必须在 busybox 所在目录**——脚本里全用 `./busybox` 相对路径
- **每条命令都 fork+exec 一次 busybox**——约 55 次进程创建，内存压力大
- **评测标记格式：** `testcase busybox <command> success/fail`

### 6.2 当前 init-rv.c 里已有的东西

`user/init-rv.c` 中已经准备好了你需要的变量和框架：

```c
// 第 14-15 行：busybox 路径
char bb_path_musl[] = "/musl/";
char bb_path_glibc[] = "/glibc/";

// 第 17 行：测试脚本启动参数
char *bb_testcode[10] = {"busybox", "sh", "busybox_testcode.sh", NULL};

// 第 175-191 行：test_pre() 末尾注释掉的代码
//   chdir(bb_path_musl);
//   execve("busybox", bb_testcode, NULL);
//   ...
//   chdir(bb_path_glibc);
//   execve("busybox", bb_testcode, NULL);
```

### 6.3 步骤 A：先验证单条 busybox 命令（确认基础可用）

把 `main()` 里当前的"hello"测试改成"sh -c exit"——不阻塞，不依赖交互输入：

```c
// main() 第 448 行附近，替换为：
chdir("/glibc/");
int pid = fork();
if (pid == 0) {
    char *argv[] = {"busybox", "sh", "-c", "exit", 0};
    execve("busybox", argv, NULL);
    printf("execve busybox sh -c exit FAILED\n");
    exit(1);
}
wait(0);
```

编译跑一次：`make kernel-rv -j$(nproc) && timeout 30 qemu-system-riscv64 ...`

- 弹出后没有 `unknown sys call` 且继续执行了 `test_pre()` → busybox sh 基础可用
- 如果卡住 → 缺新的 syscall，看 `unknown sys call N` 继续补

### 6.4 步骤 B：在 test_pre() 末尾追加 busybox 测试

`test_pre()` 跑完 basic-glibc 和 basic-musl 后，紧接着跑 busybox 测试。不修改原有代码，在 `test_pre()` 的 `return` 前追加：

```c
// ===== busybox 测试（追加在 test_pre() return 之前）=====
printf("\n--- busybox test start ---\n");

// glibc 版本
chdir(bb_path_glibc);
pid = fork();
if (pid < 0) {
    printf("init: fork failed for busybox-glibc\n");
} else if (pid == 0) {
    execve("busybox", bb_testcode, NULL);
    printf("init: exec busybox_testcode.sh (glibc) failed\n");
    exit(1);
} else {
    wait(0);
}

// musl 版本
chdir(bb_path_musl);
pid = fork();
if (pid < 0) {
    printf("init: fork failed for busybox-musl\n");
} else if (pid == 0) {
    execve("busybox", bb_testcode, NULL);
    printf("init: exec busybox_testcode.sh (musl) failed\n");
    exit(1);
} else {
    wait(0);
}

printf("--- busybox test end ---\n\n");
// ===== busybox 测试结束 =====
```

### 6.5 步骤 C：处理 `./busybox` 路径问题

`busybox_testcode.sh` 里所有命令都用 `./busybox`：

```bash
./busybox echo "#### OS COMP TEST GROUP START busybox-glibc ####"
./busybox ash -c exit
./busybox cal
...
```

你有两个选择：

**选择 1：改脚本（如果镜像可写）**——把 `./busybox` 全部替换为 `busybox`

```bash
# 挂载 SD 卡镜像后执行
sudo mount sdcard-rv.img /mnt
sed -i 's|\./busybox|busybox|g' /mnt/glibc/busybox_testcode.sh
sed -i 's|\./busybox|busybox|g' /mnt/musl/busybox_testcode.sh
sudo umount /mnt
```

**选择 2：不改脚本，确保内核的 exec 能找到 `./busybox`**

因为已经 `chdir()` 到 busybox 所在目录，当前内核的 `execve("busybox", ...)` 会解析路径。如果内核 exec 支持相对路径（`./busybox`），那`chdir` 后脚本里的 `./busybox` 就能找到。验证方法——在 init 里手动测试：

```c
chdir("/glibc/");
char *argv[] = {"busybox", "ls", 0};
execve("busybox", argv, NULL);   // 如果这个能跑
                                  // 脚本里的 ./busybox 就也能跑
```

### 6.6 预期问题和调试顺序

busybox 测试集跑起来后，你大概率会依次遇到：

| 阶段 | 现象 | 原因 | 对策 |
|------|------|------|------|
| 1 | `unknown sys call N` | busybox sh 初始化还在调新 syscall | `grep "unknown sys"` → 实现之 |
| 2 | 能进脚本，但某条命令 `fail` | 那个 applet 依赖的 syscall 没完全实现 | 看哪条命令 fail，针对性查 |
| 3 | 跑到一半 panic/卡死 | fork 子进程太多，内存耗尽 | 缩减测试集（注释掉 busybox_cmd.txt 尾部命令），或修内存回收 |
| 4 | shell 脚本语法报错 | 内核的 pipe/重定向支持不完整 | 先从简单命令开始逐条跑 |

**最高效调试方式：**

```bash
# 不要每次跑完全部测试，只跑 glibc 单条命令
make kernel-rv -j$(nproc) && \
timeout 10 qemu-system-riscv64 \
  -machine virt -kernel kernel-rv -m 256M -nographic -smp 1 \
  -bios default \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -no-reboot 2>&1 | tee /tmp/qemu.log

# 快速看缺失的 syscall
grep "unknown sys" /tmp/qemu.log | sort -t' ' -k5 -n | uniq
```

### 6.7 快速参考：init-rv.c 里所有可调用的测试入口

```c
// 已启用的：
test_pre();           // basic 30 个 syscall 测试（glibc + musl）

// 在 #if 0 中，可取消注释：
test_final();         // interrupts + copy_file_range + splice 测试
test_on_site();       // git 测试
run_dir_testscripts("/glibc");  // 自动扫描目录执行所有 *_testcode.sh
```

一旦 busybox 测试稳定通过，逐步取消 `#if 0` 启用更多测试组。

---

> **总结：** 修完 getpgid 后，从 `busybox sh -c exit` 开始逐条验证，然后在 `test_pre()` 末尾追加 6.4 的代码跑全量 busybox 测试集。遇到 `unknown sys call` 就补，遇到 `fail` 就针对那条命令查 syscall 实现——循环迭代，直到 busybox 全部通过。
