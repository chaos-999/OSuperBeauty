# 🧪 autotest-for-oskernel 本地评测完整指南

> 📅 生成时间：`2026-06-01 10:46 (GMT+8)`
>
> 目标：手把手教你用比赛官方评测引擎在本地跑完整评测，每一步都解释"为什么这样做"以及"背后发生了什么"

---

## 前置知识：这东西是干嘛的

[autotest-for-oskernel](https://github.com/oscomp/autotest-for-oskernel) 是**全国大学生操作系统大赛（OSCOMP）的官方本地评测工具**。它就是比赛评测机在服务器上跑的那套东西，你下载到本地可以先自己跑一遍，不用每次提交等线上排队。

**核心原理一句话：** Docker 容器里启动你的内核 + 官方测试镜像 → 抓取串口输出 → 正则匹配打分 → 输出 JSON 分数。

```
你的 OS 项目 ──make all──→ kernel-rv + kernel-la
                                  │
                  ┌───────────────┘
                  ▼
Docker 容器 ──→ QEMU 启动 ──→ 内核跑测试脚本 ──→ 抓串口输出
                                                     │
                  ┌──────────────────────────────────┘
                  ▼
        评分脚本 (judge_*.py) ──→ 正则解析 ──→ JSON 分数
```

---

## 一、环境检查（你当前的现状）

### 1.1 已有资源

```
/home/chaos/
├── autotest-for-oskernel/     ✅ 已克隆（评测引擎源码）
├── testsuits-for-oskernel/    ✅ 已克隆（测试程序源码，可用于自行构建镜像）
└── my-springos/               ✅ 你的操作系统项目（要被评测的对象）
```

### 1.2 Docker 状态

```bash
docker --version
# Docker version 29.4.3 ✅ 已安装
```

### 1.3 还需要准备的东西

| 东西 | 状态 | 说明 |
|------|------|------|
| Docker 镜像 `zhouzhouyi/os-contest:20260510` | ❌ 还没拉 | 包含交叉编译工具链 + QEMU |
| SD 卡测试镜像（`.img.gz`） | ❌ 还没下载 | 包含所有预编译测试程序的 ext4 磁盘镜像 |
| kernel.zip（评测引擎打包） | ❌ 还没打包 | 把 `kernel/` 目录打成 zip |

---

## 二、第一步：拉 Docker 评测镜像

```bash
sudo docker pull zhouzhouyi/os-contest:20260510
```

**这一步在做什么？**

```
Docker Hub
  │
  └─ zhouzhouyi/os-contest:20260510
       ├── Ubuntu 基础系统
       ├── RISC-V 交叉编译工具链 (riscv64-linux-gnu-gcc)
       ├── LoongArch 交叉编译工具链 (loongarch64-linux-gnu-gcc)
       ├── qemu-system-riscv64   ← RISC-V 模拟器
       ├── qemu-system-loongarch64 ← LoongArch 模拟器
       ├── Python 3.9 + pygrading 评测框架
       ├── zip / gzip / 常用工具
       └── /coursegrader/ 目录结构（评测引擎的工作区）
```

> **为什么用 Docker？** 评测需要特定版本的交叉编译工具链和 QEMU。Docker 保证你本地的编译环境和比赛评测机**完全一致**，不会出现"我本地能跑但评测机不过"的情况。

---

## 三、第二步：准备测试数据目录

测试数据目录需要两样东西：**评分脚本**（judge_*.py）和 **SD 卡测试镜像**（.img.gz）。

### 3.1 创建目录并复制评分脚本

```bash
mkdir -p /home/chaos/oscomp-testdata
cp /home/chaos/autotest-for-oskernel/kernel/judge/* /home/chaos/oscomp-testdata/
```

**复制了什么？**

```
oscomp-testdata/
├── judge_basic-glibc.py       ← basic 测试评分（glibc 版本）
├── judge_basic-musl.py        ← basic 测试评分（musl 版本）
├── judge_busybox-glibc.py     ← busybox 测试评分
├── judge_busybox-musl.py
├── judge_lua-glibc.py         ← lua 解释器测试
├── judge_lua-musl.py
├── judge_libctest-glibc.py    ← libc 兼容性测试
├── judge_libctest-musl.py
├── judge_libcbench-glibc.py   ← libc 性能基准
├── judge_libcbench-musl.py
├── judge_lmbench-glibc.py     ← 系统性能基准
├── judge_lmbench-musl.py
├── judge_iozone-glibc.py      ← 文件系统性能
├── judge_iozone-musl.py
├── judge_iperf-glibc.py       ← 网络吞吐测试
├── judge_iperf-musl.py
├── judge_netperf-glibc.py     ← 网络延迟测试
├── judge_netperf-musl.py
├── judge_cyclictest-glibc.py  ← 实时性测试
├── judge_cyclictest-musl.py
├── judge_ltp-glibc.py         ← Linux Test Project 全量
├── judge_ltp-musl.py
├── config.json                ← QEMU 配置（内存、超时等）
├── output.txt                 ← 输出占位文件
├── cancel_purge               ← 清理脚本
└── postwork.py                ← 后处理占位
```

**为什么分 glibc 和 musl 两套？** 同一组测试程序分别用 glibc 和 musl（两种不同 C 标准库）编译，行为可能不同。比赛两套都会跑、分别打分。

### 3.2 下载 SD 卡测试镜像

```bash
cd /home/chaos/oscomp-testdata

# 下载两个架构的测试镜像（压缩包）
wget https://github.com/oscomp/testsuits-for-oskernel/releases/download/pre-20250615/sdcard-rv.img.xz
wget https://github.com/oscomp/testsuits-for-oskernel/releases/download/pre-20250615/sdcard-la.img.xz

# 解压 xz → 再用 gzip 压缩（评测脚本要求 .img.gz 格式）
unxz sdcard-rv.img.xz && gzip sdcard-rv.img
unxz sdcard-la.img.xz && gzip sdcard-la.img
```

**SD 卡镜像里有什么？**

```
sdcard-rv.img (ext4, 无分区表, 约 4GB)
/
├── glibc/
│   ├── busybox                    ← busybox ELF 二进制
│   ├── basic/                     ← 32 个 basic 测试程序
│   │   ├── run-all.sh
│   │   ├── brk → 测试 brk syscall
│   │   ├── chdir → 测试 chdir
│   │   ├── clone → 测试 clone
│   │   └── ... 共 32 个
│   ├── lua/                       ← Lua 解释器 + 测试脚本
│   ├── libc-test/                 ← C 标准库兼容测试
│   ├── lib/                       ← libc.so 等运行时库
│   ├── basic_testcode.sh          ← ★ 测试入口脚本
│   ├── busybox_testcode.sh
│   ├── lua_testcode.sh
│   └── ... 其他 *_testcode.sh
│
└── musl/                           ← 同上，但程序用 musl 编译
    └── (相同结构)
```

**你的内核需要做什么？** 启动后自动扫描磁盘 → 找到 `glibc/` 和 `musl/` 下的 `*_testcode.sh` → 用 `busybox sh` 逐一执行 → 输出结果到串口 → 关机。

---

## 四、第三步：打包评测引擎

```bash
cd /home/chaos/autotest-for-oskernel/kernel
zip ../kernel.zip -r *
```

**这一步在做什么？** 把评测引擎的 Python 代码（`kernel/` 目录）压缩成 `kernel.zip`，Docker 容器会以 `python3 /cg/kernel.zip` 的方式运行它。

```
kernel/
├── __main__.py          ← 入口：创建 Job 对象，调用 prework → run → postwork
├── prework.py           ← 第一步：编译你的 OS（make all）
├── run.py               ← 第二步：启动 QEMU + 抓输出 + 调评分脚本
├── run_qemu.py          ← QEMU 启动逻辑（RISC-V + LoongArch 双架构）
├── postwork.py          ← 第三步：清理 + 汇总
├── sdcardwork.py        ← K210 开发板 SD 卡烧录（QEMU 评测不用）
├── parse_output_2023.py ← 解析性能测试输出（iozone/iperf 等）
├── utils.py             ← 工具函数 + 日志
├── exception.py         ← 异常处理
├── verdict.py           ← 评测结论
├── scene_output.py      ← 场景测试输出解析
├── net_relay.py         ← 网络继电器控制（开发板用）
├── judge/               ← 评分脚本（已被你复制到 testdata）
├── baselines/           ← 性能基准数据
├── templates/           ← HTML 报告模板
└── pygrading/           ← 评测框架库
```

---

## 五、第四步：运行评测

### 5.1 完整命令

```bash
cd /home/chaos

sudo docker run --rm \
  -v /home/my-springos:/coursegrader/submit \
  -v /home/chaos/oscomp-testdata:/coursegrader/testdata \
  -v /home/chaos/autotest-for-oskernel:/cg \
  -v /home/chaos/oscomp-testdata:/mnt/cghook/ \
  zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip

cd 

docker run --rm \
  -v /home/zhangshuoyu/OSuperBeauty:/coursegrader/submit \
  -v /home/zhangshuoyu/oscomp-testdata:/coursegrader/testdata \
  -v /home/zhangshuoyu/autotest-for-oskernel:/cg \
  -v /home/zhangshuoyu/oscomp-testdata:/mnt/cghook/ \
  zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip

cd 

docker run --rm \
  -v /home/angel/OSuperBeauty:/coursegrader/submit \
  -v /home/angel/oscomp-testdata:/coursegrader/testdata \
  -v /home/angel/autotest-for-oskernel:/cg \
  -v /home/angel/oscomp-testdata:/mnt/cghook/ \
  zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip
```

### 5.2 每一个 `-v` 参数的含义

| 参数 | 含义 | 你的路径 |
|------|------|---------|
| `-v /home/my-springos:/coursegrader/submit` | 你的 OS 项目 → 容器内评测引擎的"待评测目录" | `/home/my-springos` |
| `-v /home/chaos/oscomp-testdata:/coursegrader/testdata` | 评分脚本 + SD 卡镜像 → 容器内的测试数据目录 | `/home/chaos/oscomp-testdata` |
| `-v /home/chaos/autotest-for-oskernel:/cg` | 评测引擎源码 → 容器内 `/cg`，`kernel.zip` 也在这 | `/home/chaos/autotest-for-oskernel` |
| `-v /home/chaos/oscomp-testdata:/mnt/cghook/` | 钩子目录：放清理脚本和实时日志 | `/home/chaos/oscomp-testdata` |
| `--rm` | 容器退出后自动删除 | — |

> **为什么同一个目录要挂两次？** `testdata` 既存放评分脚本（评测引擎从这里读取）、又作为 cghook 目录（写清理脚本和实时日志）。两个挂载点用途不同，但指向同一份数据。

### 5.3 建议：先只跑 basic 测试

完整评测跑 12 组测试要很久（basic + busybox + lua + libctest + ...）。建议先只保留 basic 的评分脚本：

```bash
# 只保留 basic 的两个评分脚本
cd /home/chaos/oscomp-testdata
mkdir -p /tmp/judge-backup
mv judge_* /tmp/judge-backup/
mv /tmp/judge-backup/judge_basic-glibc.py .
mv /tmp/judge-backup/judge_basic-musl.py .
# config.json 和 output.txt 不要动
```

评测引擎自动发现目录下有哪些 `judge_*.py` 就跑哪些，不会因为缺少脚本报错。

---

## 六、评测全链路走读（从启动到打分）

### 6.1 完整时间线

```
评测机执行的操作                             你的代码/内核里发生的事
───────────────                             ──────────────────

Step 0: prework.py 启动
  ├── 写 cancel_purge 清理脚本到 /mnt/cghook/
  ├── 检查 submit 目录不为空
  ├── 检查没有禁止的文件名
  │   (os_flash_out.txt / os.bin 等)
  │
  └── ★ cd /coursegrader/submit && make all
      │                                    → Makefile: all 目标被执行
      │                                    → 编译出 kernel-rv (RISC-V ELF)
      │                                    → 编译出 kernel-la (LoongArch ELF)
      │                                    → 可选的 disk.img
      │
      ├── 编译成功 → 继续                     如果编译失败 → CG.CompileError
      └── 编译失败 → 终止评测

Step 1: run.py 启动
  ├── cp sdcard-rv.img.gz → submit 目录
  ├── cp sdcard-la.img.gz → submit 目录
  │
  ├── ★ 启动两个并行线程
  │   ├── 线程 1: run_qemu (RISC-V)
  │   └── 线程 2: run_qemu_loong (LoongArch)
  │
  └── 等待两个线程都完成

Step 2: run_qemu.py (RISC-V 线程)
  ├── gzip -d sdcard-rv.img.gz          ← 解压测试镜像
  ├── 读取 config: smp=1, mem=1G, timeout=3600
  │
  └── ★ 启动 QEMU:
      qemu-system-riscv64 \
        -machine virt \
        -kernel kernel-rv \             → 你的内核 ELF
        -m 1G \                          → 给 1GB 内存
        -nographic \                     → 无图形界面，串口输出
        -smp 1 \                         → 单核
        -bios default \                  → 用 OpenSBI
        -drive file=sdcard-rv.img,... \  → 挂载 ext4 测试镜像
        -no-reboot \                     → 关机后不重启，直接退出
        -device virtio-net-device,... \  → 虚拟网卡（网络测试用）
        -rtc base=utc                    → 时钟基准 UTC
                                          ↓
                                    ★ 你的内核启动
                                      ├── OpenSBI → boot/rv/entry.S
                                      ├── boot/main.c 初始化各子系统
                                      ├── forkret() → filesystem_init()
                                      │   └── 挂载 ext4 磁盘镜像
                                      ├── 加载 init 进程
                                      │
                                      ├── init 扫描磁盘:
                                      │   ├── 找到 glibc/basic_testcode.sh
                                      │   ├── 找到 glibc/busybox_testcode.sh
                                      │   ├── 找到 musl/basic_testcode.sh
                                      │   └── ...
                                      │
                                      ├── 执行 basic_testcode.sh:
                                      │   busybox echo "#### OS COMP TEST GROUP
                                      │          START basic-glibc ####"
                                      │   cd basic && ./run-all.sh
                                      │     ├── ./brk  → 屏幕输出测试结果
                                      │     ├── ./chdir → 屏幕输出
                                      │     └── ./yield → 屏幕输出
                                      │   busybox echo "#### OS COMP TEST GROUP
                                      │          END basic-glibc ####"
                                      │
                                      ├── 执行 busybox_testcode.sh: (同上)
                                      ├── 执行 lua_testcode.sh: (同上)
                                      └── ...
                                          ↓
                                      全部测试完成
                                      shutdown() → QEMU 退出

Step 3: 抓取串口输出
  ├── 输出写入 os_serial_out_rv.txt (RISC-V)
  └── 输出写入 os_serial_out_la.txt (LoongArch)

Step 4: parse_serial_out_new() 解析输出
  逐行读取串口输出:
  │
  ├── 看到 "#### OS COMP TEST GROUP START basic-glibc ####"
  │   └── 启动 judge_basic-glibc.py 子进程
  │   └── 把后续输出逐行 pipe 给评分脚本
  │
  ├── 看到 "#### OS COMP TEST GROUP END basic-glibc ####"
  │   └── 关闭 stdin → 评分脚本输出 JSON → 记录分数
  │
  ├── 看到 "#### OS COMP TEST GROUP START busybox-glibc ####"
  │   └── 启动 judge_busybox-glibc.py → pipe 输出 → 记录分数
  │
  └── ... 所有 *_testcode.sh 都处理完

Step 5: postwork.py 汇总
  └── 合并所有评分脚本的 JSON → 最终评测报告
```

### 6.2 评分脚本怎么打分？

以 `judge_basic-glibc.py` 为例：

```python
# 评分脚本的核心逻辑：正则表达式匹配屏幕输出
# 比如测试 brk 时，你的内核应该输出：
#   ========== START test_brk ==========
#   Before alloc,heap pos: 0
#   After alloc,heap pos: 64
#   Alloc again,heap pos: 128
#   ========== END test_brk ==========

# 评分脚本会用正则匹配 "heap pos: 64" 等关键字
# 匹配到了 → 得分；没匹配到 → 0 分
```

**关键要求：你的内核必须输出带标记格式的文本！**

```
#### OS COMP TEST GROUP START basic-glibc ####  ← 评分脚本开始监听
========== START test_brk ==========              ← 单个测试开始
...测试输出...
========== END test_brk ==========                ← 单个测试结束
#### OS COMP TEST GROUP END basic-glibc ####    ← 评分脚本停止监听并打分
```

如果没有这些标记，评分脚本根本不知道你的内核在跑哪个测试组，**全部 0 分**。

### 6.3 即使测试程序不存在也能"跳过"

如果你的内核还没实现某些 syscall，对应的测试程序会崩溃，但标记格式仍然可以输出：

```
#### OS COMP TEST GROUP START iperf-glibc ####
iperf: execve failed (missing network stack)
#### OS COMP TEST GROUP END iperf-glibc ####
```

评分脚本看到 START 和 END 标记，即使中间没有有效输出，也能正常处理（给 0 分但不报错）。

> **官方允许跳过：** "您可以根据操作系统的完成度自由选择跳过其中若干个测试点，未被运行的测试点将不计分。"

---

## 七、如何解读输出

### 7.1 Docker 控制台输出

正常跑完会看到类似：

```
[os autotest]: Compile Start
[os autotest]: call make all to compile
编译完成
[os autotest]: Compile Succeed
运行：qemu-system-riscv64 -machine virt -kernel kernel-rv ...
正在评测：os_serial_out_rv.txt : basic-glibc
正在评测：os_serial_out_rv.txt : basic-musl
评测即将完成....
{
  "basic-glibc": {"score": 85, "details": {...}},
  "basic-musl": {"score": 72, "details": {...}}
}
```

### 7.2 常见错误信息

| 输出 | 原因 | 解决 |
|------|------|------|
| `No submit file` | submit 目录为空 | 检查 `-v` 挂载路径是否正确 |
| `CompileError: 编译出错` | `make all` 失败 | 先在本地 `cd /home/my-springos && make all` 确认能过 |
| `FAIL to run QEMU` | QEMU 启动失败 | 检查 `kernel-rv` 是否在项目根目录生成 |
| `QEMU 超时` | 内核没在超时时间内关机 | 检查 init 流程最后是否调用了 `shutdown()`，或增大 `config.json` 中的 `qemu.timeout` |
| 评分全部 0 分 | 串口输出格式不对 | 检查 init 是否输出了 `#### OS COMP TEST GROUP START xxx ####` 标记 |

---

## 八、配置调优

### 8.1 config.json

`oscomp-testdata/config.json` 的内容：

```json
{
    "debug": false,
    "qemu.smp": 1,
    "qemu.mem": "1G",
    "qemu.timeout": 3600
}
```

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `debug` | false | 是否开启调试模式 |
| `qemu.smp` | 1 | QEMU 模拟的 CPU 核数 |
| `qemu.mem` | "1G" | QEMU 内存大小 |
| `qemu.timeout` | 3600 | QEMU 超时（秒），超时后强制杀死 |

### 8.2 加速调试的建议

```bash
# 调小超时（调试阶段不需要等 1 小时）
# 修改 config.json:
# "qemu.timeout": 300     ← 5 分钟，够跑 basic 了

# 调大内存（你的内核可能需要）
# "qemu.mem": "2G"

# 多核（如果你的内核支持 SMP）
# "qemu.smp": 2
```

---

## 九、常见问题

### Q：Docker pull 太慢怎么办？

```bash
# 配置国内镜像加速器（阿里云 / 中科大）
sudo tee /etc/docker/daemon.json <<EOF
{
  "registry-mirrors": [
    "https://docker.mirrors.ustc.edu.cn",
    "https://hub-mirror.c.163.com"
  ]
}
EOF
sudo systemctl restart docker
```

### Q：SD 卡镜像不想从 GitHub 下载（太慢），能用本地的 testsuits 自己构建吗？

可以。`testsuits-for-oskernel` 仓库提供了自行构建的方法：

```bash
cd /home/chaos/testsuits-for-oskernel
make docker    # 进入 Docker 构建环境
make           # 构建所有测试程序 + SD 卡镜像
# 生成的 sdcard-*.img 在 sdcard/riscv/ 和 sdcard/loongarch/ 下
```

但需要完整的交叉编译环境，且构建时间较长。**推荐直接用预编译的 release 镜像**。

### Q：我只想测 RISC-V，不想跑 LoongArch 怎么办？

两种方式：

**方式一（推荐）：** 删掉 LoongArch 的 SD 卡镜像，评测脚本检测到文件不存在时会自动跳过：

```bash
rm /home/chaos/oscomp-testdata/sdcard-la.img.gz
```

**方式二：** 修改 `run.py`，注释掉 LoongArch 的线程启动（不推荐，改了官方代码）。

### Q：评测结果可以和线上平台完全一致吗？

**基本一致**，但有几个变量：

- 线上评测机可能用不同的 QEMU 版本（行为和性能可能有细微差异）
- 线上有额外的时间限制和资源限制
- 网络测试组（iperf/netperf）在本地 Docker 环境和线上环境网络拓扑不同

**basic / busybox / lua 这些纯 syscall 测试，本地和线上结果应该完全一致。**

### Q：Docker 运行后文件被锁死怎么办？

```bash
# 原因：强制 Ctrl+C 退出导致
# 解决：
sudo rm -f /home/chaos/oscomp-testdata/cancel_purge
# 如果还不行，重启 Docker 守护进程：
sudo systemctl restart docker
```

---

## 十、速查：从零到跑的完整命令序列

```bash
# ====== 第 1 步: 拉镜像（只需一次） ======
sudo docker pull zhouzhouyi/os-contest:20260510

# ====== 第 2 步: 准备测试数据（只需一次） ======
mkdir -p /home/chaos/oscomp-testdata
cp /home/chaos/autotest-for-oskernel/kernel/judge/* /home/chaos/oscomp-testdata/
cd /home/chaos/oscomp-testdata
wget https://github.com/oscomp/testsuits-for-oskernel/releases/download/pre-20250615/sdcard-rv.img.xz
wget https://github.com/oscomp/testsuits-for-oskernel/releases/download/pre-20250615/sdcard-la.img.xz
unxz sdcard-rv.img.xz && gzip sdcard-rv.img
unxz sdcard-la.img.xz && gzip sdcard-la.img

# ====== 第 3 步: 打包评测引擎（每次改完评测脚本后重新打包） ======
cd /home/chaos/autotest-for-oskernel/kernel && zip ../kernel.zip -r *

# ====== 第 4 步: 先确认你的内核能正常编译 ======
cd /home/my-springos && make all
# 确认 kernel-rv 已生成

# ====== 第 5 步: 运行评测（每次改完内核后执行） ======
cd /home/chaos
sudo docker run --rm \
  -v /home/my-springos:/coursegrader/submit \
  -v /home/chaos/oscomp-testdata:/coursegrader/testdata \
  -v /home/chaos/autotest-for-oskernel:/cg \
  -v /home/chaos/oscomp-testdata:/mnt/cghook/ \
  zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip
```

---

> **总结一句话：** autotest 本质上是一个 Docker 里的 Python 脚本，它做了三件事——① `make all` 编译你的内核，② 用 QEMU 启动内核 + 挂载测试镜像并抓取串口输出，③ 把输出喂给 `judge_*.py` 评分脚本用正则匹配打分。你只需要准备好 SD 卡镜像和评分脚本，一条 `docker run` 命令就能在本地复现线上评测的完整流程。
