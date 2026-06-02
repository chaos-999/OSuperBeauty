# OSuperBeauty

队伍名称：超级美少女队
学校名称：中国人民解放军国防科大学
队伍成员：陈浩男 张朔瑜 张佳安琪
指导教师：文艳军

yet still not Ready to write


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
