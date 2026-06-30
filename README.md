# OSuperBeauty

> **初赛答辩 PPT**：[`pre.pptx`](pre.pptx) | **开发总结报告**：[`OSuperBeauty_开发总结报告.md`](OSuperBeauty_开发总结报告.md)
>
> PPT 中的评测得分记录来自提交 [`aeda0ff`](https://gitlab.eduxiji.net/T2026900029911083/OSuperBeauty/-/commit/aeda0ff13839b373c58676814855cb21b06f1df1)，后续提交对评测表现有进一步改进。

队伍名称：超级美少女队
学校名称：中国人民解放军国防科大学
队伍成员：陈浩男 张朔瑜 张佳安琪
指导教师：文艳军

## 项目简介

OSuperBeauty 是一个基于 [SpringOS](https://gitlab.eduxiji.net/educg-group-36002-2710490/T202510558995330-264)（2025 优秀参赛作品，中山大学「静春山」队）二次开发的 **RISC-V / LoongArch 双架构操作系统内核**。项目以通过 OS 内核竞赛评测为目标，在 SpringOS 的模块化内核架构之上，系统性地修复了 10+ 项评测阻塞性 bug，扩展系统调用至约 90 个，实现了 musl libc-test 的全量支持（96 PASS），并完成了 ext4 LRU use-after-free 等非确定性内核 bug 的根因定位。

**核心特性**：
- **双架构支持**：RISC-V 64 位（Sv39 分页）和 LoongArch 64 位，支持 QEMU virt 平台
- **EXT4 文件系统**：基于 lwext4 库，支持 VFS 抽象层、符号链接、动态链接器加载
- **Linux ABI 兼容**：约 90 个系统调用，完整 errno 语义，glibc/musl 双 C 库兼容
- **内存管理**：伙伴分配器、VMA 懒分配、mmap/munmap/mprotect、CoW fork
- **进程管理**：六状态模型、fork/exec/wait/clone、信号递送、futex 同步
- **设备驱动**：UART、VirtIO 块设备/网络、PLIC 中断控制器

## 快速开始

### 编译

```bash
# RISC-V 内核
make kernel-rv -j$(nproc)

# LoongArch 内核
make kernel-la -j$(nproc)
```

### 本地运行

```bash
# RISC-V
make qemu

# LoongArch（需要 Docker，因为 Ubuntu 22.04 的 QEMU 6.2 不支持 LoongArch）
make qemu-la
```

等价原始 QEMU 命令：

```bash
# RISC-V
qemu-system-riscv64 -machine virt -kernel kernel-rv -m 256M -nographic -smp 1 \
  -bios default -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 -no-reboot

# LoongArch
qemu-system-loongarch64 -machine virt -kernel kernel-la -m 1G -nographic -smp 1 \
  -drive file=sdcard-la.img,if=none,format=raw,id=x0 \
  -device virtio-blk-pci,drive=x0 -no-reboot \
  -device virtio-net-pci,netdev=net0 -netdev user,id=net0 -rtc base=utc
```

### 本地测评（模拟线上评分）

```bash
# RISC-V 完整测评
make test

# LoongArch 完整测评
make test-la
```

等价 Docker 命令：

```bash
# 测评（编译 + 运行 + 评分）
sudo docker run --rm \
  -v $(pwd):/coursegrader/submit \
  -v /path/to/oscomp-testdata:/coursegrader/testdata \
  -v /path/to/autotest-for-oskernel:/cg \
  -v /path/to/oscomp-testdata:/mnt/cghook/ \
  zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip

# 仅编译（使用 Docker 中的交叉编译器）
sudo docker run --rm -v $(pwd):/src -w /src \
  zhouzhouyi/os-contest:20260510 \
  make kernel-la -j$(nproc)

# 仅运行（使用 Docker 中的 QEMU）
sudo docker run --rm --privileged -v $(pwd):/src -w /src \
  zhouzhouyi/os-contest:20260510 \
  qemu-system-loongarch64 -machine virt -kernel kernel-la -m 1G -nographic \
    -drive file=sdcard-la.img,if=none,format=raw,id=x0 \
    -device virtio-blk-pci,drive=x0 -no-reboot \
    -device virtio-net-pci,netdev=net0 -netdev user,id=net0 -rtc base=utc
```

### Makefile 目标速查

| 目标 | 说明 |
|------|------|
| `make kernel-rv` | 编译 RISC-V 内核 |
| `make kernel-la` | 编译 LoongArch 内核 |
| `make kernel-rv-sh` | 编译 RISC-V 内核（sh 变体） |
| `make kernel-la-sh` | 编译 LoongArch 内核（sh 变体） |
| `make qemu` / `qemu-la` | 编译 + 运行 QEMU |
| `make qemu-gdb` / `qemu-gdb-la` | 编译 + GDB 调试 |
| `make test` / `test-la` | 本地测评（Docker） |
| `make clean` | 清理编译产物 |
| `make clean-test` | 清理测评临时文件 |
| `make tags` | 生成 etags 符号表 |

## 参考项目

本项目基于以下开源项目：

- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) : MIT的教学操作系统，提供了基础的内核框架，OSuperbeauty以xv6-riscv为baseline进行重构开发，重构了项目结构，优化启动流程和底层驱动，重构了内存管理，新增信号机制和Futex
- [xv6-loongarch-exp](https://github.com/SKT-CPUOS/xv6-loongarch-exp) : xv6的龙芯版本实现，OSuperbeauty参考了xv6-loongarch-exp龙芯架构的实现，重构底层驱动，重构龙芯版本执行流程，按照龙芯2K1000LA处理器文档增加上板启动脚本，重构地址映射
- [cabbageOS](https://gitlab.eduxiji.net/T202410487992456/cabbageos)：2024优秀参赛作品，OSuperbeauty参考了VFS以及EXT4文件系统实现
- [xv6-vf2](https://github.com/michaelengel/xv6-vf2)：xv6移植VisionFive2的实现，OSuperbeauty参考了xv6-vf2移植VisionFive2开发板的启动实现，重构了中断异常处理和启动逻辑
- [network-for-xv6-riscv](https://github.com/wusskk/network-for-xv6-riscv?tab=readme-ov-file)：xv6增加网络模块的实现，OSuperbeauty参考了network-for-xv6-riscv关于网络部分的实现，新增发送和接收缓冲区功能，TCP发送和接收函数，DNS解析功能
- [lwext4](https://github.com/gkostka/lwext4): 轻量级EXT4文件系统库，用于现代文件系统支持
- [SpringOS](https://gitlab.eduxiji.net/educg-group-36002-2710490/T202510558995330-264)：2025优秀参赛作品，OSuperbeauty参考了部分内核实现，并在此基础上做了针对测试集的评测优化

## 参考资料

- [xv6 Book](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf) - xv6操作系统教程
- [RISC-V ISA Manual](https://riscv.org/specifications/) - RISC-V指令集架构规范
- [SBI Specification](https://github.com/riscv-non-isa/riscv-sbi-doc) - RISC-V SBI接口规范
- [Linux System Call Reference](https://man7.org/linux/man-pages/man2/syscalls.2.html) - Linux系统调用参考
- [VisionFive2 Reference](https://doc.rvspace.org/VisionFive2/Datasheet/) - VisionFive2文档
- [2k1000LA Reference](https://www.loongson.cn/uploads/images/2022090113542571398.%E9%BE%99%E8%8A%AF2K1000LA%E5%A4%84%E7%90%86%E5%99%A8%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C.pdf) - 龙芯2K1000LA处理器文档

## 许可证

/kernel/fs/lwext4 和 /include/fs/ext4 目录下的文件采用 GPL-2.0 许可证，这使得整个 OSuperbeauty 也采用 [GPL-2.0](https://opensource.org/license/gpl-2-0) 许可证
