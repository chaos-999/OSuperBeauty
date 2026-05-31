# SpringOS

队伍名称：静春山
学校名称：中山大学
队伍成员：陈思谋
指导教师：陈鹏飞

SpringOS是基于xv6-riscv的操作系统，支持RISC-V架构，LoongArch架构，可以在VisionFive2及龙芯2K1000LA开发板上运行。

当前为主分支，是决赛截止时提交的内容

## 项目特性

- **RISC-V架构**: 支持RISC-V 64位多处理器
- **LoongArch架构**: 支持LoongArch 64位多处理器
- **现代文件系统**: 集成EXT4文件系统支持
- **VFS架构**: 实现虚拟文件系统抽象层
- **信号系统**: 完整signal实现
- **Futex**: Futex实现
- **系统调用**: 完整的Linux兼容系统调用接口
- **网络模块**: TCP发送和接收函数，DNS解析功能，http服务端实现
- **VisionFive2**: 支持VisionFive2上板运行
- **2K1000星云板**: 支持2K1000星云板上板运行

## 项目分支
- main ： 项目主分支，包含qemu和VisionFive2及2K1000星云板上板
- net  ： 网络分支，包含网络模块实现
- on-site : 现场赛分支

## 构建和运行

```bash
make clean         # 清理构建文件
make all           # 构建内核
make qemu          # 在QEMU中运行RISCV系统
make qemu-la       # 在QEMU中运行LoongArch系统
make vf2           # 构建VisionFive2内核
make 2k1000        # 构建2k1000星云板内核
make qemu-sh       # 在QEMU中运行RISCV-shell系统
make qemu-la-sh    # 在QEMU中运行LoongArch-shell系统
make qemu-gdb      # 进行gdb调试
```

## 项目文档

技术文档位于 `/doc` 目录：
- [决赛文档](final-doc.pdf) - 决赛演示幻灯片
- [决赛幻灯片](final-ppt.pdf) - 决赛演示幻灯片
- [决赛演示视频](https://pan.baidu.com/s/1l6nM_kRB-vhkD5LBCnZK-g?pwd=e32s) - 决赛成果演示视频
---------------------------------------------------------------------------------------------------------------------------------------------------
- [阶段性成果幻灯片](stage.pptx) - 阶段性成果演示幻灯片
- [阶段性成果演示视频](https://pan.baidu.com/s/1g1h6aDVpZo7643xDxY-PFg?pwd=dsmx) - 阶段性成果演示视频
---------------------------------------------------------------------------------------------------------------------------------------------------
- [初赛幻灯片](pre.pptx) - 初赛演示幻灯片
- [初赛演示视频](https://pan.baidu.com/s/1MuKaoRHOWxm33fQTyf67zQ?pwd=cc78) - 初赛演示视频

## 参考项目

本项目基于以下开源项目：

- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) : MIT的教学操作系统，提供了基础的内核框架，SpringOS以xv6-riscv为baseline进行重构开发，重构了项目结构，优化启动流程和底层驱动，重构了内存管理，新增信号机制和Futex
- [xv6-loongarch-exp](https://github.com/SKT-CPUOS/xv6-loongarch-exp) : xv6的龙芯版本实现，SpringOS参考了xv6-loongarch-exp龙芯架构的实现，重构底层驱动，重构龙芯版本执行流程，按照龙芯2K1000LA处理器文档增加上板启动脚本，重构地址映射
- [cabbageOS](https://gitlab.eduxiji.net/T202410487992456/cabbageos)：2024优秀参赛作品，SpringOS参考了VFS以及EXT4文件系统实现
- [xv6-vf2](https://github.com/michaelengel/xv6-vf2)：xv6移植VisionFive2的实现，SpringOS参考了xv6-vf2移植VisionFive2开发板的启动实现，重构了中断异常处理和启动逻辑
- [network-for-xv6-riscv](https://github.com/wusskk/network-for-xv6-riscv?tab=readme-ov-file)：xv6增加网络模块的实现，SpringOS参考了network-for-xv6-riscv关于网络部分的实现，新增发送和接收缓冲区功能，TCP发送和接收函数，DNS解析功能
- [lwext4](https://github.com/gkostka/lwext4): 轻量级EXT4文件系统库，用于现代文件系统支持

## 参考资料

- [xv6 Book](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf) - xv6操作系统教程
- [RISC-V ISA Manual](https://riscv.org/specifications/) - RISC-V指令集架构规范
- [SBI Specification](https://github.com/riscv-non-isa/riscv-sbi-doc) - RISC-V SBI接口规范
- [Linux System Call Reference](https://man7.org/linux/man-pages/man2/syscalls.2.html) - Linux系统调用参考
- [VisionFive2 Reference](https://doc.rvspace.org/VisionFive2/Datasheet/) - VisionFive2文档
- [2k1000LA Reference](https://www.loongson.cn/uploads/images/2022090113542571398.%E9%BE%99%E8%8A%AF2K1000LA%E5%A4%84%E7%90%86%E5%99%A8%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C.pdf) - 龙芯2K1000LA处理器文档

## 许可证

/kernel/fs/lwext4 和 /include/fs/ext4 目录下的文件采用 GPL-2.0 许可证，这使得整个 SpringOS 也采用 [GPL-2.0](https://opensource.org/license/gpl-2-0) 许可证
