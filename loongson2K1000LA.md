# LOONGSON

龙芯2K1000LA处理器

用户手册

V1.0

2022年5月

龙芯中科技术股份有限公司

# 版权声明

本文档版权归龙芯中科技术股份有限公司所有，并保留一切权利。未经书面许可，任何公司和个人不得将此文档中的任何部分公开、转载或以其他方式散发给第三方。否则，必将追究其法律责任。

# 免责声明

本文档仅提供阶段性信息，所含内容可根据产品的实际情况随时更新，恕不另行通知。如因文档使用不当造成的直接或间接损失，本公司不承担任何责任。

# 龙芯中科技术股份有限公司

Loongson Technology Corporation Limited地址：北京市海淀区中关村环保科技示范园龙芯产业园2号楼Building No.2, Loongson Industrial Park, Zhongguancun Environmental Protection Park电话(Tel)：010- 62546668传真(Fax)：010- 62600826

# 阅读指南

《龙芯2K1000LA处理器用户手册》主要介绍龙芯2K1000LA架构与寄存器描述，对芯片系统架构、主要模块的功能与配置、寄存器列表及位域进行详细说明。

# 版本信息

<table><tr><td rowspan="3" colspan="2">版本信息</td><td>文档名</td><td>龙芯 2K1000LA 处理器用户手册</td></tr><tr><td>版本号</td><td>V1.0</td></tr><tr><td>创建人</td><td>芯片研发部</td></tr><tr><td colspan="4">更新历史</td></tr><tr><td>序号.</td><td>版本号</td><td colspan="2">更新内容</td></tr><tr><td>1</td><td>V1.0</td><td colspan="2">第一版</td></tr></table>

手册信息反馈：service@loongson.cn

# 目录

目录图目录 .IX表目录 .X1概述 .11.1 体系结构框图 .11.2 芯片主要功能 .21.2.1 处理器核 .21.2.2 内存接口 .21.2.3 PCIE 接口 .31.2.4 GPU .31.2.5 显示控制器 .31.2.6 SATA 控制器 .31.2.7 USB2.0 控制器 .31.2.8 GMAC 控制器 .31.2.9 VPU 解码器 .41.2.10 CAMERA 控制器 .41.2.11 HDA 控制器 .41.2.12 I2S 控制器 .41.2.13 NAND 控制器 .51.2.14 SPI 控制器 .51.2.15 UART .51.2.16 I2C 总线 .51.2.17 PWM .61.2.18 HPET .61.2.19 RTC .61.2.20 Watchdog .61.2.21 中断控制器 .61.2.22 CAN .61.2.23 ACPI 功耗管理 .61.2.24 GPIO .71.2.25 加解密模块 .71.2.26 SDIO 控制器 .72 引脚定义 .82.1 约定 .82.2 DDR3 接口 .82.3 PCIE 接口 .92.4 DVO 显示接口 .92.5 GMAC 接口 .112.6 SATA 接口 .122.7 USB 接口 .122.8 CAMERA 接口 .122.9 HDA 接口 .132.10 SPI 接口 .142.11 I2C 接口 .142.12 UART 接口 .142.13 CAN 接口 .152.14 NAND 接口 .162.15 SDIO 接口 .162.16 GPIO .162.17 PWM .17

2.18 PLL 电源接口 ..... 172.19 电源管理接口 ..... 172.20 测试接口 ..... 182.21 JTAG 接口 ..... 182.22 时钟信号 ..... 182.23 RTC 相关信号 ..... 182.24 系统相关信号 ..... 192.25 外设功能复用表 ..... 203 时钟结构 ..... 213.1 NODE PLL ..... 213.2 PIX PLL ..... 213.3 DDR PLL ..... 223.4 DC PLL ..... 223.5 内部 PLL 配置方法 ..... 233.5.1 硬件配置 ..... 233.5.2 软件配置 ..... 233.6 BOOT 时钟 ..... 243.7 USB 参考时钟 ..... 243.8 PCIE 参考时钟 ..... 243.9 SATA 参考时钟输入 ..... 244 电源管理 ..... 254.1 电源管理模块介绍 ..... 254.2 电源级别 ..... 254.3 控制引脚说明 ..... 255 芯片配置寄存器 ..... 275.1 通用配置寄存器 0 ..... 345.2 通用配置寄存器 1 ..... 365.3 通用配置寄存器 2 ..... 375.4 APBDMA 配置寄存器 ..... 395.5 USB PHY0/1 配置寄存器 ..... 405.6 USB PHY2/3 配置寄存器 ..... 445.7 SATA 配置寄存器 ..... 455.8 NODE PLL 低 64 位配置寄存器 ..... 465.9 NODE PLL 高 64 位配置寄存器 ..... 475.10 DDR PLL 低 64 位配置寄存器 ..... 475.11 DDR PLL 高 64 位配置寄存器 ..... 485.12 DC PLL 低 64 位配置寄存器 ..... 485.13 DC PLL 高 64 位配置寄存器 ..... 495.14 PIX0 PLL 低 64 位配置寄存器 ..... 495.15 PIX0 PLL 高 64 位配置寄存器 ..... 505.16 PIX1 PLL 低 64 位配置寄存器 ..... 515.17 PIX1 PLL 高 64 位配置寄存器 ..... 515.18 FREQSCALE 配置寄存器 ..... 525.19 PCIE0 配置寄存器 0 ..... 525.20 PCIE0 配置寄存器 1 ..... 535.21 PCIE0 PHY 配置控制寄存器 ..... 545.22 PCIE1 配置寄存器 0 ..... 545.23 PCIE1 配置寄存器 1 ..... 555.24 PCIE1 PHY 配置控制寄存器 ..... 565.25 DMA 命令控制寄存器(DMA_ORDER) ..... 565.26 PICICFG2_RECFG 寄存器 ..... 57

5.27 PCICFG30_RECFG 寄存器 575.28 PCICFG31_RECFG 寄存器 585.29 PCICFG40_RECFG 寄存器 595.30 PCICFG41_RECFG 寄存器 595.31 PCICFG42_RECFG 寄存器 605.32 PCICFG5_RECFG 寄存器 605.33 PCICFG6_RECFG 寄存器 615.34 PCICFG7_RECFG 寄存器 615.35 PCICFG8_RECFG 寄存器 625.36 PCICFGF_RECFG 寄存器 635.37 PCICFG10_RECFG 寄存器 635.38 PCICFG11_RECFG 寄存器 64

# 6 地址空间分配 65

# 6.1 一级交叉开关 66

6.2 二级交叉开关 66

# 6.3 IO 互连网络 69

6.3.1 IO 设备的配置空间 706.3.2 APB 配置头 776.3.3 GMAC0/1 配置头 786.3.4 USB OTG 配置头 786.3.5 USBEHCI 配置头 796.3.6 USB OHCI 配置头 796.3.7 GPU 配置头 796.3.8 DC 配置头 806.3.9 HDA 配置头 806.3.10 SATA 配置头 816.3.11 PCIE 配置头 816.3.12 DMA 配置头 826.3.13 VPU 配置头 826.3.14 CAMERA 配置头 836.3.15 N/A 处理 836.4 IODMA 请求 836.5 APB 设备路由 867 处理器核间中断与通信 888 温度传感器 908.1 实时温度采样 908.2 高低温中断触发 908.3 高温自动降频设置 919 I/O 中断 939.1 中断触发类型 949.2 中断分发模式 949.3 中断相关寄存器描述 949.4 GPIO 中断 999.5 MSI 中断 999.6 硬件中断负载均衡功能举例 10010 SPI 控制器 10310.1 访问地址 10310.2 SPI 控制器结构 10310.3 配置寄存器 10310.3.1 控制寄存器(SPCR) 10310.3.2 状态寄存器(SPSR) 104

10.3.3 数据寄存器(TxFIFO/RxFIFO) 10410.3.4 外部寄存器(SPER) 10410.3.5 参数控制寄存器(SFC_PARAM) 10510.3.6 片选控制寄存器(SFC_SOFTCS) 10510.3.7 时序控制寄存器(SFC_TIMING) 10510.4 接口时序 10610.4.1 SPI 主控制器接口时序 10610.4.2 SPI Flash 访问时序 10610.5 软件编程指南 10710.5.1 SPI 主控制器的读写操作 10710.5.2 硬件 SPI Flash 读 10710.5.3 混合访问 SPI Flash 和 SPI 主控制器 10811 LocalIO 控制器 10911.1 访问地址及引脚复用 10911.2 LOCALIO 控制器功能概述 10912 DDR3 控制器 11112.1 访问地址 11112.2 DDR3 SDRAM 控制器功能概述 11112.3 DDR3 SDRAM 读操作协议 11112.4 DDR3 SDRAM 写操作协议 11212.5 DDR3 控制器寄存器 11213 GPIO 11613.1 GPIO 方向控制 11613.2 GPIO 输出设置 11613.3 GPIO 输入采样 11613.4 GPIO 中断使能 11613.5 GPIO 复用关系 11714 APB 设备（Dev 2） 11914.1 内部设备地址路由 11915 UART 控制器 12115.1 概述 12115.2 访问地址及引脚复用 12115.3 控制器结构 12115.4 寄存器描述 12215.4.1 数据寄存器（DAT） 12215.4.2 中断使能寄存器（IER） 12215.4.3 中断标识寄存器（IIR） 12315.4.4 FIFO 控制寄存器（FCR） 12315.4.5 线路控制寄存器（LCR） 12415.4.6 MODEM 控制寄存器（MCR） 12415.4.7 线路状态寄存器（LSR） 12515.4.8 MODEM 状态寄存器（MSR） 12615.4.9 分频锁存器 12616 CAN 12716.1 访问地址及引脚复用 12716.2 标准模式 12716.2.1 控制寄存器（CR） 12816.2.2 命令寄存器（CMR） 12916.2.3 状态寄存器（SR） 12916.2.4 中断寄存器（IR） 13016.2.5 验收代码寄存器（ACR） 130

16.2.6 验收屏蔽寄存器（AMR） 13016.2.7 发送缓冲区列表 13116.2.8 接收缓冲区列表 13116.3 扩展模式 13116.3.1 模式寄存器（MOD） 13416.3.2 命令寄存器（CMR） 13416.3.3 状态寄存器（SR） 13516.3.4 中断寄存器（IR） 13516.3.5 中断使能寄存器（IER） 13616.3.6 仲裁丢失捕捉寄存器 13616.3.7 错误警报限制寄存器（EMLR） 13716.3.8 RX 错误计数寄存器（RXERR） 13816.3.9 TX 错误计数寄存器（TXERR） 13816.3.10 验收滤波器 13816.3.11 RX 信息计数寄存器（RMCR） 13816.4 公共寄存器 13816.4.1 总线定时寄存器 0（BTR0） 13816.4.2 总线定时寄存器 1（BTR1） 13916.4.3 输出控制寄存器（OCR） 13917 I2C 控制器 14017.1 概述 14017.2 访问地址及引脚复用 14017.3 I2C 控制器结构 14017.4 I2C 控制器寄存器说明 14117.4.1 分频锁存器低字节寄存器（PRERlo） 14117.4.2 分频锁存器高字节寄存器（PRERhi） 14117.4.3 控制寄存器（CTR） 14117.4.4 发送数据寄存器（TXR） 14217.4.5 接受数据寄存器（RXR） 14217.4.6 命令控制寄存器（CR） 14217.4.7 状态寄存器（SR） 14318 PWM 控制器 14418.1 概述 14418.2 访问地址及引脚复用 14418.3 寄存器描述 14418.4 功能说明 14518.4.1 脉宽调制功能 14518.4.2 脉冲测量功能 14518.4.3 防死区功能 14619 HPET 控制器 14719.1 概述 14719.2 访问地址 14719.3 寄存器描述 14720 NAND 控制器 15120.1 NAND 控制器结构描述 15120.2 访问地址及引脚复用 15120.3 NAND 寄存器配置描述 15120.3.1 命令寄存器 NAND_CMD（偏移地址 0x00） 15120.3.2 页内偏移地址寄存器 ADDR_C（偏移地址 0x04） 15220.3.3 页地址寄存器 ADDR_R（偏移地址 0x08） 15220.3.4 时序寄存器 NAND_TIMING（偏移地址 0x0C） 152

20.3.5 ID 寄存器 ID_L (偏移地址 0x10) 15320.3.6 ID 和状态寄存器 STATUS & ID_H (偏移地址 0x14) 15320.3.7 参数配置寄存器 NAND_PARAMETER (偏移地址 0x18) 15320.3.8 操作数量寄存器 NAND_OP_NUM (偏移地址 0x1CY) 15320.3.9 映射寄存器 CS_RDY_MAP (偏移地址 0x20) 15420.3.10 DMA 读写数据寄存器 DMA_ADDRESS (偏移地址 0x40) 15420.4 NAND ADDR 说明 15420.5 NAND- FLASH 读写操作举例 15720.6 NAND ECC 说明 15720.7 支持 NAND 型号 15921 电源管理模块 16021.1 概述 16021.2 访问地址 16021.3 寄存器描述 16022 RTC 17022.1 概述 17022.2 访问地址 17022.3 寄存器描述 17022.3.1 寄存器地址列表 17022.3.2 SYS_TOYWRITE0 17122.3.3 SYS_TOYWRITE1 17122.3.4 SYS_TOYREAD0 17122.3.5 SYS_TOYREAD1 17122.3.6 SYS_TOYMATCH0/1/2 17222.3.7 SYS_RTCCTRL 17222.3.8 SYS_RTCWRITE 17322.3.9 SYS_RTCREAD 17322.3.10 SYS_RTCMATCH0/1/2 17323 加解密 17423.1 DES 17423.1.1 DES 功能概述 17423.1.2 DES 访问地址： 17423.1.3 DES 寄存器描述 17423.2 AES 17523.2.1 AES 功能概述 17523.2.2 AES 访问地址： 17623.2.3 AES 寄存器描述 17623.3 RSA 17823.3.1 RSA 访问地址： 17823.4 RNG 17823.4.1 RNG 访问地址： 17824 SDIO 控制器 17924.1 功能概述 17924.2 访问地址及引脚复用 17924.3 SDIO 协议概述 17924.4 寄存器描述 18024.5 软件编程指南 18624.5.1 SD Memory 卡软件编程说明 18624.5.2 SDIO 卡软件编程说明 18824.6 支持 SDIO 型号 18925 I2S 控制器 19025.1 概述 190

# 25.2 访问地址及引脚复用 190

25.3 接口协议 19025.4 专用寄存器 19125.5 配置操作 19226 GMAC 控制器（Dev 3） 19426.1 访问地址及引脚复用 19427 OTG 控制器（Dev 4, Fun 0） 19527.1 概述 19527.2 访问地址 19528 USB 控制器（Dev 4, Fun 1/2） 19628.1 总体概述 19628.2 访问地址 19629 图形处理器（Dev 5） 19829.1 访问地址 19830 显示控制器（Dev 6） 19930.1 概述 19930.2 访问地址及引脚复用 19931 HDA 控制器（Dev 7） 20031.1 功能概述 20031.2 访问地址 20032 SATA 控制器（Dev 8） 20132.1 SATA 总体描述 20132.2 访问地址 20132.3 SATA 控制器内部寄存器描述 20133 PCIE 控制器（Dev 9/A/B/C/D/E） 20333.1 总体结构 20333.2 访问地址 20333.3 地址空间划分 20433.4 软件编程指南 20533.4.1 PCIE 控制器使能 20533.4.2 PCIE 配置头访问 20533.4.3 PCIE 链路建立(Linkup) 20533.4.4 TYPE1 类型配置访问 20633.4.5 PCIE PHY 配置方法 20633.5 常用例程 20634 DMA 控制器（Dev F） 20934.1 DMA 控制器结构描述 20934.2 访问地址 20934.3 DMA 控制器与 APB 设备的交互 21034.4 DMA 描述符 21034.4.1 DMA_ORDER_ADDR_LOW 21034.4.2 DMA_SADDR 21034.4.3 DMA_DADDR 21134.4.4 DMA_LENGTH 21134.4.5 DMA_STEP_LENGTH 21134.4.6 DMA_STEP_TIMES 21234.4.7 DMA_GMD 21234.4.8 DMA_ORDER_ADDR_HIGH 21334.4.9 DMA_SADDR_HIGH 21335 VPU 控制器（Dev 16） 21435.1 访问地址 214

36 CAMERA 接口控制器（Dev 17） 21536.1 功能概述 21536.2 访问地址 215

# 图目录

图 1- 1 龙芯 2K1000 结构图 ..... 2图 3- 1 NODE PLL 结构图 ..... 21图 3- 2 PIX PLL 结构图 ..... 22图 3- 3 DDR PLL 结构图 ..... 22图 3- 4 DC PLL 结构图 ..... 23图 3- 5 BOOT 时钟结构图 ..... 24图 6- 1 二级交叉开关地址路由示意图 ..... 66图 6- 2 IO 互连结构图 ..... 70图 6- 3 64 位配置访问地址格式 ..... 70图 6- 4 32 位配置访问地址格式 ..... 71图 9- 1 龙芯 2K1000 处理器中断路由示意图 ..... 93图 10- 1 SPI 主控制器接口时序 ..... 106图 10- 2 SPI Flash 标准读时序 ..... 106图 10- 3 SPI Flash 快速读时序 ..... 106图 10- 4 SPI Flash 双向 I/O 读时序 ..... 107图 11- 1 LocalIO 读时序 ..... 109图 11- 2 LocalIO 写时序 ..... 110图 12- 1 DDR3 SDRAM 读操作协议 ..... 112图 12- 2 DDR3 SDRAM 写操作协议 ..... 112图 15- 1 UART 控制器结构 ..... 122图 17- 1 I2C 主控制器结构 ..... 141图 18- 1 防死区功能 ..... 146图 24- 1 SD 卡多块写操作示意图 ..... 180图 24- 2 SD 卡多块读操作示意图 ..... 180图 24- 3 SD Memory 卡初始化流程示意图 ..... 187图 25- 1 I2S 传输协议 ..... 191图 28- 1 USB 主机控制器模块图 ..... 196图 33- 1 PCIE 控制器结构 ..... 203

# 表目录

表2- 1信号类型代码 8表2- 2DDR3SDRAM控制器接口信号 8表2- 3PCIE总线信号 9表2- 4DVO接口信号 9表2- 5DVO接口RGB对应关系 10表2- 6DVO0与LI0复用关系 10表2- 7DVO0与UART复用关系 10表2- 8GMAC接口信号 11表2- 9GMAC1与GPIO复用关系 11表2- 10SATA接口信号 12表2- 11SATA与GPIO复用关系 12表2- 12USB接口信号 12表2- 13CAMERA接口信号 12表2- 14CAMERA与DVO1复用关系 13表2- 15HDA接口信号 13表2- 16HDA与I2S复用关系 13表2- 17HDA与GPIO复用关系 13表2- 18SPI接口信号 14表2- 19I2C接口信号 14表2- 20I2C与GPIO复用关系 14表2- 21UART接口信号 14表2- 22UART接口复用关系 15表2- 23CAN接口信号 15表2- 24CAN与GPIO复用关系 15表2- 25NAND接口信号 16表2- 26NAND与GPIO复用关系 16表2- 27SDIO接口信号 16表2- 28SDIO与GPIO复用关系 16表2- 29GPIO信号 17表2- 30PWM信号 17表2- 31PWM与GPIO复用关系 17表2- 32PLL电源接口 17表2- 33电源管理接口 17

表2- 34 测试接口 ..... 18表2- 35 JTAG接口 ..... 18表2- 36 时钟信号 ..... 18表2- 37 时钟信号 ..... 18表2- 38 系统相关信号 ..... 19表2- 39 外设功能复用表 ..... 20表3- 1 PLL硬件配置 ..... 23表4- 1 ACPI状态说明 ..... 25表4- 2 控制引脚说明 ..... 25表5- 1 芯片配置寄存器列表 ..... 27表5- 2 通用配置寄存器0 ..... 34表5- 3 通用配置寄存器1 ..... 36表5- 4 通用配置寄存器2 ..... 37表5- 5 APBDMA配置寄存器 ..... 39表5- 6 USB 0/1 PHY配置寄存器 ..... 40表5- 7 USB 2/3 PHY配置寄存器 ..... 44表5- 8 SATA配置寄存器 ..... 45表5- 9 NODEPLL低64位配置寄存器 ..... 46表5- 10 NODEPLL高64位配置寄存器 ..... 47表5- 11 DDRPLL低64位配置寄存器 ..... 47表5- 12 DDRPLL高64位配置寄存器 ..... 48表5- 13 DCPLL低64位配置寄存器 ..... 48表5- 14 DCPLL高64位配置寄存器 ..... 49表5- 15 PIX0PLL低64位配置寄存器 ..... 49表5- 16 PIX0PLL高64位配置寄存器 ..... 50表5- 17 PIX1PLL低64位配置寄存器 ..... 51表5- 18 PIX1PLL高64位配置寄存器 ..... 51表5- 19 FRESCALE配置寄存器 ..... 52表5- 20 PCIE0配置寄存器0 ..... 52表5- 21 PCIE0配置寄存器1 ..... 53表5- 22 PCIE0PHY配置寄存器 ..... 54表5- 23 PCIE1配置寄存器0 ..... 54表5- 24 PCIE1配置寄存器1 ..... 55表5- 25 PCIE1PHY配置寄存器 ..... 56

表5- 26 DMA命令控制寄存器. .56表5- 27 PCICFG2_RECFG配置寄存器 .57表5- 28 PCICFG30_RECFG配置寄存器 .57表5- 29 PCICFG31_RECFG配置寄存器 .58表5- 30 PCICFG40_RECFG配置寄存器 .59表5- 31 PCICFG41_RECFG配置寄存器 .59表5- 32 PCICFG42_RECFG配置寄存器 .60表5- 33 PCICFG5_RECFG配置寄存器 .60表5- 34 PCICFG6_RECFG配置寄存器 .61表5- 35 PCICFG7_RECFG配置寄存器 .61表5- 36 PCICFG8_RECFG配置寄存器 .62表5- 37 PCICFGf_RECFG配置寄存器 .63表5- 38 PCICFG10_RECFG配置寄存器 .63表5- 39 PCICFG11_RECFG配置寄存器 .64表6- 1芯片地址空间划分 .65表6- 2一级交叉开关路由规则 .66表6- 3二级交叉开关处标号与所述模块的对应关系 .67表6- 4 MMAP字段对应的该空间访问属性 .67表6- 5二级交叉开关地址窗口转换寄存器表 .67表6- 6各个设备的配置头访问对应关系 .71表6- 7 Type0类型配置头 .72表6- 8 Type0的配置头寄存器 .72表6- 9 Type1类型配置头 .74表6- 10 Type1的配置头寄存器 .75表6- 11 APB总线控制器的配置头缺省值 .77表6- 12 GMAC0控制器的配置头缺省值 .78表6- 13 USB- OTG控制器的配置头缺省值 .78表6- 14 USB- EHCI控制器的配置头缺省值 .79表6- 15 USB- OHCI控制器的配置头缺省值 .79表6- 16 GPU控制器的配置头缺省值 .80表6- 17 DC控制器的配置头缺省值 .80表6- 18 HDA控制器的配置头缺省值 .80表6- 19 SATA控制器的配置头缺省值 .81表6- 20PCIE0Port0的配置头缺省值 .81

表6- 21 DMA控制器的配置头缺省值 .82表6- 22 VPU解码器的配置头缺省值 .82表6- 23 CAMERA控制器的配置头缺省值 .83表6- 24 MMAP字段对应的该空间访问属性 .84表6- 25 IO设备DMA访存地址转换寄存器表 .84表6- 26 APB设备地址译码 .86表7- 1处理器核间中断相关的寄存器及其功能描述 .88表7- 20号处理器核核间中断与通信寄存器列表 .88表7- 31号处理器核的核间中断与通信寄存器列表 .88表8- 1温度采样寄存器说明 .90表8- 2高低温中断寄存器说明 .91表8- 3高温降频控制寄存器说明 .92表9- 1中断控制寄存器属性 .94表9- 2中断控制寄存器地址 .96表9- 3中断路由寄存器的说明 .97表9- 4中断路由寄存器地址 .97表9- 5 GPIO中断 .99表9- 6 MSI中断相关寄存器 .100表10- 1SPI控制器地址空间分配 .103表10- 2SPI配置寄存器列表 .103表10- 3SPI控制寄存器(SPCR) .103表10- 4SPI状态寄存器(SPSR) .104表10- 5SPI数据寄存器(TxFIFO/RXFIFO) .104表10- 6SPI外部寄存器(SPER) .104表10- 7SPI分频系数 .104表10- 8SPI参数控制寄存器(SFC_PARAM) .105表10- 9SPI片选控制寄存器(SFC_SOFTCS) .105表10- 10SPI时序控制寄存器(SFC_TIMING) .105表11- 1LocalIO地址空间分配 .109表12- 1内存控制器地址空间分配 .111表12- 2DDR3控制器配置寄存器 .112表13- 1 GPIO配置寄存器 .116表13- 2 GPIO方向控制 .116表13- 3 GPIO输出设置 .116

表 13- 4 GPIO 输入采样 ..... 116表 13- 5 GPIO 中断使能 ..... 116表 13- 6 GPIO 复用关系 ..... 117表 14- 1 APB 配置访问信息 ..... 119表 14- 2 APB 设备地址译码 ..... 119表 15- 1 UART 控制器物理地址构成 ..... 121表 15- 2 UART 数据寄存器 ..... 122表 15- 3 UART 中断使能寄存器 ..... 123表 15- 4 UART 中断标识寄存器 ..... 123表 15- 5 UART 中断控制功能表 ..... 123表 15- 6 UART 的 FIFO 控制寄存器 ..... 124表 15- 7 UART 线路控制寄存器 ..... 124表 15- 8 UART 的 MODEM 控制寄存器 ..... 125表 15- 9 UART 线路状态寄存器 ..... 125表 15- 10 UART 的 MODEM 状态寄存器 ..... 126表 15- 11 UART 分频锁存器 1 ..... 126表 15- 12 UART 分频锁存器 2 ..... 126表 16- 1 CAN 内部寄存器物理地址构成 ..... 127表 18- 1 PWM 寄存器列表 ..... 144表 18- 2 PWM 控制寄存器设置 ..... 144表 25- 1 寄存器定义 ..... 191表 25- 2 标识寄存器 ..... 191表 25- 3 配置寄存器 ..... 191表 25- 4 控制寄存器 ..... 192表 34- 1 DMA ORDER 寄存器 ..... 209

# 1概述

1概述龙芯2K1000LA处理器（简称龙芯2K1000）主要面向于网络应用，兼顾平板应用及工控领域应用。片内集成2个LA264处理器核，采用LoongArch指令系统（龙架构），主频1GHz，64位DDR3控制器，并集成各种系统IO接口。

龙芯2K1000的主要特征如下：

$\bullet$  片内集成两个64位的双发射超标量LA264处理器核，主频1GHz $\bullet$  片内集成共享的1MB二级Cache $\bullet$  片内集成GPU $\bullet$  片内集成显示控制器，支持双路DVO显示 $\bullet$  片内集成64位533MHz的DDR3控制器 $\bullet$  片内集成2个  $\times 4$  PCIE2.0接口；可以拆分为6个独立x1接口 $\bullet$  片内集成1个SATA2.0接口 $\bullet$  片内集成4个USB2.0接口 $\bullet$  片内集成2个RGMII千兆网接口 $\bullet$  片内集成HDA/I2S接口 $\bullet$  片内集成RICHPET模块 $\bullet$  片内集成12个UART控制器 $\bullet$  片内集成1个NAND控制器 $\bullet$  片内集成2个CAN控制器 $\bullet$  片内集成1个SDIO控制器 $\bullet$  片内集成2个I2C控制器 $\bullet$  片内集成1个LIO控制器 $\bullet$  片内集成1个VPU解码器 $\bullet$  片内集成1个CAMERA接口控制器 $\bullet$  片内集成1个温度传感器 $\bullet$  集成动态功耗控制模块 $\bullet$  采用FC- BGA封装

# 1.1 体系结构框图

1.1 体系结构框图龙芯2K1000的结构如图1- 1所示。一级交叉开关连接两个处理器核、两个二级Cache以及IO子网络（Cache访问路径）。二级交叉开关连接两个二级Cache、内存控制器、启动模块（SPI或者LIO）以及IO子网络（Uncache访问路径）。IO子网络连接一级交叉开关，

以减少处理器访问延迟。IO 子网络中包括需要 DMA 的模块（PCIE、GMAC、SATA、USB、HDA/I2S、NAND、SDIO、DC、GPU、VPU、CAMERA 和加解密模块）和不需要 DMA 的模块，需要 DMA 的模块可以通过 Cache 或者 Uncache 方式访问内存。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/219441139907432c6e11e284d1991e42554dff2e8ee20e09788e2706c469752a.jpg)  
图 1-1 龙芯 2K1000 结构图

# 1.2 芯片主要功能

# 1.2.1 处理器核

- LA264- 采用 LoongArch 指令系统（龙架构）- 32KB 数据 Cache 和 32KB 的指令 Cache- 1M 共享二级 Cache- 通过目录协议维护 I/O DMA 访问的 Cache 一致性

# 1.2.2 内存接口

- 64 位 DDR3 控制器，最高工作频率 533MHz- 不支持 ECC- 可配置为 32/16 位模式- 支持命令调度

# 1.2.3 PCIE接口

兼容PCIE2.0 $\bullet$  双独立X4接口 $\bullet$  其中一路X4接口可以配置为4个X1接口 $\bullet$  其中一路X4接口可以配置为2个X1接口

# 1.2.4 GPU

支持OpenVGYUV色域空间转换高质量缩放

# 1.2.5 显示控制器

双DVO(通用并行显示接口)输出硬件光标伽玛校正输出抖动最高支持165MHz1080p支持线性显示缓冲上电序列控制

# 1.2.6 SATA控制器

1个SATA端口支持SATA1.5Gbps和SATA2代3Gbps的传输兼容串行ATA2.6规范和AHCI1.1规范

# 1.2.7 USB2.0控制器

4个独立的USB2.0的HOST端口其中端口0固定为OTG工作模式兼容USB1.1和USB2.0内部EHCI控制和实现高速传输可达480Mbps内部OHCI控制和实现全速和低速传输低功耗管理

# 1.2.8 GMAC控制器

两路10/100/1000Mbps自适应以太网MAC $\bullet$  双网卡均兼容IEEE802.3 $\bullet$  对外部PHY实现RGMII接口

- 半双工/全双工自适应- Timestamp 功能- 半双工时，支持碰撞检测与重发（CSMA/CD）协议- 支持 CRC 校验码的自动生成与校验，支持前置符生成与删除- 支持网络开机

1.2.9 VPU 解码器

- 支持 H264- 支持 MPEG-4- 支持 MPEG-2- 支持 MPEG-1- 支持 JPEG

1.2.10 CAMERA 控制器

- 兼容 ITU-R BT 601/656 8-bit 模式外部接口（支持同步信号产生的同步或是嵌入式同步）- 使用内嵌的 DMA 方式进行存取数据操作- 8-bit 视频数据输入，输入数据顺序固定，为 U01Y0V01Y1U23Y2V23Y3……（因为这是最为常用的 4：2：2 格式的数据顺序）- 独立于图片尺寸的水平和垂直的尺寸设置- 可编程水平、垂直同步信号极性

1.2.11 HDA 控制器

- 支持 16，18 和 20 位采样精度支持可变速率- 最高达 192KHz- 7.1 频道环绕立体声输出- 三路音频输入

1.2.12 I2S 控制器

- 支持 master 模式下 I2S 输入- 支持 master 模式下 I2S 输出- 支持 8、16、18、20、24、32 位宽- 支持单声道和立体声道音频数据- 支持 (16、22.05、32、44.1、48)kHz 采样频率- 支持 DMA 传输模式

# 1.2.13 NAND 控制器

最大支持单片16GB NAND Flash- 最大支持 4 个片选- 支持 MLC- 支持 512/2K/4K/8K 页

# 1.2.14 SPI 控制器

- 双缓冲接收器- 极性和相位可编程的串行时钟- 主模式支持- 支持到 4 个的变长字节传输- 支持系统启动- 支持标准读、连续地址读、快速读、双路 I/O 等 SPI Flash 读模式

# 1.2.15 UART

- 3 个全功能 UART 和流控 TXD,RXD,CTS, RTS, DSR,DTR,DCD, RI- 最多 12 个 UART 接口- 在寄存器与功能上兼容 NS16550A- 两路全双工异步数据接收/发送- 可编程的数据格式- 16 位可编程时钟计数器- 支持接收超时检测- 带仲裁的多中断系统

# 1.2.16 I2C 总线

- 兼容 SMBUS（100Kbps）- 与 PHILIPS I2C 标准相兼容- 履行双向同步串行协议- 只实现主设备操作- 能够支持多主设备的总线- 总线的时钟频率可编程- 可以产生开始/停止/应答等操作- 能够对总线的状态进行探测- 支持低速和快速模式- 支持 7 位寻址和 10 位寻址- 支持时钟延伸和等待状态

# 1.2.17PWM

32位计数器 $\bullet$  支持脉冲生成及捕获4路控制器

# 1.2.18HPET

32位计数器支持1个周期性中断支持2个非周期性中断

# 1.2.19RTC

计时精确到0.1秒可产生3个计时中断支持定时开机功能

# 1.2.20 Watchdog

32比特计数器及初始化寄存器低功耗模式暂停功能

# 1.2.21中断控制器

支持软件设置中断支持电平与边沿触发支持中断屏蔽与使能支持多种中断分发模式

# 1.2.22CAN

符合CAN2.0规范两路CAN接口支持中断复用GPIO

# 1.2.23ACPI功耗管理

处理器核动态频率电压调节全芯片时钟门控PHY可关断USB/GMAC可唤醒来电可自动启动

# 1.2.24 GPIO

- 4位专用 GPIO 引脚，56 位复用 GPIO 引脚- 其余引脚与其他接口相复用，使用各个接口电压域

# 1.2.25 加解密模块

- AES、DES 算法支持- RSA 算法支持

# 1.2.26 SDIO 控制器

- 1 路独立 SDIO 控制器- 兼容 SD Memory 2.0/MMC/SDIO 2.0 协议

# 2引脚定义

2 引脚定义龙芯2K1000的引脚进行了大量的功能复用。对于有复用关系的引脚，在介绍完其本身的功能之外会同时在下方给出其他复用功能的描述。

# 2.1约定

本章对龙芯2K1000引脚定义的说明使用以下约定：

信号名

信号名的选取以方便记忆和明确标识功能为原则。低有效信号以n结尾，高有效信号则不带n。

类型

信号的输入输出类型由一个代码表示，见表2- 1。

表2-1信号类型代码  

<table><tr><td>代码</td><td>描述</td></tr><tr><td>A</td><td>模型</td></tr><tr><td>DIFF I/O</td><td>双向差分</td></tr><tr><td>DIFF IN</td><td>差分输入</td></tr><tr><td>DIFF OUT</td><td>差分输出</td></tr><tr><td>I</td><td>输入</td></tr><tr><td>I/O</td><td>双向</td></tr><tr><td>O</td><td>输出</td></tr><tr><td>OD</td><td>开漏输出</td></tr><tr><td>P</td><td>电源</td></tr><tr><td>G</td><td>地</td></tr></table>

# 2.2 DDR3接口

表2-2DDR3SDRAM控制器接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>DDR_DQ[63:0]</td><td>I/O</td><td>DDR3 SDRAM 数据总线信号</td></tr><tr><td>DDR_DQSp[7:0]</td><td>DIFF I/O</td><td>DDR3 SDRAM 数据选通</td></tr><tr><td>DDR_DQSn[7:0]</td><td></td><td></td></tr><tr><td>DDR_DQM[7:0]</td><td>O</td><td>DDR3 SDRAM 数据屏蔽</td></tr><tr><td>DDR_A[15:0]</td><td>O</td><td>DDR3 SDRAM 地址总线信号</td></tr><tr><td>DDR_BA[2:0]</td><td>O</td><td>DDR3 SDRAM 逻辑 BANK 地址信号</td></tr><tr><td>DDR_WEn</td><td>O</td><td>DDR3 SDRAM 写使能信号</td></tr><tr><td>DDR_CASn</td><td>O</td><td>DDR3 SDRAM 列地址选择信号</td></tr><tr><td>DDR_RASn</td><td>O</td><td>DDR3 SDRAM 行地址选择信号</td></tr><tr><td>DDR_CSn[3:0]</td><td>O</td><td>DDR3 SDRAM 片选信号</td></tr><tr><td>DDR_CKE[3:0]</td><td>O</td><td>DDR3 SDRAM 时钟使能信号</td></tr></table>

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>DDR_CKp[7:0]</td><td rowspan="2">DIFF OUT</td><td rowspan="2">DDR3 SDRAM 差分时钟输出信号</td></tr><tr><td>DDR_CKn[7:0]</td></tr><tr><td>DDR_ODT[3:0]</td><td>O</td><td>DDR3 SDRAM ODT 信号</td></tr><tr><td>DDR_RESETn</td><td>O</td><td>DDR3 SDRAM 复位控制信号</td></tr></table>

# 2.3 PCIE接口

表2-3PCIE总线信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>PCIE[1:0] _REFCLKp
PCIE[1:0] _REFCLKn</td><td>DIFF IN</td><td>PCIE 参考时钟输入</td></tr><tr><td>PCIE0 _REFCLKp[3:0]
PCIE0 _REFCLKn[3:0]</td><td>DIFF OUT</td><td>PCIE0 参考时钟输出</td></tr><tr><td>PCIE1 _REFCLKp[1:0]
PCIE1 _REFCLKn[1:0]</td><td>DIFF OUT</td><td>PCIE1 参考时钟输出</td></tr><tr><td>PCIE[1:0] _REFRES</td><td>A</td><td>外部参考电阻，通过 200ohm(+/-1%)电阻连至地</td></tr><tr><td>PCIE[1:0] _TXp[3:0]
PCIE[1:0] _TXn[3:0]</td><td>DIFF OUT</td><td>PCIE 差分数据输出</td></tr><tr><td>PCIE[1:0] _RXp[3:0]
PCIE[1:0] _RXn[3:0]</td><td>DIFF IN</td><td>PCIE 差分数据输入</td></tr><tr><td>PCIE0 _PRSNT[3:0]</td><td>I</td><td>PCIE0 插卡检测,bit5-1 任意一位置低时 PCIE0 工作在 4X1 模式，否则工作在 1X4 模式</td></tr><tr><td>PCIE1 _PRSNT[1:0]</td><td>I</td><td>PCIE1 插卡检测，bit1 置低时 PCIE1 工作在 2X1 模式，否则工作在 1X4 模式</td></tr><tr><td>PCIE_RSTn</td><td>O</td><td>PCIE 复位</td></tr></table>

# 2.4 DVO显示接口

表2-4DVO接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>DVO[1:0] CLKp</td><td>O</td><td>DVO 时钟输出</td></tr><tr><td>DVO[1:0] CLKn</td><td>O</td><td>DVO 时钟输出，与 DVO*CLKp 相差 180°，非差分关系</td></tr><tr><td>DVO[1:0] HSYNC</td><td>O</td><td>DVO 水平同步</td></tr><tr><td>DVO[1:0] VSYNC</td><td>O</td><td>DVO 垂直同步</td></tr><tr><td>DVO[1:0] DE</td><td>O</td><td>DVO 数据有效</td></tr><tr><td>DVO[1:0] D[23:0]</td><td>O</td><td>DVO 显示数据
[23:16]为 R 数据
[15:08]为 G 数据
[07:00]为 B 数据</td></tr></table>

DVO接口数据信号与RGB对应关系如下：

表2-5DVO接口RGB对应关系  

<table><tr><td>DVO 接口信号</td><td>24 位模式</td><td>18 位模式</td></tr><tr><td>DVO_D0</td><td>B0</td><td></td></tr><tr><td>DVO_D1</td><td>B1</td><td></td></tr><tr><td>DVO_D2</td><td>B2</td><td>B0</td></tr><tr><td>DVO_D3</td><td>B3</td><td>B1</td></tr><tr><td>DVO_D4</td><td>B4</td><td>B2</td></tr><tr><td>DVO_D5</td><td>B5</td><td>B3</td></tr><tr><td>DVO_D6</td><td>B6</td><td>B4</td></tr><tr><td>DVO_D7</td><td>B7</td><td>B5</td></tr><tr><td>DVO_D8</td><td>G0</td><td></td></tr><tr><td>DVO_D9</td><td>G1</td><td></td></tr><tr><td>DVO_D10</td><td>G2</td><td>G0</td></tr><tr><td>DVO_D11</td><td>G3</td><td>G1</td></tr><tr><td>DVO_D12</td><td>G4</td><td>G2</td></tr><tr><td>DVO_D13</td><td>G5</td><td>G3</td></tr><tr><td>DVO_D14</td><td>G6</td><td>G4</td></tr><tr><td>DVO_D15</td><td>G7</td><td>G5</td></tr><tr><td>DVO_D16</td><td>R0</td><td></td></tr><tr><td>DVO_D17</td><td>R1</td><td></td></tr><tr><td>DVO_D18</td><td>R2</td><td>R0</td></tr><tr><td>DVO_D19</td><td>R3</td><td>R1</td></tr><tr><td>DVO_D20</td><td>R4</td><td>R2</td></tr><tr><td>DVO_D21</td><td>R5</td><td>R3</td></tr><tr><td>DVO_D22</td><td>R6</td><td>R4</td></tr><tr><td>DVO_D23</td><td>R7</td><td>R5</td></tr></table>

DVO0接口与LIO以及UART有复用关系，如表2- 6和表2- 7DVO0与UART复用关系所示。复用配置请参考表5- 3通用配置寄存器1和表5- 4通用配置寄存器2。

表2-6DVO0与LIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>DVO0_CLKp</td><td>LIO_RDn</td><td>O</td><td>LIORDn 输出</td></tr><tr><td>DVO0_CLKr</td><td>LIO_WRn</td><td>O</td><td>LIOWRn 输出</td></tr><tr><td>DVO0_HSYNC</td><td>LIO_DEN</td><td>O</td><td>LIO 数据使能</td></tr><tr><td>DVO0_VSYNC</td><td>LIO_DIR</td><td>O</td><td>LIO 方向控制，0 代表读，1 代表写</td></tr><tr><td>DVO0_DE</td><td>LIO_ADLOCK</td><td>O</td><td>LIO 地址/数据选择信号</td></tr><tr><td>DVO0_D[15:0]</td><td>LIO_AD[15:0]</td><td>I/O</td><td>LIO 双向 AD 信号</td></tr><tr><td>DVO0_D[22:16]</td><td>LIO_A[6:0]</td><td>O</td><td>LIO 地址低位</td></tr><tr><td>DVO0_D23</td><td>LIO_CSn</td><td>O</td><td>LIO 片选信号</td></tr></table>

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>DVOO_HSYNC</td><td>UART1_TXD</td><td>O</td><td>串口数据输出</td></tr><tr><td>DVOO_VSYNC</td><td>UART1_RXD</td><td>I</td><td>串口数据输入</td></tr><tr><td>DVOO_DE</td><td>UART1_RTS</td><td>O</td><td>串口数据传输请求</td></tr><tr><td>DVOO_D00</td><td>UART1_DTR</td><td>O</td><td>串口初始化完成</td></tr><tr><td>DVOO_D01</td><td>UART1_RI</td><td>I</td><td>外部 MODEM 探测到振铃信号</td></tr><tr><td>DVOO_D02</td><td>UART1_CTS</td><td>I</td><td>设备接受数据就绪</td></tr><tr><td>DVOO_D03</td><td>UART1_DSR</td><td>I</td><td>设备初始化完成</td></tr><tr><td>DVOO_D04</td><td>UART1_DCD</td><td>I</td><td>外部 MODEM 探测到载波信号</td></tr><tr><td>DVOO_D05</td><td>UART2_TXD</td><td>O</td><td>串口数据输出</td></tr><tr><td>DVOO_D06</td><td>UART2_RXD</td><td>I</td><td>串口数据输入</td></tr><tr><td>DVOO_D07</td><td>UART2_RTS</td><td>O</td><td>串口数据传输请求</td></tr><tr><td>DVOO_D08</td><td>UART2_DTR</td><td>O</td><td>串口初始化完成</td></tr><tr><td>DVOO_D09</td><td>UART2_RI</td><td>I</td><td>外部 MODEM 探测到振铃信号</td></tr><tr><td>DVOO_D11</td><td>UART2_CTS</td><td>I</td><td>设备接受数据就绪</td></tr><tr><td>DVOO_D12</td><td>UART2_DSR</td><td>I</td><td>设备初始化完成</td></tr><tr><td>DVOO_D13</td><td>UART2_DCD</td><td>I</td><td>外部 MODEM 探测到载波信号</td></tr></table>

DVO1接口与CAMERA接口有复用关系，参考2.8节。

# 2.5 GMAC接口

表2-8GMAC接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>GMAC[1:0]TXCK</td><td>O</td><td>RGMII发送时钟</td></tr><tr><td>GMAC[1:0]TCTL</td><td>O</td><td>RGMII发送控制</td></tr><tr><td>GMAC[1:0]TXD[3:0]</td><td>O</td><td>RGMII发送数据</td></tr><tr><td>GMAC[1:0]RXCK</td><td>I</td><td>RGMII接收时钟</td></tr><tr><td>GMAC[1:0]RCTL</td><td>I</td><td>RGMII接收控制</td></tr><tr><td>GMAC[1:0]RXD[3:0]</td><td>I</td><td>RGMII接收数据</td></tr><tr><td>GMAC[1:0]MDCK</td><td>O</td><td>SMA接口时钟</td></tr><tr><td>GMAC[1:0]MDIO</td><td>I/O</td><td>SMA接口数据</td></tr></table>

GMAC1接口与GPIO有复用关系，如下表所示。复用配置请参考表5- 2通用配置寄存器0。

表2-9GMAC1与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>GMAC1_TXCK</td><td>-</td><td>-</td><td>-</td></tr><tr><td>GMAC1_TCTL</td><td>GPIO13</td><td>I/O</td><td>通用输入输出 13</td></tr><tr><td>GMAC1_TXD[3:0]</td><td>GPIO[12:9]</td><td>I/O</td><td>通用输入输出 12:9</td></tr><tr><td>GMAC1_RXCK</td><td>-</td><td>-</td><td>-</td></tr></table>

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>GMAC1_RCTL</td><td>GPIO8</td><td>I/O</td><td>通用输入输出 8</td></tr><tr><td>GMAC1_RXD[3:0]</td><td>GPIO[7:4]</td><td>I/O</td><td>通用输入输出 7-4</td></tr><tr><td>GMAC1_MDCK</td><td>-</td><td>-</td><td>-</td></tr><tr><td>GMAC1_MDIO</td><td>-</td><td>-</td><td>-</td></tr></table>

# 2.6 SATA接口

表2-10SATA接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>SATA_REFCLKp
SATA_REFCLKn</td><td>I</td><td>差分 100MHz 参考时钟输入(内部有备份时钟,通过软件控制)</td></tr><tr><td>SATA_REFRES</td><td>A</td><td>外部参考电阻,通过 200ohm(+/-1%)电阻连至地</td></tr><tr><td>SATA_TXp
SATA_TXn</td><td>DIFF OUT</td><td>SATA 差分数据输出</td></tr><tr><td>SATA_RXp
SATA_RXn</td><td>DIFF IN</td><td>SATA 差分数据输入</td></tr><tr><td>SATA_LEDn</td><td>O</td><td>SATA 工作状态,低表示有数据传输</td></tr></table>

SATA接口的SATA_LEDn与GPIO有复用关系，如下表所示。复用配置请参考表5- 2通用配置寄存器0。

表2-11SATA与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>SATA_LEDn</td><td>GPIO14</td><td>I/O</td><td>通用输入输出 14</td></tr></table>

# 2.7 USB接口

表2-12USB接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>USB_XI
USB_XO</td><td>I/O</td><td>必须在 USB_XO 上接晶振，USB_XI 保留不用</td></tr><tr><td>USB[3:0]TXRTUNE</td><td>A</td><td>参考电阻，通过 200ohm/1% 电阻连接到地</td></tr><tr><td>USB[3:0]DP</td><td>I/O</td><td>USB D+</td></tr><tr><td>USB[3:0]DM</td><td>I/O</td><td>USB D</td></tr><tr><td>USB[3:1]OC</td><td>I</td><td>USB 过流检测输入，需注意该信号为高有效</td></tr><tr><td>USB0_OC</td><td>O</td><td>OTG DRVVBUS 输出</td></tr><tr><td>USB0_ID</td><td>I</td><td>USB0 OTG ID 输入</td></tr><tr><td>USB0_VBUS</td><td>A</td><td>USB0 OTG VBUS 输入</td></tr></table>

# 2.8 CAMERA接口

表2- 13CAMERA接口信号

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>CAM_PCLK</td><td>I</td><td>像素时钟，被摄像头的处理器驱动</td></tr><tr><td>CAM_HSYNC</td><td>I</td><td>水平同步，被摄像头的处理器驱动</td></tr><tr><td>CAM_VSYNC</td><td>I</td><td>帧同步，被摄像头的处理器驱动</td></tr><tr><td>CAM_DATA[7:0]</td><td>I</td><td>像素数据，被摄像头的处理器驱动</td></tr><tr><td>CAM_CLOCK</td><td>O</td><td>XCLK 被摄像头控制器驱动，用于摄像头模块</td></tr></table>

注：控制信号都是单向的，只支持camera提供时钟和同步信号的模式

表2-14CAMERA与DVO1复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>CAM_PCLK</td><td>DVO1_CKN</td><td>I</td><td>DVO1 负时钟输出</td></tr><tr><td>CAM_HSYNC</td><td>DVO1_HSYNC</td><td>I</td><td>DVO1 水平同步信号</td></tr><tr><td>CAM_VSYNC</td><td>DVO1_VSYNC</td><td>I</td><td>DVO1 垂直同步信号</td></tr><tr><td>CAM_DATA[7:0]</td><td>DVO1_D[7:0]</td><td>I</td><td>DVO1 数据输出信号</td></tr><tr><td>CAM_CLOCK</td><td>DVO1_CKP</td><td>O</td><td>DVO1 正时钟输出</td></tr></table>

# 2.9 HDA接口

表2-15HDA接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>HDA_BITCLK</td><td>O</td><td>HDA_BITCLK 输出</td></tr><tr><td>HDA_SDIO</td><td>I</td><td>HDA 数据输入，连接第一个 codec</td></tr><tr><td>HDA_SDII</td><td>I</td><td>HDA 数据输入，连接第二个 codec</td></tr><tr><td>HDA_SDIO</td><td>I</td><td>HDA 数据输入，连接第三个 codec</td></tr><tr><td>HDA_SDO</td><td>O</td><td>HDA 数据输出</td></tr><tr><td>HDA_SYNC</td><td>O</td><td>HDA 同步</td></tr><tr><td>HDA_RESET</td><td>O</td><td>HDA 复位</td></tr></table>

HDA接口与I2S以及GPIO复用，具体复用关系如下。复用配置请参考表5- 2通用配置寄存器0。

表2-16HDA与I2S复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>HDA_BITCLK</td><td>I2S_BCLK</td><td>O</td><td>I2S bit 时钟</td></tr><tr><td>HDA_SDIO</td><td>I2S_DI</td><td>I</td><td>I2S 数据输入</td></tr><tr><td>HDA_SDII</td><td>-</td><td>-</td><td>-</td></tr><tr><td>HDA_SDIO</td><td>-</td><td>-</td><td>-</td></tr><tr><td>HDA_SDO</td><td>I2S_DO</td><td>O</td><td>I2S 数据输出</td></tr><tr><td>HDA_SYNC</td><td>I2S_MCLK</td><td>O</td><td>I2S MCLK</td></tr><tr><td>HDA_RESET</td><td>I2S_LR</td><td>O</td><td>I2S 左右声道选择</td></tr></table>

# 表2-17HDA与GPIO复用关系

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>HDA_BITCLK</td><td>GPIO24</td><td>I/O</td><td>通用输入输出 24</td></tr><tr><td>HDA_SDI0</td><td>GPIO28</td><td>I/O</td><td>通用输入输出 28</td></tr><tr><td>HDA_SDI1</td><td>GPIO29</td><td>I/O</td><td>通用输入输出 29</td></tr><tr><td>HDA_SDI2</td><td>GPIO30</td><td>I/O</td><td>通用输入输出 30</td></tr><tr><td>HDA_SDO</td><td>GPIO27</td><td>I/O</td><td>通用输入输出 27</td></tr><tr><td>HDA_SYNC</td><td>GPIO25</td><td>I/O</td><td>通用输入输出 25</td></tr><tr><td>HDA_RESETn</td><td>GPIO26</td><td>I/O</td><td>通用输入输出 26</td></tr></table>

# 2.10SPI接口

表2-18SPI接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>SPI_SCK</td><td>O</td><td>SPI 时钟输出</td></tr><tr><td>SPI_CSn0</td><td>O</td><td>SPI 片选 0</td></tr><tr><td>SPI_CSn1</td><td>O</td><td>SPI 片选 1</td></tr><tr><td>SPI_CSn2</td><td>O</td><td>SPI 片选 2</td></tr><tr><td>SPI_CSn3</td><td>O</td><td>SPI 片选 3</td></tr><tr><td>SPI_SDO</td><td>O</td><td>SPI 数据输出</td></tr><tr><td>SPI_SDI</td><td>I</td><td>SPI 数据输入</td></tr></table>

# 2.11I2C接口

表2-19I2C接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>I2C0_SCL</td><td>O</td><td>I2C0 时钟</td></tr><tr><td>I2C0_SDA</td><td>I/O</td><td>I2C0 数据</td></tr><tr><td>I2C1_SCL</td><td>O</td><td>I2C1 时钟</td></tr><tr><td>I2C1_SDA</td><td>I/O</td><td>I2C1 数据</td></tr></table>

I2C与GPIO有复用，复用关系见下表。复用配置请参考表5- 2通用配置寄存器0。

表2-20I2C与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>I2C0_SCL</td><td>GPIO16</td><td>I/O</td><td>通用输入输出 16</td></tr><tr><td>I2C0_SDA</td><td>GPIO17</td><td>I/O</td><td>通用输入输出 17</td></tr><tr><td>I2C1_SCL</td><td>GPIO18</td><td>I/O</td><td>通用输入输出 18</td></tr><tr><td>I2C1_SDA</td><td>GPIO19</td><td>I/O</td><td>通用输入输出 19</td></tr></table>

# 2.12UART接口

表2- 21UART接口信号

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>UART_TXD</td><td>O</td><td>串口数据输出</td></tr><tr><td>UART_RXD</td><td>I</td><td>串口数据输入</td></tr><tr><td>UART_RTS</td><td>O</td><td>串口数据传输请求</td></tr><tr><td>UART_DTR</td><td>O</td><td>串口初始化完成</td></tr><tr><td>UART_RI</td><td>I</td><td>外部 MODEM 探测到振铃信号</td></tr><tr><td>UART_CTS</td><td>I</td><td>设备接受数据就绪</td></tr><tr><td>UART_DSR</td><td>I</td><td>设备初始化完成</td></tr><tr><td>UART_DCD</td><td>I</td><td>外部 MODEM 探测到载波信号</td></tr></table>

UART与DVO接口有复用关系，具体见2.4节。

2K1000仅有一个独立的全功能串口，该串口通过设置可以工作在  $2\mathrm{x}4$  和  $4\mathrm{x}2$  模式，各种模式的管脚对应关系如下。其它引脚复用的UART接口的内部复用关系也如下表所示。

表2-22UART接口复用关系  

<table><tr><td>1x8</td><td>2x4</td><td>4x2</td></tr><tr><td>TXD0(O)</td><td>TXD0(O)</td><td>TXD0(O)</td></tr><tr><td>RTS0(O)</td><td>RTS0(O)</td><td>TXD5(O)</td></tr><tr><td>DTR0(O)</td><td>TXD3(O)</td><td>TXD3(O)</td></tr><tr><td>RXD0(I)</td><td>RXD0(I)</td><td>RXD0(I)</td></tr><tr><td>CTS0(I)</td><td>CTS0(I)</td><td>RXD5(I)</td></tr><tr><td>DSR0(I)</td><td>RXD3(I)</td><td>RXD3(I)</td></tr><tr><td>DCD0(I)</td><td>CTS3(I)</td><td>RXD4(I)</td></tr><tr><td>RI0(I)</td><td>RTS3(O)</td><td>TXD4(O)</td></tr></table>

# 2.13CAN接口

表2-23CAN接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>CAN0_RX</td><td>I</td><td>CAN 通道 0 数据接收</td></tr><tr><td>CAN0_TX</td><td>O</td><td>CAN 通道 0 数据发送</td></tr><tr><td>CAN1_RX</td><td>I</td><td>CAN 通道 1 数据接收</td></tr><tr><td>CAN1_TX</td><td>O</td><td>CAN 通道 1 数据发送</td></tr></table>

CAN接口与GPIO有复用，如下表所示。复用配置请参考表5- 2通用配置寄存器0。

表2-24CAN与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>CAN0_RX</td><td>GPIO32</td><td>I/O</td><td>通用输入输出 32</td></tr><tr><td>CAN0_TX</td><td>GPIO33</td><td>I/O</td><td>通用输入输出 33</td></tr><tr><td>CAN1_RX</td><td>GPIO34</td><td>I/O</td><td>通用输入输出 34</td></tr><tr><td>CAN1_TX</td><td>GPIO35</td><td>I/O</td><td>通用输入输出 35</td></tr></table>

# 2.14 NAND 接口

表2-25NAND接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>NAND_CEn[3:0]</td><td>O</td><td>NAND 片选 3-0</td></tr><tr><td>NAND_CLE</td><td>O</td><td>NAND 命令锁存</td></tr><tr><td>NAND_ALE</td><td>I</td><td>NAND 地址锁存</td></tr><tr><td>NAND_WRn</td><td>O</td><td>NAND 写信号</td></tr><tr><td>NAND_RDn</td><td>I</td><td>NAND 读信号</td></tr><tr><td>NAND_RDYn[3:0]</td><td>I</td><td>NAND 准备好输入 3-0</td></tr><tr><td>NAND_D[7:0]</td><td>I/O</td><td>NAND 命令/地址/数据线</td></tr></table>

NAND与GPIO有复用，复用关系见下表。复用配置请参考表5- 2通用配置寄存器0。

表2-26NAND与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>NAND_CEn[3:0]</td><td>GPIO[47:44]</td><td>I/O</td><td>通用输入输出 47-44</td></tr><tr><td>NAND_CLE</td><td>GPIO48</td><td>I/O</td><td>通用输入输出 48</td></tr><tr><td>NAND_ALE</td><td>GPIO49</td><td>I/O</td><td>通用输入输出 49</td></tr><tr><td>NAND_WRn</td><td>GPIO50</td><td>I/O</td><td>通用输入输出 50</td></tr><tr><td>NAND_RDn</td><td>GPIO51</td><td>I/O</td><td>通用输入输出 51</td></tr><tr><td>NAND_RDYn[3:0]</td><td>GPIO55:52</td><td>I/O</td><td>通用输入输出 55:52</td></tr><tr><td>NAND_D[7:0]</td><td>GPIO[63:56]</td><td>I/O</td><td>通用输入输出 63-56</td></tr></table>

# 2.15 SDIO 接口

表2-27SDIO接口信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>SDIO_CLK</td><td>O</td><td>SDIO 时钟输出</td></tr><tr><td>SDIO_CMD</td><td>I/O</td><td>SDIO 命令输入输出</td></tr><tr><td>SDIO_DATA[3:0]</td><td>I/O</td><td>SDIO 数据信号</td></tr></table>

SDIO与GPIO有复用，复用关系见下表。复用配置请参考表5- 2通用配置寄存器0。

表2-28SDIO与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>SDIO_CLK</td><td>GPIO41</td><td>I/O</td><td>通用输入输出 41</td></tr><tr><td>SDIO_CMD</td><td>GPIO40</td><td>I/O</td><td>通用输入输出 40</td></tr><tr><td>SDIO_DATA[3:0]</td><td>GPIO[39:36]</td><td>I/O</td><td>通用输入输出 39-36</td></tr></table>

# 2.16 GPIO

下表仅列出专用的4个GPIO引脚信号，其他GPIO为复用信号，可参考其他信号定义。默认情况下所有与GPIO复用的引脚为GPIO功能（GMAC1除外），且都为输入状态。

表2-29GPIO信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>GPIO00</td><td>I/O</td><td>通用输入输出</td></tr><tr><td>GPIO01</td><td>I/O</td><td>通用输入输出</td></tr><tr><td>GPIO02</td><td>I/O</td><td>通用输入输出</td></tr><tr><td>GPIO03</td><td>I/O</td><td>通用输入输出</td></tr></table>

# 2.17PWM

表2-30PWM信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>PWM[3:0]</td><td>O</td><td>PWM 输出</td></tr></table>

PWM与GPIO有复用，复用关系如下。复用配置请参考表5- 2通用配置寄存器0。

表2-31PWM与GPIO复用关系  

<table><tr><td>信号名称</td><td>复用名称</td><td>复用类型</td><td>复用信号描述</td></tr><tr><td>PWM[3:0]</td><td>GPIO[23:20]</td><td>I/O</td><td>通用输入输出 23-20</td></tr></table>

# 2.18PLL电源接口

表2-32PLL电源接口  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>PLL_NODE_VDD</td><td>P</td><td>NODE PLL 电源</td></tr><tr><td>PLL_SOC_VDD</td><td>P</td><td>SOC PLL 电源</td></tr><tr><td>PLL_DDR_VDD</td><td>P</td><td>DDR PLL 电源</td></tr><tr><td>PLL_PIX0_VDD</td><td>P</td><td>PIXEL0 PLL 电源</td></tr><tr><td>PLL_PIX1_VDD</td><td>P</td><td>PIXEL1 PLL 电源</td></tr><tr><td>PLL_NODE_VSS</td><td>P</td><td>NODE PLL 地</td></tr><tr><td>PLL_SOC_VSS</td><td>P</td><td>SOC PLL 地</td></tr><tr><td>PLL_DDR_VSS</td><td>P</td><td>DDR PLL 地</td></tr><tr><td>PLL_PIX0_VSS</td><td>P</td><td>PIXEL0 PLL 地</td></tr><tr><td>PLL_PIX1_VSS</td><td>P</td><td>PIXEL1 PLL 地</td></tr></table>

# 2.19电源管理接口

表2-33电源管理接口  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>ACPI_SYSRSTn</td><td>I</td><td>系统复位</td></tr><tr><td>ACPI_RINGn</td><td>I</td><td>振铃唤醒</td></tr><tr><td>ACPI_WAKEEn</td><td>I</td><td>PCIE 唤醒</td></tr><tr><td>ACPI_LID</td><td>I</td><td>屏盖状态</td></tr><tr><td>ACPI_PWRTYPE</td><td>I</td><td>供电来源</td></tr><tr><td>ACPI_BATLOWn</td><td>I</td><td>电源电量低</td></tr></table>

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>ACPI SUSSTATn</td><td>O</td><td>低功耗状态</td></tr><tr><td>ACPI S3n</td><td>O</td><td>$3 状态</td></tr><tr><td>ACPI S4n</td><td>O</td><td>$4 状态</td></tr><tr><td>ACPI S5n</td><td>O</td><td>$5 状态</td></tr><tr><td>ACPI VID[5:0]</td><td>O</td><td>调压控制</td></tr><tr><td>ACPI PLTRSTn</td><td>O</td><td>平台复位</td></tr><tr><td>ACPI SLPLANn</td><td>O</td><td>网络电源控制</td></tr><tr><td>ACPI PWRBTNn</td><td>I</td><td>电源开关</td></tr><tr><td>ACPI PWROK</td><td>I</td><td>电源有效</td></tr><tr><td>ACPI EN</td><td>I</td><td>ACPI 使能</td></tr></table>

# 2.20测试接口

表2-34测试接口  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>ACPI_DOTTESTn</td><td>I</td><td>测试模式控制(RTC 电压域)
0: 测试模式
1: 功能模式</td></tr></table>

# 2.21JTAG接口

表2-35JTAG接口  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>JTAG_SEL</td><td>I</td><td>JTAG 选择(1: JTAG; 0: CPU JTAG)</td></tr><tr><td>JTAG_TCK</td><td>I</td><td>JTAG 时钟</td></tr><tr><td>JTAG_TDI</td><td>I</td><td>JTAG 数据输入</td></tr><tr><td>JTAG_TMS</td><td>I</td><td>JTAG 模式</td></tr><tr><td>JTAG_TRST</td><td>I</td><td>JTAG 复位</td></tr><tr><td>JTAG_TDO</td><td>O</td><td>JTAG 数据输出</td></tr></table>

# 2.22时钟信号

表2-36时钟信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>SYS_SYSCLK</td><td>I</td><td>100MHz 参考时钟</td></tr><tr><td>SYS_TESTCLK</td><td>I</td><td>测试时钟输入， 默认不用连接</td></tr></table>

# 2.23RTC相关信号

表2-37时钟信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>RTC_RSMRSTn</td><td>I</td><td>RSM 域复位,要求在 RSM 域电源稳定 1ms 后拉高,</td></tr></table>

<table><tr><td></td><td></td><td>在 RSM 域电源降至 95%及以下时立即拉低。</td></tr><tr><td>RTC_RSTn</td><td>I</td><td>RTC 域复位，建议在 RTC 电源稳定 500ms 后再解除复位。</td></tr><tr><td>RTC_XI</td><td>I/O</td><td>32.768KHz 晶体接口</td></tr><tr><td>RTC_XO</td><td>I/O</td><td>32.768KHz 晶体接口</td></tr></table>

# 2.24系统相关信号

表2-38系统相关信号  

<table><tr><td>信号名称</td><td>类型</td><td>描述</td></tr><tr><td>SYS_CLKSEL[1:0]</td><td>I</td><td>PLL 时钟配置输入
00=低频模式
01=高频模式
10=软件模式(DFT)
11=bypass 模式</td></tr><tr><td>SYS_BOOTSEL[1:0]</td><td>I</td><td>启动选择输入
00=LIO
01=SPI(DFT)</td></tr><tr><td>SYS_USBCLKMODE[1:0]</td><td>I</td><td>USB 时钟输入配置输入
00=保留
01=保留
10=one 12MHz clock input
11=use sysclk(DFT)</td></tr><tr><td>SYS_PCIECLKSEL</td><td>I</td><td>PCIE 参考时钟选择输入
{SYS_PCIECLKSEL, SYS_PCIECLKDIV:
00=内部 100MHz 时钟
01=保留
10=外部 100MHz 时钟
11=外部 200MHz 时钟</td></tr><tr><td>SYS_PCIECLKDIV</td><td>I</td><td>PCIE 参考时钟选择输入
{SYS_PCIECLKSEL, SYS_PCIECLKDIV:
00=内部 100MHz 时钟
01=保留
10=外部 100MHz 时钟
11=外部 200MHz 时钟</td></tr><tr><td>SYS_NANDRSRD</td><td>I</td><td>NAND ECC 功能使能输入,
1=enable
0=disable(DFT)</td></tr><tr><td>SYS_NANDTYPE[1:0]</td><td>I</td><td>NAND 类型选择
00=512Mb(page 512B)
01=1Gb(page 2KB)
10=16Gb(page 4KB)
11=128Gb(page 8KB)</td></tr></table>

# 2.25外设功能复用表

模块层次的功能复用关系如下表所示：

表2-39外设功能复用表  

<table><tr><td>功能0</td><td>功能1</td><td>功能2</td><td>功能3</td><td>功能4</td><td>功能5</td></tr><tr><td>DDR3</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>PCIE</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>SATA</td><td>GPIO(1)</td><td></td><td></td><td></td><td></td></tr><tr><td>USB</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>GMAC0</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>GMAC1</td><td>GPIO(14)</td><td></td><td></td><td></td><td></td></tr><tr><td rowspan="9">DVO0</td><td rowspan="8"></td><td rowspan="8">Local Bus</td><td rowspan="4">UART1(8)</td><td rowspan="2">UART1(4)</td><td>UART1(2)</td></tr><tr><td>UART8(2)</td></tr><tr><td rowspan="2">UART6(4)</td><td>UART6(2)</td></tr><tr><td>UART7(2)</td></tr><tr><td rowspan="4">UART2(8)</td><td rowspan="2">UART2(4)</td><td>UART2(2)</td></tr><tr><td>UART11(2)</td></tr><tr><td rowspan="2">UART9(4)</td><td>UART9(2)</td></tr><tr><td>UART10(2)</td></tr><tr><td></td><td></td><td></td><td></td><td></td></tr><tr><td rowspan="2">DVO1</td><td></td><td></td><td rowspan="2">CAMERA</td><td></td><td></td></tr><tr><td></td><td></td><td></td><td></td></tr><tr><td>CAN</td><td>GPIO(4)</td><td></td><td></td><td></td><td></td></tr><tr><td>HDA</td><td>GPIO(7)</td><td></td><td></td><td>I2S</td><td></td></tr><tr><td>SPI</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>RTC</td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>I2C</td><td>GPIO(4)</td><td></td><td></td><td></td><td></td></tr><tr><td></td><td></td><td></td><td rowspan="4">UART0(8)</td><td rowspan="2">UART0(4)</td><td>UART0(2)</td></tr><tr><td></td><td></td><td></td><td rowspan="2">UART5(2)</td></tr><tr><td></td><td></td><td></td><td rowspan="2">UART3(2)</td></tr><tr><td></td><td></td><td></td><td>UART4(2)</td></tr><tr><td>NAND</td><td>GPIO(16)</td><td></td><td></td><td></td><td></td></tr><tr><td>CPU_JTAG</td><td></td><td>JTAG</td><td></td><td></td><td></td></tr><tr><td></td><td>GPIO(4)</td><td></td><td></td><td></td><td></td></tr><tr><td>PWM</td><td>GPIO(4)</td><td></td><td></td><td></td><td></td></tr><tr><td>SDIO</td><td>GPIO(6)</td><td></td><td></td><td></td><td></td></tr><tr><td>ACPI</td><td></td><td></td><td></td><td></td><td></td></tr></table>

# 3时钟结构

龙芯2K1000由一个100MHz时钟作为参考时钟，内部共有5个独立的PLL，其中每个PLL最多可以提供3组频率上相互依赖的时钟输出。这5个PLL的用途分别为：

一个NODEPLL仅用于产生node时钟，该时钟为芯片的主要时钟，供CPU核、二级Cache、一二级交叉开关以及IO子网络使用；两个PIXPLL各产生一个像素时钟供DC使用，以便支持双路独立显示；一个DCPLL同时产生DC控制器的内部时钟和GMAC控制器时钟，进一步产生APB，SATA以及USB的时钟；一个DDRPLL同时产生DDR、GPU以及HDA的时钟；

除了内部的PLL之外，对于SATA、PCIE、USB这几个采用PHY自己产生时钟的模块，也使用外部输入的同一个参考时钟进行了参考时钟通路设计。

还有一个启动时钟，直接使用100MHz参考时钟通过分频得到。

# 3.1 NODEPLL

nodeclock的产生结构图如图3- 1所示，

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/7949ef1335ed9bfd68b498d3ed05fb22a4f2de9d20232fd06dde615dda2316ec.jpg)  
图3-1NODEPLL结构图

输出时钟频率的计算方式如下：

node_clock=refclk/L1_div_ref\*L1_loopc/L2_divout;

node_clock的工作频率在1GHz左右，其PLL的分频系数以及倍频系数可以任意配置，但是需要保证可配分频器的输出refclk/L1_div_ref在  $20\sim 40\mathrm{MHz}$  范围内。PLL倍频值refclk/L1_div_ref\*L1_loopc需要在  $1.2\mathrm{GHz}\sim 3.2\mathrm{GHz}$  。该限制对其他4个内部PLL也适用，所以下文不再赘述。

输出的时钟还可以经由freq_scale模块进行细粒度分频控制。具体分频方法请参考5.18节。

# 3.2 PIXPLL

pix pll结构基本与node pll结构相同，但是不包含freq_scale模块。2K1000内包含两个独立的pix pll用于双路显示输出。像素时钟pix clock频率范围100- 250MHz。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/f44fe67fc380ac110134d09771c2d3b257143e810a292b9e37361676b5f7c4b3.jpg)  
图3-2PIXPLL结构图

# 3.3 DDRPLL

DDRPLL会输出三个时钟，分别为：

ddr_clock用于内存控制器，频率范围400- 700MHz  gpu_clock用于GPU模块，频率范围300- 500MHz  hda_clock用于HDA模块，频率固定为24MHz

因为三个时钟共用一个PLL，只是通过设置各自的L2_divout值来实现不同的频率输出。所以在调整其中一个模块时钟时，如果对公用PLL的倍频系数进行了调整，那么需要注意对其他时钟的影响。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/9dc1c05534103d030a8c2139624506c72f6b72732fdc03b444cb732bc7249b0b.jpg)  
图3-3DDRPLL结构图

# 3.4 DCPLL

DCPLL输出两个时钟：

dc_clock用于DC控制器，频率可设为200MHz左右，需保证其频率大于像素时钟.该时钟也作为VPU模块的主时钟。gmac_clock用于GMAC控制器，频率固定125MHz。

APB、SATA以及USB控制器的时钟由125MHzgmac时钟分频产生，所以这三个模块时钟频率最高为125MHz，通过freq_scale模块可以进一步调整分频系数。具体分频方法请参考5.18节。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/16b6136759fe7f8938e66276e201a30a06197b8d642ea818b2c6612eab733d9c.jpg)  
图3-4DCPLL结构图

# 3.5 内部PLL配置方法

以上5个内部PLL提供硬件配置和软件配置两种配置方法。这两种配置方法通过SYS_CLKSEL[1:0]的设置来区分。

# 3.5.1 硬件配置

具体如下表所示。

表3-1PLL硬件配置  

<table><tr><td>SYS_CLKSEL</td><td>00(硬件低频)</td><td>01（硬件高频）</td><td>10</td><td>11</td></tr><tr><td>NODE</td><td>0.875G</td><td>1G</td><td rowspan="11">软件配置</td><td rowspan="11">硬件 bypass所有PLL，所有时钟频率与参考时钟相同（100MHz）</td></tr><tr><td>DDR</td><td>480M</td><td>600M</td></tr><tr><td>GPU</td><td>320M</td><td>400M</td></tr><tr><td>HDA</td><td>24M</td><td>24M</td></tr><tr><td>DC</td><td>200M</td><td>400M</td></tr><tr><td>PIX0</td><td>100M</td><td>200M</td></tr><tr><td>PIX1</td><td>100M</td><td>200M</td></tr><tr><td>GMAC</td><td>不可硬件配置</td><td>不可硬件配置</td></tr><tr><td>SATA</td><td>85M</td><td>150M</td></tr><tr><td>USB</td><td>85M</td><td>150M</td></tr><tr><td>APB</td><td>85M</td><td>150M</td></tr></table>

# 3.5.2 软件配置

当SYS_CLKSEL设置为2'b10时表示PLL频率通过软件配置。这种配置下，默认对应的时钟频率为外部参考时钟频率，即所有PLL输出都是SYS_SYSCLK，需要在处理器启动过程中对时钟进行软件配置。各个时钟设置的过程应该按照以下方式：

1. 将对应的PLL的PD信号设置为1；2. 设置寄存器除了sel_pll_*及soft_set_pll之外的其它寄存器，即这两个寄存器在设置的过程中写为0；3. 将对应的PLL的PD信号设置为0；4. 其他寄存器值不变，将soft_set_pll设置为1；5. 等待寄存器中的锁定信号locked_*为1；6. 设置sel_pll_*为1，此时对应的时钟频率将切换为软件设置的频率。具体的配置寄存器说明请参考第5章。

# 3.6 BOOT时钟

BOOT时钟通过100MHz参考时钟分频得到，用于SPI和LIO两个启动模块，同时该时钟也作为CAMERA模块的主时钟。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/bb6d3d1ba24b28f52543b975a7a9f67e4f9320678fa73a6e2535197467814762.jpg)  
图3-5BOOT时钟结构图

# 3.7 USB参考时钟

USBPHY为4个独立的单端口PHY，参考时钟输入提供以下2种方式供选择：

使用1个12MHz晶振输入，其它使用0号PHY的时钟输出作为参考源不使用单独的参考时钟输入，都使用50MHz的内部参考时钟（100MHz的SYSCLK经二分频后）

# 3.8 PCIE参考时钟

PCIE采用两个独立的  $\mathrm{x4PHY}$  ，为了简化主板设计，提供以下2种方式供选择，使用引脚进行控制。

使用2个100MHz差分输入不使用单独的参考时钟输入，使用100MHz的内部参考时钟

# 3.9 SATA参考时钟输入

SATAPHY与PCIEPHY类似，也提供以下2种方式供选择，使用寄存器进行选择。

使用1个100MHz差分输入不使用单独的参考时钟输入，都使用100MHz的内部参考时钟

# 4电源管理

本章对电源管理进行简单介绍，具体寄存器及使用方法请参考第21章。

# 4.1 电源管理模块介绍

4.1 电源管理模块介绍- 龙芯2K1000电源管理模块提供系统功耗管理实现机制。- 支持Advanced Configuration and Power Interface, Version 4.0a(ACPI)，提供相应的功耗管理功能。- 系统休眠与唤醒，支持ACPI S3（待机到内存），ACPI S4（待机到硬盘），ACPI S5（软关机），并且支持电源失效检测和自动系统恢复。支持多种唤醒方式（USB，GMAC，电源开关等）。- 动态性能功耗控制，支持处理器核DVFS控制，支持动态关闭媒体解码协处理器电源。- 系统时钟控制，模块时钟门控，多种方式调节频率。- 提供温度管理控制功能。支持3级报警机制。

# 4.2 电源级别

表4- 1显示了系统支持的ACPI状态及其相关说明

表4-1ACPI状态说明  

<table><tr><td>状态</td><td>描述</td></tr><tr><td>G0/S0</td><td>全部工作，该模式下系统全部工作，处理器子状态可由Cx或Px决定，媒体处理器由状态Dx决定。</td></tr><tr><td>G1/S1</td><td>暂不支持</td></tr><tr><td>G1/S3</td><td>Suspend to RAM(STR)，上下文保存到内存</td></tr><tr><td>G1/S4</td><td>Suspend to Disk(STD)，保存到硬盘，除唤醒电路全部掉电</td></tr><tr><td>G2/S5</td><td>Soft off，只有唤醒电路上电</td></tr><tr><td>G3</td><td>Mechanical off，所有供电失效</td></tr></table>

# 4.3 控制引脚说明

表4- 2为电源管理部分的IO信号描述。

表4-2控制引脚说明  

<table><tr><td>名称</td><td>类型</td><td>描述</td><td>供电</td></tr><tr><td>ACPI_SYSRSTn</td><td></td><td>系统复位</td><td>RSM3V3</td></tr><tr><td>RTC_RSMRSTn</td><td></td><td>复位 Resume 域逻辑，该信号需在 resume 域上电稳定后保持一段时间有效（推荐&amp;gt;5ms）</td><td>RTC2V5</td></tr><tr><td>RTC_RSTn</td><td></td><td>电池更换后，重启 RTC 逻辑</td><td>RTC2V5</td></tr><tr><td>ACPI_PLTRSTn</td><td>D</td><td>对系统平台其它设备进行复位</td><td>RSM3V3</td></tr></table>

<table><tr><td>ACPI_PWROK</td><td>I</td><td>主供电电源上电稳定，如有多个供电，该信号表示最后一个电源稳定。</td><td>RSM3V3</td></tr><tr><td>ACPI_PWRBTN</td><td>I</td><td>电源按钮</td><td>RSM3V3</td></tr><tr><td>ACPI_RINGn</td><td>I</td><td>Modem 唤醒信号</td><td>RSM3V3</td></tr><tr><td>ACPI_WAKEEn</td><td>I</td><td>PCIE 边带唤醒信号</td><td>RSM3V3</td></tr><tr><td>ACPI_BATLOWn</td><td>I</td><td>电池电量低</td><td>RSM3V3</td></tr><tr><td>ACPI_LID</td><td>I</td><td>显示器开关信号</td><td>RSM3V3</td></tr><tr><td>ACPI_PWRTYPE</td><td>I</td><td>识别电池供电和电源供电，1 指示 AC Power；0 指示 System Battery</td><td>RSM3V3</td></tr><tr><td>ACPI_SUSSTATn</td><td>O</td><td>指示系统将要进入低功耗状态</td><td>RSM3V3</td></tr><tr><td>ACPI_S3n</td><td>O</td><td>STR，待机到内存指示信号</td><td>RSM3V3</td></tr><tr><td>ACPI_S4n</td><td>O</td><td>STD，待机到硬盘指示信号</td><td>RSM3V3</td></tr><tr><td>ACPI_S5n</td><td>O</td><td>Soft off，只有唤醒电路上电</td><td>RSM3V3</td></tr><tr><td>ACPI_SLPLANn</td><td>O</td><td>以太网 PHY 休眠指示信号</td><td>RSM3V3</td></tr><tr><td>ACPI_VID[5:0]</td><td>O</td><td>电压值指示信号</td><td>RSM3V3</td></tr></table>

# 5芯片配置寄存器

龙芯2K1000有大量的配置寄存器，多数分布于各个功能模块中，本节介绍芯片级的配置寄存器。

表5-1芯片配置寄存器列表  

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe00000</td><td>Version</td><td>寄存器版本</td></tr><tr><td>0x1fe00008</td><td>Feature</td><td>芯片特性</td></tr><tr><td>0x1fe00010</td><td>Vendor</td><td>厂商名称</td></tr><tr><td>0x1fe00020</td><td>ID</td><td>芯片名称</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe00480</td><td>PLL_SYS_0</td><td>系统的PLL低64位配置</td></tr><tr><td>0x1fe00488</td><td>PLL_SYS_1</td><td>系统的PLL高64位配置</td></tr><tr><td>0x1fe00490</td><td>PLL_DDR_0</td><td>内存控制器的PLL低64位配置</td></tr><tr><td>0x1fe00498</td><td>PLL_DDR_1</td><td>内存控制器的PLL高64位配置</td></tr><tr><td>0x1fe004a0</td><td>PLL_DC_0</td><td>DC的PLL低64位配置</td></tr><tr><td>0x1fe004a8</td><td>PLL_DC_1</td><td>DC的PLL高64位配置</td></tr><tr><td>0x1fe004b0</td><td>PLL_PIX0_0</td><td>PIX0的PLL低64位配置</td></tr><tr><td>0x1fe004b8</td><td>PLL_PIX0_1</td><td>PIX0的PLL高64位配置</td></tr><tr><td>0x1fe004c0</td><td>PLL_PIX1_0</td><td>PIX1的PLL低64位配置</td></tr><tr><td>0x1fe004c8</td><td>PLL_PIX1_1</td><td>PIX1的PLL高64位配置</td></tr><tr><td>0x1fe004d0</td><td>FREQSCALE</td><td>各设备的分频控制</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe00500</td><td>GPIOO_OEN</td><td>GPIO的低64位输出使能</td></tr><tr><td>0x1fe00508</td><td>GPIOI_OEN</td><td>保留</td></tr><tr><td>0x1fe00510</td><td>GPIOO_O</td><td>GPIO的低64位输出值</td></tr><tr><td>0x1fe00518</td><td>GPIOI_O</td><td>保留</td></tr><tr><td>0x1fe00520</td><td>GPIOO_I</td><td>GPIO的低64位输入值</td></tr><tr><td>0x1fe00528</td><td>GPIOI_I</td><td>保留</td></tr><tr><td>0x1fe00530</td><td>GPIOO_INT</td><td>GPIO的低64位中断</td></tr><tr><td>0x1fe00538</td><td>GPIOI_INT</td><td>保留</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe00580</td><td>PCIE0_REG0</td><td>PCIE0的配置寄存器0</td></tr><tr><td>0x1fe00588</td><td>PCIE0_REG1</td><td>PCIE0的配置寄存器1</td></tr><tr><td>0x1fe00590</td><td>PCIE0PHY</td><td>PCIE0的PHY配置寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe005a0</td><td>PCIE1_REG0</td><td>PCIE1的配置寄存器0</td></tr><tr><td>0x1fe005a8</td><td>PCIE1_REG1</td><td>PCIE1的配置寄存器1</td></tr><tr><td>0x1fe005b0</td><td>PCIE1PHY</td><td>PCIE1的PHY配置寄存器</td></tr><tr><td></td><td></td><td></td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe00c00</td><td>CONFDMA0</td><td>DMA0控制器的配置寄存器</td></tr><tr><td>0x1fe00c10</td><td>CONFMA1</td><td>DMA1控制器的配置寄存器</td></tr><tr><td>0x1fe00c20</td><td>CONFMA2</td><td>DMA2控制器的配置寄存器</td></tr><tr><td>0x1fe00c30</td><td>CONFMA3</td><td>DMA3控制器的配置寄存器</td></tr><tr><td>0x1fe00c40</td><td>CONFMA4</td><td>DMA4控制器的配置寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01000</td><td>CORE0_IPISR</td><td>0号处理器核的IPI_Status寄存器</td></tr><tr><td>0x1fe01004</td><td>CORE0_IPIEN</td><td>0号处理器核的IPI_Enable寄存器</td></tr><tr><td>0x1fe01008</td><td>CORE0_IPISET</td><td>0号处理器核的IPI_Set寄存器</td></tr><tr><td>0x1fe0100c</td><td>CORE0_IPICLR</td><td>0号处理器核的IPI_Clear寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01020</td><td>CORE0_BUF0</td><td>0号处理器核的IPI-MailBox0寄存器</td></tr><tr><td>0x1fe01028</td><td>CORE0_BUF1</td><td>0号处理器核的IPI-MailBox1寄存器</td></tr><tr><td>0x1fe01030</td><td>CORE0_BUF2</td><td>0号处理器核的IPI-MailBox2寄存器</td></tr><tr><td>0x1fe01038</td><td>CORE0_BUF3</td><td>0号处理器核的IPI-MailBox3寄存器</td></tr><tr><td>0x1fe01040</td><td>CORE0_INTISR0</td><td>路由给CORE0的低32位中断状态</td></tr><tr><td>0x1fe01048</td><td>CORE0_INTISR1</td><td>路由给CORE0的高32位中断状态</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01100</td><td>CORE0_IPISR</td><td>1号处理器核的IPI_Status寄存器</td></tr><tr><td>0x1fe01104</td><td>CORE0_IPIEN</td><td>1号处理器核的IPI_Enable寄存器</td></tr><tr><td>0x1fe01108</td><td>CORE0_IPISET</td><td>1号处理器核的IPI_Set寄存器</td></tr><tr><td>0x1fe0110c</td><td>CORE0_IPICLR</td><td>1号处理器核的IPI_Clear寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01120</td><td>CORE0_BUF0</td><td>1号处理器核的IPI-MailBox0寄存器</td></tr><tr><td>0x1fe01128</td><td>CORE0_BUF1</td><td>1号处理器核的IPI-MailBox1寄存器</td></tr><tr><td>0x1fe01130</td><td>CORE0_BUF2</td><td>1号处理器核的IPI-MailBox2寄存器</td></tr><tr><td>0x1fe01138</td><td>CORE0_BUF3</td><td>1号处理器核的IPI-MailBox3寄存器</td></tr><tr><td>0x1fe01140</td><td>CORE0_INTISR0</td><td>路由给CORE1的低32位中断状态</td></tr><tr><td>0x1fe01148</td><td>CORE0_INTISR1</td><td>路由给CORE1的高32位中断状态</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01400</td><td>ENTRY0_0</td><td>8位中断路由寄存器[0--7]</td></tr><tr><td>0x1fe01408</td><td>ENTRY8_0</td><td>8位中断路由寄存器[8--15]</td></tr><tr><td>0x1fe01410</td><td>ENTRY16_0</td><td>8位中断路由寄存器[16--23]</td></tr><tr><td>0x1fe01418</td><td>ENTRY24_0</td><td>8位中断路由寄存器[24--31]</td></tr><tr><td>0x1fe01420</td><td>INTISR_0</td><td>低32位中断状态寄存器</td></tr><tr><td>0x1fe01424</td><td>INTIEN_0</td><td>低32位中断使能状态寄存器</td></tr><tr><td>0x1fe01428</td><td>INTSET_0</td><td>低32位设置使能寄存器</td></tr><tr><td>0x1fe0142c</td><td>INTCLR_0</td><td>低32位中断清除寄存器，清除使能寄存器和脉冲触发的中断</td></tr><tr><td>0x1fe01430</td><td>INTPOL_0</td><td>低32位中断极性寄存器</td></tr><tr><td>0x1fe01434</td><td>INTEDGE_0</td><td>低32位触发方式寄存器(1：脉冲触发；0：电平触发)</td></tr><tr><td>0x1fe01438</td><td>BOUNDCE_0</td><td>低32位中断分发模式控制，与auto合用。</td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe0143c</td><td>AUTO_0</td><td>低32位中断分发模式控制，与bounce合用。</td></tr><tr><td>0x1fe01440</td><td>ENTRY70_1</td><td>8位中断路由寄存器[32-39]</td></tr><tr><td>0x1fe01448</td><td>ENTRY78_1</td><td>8位中断路由寄存器[40-47]</td></tr><tr><td>0x1fe01450</td><td>ENTRY716_1</td><td>8位中断路由寄存器[48-55]</td></tr><tr><td>0x1fe01458</td><td>ENTRY724_1</td><td>8位中断路由寄存器[56-63]</td></tr><tr><td>0x1fe01460</td><td>INTISR_1</td><td>高32位中断状态寄存器</td></tr><tr><td>0x1fe01464</td><td>INTIEIN_1</td><td>高32位中断使能状态寄存器</td></tr><tr><td>0x1fe01468</td><td>INTSI_1</td><td>高32位设置使能寄存器</td></tr><tr><td>0x1fe0146c</td><td>INTCLR_1</td><td>高32位中断清除寄存器，清除使能寄存器和脉冲触发的中断</td></tr><tr><td>0x1fe01470</td><td>INTPCL_1</td><td>高32位中断极性寄存器</td></tr><tr><td>0x1fe01474</td><td>INTEDGE_1</td><td>高32位触发方式寄存器(1：脉冲触发；0：电平触发)</td></tr><tr><td>0x1fe01478</td><td>BOUNDCE_1</td><td>高32位中断分发模式控制，与auto合用。</td></tr><tr><td>0x1fe0147c</td><td>AUTO_1</td><td>高32位中断分发模式控制，与bounce合用。</td></tr><tr><td>0x1fe014a0</td><td>INT MSI_0</td><td>保留</td></tr><tr><td>0x1fe014a8</td><td>INT MSI_EN_0</td><td>保留</td></tr><tr><td>0x1fe014b0</td><td>INT MSI_ADDR_0</td><td>MSI地址0</td></tr><tr><td>0x1fe014b4</td><td>INT MSI TRIGGER_EN_0</td><td>32位MSI中断使能，每位对应一个写入MSI地址0的中断</td></tr><tr><td>0x1fe014e0</td><td>INT MSI_1</td><td>保留</td></tr><tr><td>0x1fe014e8</td><td>INT MSI_EN_1</td><td>保留</td></tr><tr><td>0x1fe014f0</td><td>INT MSI_ADDR_1</td><td>MSI地址1</td></tr><tr><td>0x1fe014f4</td><td>INT MSI TRIGGER_EN_1</td><td>32位MSI中断使能，每位对应一个写入MSI地址1的中断</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe01500</td><td>Thsens_int_ctrl_Hi</td><td>温度传感器高温中断控制寄存器</td></tr><tr><td>0x1fe01508</td><td>Thsens_int_ctrl_lo</td><td>温度传感器低温中断控制寄存器</td></tr><tr><td>0x1fe01510</td><td>Thsens_int_status/clr</td><td>温度传感器中断状态寄存器</td></tr><tr><td>0x1fe01520</td><td>Thsens_scale_hi</td><td>高温自动降频控制寄存器</td></tr><tr><td>0x1fe01528</td><td>Thsens_scale_lo</td><td>保留</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe02000</td><td>CPU_WIN0_BASE</td><td>cache通路二级窗口0的基地址</td></tr><tr><td>0x1fe02008</td><td>CPU_WIN1_BASE</td><td>cache通路二级窗口1的基地址</td></tr><tr><td>0x1fe02010</td><td>CPU_WIN2_BASE</td><td>cache通路二级窗口2的基地址</td></tr><tr><td>0x1fe02018</td><td>CPU_WIN3_BASE</td><td>cache通路二级窗口3的基地址</td></tr><tr><td>0x1fe02020</td><td>CPU_WIN4_BASE</td><td>cache通路二级窗口4的基地址</td></tr><tr><td>0x1fe02028</td><td>CPU_WIN5_BASE</td><td>cache通路二级窗口5的基地址</td></tr><tr><td>0x1fe02030</td><td>CPU_WIN6_BASE</td><td>cache通路二级窗口6的基地址</td></tr><tr><td>0x1fe02038</td><td>CPU_WIN7_BASE</td><td>cache通路二级窗口7的基地址</td></tr><tr><td>0x1fe02040</td><td>CPU_WIN0_MASK</td><td>cache通路二级窗口0的掩码</td></tr><tr><td>0x1fe02048</td><td>CPU_WIN1_MASK</td><td>cache通路二级窗口1的掩码</td></tr><tr><td>0x1fe02050</td><td>CPU_WIN2_MASK</td><td>cache通路二级窗口2的掩码</td></tr><tr><td>0x1fe02058</td><td>CPU_WIN3_MASK</td><td>cache通路二级窗口3的掩码</td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe02060</td><td>CPU_WIN4_MASK</td><td>cache 通路二级窗口 4 的掩码</td></tr><tr><td>0x1fe02068</td><td>CPU_WIN5_MASK</td><td>cache 通路二级窗口 5 的掩码</td></tr><tr><td>0x1fe02070</td><td>CPU_WIN6_MASK</td><td>cache 通路二级窗口 6 的掩码</td></tr><tr><td>0x1fe02078</td><td>CPU_WIN7_MASK</td><td>cache 通路二级窗口 7 的掩码</td></tr><tr><td>0x1fe02080</td><td>CPU_WIN8_MMAP</td><td>cache 通路二级窗口 0 的新基地址</td></tr><tr><td>0x1fe02088</td><td>CPU_WIN1_MMAP</td><td>cache 通路二级窗口 1 的新基地址</td></tr><tr><td>0x1fe02090</td><td>CPU_WIN2_MMAP</td><td>cache 通路二级窗口 2 的新基地址</td></tr><tr><td>0x1fe02098</td><td>CPU_WIN3_MMAP</td><td>cache 通路二级窗口 3 的新基地址</td></tr><tr><td>0x1fe020a0</td><td>CPU_WIN4_MMAP</td><td>cache 通路二级窗口 4 的新基地址</td></tr><tr><td>0x1fe020a8</td><td>CPU_WIN5_MMAP</td><td>cache 通路二级窗口 5 的新基地址</td></tr><tr><td>0x1fe020b0</td><td>CPU_WIN6_MMAP</td><td>cache 通路二级窗口 6 的新基地址</td></tr><tr><td>0x1fe020b8</td><td>CPU_WIN7_MMAP</td><td>cache 通路二级窗口 7 的新基地址</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe02100</td><td>PCI_WIN0_BASE</td><td>uncache 通路二级窗口 0 的基地址</td></tr><tr><td>0x1fe02108</td><td>PCI_WIN1_BASE</td><td>uncache 通路二级窗口 1 的基地址</td></tr><tr><td>0x1fe02110</td><td>PCI_WIN2_BASE</td><td>uncache 通路二级窗口 2 的基地址</td></tr><tr><td>0x1fe02118</td><td>PCI_WIN3_BASE</td><td>uncache 通路二级窗口 3 的基地址</td></tr><tr><td>0x1fe02120</td><td>PCI_WIN4_BASE</td><td>uncache 通路二级窗口 4 的基地址</td></tr><tr><td>0x1fe02128</td><td>PCI_WIN5_BASE</td><td>uncache 通路二级窗口 5 的基地址</td></tr><tr><td>0x1fe02130</td><td>PCI_WIN6_BASE</td><td>uncache 通路二级窗口 6 的基地址</td></tr><tr><td>0x1fe02138</td><td>PCI_WIN7_BASE</td><td>uncache 通路二级窗口 7 的基地址</td></tr><tr><td>0x1fe02140</td><td>PCI_WIN0_MASK</td><td>uncache 通路二级窗口 0 的掩码</td></tr><tr><td>0x1fe02148</td><td>PCI_WIN1_MASK</td><td>uncache 通路二级窗口 1 的掩码</td></tr><tr><td>0x1fe02150</td><td>PCI_WIN2_MASK</td><td>uncache 通路二级窗口 2 的掩码</td></tr><tr><td>0x1fe02158</td><td>PCI_WIN3_MASK</td><td>uncache 通路二级窗口 3 的掩码</td></tr><tr><td>0x1fe02160</td><td>PCI_WIN4_MASK</td><td>uncache 通路二级窗口 4 的掩码</td></tr><tr><td>0x1fe02168</td><td>PCI_WIN5_MASK</td><td>uncache 通路二级窗口 5 的掩码</td></tr><tr><td>0x1fe02170</td><td>PCI_WIN6_MASK</td><td>uncache 通路二级窗口 6 的掩码</td></tr><tr><td>0x1fe02178</td><td>PCI_WIN7_MASK</td><td>uncache 通路二级窗口 7 的掩码</td></tr><tr><td>0x1fe02180</td><td>PCI_WIN0_MMAP</td><td>uncache 通路二级窗口 0 的新基地址</td></tr><tr><td>0x1fe02188</td><td>PCI_WIN1_MMAP</td><td>uncache 通路二级窗口 1 的新基地址</td></tr><tr><td>0x1fe02190</td><td>PCI_WIN2_MMAP</td><td>uncache 通路二级窗口 2 的新基地址</td></tr><tr><td>0x1fe02198</td><td>PCI_WIN3_MMAP</td><td>uncache 通路二级窗口 3 的新基地址</td></tr><tr><td>0x1fe021a0</td><td>PCI_WIN4_MMAP</td><td>uncache 通路二级窗口 4 的新基地址</td></tr><tr><td>0x1fe021a8</td><td>PCI_WIN5_MMAP</td><td>uncache 通路二级窗口 5 的新基地址</td></tr><tr><td>0x1fe021b0</td><td>PCI_WIN6_MMAP</td><td>uncache 通路二级窗口 6 的新基地址</td></tr><tr><td>0x1fe021b8</td><td>PCI_WIN7_MMAP</td><td>uncache 通路二级窗口 7 的新基地址</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe02400</td><td>XBAR_WIN4_BASE0</td><td>IO 设备（除 PCIE）DMA 访问窗口 0 基地址</td></tr><tr><td>0x1fe02408</td><td>XBAR_WIN4_BASE1</td><td>IO 设备（除 PCIE）DMA 访问窗口 1 基地址</td></tr><tr><td>0x1fe02410</td><td>XBAR_WIN4_BASE2</td><td>IO 设备（除 PCIE）DMA 访问窗口 2 基地址</td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe02418</td><td>XBAR_WIN4_BASE3</td><td>IO 设备（除 PCIE）DMA 访问窗口 3 基地址</td></tr><tr><td>0x1fe02420</td><td>XBAR_WIN4_BASE4</td><td>IO 设备（除 PCIE）DMA 访问窗口 4 基地址</td></tr><tr><td>0x1fe02428</td><td>XBAR_WIN4_BASE5</td><td>IO 设备（除 PCIE）DMA 访问窗口 5 基地址</td></tr><tr><td>0x1fe02430</td><td>XBAR_WIN4_BASE6</td><td>IO 设备（除 PCIE）DMA 访问窗口 6 基地址</td></tr><tr><td>0x1fe02438</td><td>XBAR_WIN4_BASE7</td><td>IO 设备（除 PCIE）DMA 访问窗口 7 基地址</td></tr><tr><td>0x1fe02440</td><td>XBAR_WIN4_MASK0</td><td>IO 设备（除 PCIE）DMA 访问窗口 0 掩码</td></tr><tr><td>0x1fe02448</td><td>XBAR_WIN4_MASK1</td><td>IO 设备（除 PCIE）DMA 访问窗口 1 掩码</td></tr><tr><td>0x1fe02450</td><td>XBAR_WIN4_MASK2</td><td>IO 设备（除 PCIE）DMA 访问窗口 2 掩码</td></tr><tr><td>0x1fe02458</td><td>XBAR_WIN4_MASK3</td><td>IO 设备（除 PCIE）DMA 访问窗口 3 掩码</td></tr><tr><td>0x1fe02460</td><td>XBAR_WIN4_MASK4</td><td>IO 设备（除 PCIE）DMA 访问窗口 4 掩码</td></tr><tr><td>0x1fe02468</td><td>XBAR_WIN4_MASK5</td><td>IO 设备（除 PCIE）DMA 访问窗口 5 掩码</td></tr><tr><td>0x1fe02470</td><td>XBAR_WIN4_MASK6</td><td>IO 设备（除 PCIE）DMA 访问窗口 6 掩码</td></tr><tr><td>0x1fe02478</td><td>XBAR_WIN4_MASK7</td><td>IO 设备（除 PCIE）DMA 访问窗口 7 掩码</td></tr><tr><td>0x1fe02480</td><td>XBAR_WIN4_MMAP0</td><td>IO 设备（除 PCIE）DMA 访问窗口 0 新基地址</td></tr><tr><td>0x1fe02488</td><td>XBAR_WIN4_MMAP1</td><td>IO 设备（除 PCIE）DMA 访问窗口 1 新基地址</td></tr><tr><td>0x1fe02490</td><td>XBAR_WIN4_MMAP2</td><td>IO 设备（除 PCIE）DMA 访问窗口 2 新基地址</td></tr><tr><td>0x1fe02498</td><td>XBAR_WIN4_MMAP3</td><td>IO 设备（除 PCIE）DMA 访问窗口 3 新基地址</td></tr><tr><td>0x1fe024a0</td><td>XBAR_WIN4_MMAP4</td><td>IO 设备（除 PCIE）DMA 访问窗口 4 新基地址</td></tr><tr><td>0x1fe024a8</td><td>XBAR_WIN4_MMAP5</td><td>IO 设备（除 PCIE）DMA 访问窗口 5 新基地址</td></tr><tr><td>0x1fe024b0</td><td>XBAR_WIN4_MMAP6</td><td>IO 设备（除 PCIE）DMA 访问窗口 6 新基地址</td></tr><tr><td>0x1fe024b8</td><td>XBAR_WIN4_MMAP7</td><td>IO 设备（除 PCIE）DMA 访问窗口 7 新基地址</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe02500</td><td>XBAR_WIN5_BASE0</td><td>PCIE 设备 DMA 访问窗口 0 基地址</td></tr><tr><td>0x1fe02508</td><td>XBAR_WIN5_BASE1</td><td>PCIE 设备 DMA 访问窗口 1 基地址</td></tr><tr><td>0x1fe02510</td><td>XBAR_WIN5_BASE2</td><td>PCIE 设备 DMA 访问窗口 2 基地址</td></tr><tr><td>0x1fe02518</td><td>XBAR_WIN5_BASE3</td><td>PCIE 设备 DMA 访问窗口 3 基地址</td></tr><tr><td>0x1fe02520</td><td>XBAR_WIN5_BASE4</td><td>PCIE 设备 DMA 访问窗口 4 基地址</td></tr><tr><td>0x1fe02528</td><td>XBAR_WIN5_BASE5</td><td>PCIE 设备 DMA 访问窗口 5 基地址</td></tr><tr><td>0x1fe02530</td><td>XBAR_WIN5_BASE6</td><td>PCIE 设备 DMA 访问窗口 6 基地址</td></tr><tr><td>0x1fe02538</td><td>XBAR_WIN5_BASE7</td><td>PCIE 设备 DMA 访问窗口 7 基地址</td></tr><tr><td>0x1fe02540</td><td>XBAR_WIN5_MASK0</td><td>PCIE 设备 DMA 访问窗口 0 掩码</td></tr><tr><td>0x1fe02548</td><td>XBAR_WIN5_MASK1</td><td>PCIE 设备 DMA 访问窗口 1 掩码</td></tr><tr><td>0x1fe02550</td><td>XBAR_WIN5_MASK2</td><td>PCIE 设备 DMA 访问窗口 2 掩码</td></tr><tr><td>0x1fe02558</td><td>XBAR_WIN5_MASK3</td><td>PCIE 设备 DMA 访问窗口 3 掩码</td></tr><tr><td>0x1fe02560</td><td>XBAR_WIN5_MASK4</td><td>PCIE 设备 DMA 访问窗口 4 掩码</td></tr><tr><td>0x1fe02568</td><td>XBAR_WIN5_MASK5</td><td>PCIE 设备 DMA 访问窗口 5 掩码</td></tr><tr><td>0x1fe02570</td><td>XBAR_WIN5_MASK6</td><td>PCIE 设备 DMA 访问窗口 6 掩码</td></tr><tr><td>0x1fe02578</td><td>XBAR_WIN5_MASK7</td><td>PCIE 设备 DMA 访问窗口 7 掩码</td></tr><tr><td>0x1fe02580</td><td>XBAR_WIN5_MMAP0</td><td>PCIE 设备 DMA 访问窗口 0 新基地址</td></tr><tr><td>0x1fe02588</td><td>XBAR_WIN5_MMAP1</td><td>PCIE 设备 DMA 访问窗口 1 新基地址</td></tr><tr><td>0x1fe02590</td><td>XBAR_WIN5_MMAP2</td><td>PCIE 设备 DMA 访问窗口 2 新基地址</td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe02598</td><td>XBAR_WIN5_MMAP3</td><td>PCIE 设备 DMA 访问窗口 3 新基地址</td></tr><tr><td>0x1fe025a0</td><td>XBAR_WIN5_MMAP4</td><td>PCIE 设备 DMA 访问窗口 4 新基地址</td></tr><tr><td>0x1fe025a8</td><td>XBAR_WIN5_MMAP5</td><td>PCIE 设备 DMA 访问窗口 5 新基地址</td></tr><tr><td>0x1fe025b0</td><td>XBAR_WIN5_MMAP6</td><td>PCIE 设备 DMA 访问窗口 6 新基地址</td></tr><tr><td>0x1fe025b8</td><td>XBAR_WIN5_MMAP7</td><td>PCIE 设备 DMA 访问窗口 7 新基地址</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe02810</td><td>CONFAML</td><td>QOS 控制使能寄存器</td></tr><tr><td>0x1fe02818</td><td>CONFAML0</td><td>AML0 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02820</td><td>CONFAML1</td><td>AML0 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02828</td><td>CONFAML2</td><td>AML1 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02830</td><td>CONFAML3</td><td>AML1 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02838</td><td>CONFAML4</td><td>AML2 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02840</td><td>CONFAML5</td><td>AML2 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02848</td><td>CONFAML6</td><td>AML3 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02850</td><td>CONFAML7</td><td>AML3 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02858</td><td>CONFAML8</td><td>AML4 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02860</td><td>CONFAML9</td><td>AML4 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02868</td><td>CONFAML10</td><td>AML5 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02870</td><td>CONFAML11</td><td>AML5 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02878</td><td>CONFAML12</td><td>AML6 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02880</td><td>CONFAML13</td><td>AML6 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02888</td><td>CONFAML14</td><td>AML7 的读操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02890</td><td>CONFAML15</td><td>AML7 的写操作带宽、延迟配置寄存器</td></tr><tr><td>0x1fe02898</td><td>CONFAML16</td><td>AML0-3 的延迟屏蔽配置寄存器</td></tr><tr><td>0x1fe028a0</td><td>CONFAML17</td><td>AML4-7 的延迟屏蔽配置寄存器</td></tr><tr><td>0x1fe028a8</td><td>CONFAML18</td><td>AML0-3 的带宽屏蔽配置寄存器</td></tr><tr><td>0x1fe028b0</td><td>CONFAML19</td><td>AML4-7 的带宽屏蔽配置寄存器</td></tr><tr><td>0x1fe028b8</td><td>CONFAML20</td><td>AML0 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028c0</td><td>CONFAML21</td><td>AML0 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028c8</td><td>CONFAML22</td><td>AML1 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028d0</td><td>CONFAML23</td><td>AML1 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028d8</td><td>CONFAML24</td><td>AML2 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028e0</td><td>CONFAML25</td><td>AML2 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028e8</td><td>CONFAML26</td><td>AML3 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028f0</td><td>CONFAML27</td><td>AML3 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe028f8</td><td>CONFAML28</td><td>AML4 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02900</td><td>CONFAML29</td><td>AML4 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02908</td><td>CONFAML30</td><td>AML5 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02910</td><td>CONFAML31</td><td>AML5 的写操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02918</td><td>CONFAML32</td><td>AML6 的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02920</td><td>CONFAML33</td><td>AML6 的写操作实时带宽监测寄存器</td></tr></table>

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe02928</td><td>CONFAML34</td><td>AML7的读操作实时带宽监测寄存器</td></tr><tr><td>0x1fe02930</td><td>CONFAML35</td><td>AML7的写操作实时带宽监测寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe03000</td><td>PCICFG_HEADER2</td><td>APB设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03040</td><td>PCICFG_HEADER30</td><td>GMAC0设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03080</td><td>PCICFG_HEADER31</td><td>GMAC1设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe030c0</td><td>PCICFG_HEADER40</td><td>USB-OTG设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03100</td><td>PCICFG_HEADER41</td><td>USB-EHCI设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03140</td><td>PCICFG_HEADER42</td><td>USB-OHCI设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03180</td><td>PCICFG_HEADER5</td><td>GPU设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe031c0</td><td>PCICFG_HEADER6</td><td>DC设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03200</td><td>PCICFG_HEADER7</td><td>HDA设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03240</td><td>PCICFG_HEADER8</td><td>SATA设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03280</td><td>N/A_HEADER</td><td>无效设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe032c0</td><td>PCICFG_HEADERf</td><td>DMA控制器的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03300</td><td>PCICFG_HEADER10</td><td>VPU设备的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td>0x1fe03340</td><td>PCICFG_HEADER11</td><td>CAMERA控制器的配置头寄存器（一般采用配置头访问形式，不建议使用配置寄存器的形式访问）</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe03800</td><td>PCICFG2_RECFG</td><td>APB设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03808</td><td>PCICFG30_RECFG</td><td>GMAC0设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03810</td><td>PCICFG31_RECFG</td><td>GMAC1设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03818</td><td>PCICFG40_RECFG</td><td>USB-OTG设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03820</td><td>PCICFG41_RECFG</td><td>USB-EHCI设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03828</td><td>PCICFG42_RECFG</td><td>USB-OHCI设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03830</td><td>PCICFG5_RECFG</td><td>GPU设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03838</td><td>PCICFG6_RECFG</td><td>DC设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03840</td><td>PCICFG7_RECFG</td><td>HDA设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03848</td><td>PCICFG8_RECFG</td><td>SATA设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03850</td><td>PCICFG9_RECFG</td><td>DMA控制器配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03858</td><td>PCICFG10_RECFG</td><td>VPU设备配置头空间的重配置使能寄存器</td></tr><tr><td>0x1fe03860</td><td>PCICFG11_RECFG</td><td>CAMERA控制器配置头空间的重配置使能寄存器</td></tr><tr><td></td><td></td><td></td></tr><tr><td>0x1fe03ff8</td><td>CHIP_ID</td><td>芯片版本号</td></tr></table>

上表中关于二级交叉开关以及DMA访问窗口相关的配置寄存器将在第6章介绍，中断

相关寄存器将在第7章介绍，这里不再赘述。下面详细介绍其他的配置寄存器。

# 5.1 通用配置寄存器0

通用配置寄存器0，包括对管脚复用的控制，以及HDA、USB、PCIE的一致性、内存控制器、RTC控制器及LIO控制器的配置等。

地址：0x1fe00420

表5-2通用配置寄存器0  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63</td><td>LIO big_mem</td><td>RW</td><td>0x0</td><td>LIO big_mem 模式控制</td></tr><tr><td>62</td><td>LIO iopf_en</td><td>RW</td><td>0x0</td><td>保留</td></tr><tr><td>61</td><td>LIO io_width_16</td><td>RW</td><td>0x1</td><td>IO 数据位宽：
0: 8 位
1: 16 位</td></tr><tr><td>60:56</td><td>LIO io_count_init_i</td><td>RW</td><td>0x1f</td><td>IO 数据读取延迟（boot 时钟周期数）</td></tr><tr><td>55</td><td>LIO rom_width16</td><td>RW</td><td>0x1</td><td>Rom 数据位宽：
0: 8 位
1: 16 位</td></tr><tr><td>54:50</td><td>LIO rom_count_init_i</td><td>RW</td><td>0x1f</td><td>ROM 数据读取延迟（boot 时钟周期数）</td></tr><tr><td>49:48</td><td>LIO clock_period_i</td><td>RW</td><td>0x0</td><td>内部等待计数器步长设置：
0: 步长为 1
1: 步长为 4
2: 步长为 2
3: 步长为 1</td></tr><tr><td>47</td><td>rtc_restart</td><td>RW</td><td>0x0</td><td>内部振荡器重启控制</td></tr><tr><td>46:44</td><td>rtc_ds</td><td>RW</td><td>0x0</td><td>内部振荡器接口控制信号</td></tr><tr><td>43:42</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>41</td><td>mc_default_reg</td><td>RW</td><td>0x1</td><td>窗口不命中处理
0: 关闭内存控制器的该功能
1: 当所有窗口不命中时，由内存控制器给出响应，防止 CPU 卡死。</td></tr><tr><td>40</td><td>mc_disable_reg</td><td>RW</td><td>0x0</td><td>DDR 配置空间关闭，高有效
DDR 控制器在内存空间中开辟了一小段配置空间(1MB @0x0ff0,0000)，在关闭后软件就可以使用这段空间。为避免意外访问，建议在配置完成后及时关闭</td></tr><tr><td>39:37</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>36</td><td>usbehci64_en</td><td>RW</td><td>0x0</td><td>USB 端口的 EHCI 控制器 64 位地址模式
0: 32 位地址模式
1: 64 位地址模式</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>35</td><td>hpet_int_mode</td><td>RW</td><td>0x0</td><td>HPET 中断模式
0: 正常模式，即 timer0/1/2 的中断都映射到中断引脚 21
1: 中断分开模式，timer0/1/2 的中断分别映射到中断引脚 21/38/39</td></tr><tr><td>34</td><td>pcie_coherent</td><td>RW</td><td>0x1</td><td>PCIE 设备的 DMA 请求类别：
0: 非一致性请求
1: 为一致性请求</td></tr><tr><td>33</td><td>usb_coherent</td><td>RW</td><td>0x1</td><td>USB 设备的 DMA 请求类别：
0: 非一致性请求
1: 为一致性请求</td></tr><tr><td>32</td><td>hda_coherent</td><td>RW</td><td>0x1</td><td>HDA 设备的 DMA 请求类别：
0: 非一致性请求
1: 为一致性请求</td></tr><tr><td>31:21</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>20</td><td>sdio_sel</td><td>RW</td><td>0x0</td><td>SDIO 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 SDIO</td></tr><tr><td>19:18</td><td></td><td></td><td></td><td></td></tr><tr><td>17:16</td><td>can_sel</td><td>RW</td><td>0x0</td><td>CAN 管脚复用控制：当专用通信接口为 0 时
0: 管脚为 GPIO
1: 管脚为 CAN
否则，管脚为专用通信接口</td></tr><tr><td>15:12</td><td>pwm_sel</td><td>RW</td><td>0x0</td><td>PWM 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 PWM</td></tr><tr><td>11:10</td><td>i2c_sel</td><td>RW</td><td>0x0</td><td>I2C 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 I2C</td></tr><tr><td>9</td><td>nand_sel</td><td>RW</td><td>0x0</td><td>NAND 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 NAND</td></tr><tr><td>8</td><td>sata_sel</td><td>RW</td><td>0x0</td><td>SATA 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 SATA</td></tr><tr><td>7</td><td>lio_sel</td><td>RW</td><td>0x0</td><td>当 dvo0_sel 为 0 时，LIO 管脚复用控制：
0: 管脚为 GPIO
1: 管脚为 LIO
注：dvo0_sel、lio_sel 和 uart1/2_sel 三者只能有一个为 1</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>6</td><td>i2s_sel</td><td>RW</td><td>0x0</td><td>表示 I2S 管脚复用控制：
0：管脚为 HDA 或 GPIO
1：管脚为 I2S
注：hda_sel、和 i2s_sel 只能有一个为 1，二者都设 0 为 GPIO 模式</td></tr><tr><td>5</td><td>-</td><td>RW</td><td>0x0</td><td>保留</td></tr><tr><td>4</td><td>hda_sel</td><td>RW</td><td>0x0</td><td>HDA 管脚复用控制：
0：管脚为 I2S 或 GPIO
1：管脚为 HDA
注：hda_sel 和 i2s_sel 只能有一个为 1，二者都设 0 为 GPIO 模式</td></tr><tr><td>3</td><td>gmac1_sel</td><td>RW</td><td>0x1</td><td>GMAC1 管脚复用控制：
0：管脚为 GPIO
1：管脚为 GMAC1</td></tr><tr><td>2</td><td>gmac1_sdb_flowctrl</td><td>RW</td><td>0x0</td><td>GMAC1 的边带流控制，当 GMAC 控制器的 EFC 使能时，可以让 MAC 产生 Pause Control Frame 控制 Rx FIFO 的流量：
0：关闭该功能
1：使能该功能</td></tr><tr><td>1</td><td>gmac0_sdb_flowctrl</td><td>RW</td><td>0x0</td><td>GMAC0 的边带流控制，当 GMAC 控制器的 EFC 使能时，可以让 MAC 产生 Pause Control Frame 控制 Rx FIFO 的流量：
0：关闭该功能
1：使能该功能</td></tr><tr><td>0</td><td>gmac_coherence_en</td><td>RW</td><td>0x1</td><td>GMAC 设备的 DMA 请求类别：
0：非一致性请求
1：为一致性请求</td></tr></table>

# 5.2 通用配置寄存器1

通用配置寄存器1，包括对管脚复用的控制以及流控制等。

地址：0x1fe00428

表5-3通用配置寄存器1  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:60</td><td>delay_hda</td><td>RW</td><td>0x0</td><td>HDA 访存通路写命令堵塞时间</td></tr><tr><td>59:56</td><td>delay_pcie0</td><td>RW</td><td>0x4</td><td>PCIE0 访存通路写命令堵塞时间</td></tr><tr><td>55:52</td><td>delay_usb</td><td>RW</td><td>0x4</td><td>USB 访存通路写命令堵塞时间</td></tr><tr><td>51:48</td><td>delay_sata</td><td>RW</td><td>0x4</td><td>SATA 访存通路写命令堵塞时间</td></tr><tr><td>47:44</td><td>delay_dc</td><td>RW</td><td>0x2</td><td>DC 访存通路写命令堵塞时间</td></tr><tr><td>43:40</td><td>delay_gpu</td><td>RW</td><td>0x2</td><td>GPU 访存通路写命令堵塞时间</td></tr><tr><td>39:36</td><td>delay_dma</td><td>RW</td><td>0x4</td><td>DMA 访存写命令堵塞时间</td></tr><tr><td>35:32</td><td>delay_gmac</td><td>RW</td><td>0x4</td><td>GMAC 访存通路写命令堵塞时间</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>31:28</td><td>awmon_channel</td><td>RW</td><td>0x0</td><td>访存写命令通路 monitor 通道选择</td></tr><tr><td>27:26</td><td>awmon_select</td><td>RW</td><td>0x0</td><td>访存写命令通路 monitor 类型选择</td></tr><tr><td>25</td><td>awmon_clear</td><td>RW</td><td>0x0</td><td>访存写命令通路 monitor 清空</td></tr><tr><td>24</td><td>awmon_start</td><td>RW</td><td>0x0</td><td>访存写命令通路 monitor 开始工作</td></tr><tr><td>23:20</td><td>usb_flush_idle</td><td>RW</td><td>0xf</td><td>设置清空 write buffer 前空间周期数</td></tr><tr><td>19</td><td>usb Prefetch</td><td>RW</td><td>0x1</td><td>使能读预取</td></tr><tr><td>18</td><td>usb_flush_wr</td><td>RW</td><td>0x0</td><td>设置写命令发出后是否清空 read buffer</td></tr><tr><td>17</td><td>usb_stop_waw</td><td>RW</td><td>0x0</td><td>是否允许在上一个写完成前发出写命令</td></tr><tr><td>16</td><td>usb_stop_raw</td><td>RW</td><td>0x1</td><td>是否允许在上一个写完成前发出读命令</td></tr><tr><td>15:14</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>13</td><td>uart2_sel</td><td>RW</td><td>0x0</td><td>当 dvo0_sel 为 0 时，UART2 管脚复用控制：
0：管脚为 GPIO
1：管脚为 UART2
注：dvo0_sel、lio_sel 和 uart1/2_sel 三者只能有一个为 1</td></tr><tr><td>12</td><td>uart1_sel</td><td>RW</td><td>0x0</td><td>当 dvo0_sel 为 0 时，UART1 管脚复用控制：
0：管脚为 GPIO
1：管脚为 UART1
注：dvo0_sel、lio_sel 和 uart1/2_sel 三者只能有一个为 1</td></tr><tr><td>11:8</td><td>uart2_enable</td><td>RW</td><td>0x1</td><td>UART2 对应的 UART 控制器：
4&#x27;b0001：8 线模式（仅 uart2）
4&#x27;b0011：4 线模式（uart2+uart9）
4&#x27;b1111：2 线模式（uart2+uart9+uart10+uart11）
其他：保留</td></tr><tr><td>7:4</td><td>uart1_enable</td><td>RW</td><td>0x1</td><td>UART1 对应的 UART 控制器：
4&#x27;b0001：8 线模式（仅 uart1）
4&#x27;b0011：4 线模式（uart1+uart6）
4&#x27;b1111：2 线模式（uart1+uart6+uart7+uart8）
其他：保留</td></tr><tr><td>3:0</td><td>uart0_enable</td><td>RW</td><td>0x1</td><td>UART0 对应的 UART 控制器：
4&#x27;b0001：8 线模式（仅 uart0）
4&#x27;b0011：4 线模式（uart0+uart3）
4&#x27;b1111：2 线模式（uart0+uart3+uart4+uart5）
其他：保留</td></tr></table>

# 5.3 通用配置寄存器2

通用配置寄存器2，包括对GPU的软复位，管脚复用及PCIE设备的使能等。地址：0x1fe00430

表5-4通用配置寄存器2  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:32</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>31:28</td><td>cam_flush_idle</td><td>R/W</td><td>0xf</td><td>cam接口空闲写入周期数</td></tr><tr><td>27</td><td>cam prefetch</td><td>R/W</td><td>0x1</td><td>cam接口预取</td></tr><tr><td>26</td><td>cam_flush_wr</td><td>R/W</td><td>0x0</td><td>cam接口请求刷出写请求</td></tr><tr><td>25</td><td>cam_stop_waw</td><td>R/W</td><td>0x0</td><td>cam接口停止写后写</td></tr><tr><td>24</td><td>cam_stop_raw</td><td>R/W</td><td>0x1</td><td>cam接口停止写后读</td></tr><tr><td>23:21</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>20</td><td>vpu_disable</td><td>RW</td><td>0x0</td><td>VPU模块使能信号，1关闭，0打开</td></tr><tr><td>19</td><td>cam_disable</td><td>RW</td><td>0x0</td><td>CAMERA模块使能信号，1关闭，0打开</td></tr><tr><td>18</td><td>cam_soft_reset</td><td>RW</td><td>0x0</td><td>CAMERA模块软件复位，低有效。</td></tr><tr><td>17</td><td>pcie1_enable</td><td>RW</td><td>0x0</td><td>PCIE1控制器使能信号，高有效。当不使能时软件扫描不到该PCIE桥。</td></tr><tr><td>16</td><td>pcie0_enable</td><td>RW</td><td>0x0</td><td>PCIE0控制器使能信号，高有效。当不使能时软件扫描不到该PCIE桥。</td></tr><tr><td>15:8</td><td>iodma_spare_rd</td><td>RW</td><td>0x0</td><td>iodma读操作最大数设置</td></tr><tr><td>7</td><td>cam_coherent</td><td>RW</td><td>0x1</td><td>CAMERA设备的DMA请求类别：
0：非一致性请求
1：为一致性请求</td></tr><tr><td>6</td><td>vpu_coherent</td><td>RW</td><td>0x1</td><td>VPU设备的DMA请求类别：
0：非一致性请求
1：为一致性请求</td></tr><tr><td>5</td><td>cam_sel</td><td>RW</td><td>0x0</td><td>使能CAMERA的管脚功能：
0：管脚保留
1：管脚为CAMERA
注：dvo1_sel、cam_sel二者只能有一个为1</td></tr><tr><td>4</td><td>dvo1_sel</td><td>RW</td><td>0x0</td><td>使能DVO1的管脚功能：
0：管脚保留
1：管脚为DVO1
注：dvo1_sel、cam_sel二者只能有一个为1</td></tr><tr><td>3</td><td>dc_coherent</td><td>RW</td><td>0x1</td><td>DC设备的DMA请求类别：
0：非一致性请求
1：为一致性请求</td></tr><tr><td>2</td><td>gpu_coherent</td><td>RW</td><td>0x1</td><td>GPU设备的DMA请求类别：
0：非一致性请求
1：为一致性请求</td></tr><tr><td>1</td><td>dvo0_sel</td><td>RW</td><td>0x0</td><td>使能DVO0的管脚功能：
0：管脚保留
1：管脚为DVO0
注：dvo0_sel、lio_sel和uart1/2_sel三者只能有一个为1</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>0</td><td>gpu_soft_reset</td><td>RW</td><td>0x0</td><td>GPU的软件复位：
1：代表复位
0：代表解复位</td></tr></table>

# 5.4 APBDMA配置寄存器

APBDMA配置寄存器用于给APB设备配置对应的DMA控制器。地址：0x1fe00438

表5-5APBDMA配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:30</td><td>Reserved</td><td>RO</td><td>0x0</td><td>保留</td></tr><tr><td>29:27</td><td>Reserved</td><td>RW</td><td>0x2</td><td>保留</td></tr><tr><td>26:24</td><td>Reserved</td><td>RW</td><td>0x1</td><td>保留</td></tr><tr><td>23:21</td><td>dma_sel7</td><td>RW</td><td>0x1</td><td>I2S控制器接收端所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>20:18</td><td>dma_sel6</td><td>RW</td><td>0x0</td><td>I2S控制器及选端所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>17:15</td><td>dma_sel5</td><td>RW</td><td>0x0</td><td>SDIO控制器所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>14:12</td><td>dma_sel4</td><td>RW</td><td>0x1</td><td>DES控制器写操作所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>11:9</td><td>dma_sel3</td><td>RW</td><td>0x2</td><td>DES控制器读操作所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>8:6</td><td>dma_sel2</td><td>RW</td><td>0x1</td><td>AES控制器与操作所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>5:3</td><td>dma_sel1</td><td>RW</td><td>0x2</td><td>AES控制器读操作所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr><tr><td>2:0</td><td>dma_sel0</td><td>RW</td><td>0x0</td><td>NAND控制器所用的DMA控制器：
0x0：对应DMA0控制器
0x1：对应DMA1控制器
0x2：对应DMA2控制器
0x3：对应DMA3控制器
0x4：对应DMA4控制器
其他：保留</td></tr></table>

# 5.5 USBPHY/01配置寄存器

配置USB接口0/1相关的电气特性地址：0x1fe00440

表5-6USB0/1PHY配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>61</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>通用模块关电模式
1：在 suspend 时，XO，Bias，PLL 模块掉电；在睡眠时，Bias，PLL 掉电
0：在 suspend 或睡眠时 XO，Bias，PLL 都有电</td></tr><tr><td>60</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dm 端口下拉电阻使能
0：D-使能
1：D-关闭</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>59</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>dp 端口下拉电阻使能
0: D+使能
1: D+关闭</td></tr><tr><td>58:57</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>该信号调整了驱动电阻，用来补偿在发送器到电缆之间的寄生电阻
11: -4Ω
10: -2Ω
01: 默认
00: +1.5Ω</td></tr><tr><td>56</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>调整了在高速模式下预加强电流在dp,dm电缆上的持续时间，高速收发器预加强电流持续时间根据时间单位来定义
1: 1X，短持续时间
0: 2X，长持续时间</td></tr><tr><td>55:54</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>调整了在高速模式从J到K或从K到J的跳变时提供的电流量，高速收发器预加强电流量根据电流量单位来定义
11: HS传输预加强3X电流
10: HS传输预加强2X电流
01: HS传输预加强1X电流
00: HS传输预加强电流关闭</td></tr><tr><td>53:52</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>在高速模式下调整dp，dm信号交叉时的电压
11: default
10: +15mv
01: -15mv
00: reserved</td></tr><tr><td>51:50</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>调整了高速波形上升、下降沿次数
11: -10%
10: default
01: +15%
00: +20%</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>49:46</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>调整高速直流电压
1111: +8.75%
1110: +7.5%
1101: +6.25%
1100: +5%
1011: +3.75%
1010: +2.5%
1001: +1.25%
1000: default
0111: -1.25%
0110: -2.5%
0101: -3.75%
0100: -5%
0011: -6.25%
0010: -7.5%
0001: -8.75%
0000: -10%</td></tr><tr><td>45:42</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>调整了低、全速单端源阻抗
1111: -5%
0111: -2.5%
0011: default
0001: +2.5%
0000: +5%</td></tr><tr><td>41:39</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>调整门限电压来检测有效的高速数据
111: -20%
110: -15%
101: -10%
100: -5%
011: default
010: +5%
001: +10%
000: +15%</td></tr><tr><td>38:36</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>调整门限电压用于检测在主控制器上的一个断开连接事件
111: +4.5%
110: +3%
101: +1.5%
100: default
011: -1.5%
010: -3%
001: -4.5%
000: -6%</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>35:33</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>调整 Vbus Valid 的门限电压
111: +9%
110: +6%
101: +3%
100: default
011: -3%
010: -6%
001: -9%
000: -12%</td></tr><tr><td>32</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>配置使能
0: 使用硬件默认配置
1: 使用该寄存器的软件配置</td></tr><tr><td>30</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>低速模式数据有效使能控制
0: 低速模式下,数据信号不受使能信号控制其有效性
1: 低速模式下,数据信号受到使能信号控制其有效性</td></tr><tr><td>29</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>通用模块关电模式
1: 在 suspend 时, XO, Bias, PLL 模块掉电; 在睡眠时, Bias, PLL 掉电
0: 在 suspend 或睡眠时 XO, Bias, PLL 都有电</td></tr><tr><td>28</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dm 端口下拉电阻使能
1: D-使能
0: D-关闭</td></tr><tr><td>27</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dp 端口下拉电阻使能
1: D+使能
0: D+关闭</td></tr><tr><td>26:25</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>24</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>23:22</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>21:20</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>19:18</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>17:14</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>13:10</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>9:7</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>6:4</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>3:1</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>0</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>配置使能
0: 使用硬件默认配置
1: 使用该寄存器的软件配置</td></tr></table>

# 5.6 USBPHY2/3配置寄存器

配置USB接口2/3相关的电气特性。地址：0x1fe00448

表5-7USB2/3PHY配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>61</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>通用模块关电模式
1：在 suspend 时，XO，Bias，PLL 模块掉电；在睡眠时，Bias，PLL 掉电
0：在 suspend 或睡眠时 XO，Bias，PLL 都有电</td></tr><tr><td>60</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dm 端口下拉电阻使能
0：D-使能
1：D-关闭</td></tr><tr><td>59</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dp 端口下拉电阻使能
0：D+使能
1：D+关闭</td></tr><tr><td>58:57</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>56</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>55:54</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>53:52</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>51:50</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>49:46</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>45:42</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>41:39</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>38:36</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>35:33</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>32</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>配置使能
0：使用硬件默认配置
1：使用该寄存器的软件配置</td></tr><tr><td>29</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>通用模块关电模式
1：在 suspend 时，XO，Bias，PLL 模块掉电；在睡眠时，Bias，PLL 掉电
0：在 suspend 或睡眠时 XO，Bias，PLL 都有电</td></tr><tr><td>28</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dm 端口下拉电阻使能
1：D-使能
0：D-关闭</td></tr><tr><td>27</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>dp 端口下拉电阻使能
1：D+使能
0：D+关闭</td></tr><tr><td>26:25</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>24</td><td>usb_phy.cfg</td><td>R/W</td><td>0</td><td>同上</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>23:22</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>21:20</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>19:18</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>17:14</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>13:10</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>9:7</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>6:4</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>3:1</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>同上</td></tr><tr><td>0</td><td>usb_phy_cfg</td><td>R/W</td><td>0</td><td>配置使能
0:使用硬件默认配置
1:使用该寄存器的软件配置</td></tr></table>

# 5.7 SATA配置寄存器

配置SATA的控制信号。

地址：0x1fe00450

表5-8SATA配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63</td><td>sata_phy_cfg</td><td>R/W</td><td>0</td><td>关闭 SATA PHY</td></tr><tr><td>62:57</td><td>sata_phy_cfg</td><td>R/W</td><td>0x3f</td><td>GEN3 模式下发送端预加重设置</td></tr><tr><td>56:51</td><td>sata_phy_cfg</td><td>R/W</td><td>0x3f</td><td>GEN2 模式下发送端预加重设置</td></tr><tr><td>50:45</td><td>sata_phy_cfg</td><td>R/W</td><td>0x3f</td><td>GEN1 模式下发送端预加重设置</td></tr><tr><td>44:38</td><td>sata_phy_cfg</td><td>R/W</td><td>0x7f</td><td>GEN3 模式下发送端摆幅设置</td></tr><tr><td>37:31</td><td>sata_phy_cfg</td><td>R/W</td><td>0x7f</td><td>GEN2 模式下发送端摆幅设置</td></tr><tr><td>30:24</td><td>sata_phy_cfg</td><td>R/W</td><td>0x7f</td><td>GEN1 模式下发送端摆幅设置</td></tr><tr><td>23</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>Sata 接口预取</td></tr><tr><td>22</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>Sata 接口读请求刷出写请求</td></tr><tr><td>21</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>Sata 接口停止写后写</td></tr><tr><td>20</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>Sata 接口停止写后读</td></tr><tr><td>19:16</td><td>sata_phy_cfg</td><td>R/W</td><td>0xf</td><td>Sata 接口空闲写入周期数</td></tr><tr><td>15</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>PHY 内部继电器使能。Vph 接 2.5V 电压时设置为 1，Vph 接 3.3V 电压时设置为 0</td></tr><tr><td>14:12</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>Rx equalizer 设置</td></tr><tr><td>11</td><td>sata_phy_cfg</td><td></td><td></td><td></td></tr><tr><td>10</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>dma cache 一致性使能</td></tr><tr><td>9</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>LANEO 发送端极性反向使能 1 代表反向，0 代表不反向</td></tr><tr><td>8</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>LANEO 接收端极性反向使能 1 代表反向，0 代表不反向</td></tr><tr><td>7</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>Spread Spectrum 使能</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>6:4</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>Spread Spectrum Clock Range</td></tr><tr><td>3</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>LANEO 软复位，0 有效</td></tr><tr><td>2</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>PHY 软复位，0 有效</td></tr><tr><td>1</td><td>sata_phy_cfg</td><td>R/W</td><td>0x0</td><td>参考时钟输入选择：
0- 选择内部时钟
1- 选择外部时钟</td></tr><tr><td>0</td><td>sata_phy_cfg</td><td>R/W</td><td>0x1</td><td>PHY 参考时钟使能</td></tr></table>

# 5.8 NODEPLL低64位配置寄存器

该寄存器用来设置NODEPLL，具体参数用法请参考第3章（时钟结构）。地址：0x1fe00480

表5-9NODEPLL低64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:54</td><td>L2_div_loop_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>53:48</td><td>L2_div_refc_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>47:42</td><td>L1_div_out</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>41:32</td><td>L1_div_loopc</td><td>R/W</td><td>0</td><td>L1PLL 信频系数</td></tr><tr><td>31:26</td><td>L1_div_ref</td><td>R/W</td><td>0</td><td>L1PLL 参考时钟分频系数</td></tr><tr><td>23</td><td>serial_mode3</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>22</td><td>serial_mode</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21</td><td>pd_pll_L2_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>20</td><td>pd_pll_L2_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>19</td><td>pd_pll_L1</td><td>R/W</td><td>0</td><td>L1PLL 关电控制，1 代表关电</td></tr><tr><td>18</td><td>locked_L2_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>17</td><td>locked_L2_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>16</td><td>locked_L1</td><td>R</td><td>0</td><td>L1PLL 锁定标志，1 代表锁定</td></tr><tr><td>15:14</td><td>lockc_L2_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>13:12</td><td>lockc_L2_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:10</td><td>lockc_L1</td><td>R/W</td><td>0</td><td>判定 L1PLL 是否锁定使用的相位的精度</td></tr><tr><td>9</td><td>locken_L2_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>8</td><td>locken_L2_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>7</td><td>locken_L1</td><td>R/W</td><td>0</td><td>允许锁定 L1PLL</td></tr><tr><td>6</td><td>bypass_refin_L2</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5</td><td>bypass_L2_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>bypass_L2_node</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>bypass_L1</td><td>R/W</td><td>0</td><td>Bypass L1PLL</td></tr><tr><td>2</td><td>pll_soft_set</td><td>R/W</td><td>0</td><td>允许软件设置PLL</td></tr><tr><td>1</td><td>sel_pll_core</td><td>R/W</td><td>0</td><td>保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>0</td><td>selPLL_node</td><td>R/W</td><td>0</td><td>NODE 时钟非软件 bypass 整个PLL</td></tr></table>

# 5.9 NODEPLL高64位配置寄存器

该寄存器用来设置NODEPLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe00488

表5-10NODEPLL高64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:34</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>33:28</td><td>L2_div_out_pcie</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>27:22</td><td>L2_div_out_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21:12</td><td>L2_div_loopc_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:6</td><td>L2_div_refc_core</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5:0</td><td>L2_div_out_node</td><td>R/W</td><td>0</td><td>L2 NODEPLL分频系数</td></tr></table>

# 5.10DDRPLL低64位配置寄存器

该寄存器用来设置DDRPLL，具体参数用法请参考第3章（时钟结构）。地址：0x1fe00490

表5-11DDRPLL低64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:54</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>53:48</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>47:42</td><td>L1_div_out</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>41:32</td><td>L1_div_loopc</td><td>R/W</td><td>0</td><td>L1PLL 信频系数</td></tr><tr><td>31:26</td><td>L1_div_ref</td><td>R/W</td><td>0</td><td>L1PLL 参考时钟分频系数</td></tr><tr><td>23</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>20</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>19</td><td>pdPLL_L1</td><td>R/W</td><td>0</td><td>L1PLL 关电控制，1 代表关电</td></tr><tr><td>18</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>17</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>16</td><td>locked_L1</td><td>R</td><td>0</td><td>L1PLL 锁定标志，1 代表锁定</td></tr><tr><td>15:14</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>13:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:10</td><td>lockc_L1</td><td>R/W</td><td>0</td><td>判定 L1PLL 是否锁定使用的相位的精度</td></tr><tr><td>9</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>8</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>7</td><td>locken_L1</td><td>R/W</td><td>0</td><td>允许锁定 L1 PLL</td></tr><tr><td>6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>bypass_L1</td><td>R/W</td><td>0</td><td>Bypass L1 PLL</td></tr><tr><td>2</td><td>pll_soft_set</td><td>R/W</td><td>0</td><td>允许软件设置 PLL</td></tr><tr><td>1</td><td>sel_PLL_gpu</td><td>R/W</td><td>0</td><td>GPU/HDA 时钟非软件 bypass 整个PLL</td></tr><tr><td>0</td><td>sel_PLL_ddr</td><td>R/W</td><td>0</td><td>DDR 时钟非软件 bypass 整个PLL</td></tr></table>

# 5.11DDRPLL高64位配置寄存器

该寄存器用来设置DDRPLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe00498

表5-12DDRPLL高64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:51</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>50:44</td><td>L2_div_out_hda</td><td>R/W</td><td>0</td><td>L2 HDA PLL 分频系数</td></tr><tr><td>43:28</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>27:22</td><td>L2_div_out_gpu</td><td>R/W</td><td>0</td><td>L2 GPU PLL 分频系数</td></tr><tr><td>21:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5:0</td><td>L2_div_out_ddr</td><td>R/W</td><td>0</td><td>L2 DDR PLL 分频系数</td></tr></table>

# 5.12DCPLL低64位配置寄存器

该寄存器用来设置DCPLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe004a0

表5-13DCPLL低64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:54</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>53:48</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>47:42</td><td>L1_div_out</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>41:32</td><td>L1_div_loopc</td><td>R/W</td><td>0</td><td>L1 PLL 信频系数</td></tr><tr><td>31:26</td><td>L1_div_ref</td><td>R/W</td><td>0</td><td>L1 PLL 参考时钟分频系数</td></tr><tr><td>23</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>20</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>19</td><td>pd_pll_L1</td><td>R/W</td><td>0</td><td>L1PLL关电控制，1代表关电</td></tr><tr><td>18</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>17</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>16</td><td>locked_L1</td><td>R</td><td>0</td><td>L1PLL锁定标志，1代表锁定</td></tr><tr><td>15:14</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>13:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:10</td><td>lockc_L1</td><td>R/W</td><td>0</td><td>判定L1PLL是否锁定使用的相位的精度</td></tr><tr><td>9</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>8</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>7</td><td>locken_L1</td><td>R/W</td><td>0</td><td>允许锁定L1PLL</td></tr><tr><td>6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>bypass_L1</td><td>R/W</td><td>0</td><td>Bypass L1PLL</td></tr><tr><td>2</td><td>pll_soft_set</td><td>R/W</td><td>0</td><td>允许软件设置PLL</td></tr><tr><td>1</td><td>sel_pll_gmac</td><td>R/W</td><td>0</td><td>GMAC时钟非软件bypass整个PLL</td></tr><tr><td>0</td><td>sel_pll_dc</td><td>R/W</td><td>0</td><td>DC时钟非软件bypass整个PLL</td></tr></table>

# 5.13DCPLL高64位配置寄存器

该寄存器用来设置DCPLL，具体参数用法请参考第3章（时钟结构）。地址：0x1fe004a8

表5-14DCPLL高64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:34</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>33:28</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>27:22</td><td>L2_div_out_gma</td><td>R/W</td><td>0</td><td>L2 GMACPLL分频系数</td></tr><tr><td>21:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5:0</td><td>L2_div_out_dc</td><td>R/W</td><td>0</td><td>L2 DCPLL分频系数</td></tr></table>

# 5.14PIX0PLL低64位配置寄存器

该寄存器用来设置PIX0PLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe004b0

表5-15PIX0PLL低64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:54</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>53:48</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>47:42</td><td>L1_div_out</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>41:32</td><td>L1_div_loopc</td><td>R/W</td><td>0</td><td>L1PLL 首频系数</td></tr><tr><td>31:26</td><td>L1_div_ref</td><td>R/W</td><td>0</td><td>L1PLL 参考时钟分频系数</td></tr><tr><td>23</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>20</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>19</td><td>pd_pll_L1</td><td>R/W</td><td>0</td><td>L1PLL 关电控制，1代表关电</td></tr><tr><td>18</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>17</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>16</td><td>locked_L1</td><td>R</td><td>0</td><td>L1PLL 锁定标志，1代表锁定</td></tr><tr><td>15:14</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>13:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:10</td><td>lockc_L1</td><td>R/W</td><td>0</td><td>判定 L1PLL 是否锁定使用的相位的精度</td></tr><tr><td>9</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>8</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>7</td><td>locken_L1</td><td>R/W</td><td>0</td><td>允许锁定 L1PLL</td></tr><tr><td>6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>bypass_L1</td><td>R/W</td><td>0</td><td>Bypass L1PLL</td></tr><tr><td>2</td><td>pll_soft_set</td><td>R/W</td><td>0</td><td>允许软件设置PLL</td></tr><tr><td>1</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>0</td><td>sel_pll_pix0</td><td>R/W</td><td>0</td><td>PIX0 时钟非软件 bypass 整个PLL</td></tr></table>

# 5.15PIX0PLL高64位配置寄存器

该寄存器用来设置PIX0PLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe004b8

表5-16PIX0PLL高64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:34</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>33:28</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>27:22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5:0</td><td>L2_div_out_pix0</td><td>R/W</td><td>0</td><td>L2 PIX0PLL 分频系数</td></tr></table>

# 5.16PIX1PLL低64位配置寄存器

该寄存器用来设置PIX1PLL，具体参数用法请参考第3章（时钟结构）。

地址：0x1fe004c0

表5-17PIX1PLL低64位配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:54</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>53:48</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>47:42</td><td>L1_div_out</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>41:32</td><td>L1_div_loopc</td><td>R/W</td><td>0</td><td>L1PLL 培顿系数</td></tr><tr><td>31:26</td><td>L1_div_ref</td><td>R/W</td><td>0</td><td>L1PLL 参考时钟分频系数</td></tr><tr><td>23</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>20</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>19</td><td>pdPLL_L1</td><td>R/W</td><td>0</td><td>L1PLL 天电控制，1代表关电</td></tr><tr><td>18</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>17</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>16</td><td>locked_L1</td><td>R</td><td>0</td><td>L1PLL 锁定标志，1代表锁定</td></tr><tr><td>15:14</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>13:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:10</td><td>lockc_L1</td><td>R/W</td><td>0</td><td>判定 L1PLL 是否锁定使用的相位的精度</td></tr><tr><td>9</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>8</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>7</td><td>locken_L1</td><td>R/W</td><td>0</td><td>允许锁定 L1PLL</td></tr><tr><td>6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>bypass_L1</td><td>R/W</td><td>0</td><td>Bypass L1PLL</td></tr><tr><td>2</td><td>pll_soft_set</td><td>R/W</td><td>0</td><td>允许软件设置PLL</td></tr><tr><td>1</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>0</td><td>selPLL_pix1</td><td>R/W</td><td>0</td><td>PIX1 时钟非软件 bypass 整个PLL</td></tr></table>

# 5.17PIX1PLL高64位配置寄存器

该寄存器用来设置PIX1PLL，具体参数用法请参考第3章（时钟结构）。地址：0x1fe004c8

表5- 18PIX1PLL高64位配置寄存器

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:34</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>33:28</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>27:22</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>21:12</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>11:6</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>5:0</td><td>L2_div_out_pix1</td><td>R/W</td><td>0</td><td>L2 PIX1 PLL 分频系数</td></tr></table>

# 5.18FREQSCALE配置寄存器

该寄存器用于配置时钟的分频系数以及CPU核的时钟使能

地址：0x1fe004d0

表5-19FRESCALE配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:34</td><td>-</td><td>R/W</td><td>0</td><td>保留</td></tr><tr><td>33</td><td>core1_en</td><td>R/W</td><td>0x1</td><td>CPU 核 1 时钟使能</td></tr><tr><td>32</td><td>core0_en</td><td>R/W</td><td>0x1</td><td>CPU 核 0 时钟使能</td></tr><tr><td>22:20</td><td>apb_freqscale</td><td>R/W</td><td>0x7</td><td>apb 时钟分频系数
计算公式为 8/(6x1)，比如设置成 3 时代表 2 分频</td></tr><tr><td>18:16</td><td>usb_freqscale</td><td>R/W</td><td>0x7</td><td>usb 时钟分频系数，计算方法同上</td></tr><tr><td>14:12</td><td>sata_freqscale</td><td>R/W</td><td>0x7</td><td>sata 时钟分频系数，计算方法同上</td></tr><tr><td>10:8</td><td>boot_freqscale</td><td>R/W</td><td>0x7</td><td>boot 时钟分频系数，计算方法同上</td></tr><tr><td>7:3</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>2:0</td><td>node_freqscale</td><td>R/W</td><td>0x7</td><td>node 时钟分频系数</td></tr></table>

Freqscale分频计算公式为：  $\mathrm{f} =$  pref \*（freqscale  $+1$  ）/8.

# 5.19PCIE0配置寄存器0

配置PCIE0PHY的控制信号。

地址：0x1fe00580

表5-20PCIE0配置寄存器0  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:59</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置 PCIE LANE0 发送端阻抗在 50 欧左右的微调值</td></tr><tr><td>58:52</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>设置 PCIE PHY 发送低摆幅幅值</td></tr><tr><td>51:45</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x10</td><td>设置 PCIE PHY 发送满摆幅幅值</td></tr><tr><td>44:39</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x30</td><td>设置 PCIE PHY 在 GEN2 模式下的-6dB tx deemphasis 值</td></tr><tr><td>38:33</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x20</td><td>设置 PCIE PHY 在 GEN2 模式下的-3.5dB tx deemphasis 值</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>32:27</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x10</td><td>设置PCIEPHY在GEN1模式下的tx deemphasis值</td></tr><tr><td>26:24</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE3接收端equalizer设置</td></tr><tr><td>23:21</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE2接收端equalizer设置</td></tr><tr><td>20:18</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE1接收端equalizer设置</td></tr><tr><td>17:15</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE0接收端equalizer设置</td></tr><tr><td>14</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>common clock mode设置，只有PCIE链路两端使用相同参考时钟时才可设置</td></tr><tr><td>13</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x1</td><td>表示PCIEPHY在P2状态下需要输出PIPE时钟</td></tr><tr><td>12</td><td>pcie_phy_cfg</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>11:5</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x19</td><td>PCIEPHYPLL的倍频数</td></tr><tr><td>4</td><td>pcie_phy_cfg</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>3</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>2</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>1</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>0</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x1</td><td>保留</td></tr></table>

# 5.20PCIE0配置寄存器1

配置PCIE0PHY的接口信号。

地址：0x1fe00588

表5-21PCIE0配置寄存器1  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:25</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>24</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIEPHY进入低功耗模式</td></tr><tr><td>23:20</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>PCIE参考时钟输出使能</td></tr><tr><td>19</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>发起resistertune请求</td></tr><tr><td>18</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>PCIEPHY内部voltage regulator使能位。Vph接2.5V电压时设置为1，Vph接3.3V电压时设置为0</td></tr><tr><td>17:15</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x4</td><td>Tx voltage boost level</td></tr><tr><td>14:10</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE3发送端阻抗在50欧左右的微调值</td></tr><tr><td>9:5</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE2发送端阻抗在50欧左右的微调值</td></tr><tr><td>4:0</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE1发送端阻抗在50欧左右的微调值</td></tr></table>

# 5.21PCIEOPHY配置控制寄存器

通过这些寄存器配置PCIEOPHY的内部寄存器

地址：0x1fe00590

表5-22PCIEOPHY配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:39</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>38</td><td>phy_cfg_reset</td><td>R/W</td><td>0</td><td>PHY配置重置，高有效</td></tr><tr><td>37:35</td><td>phy_cfg_state</td><td>R</td><td>0</td><td>PHY配置状态机状态指示</td></tr><tr><td>34</td><td>phy_cfg_done</td><td>R/W</td><td>0</td><td>PHY一次访问完成，指示此次对PHY的读写完成。写完成表示写的数据已经写入PHY内部寄存器，读完成表示读的数据已经存储到write/read data寄存器</td></tr><tr><td>33</td><td>phy_cfg_disable</td><td>R/W</td><td>0x0</td><td>0--对该组寄存器读写会触发PHY配置操作
1--对该组寄存器读写不触发PHY配置操作，反为简单的寄存器读写</td></tr><tr><td>32</td><td>phy_cfg_rw</td><td>R/W</td><td>0</td><td>开始读操作或写操作。为1时为写操作，为0时为读操作。</td></tr><tr><td>31:16</td><td>phy_cfg_data</td><td>R/W</td><td>0</td><td>PHY配置读写数据。在写操作时，将数据先写入该寄存，然后再执行写操作；在读操作时，从PHY返回的读数据存储到该寄存器。</td></tr><tr><td>15:0</td><td>phycfg_addr</td><td>R/W</td><td>0</td><td>PHY配置地址</td></tr></table>

# 5.22PCIE1配置寄存器0

配置PCIE1PHY的控制信号。

地址：0x1fe005a0

表5-23PCIE1配置寄存器0  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:59</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置 PCIE LANEO 发送端阻抗在 50 欧左右的微调值</td></tr><tr><td>58:52</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>设置 PCIE PHY 发送低摆幅幅值</td></tr><tr><td>51:45</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x10</td><td>设置 PCIE PHY 发送满摆幅幅值</td></tr><tr><td>44:39</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x30</td><td>设置 PCIE PHY 在 GEN2 模式下的-6dB tx deemphasis 值</td></tr><tr><td>38:33</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x20</td><td>设置 PCIE PHY 在 GEN2 模式下的-3.5dB tx deemphasis 值</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>32:27</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x10</td><td>设置PCIEPHY在GEN1模式下的tx deemphasis值</td></tr><tr><td>26:24</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE3接收端equalizer设置</td></tr><tr><td>23:21</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE2接收端equalizer设置</td></tr><tr><td>20:18</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE1接收端equalizer设置</td></tr><tr><td>17:15</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x2</td><td>LANE0接收端equalizer设置</td></tr><tr><td>14</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>common clock mode设置，只有PCIE链路两端使用相同参考时钟时才可设置</td></tr><tr><td>13</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x1</td><td>表示PCIEPHY在P2状态下需要输出PIPE时钟</td></tr><tr><td>12</td><td>pcie_phy_cfg</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>11:5</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x19</td><td>PCIEPHYPLL的倍频数</td></tr><tr><td>4</td><td>pcie_phy_cfg</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>3</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>2</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>1</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x0</td><td>保留</td></tr><tr><td>0</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x1</td><td>保留</td></tr></table>

# 5.23PCIE1配置寄存器1

配置PCIE1PHY的接口信号。

地址：0x1fe005a8

表5-24PCIE1配置寄存器1  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:25</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>24</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIEPHY进入低功耗模式</td></tr><tr><td>23:20</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>PCIE参考时钟输出使能</td></tr><tr><td>19</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>发起resister tune请求</td></tr><tr><td>18</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>PCIEPHY内部voltage regulator使能位。Vph接2.5V电压时设置为1，Vph接3.3V电压时设置为0</td></tr><tr><td>17:15</td><td>pcie_phy_cfg</td><td>R/W</td><td>0x4</td><td>Tx voltage boost level</td></tr><tr><td>14:10</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE3发送端阻抗在50欧左右的微调值</td></tr><tr><td>9:5</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE2发送端阻抗在50欧左右的微调值</td></tr><tr><td>4:0</td><td>pcie_phy_cfg</td><td>R/W</td><td>0</td><td>设置PCIELANE1发送端阻抗在50欧左右的微调值</td></tr></table>

# 5.24PCIE1PHY配置控制寄存器

通过这些寄存器配置PCIE1PHY的内部寄存器

地址：0x1fe005b0

表5-25PCIE1PHY配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:39</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>38</td><td>phy_cfg_reset</td><td>R/W</td><td>0</td><td>PHY配置重置，高有效</td></tr><tr><td>37:35</td><td>phy_cfg_state</td><td>R</td><td>0</td><td>PHY配置状态机状态指示</td></tr><tr><td>34</td><td>phy_cfg_done</td><td>R/W</td><td>0</td><td>PHY一次访问完成，指示此次对PHY的读写完成。写完成表示写的数据已经写入PHY内部寄存器，读完成表示读的数据已经存储到write/read data寄存器</td></tr><tr><td>33</td><td>phy_cfg_disable</td><td>R/W</td><td>0x0</td><td>0--对该组寄存器读写会触发PHY配置操作
1--对该组寄存器读写不触发PHY配置操作，反为简单的寄存器读写</td></tr><tr><td>32</td><td>phy_cfg_rw</td><td>R/W</td><td>0</td><td>开始读操作或写操作。为1时为写操作，为0时为读操作。</td></tr><tr><td>31:16</td><td>phy_cfg_data</td><td>R/W</td><td>0</td><td>PHY配置读写数据。在写操作时，将数据先写入该寄存，然后再执行写操作；在读操作时，从PHY返回的读数据存储到该寄存器。</td></tr><tr><td>15:0</td><td>phycfg_addr</td><td>R/W</td><td>0</td><td>PHY配置地址</td></tr></table>

# 5.25DMA命令控制寄存器(dma_order)

该寄存器用来控制DMA控制器。2K1000中包含5个DMA控制器，它们的结构相同，所以控制寄存器的内容也一致，所以这里仅列出一个。

DMA0地址：0x1fe00c00DMA1地址：0x1fe00c10DMA2地址：0x1fe00c20DMA3地址：0x1fe00c30DMA4地址：0x1fe00c40

表5-26DMA命令控制寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:5</td><td>ask_addr</td><td>R/W</td><td>0</td><td>64位地址高 59</td></tr><tr><td>5</td><td>-</td><td>-</td><td>0</td><td>保留</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>4</td><td>dma_stop</td><td>R/W</td><td>0</td><td>停止 DMA 操作。DMA 控制器完成当前数据读写后停止。</td></tr><tr><td>3</td><td>dma_start</td><td>R/W</td><td>0</td><td>开始 DMA 操作。DMA 控制器读取描述符地址(ask_addr)后将该位清零。</td></tr><tr><td>2</td><td>ask_valid</td><td>R/W</td><td>0</td><td>DMA 工作寄存器写回到ask_addr所指向的内存，完成后清零。</td></tr><tr><td>1</td><td>axi_coherent</td><td>R/W</td><td>0</td><td>DMA 访问地址一致性使能</td></tr><tr><td>0</td><td>dma_64bit</td><td>R/W</td><td>0</td><td>DMA 控制器 64 位地址支持</td></tr></table>

# 5.26PCICFG2_RECFG寄存器

PCICFG2_RECFG控制器用于使能APB总线控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03800

表5-27PCICFG2_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg2_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte63-56 的重配置使 能</td></tr><tr><td>55:48</td><td>Pcicfg2_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte55-48 的重配置使 能</td></tr><tr><td>47:40</td><td>Pcicfg2_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte47-40 的重配置使 能</td></tr><tr><td>39:32</td><td>Pcicfg2_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte39-32 的重配置使 能</td></tr><tr><td>31:24</td><td>Pcicfg2_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte31-24 的重配置使 能</td></tr><tr><td>23:16</td><td>Pcicfg2_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte23-16 的重配置使 能</td></tr><tr><td>15:08</td><td>Pcicfg2_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte15-08 的重配置使 能</td></tr><tr><td>07:00</td><td>Pcicfg2_reconfig[07:00]</td><td>RW</td><td>0xff</td><td>APB 设备配置头空间 Byte07-00 的重配置使 能</td></tr></table>

# 5.27PCICFG30_RECFG寄存器

PCICFG30_RECFG控制器用于使能GMAC0控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03808

# 表5-28PCICFG30_RECFG配置寄存器

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg30_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg30_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg30_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg30_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg30_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg30_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg30_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>GMAC0 设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg30_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>GMAC0 设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.28PCICFG31_RECFG寄存器

PCICFG31_RECFG控制器用于使能GMAC1控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03810

表5-29PCICFG31_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg31_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg31_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg31_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg31_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg31_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg31_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg31_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>GMAC1 设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg31_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>GMAC1 设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.29PCICFG40_RECFG寄存器

PCICFG40_RECFG控制器用于使能USB- OTG控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03818

表5-30PCICFG40_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg40_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg40_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg40_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg40_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg40_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg40_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg40_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>USB-OTG 设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg40_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>USB-OTG 设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.30PCICFG41_RECFG寄存器

PCICFG41_RECFG控制器用于使能USB- EHCI控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03820

表5-31PCICFG41_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg41_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>USB-EHCI 设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg41_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>USB-EHCI 设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg41_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>USB-EHCI 设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg41_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>USB-EHCI 设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg41_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>USB-EHCI 设备配置头空间 Byte31-24 的重配置使能</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>23:16</td><td>Pcicfg41_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>USB-EHCI设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg41_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>USB-EHCI设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg41_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>USB-EHCI设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.31PCICFG42_RECFG寄存器

PCICFG42_RECFG控制器用于使能USB- OHCI控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03828

表5-32PCICFG42_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg42_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg42_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg42_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg42_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg42_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg42_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg42_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>USB-OHCI设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg42_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>USB-OHCI设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.32PCICFG5_RECFG寄存器

PCICFG5_RECFG控制器用于使能GPU控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03830

表5-33PCICFG5_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg5_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte63-56 的重配置使能</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>55:48</td><td>Pcicfg5_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte55-48 的重配置使 能</td></tr><tr><td>47:40</td><td>Pcicfg5_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte47-40 的重配置使 能</td></tr><tr><td>39:32</td><td>Pcicfg5_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte39-32 的重配置使 能</td></tr><tr><td>31:24</td><td>Pcicfg5_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte31-24 的重配置使 能</td></tr><tr><td>23:16</td><td>Pcicfg5_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte23-16 的重配置使 能</td></tr><tr><td>15:08</td><td>Pcicfg5_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>GPU 设备配置头空间 Byte15-08 的重配置使 能</td></tr><tr><td>07:00</td><td>Pcicfg5_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>GPU 设备配置头空间 Byte07-00 的重配置使 能</td></tr></table>

# 5.33PCICFG6_RECFG寄存器

PCICFG6_RECFG控制器用于使能DC控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03838

表5-34PCICFG6_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg6_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg6_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg6_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg6_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg6_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg6_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg6_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>DC 设备配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg6_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>DC 设备配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.34PCICFG7_RECFG寄存器

PCICFG7_RECFG控制器用于使能HDA控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03840

表5-35PCICFG7_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg7_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte63-56 的重配置使 能</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>55:48</td><td>Pcicfg7_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte55-48 的重配置使 能</td></tr><tr><td>47:40</td><td>Pcicfg7_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte47-40 的重配置使 能</td></tr><tr><td>39:32</td><td>Pcicfg7_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte39-32 的重配置使 能</td></tr><tr><td>31:24</td><td>Pcicfg7_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte31-24 的重配置使 能</td></tr><tr><td>23:16</td><td>Pcicfg7_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte23-16 的重配置使 能</td></tr><tr><td>15:08</td><td>Pcicfg7_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>HDA 设备配置头空间 Byte15-08 的重配置使 能</td></tr><tr><td>07:00</td><td>Pcicfg7_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>HDA 设备配置头空间 Byte07-00 的重配置使 能</td></tr></table>

# 5.35PCICFG8_RECFG寄存器

PCICFG8_RECFG控制器用于使能SATA控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03848

表5-36PCICFG8_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg8_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte63-56 的重配置使 能</td></tr><tr><td>55:48</td><td>Pcicfg8_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte55-48 的重配置使 能</td></tr><tr><td>47:40</td><td>Pcicfg8_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte47-40 的重配置使 能</td></tr><tr><td>39:32</td><td>Pcicfg8_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte39-32 的重配置使 能</td></tr><tr><td>31:24</td><td>Pcicfg8_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte31-24 的重配置使 能</td></tr><tr><td>23:16</td><td>Pcicfg8_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte23-16 的重配置使 能</td></tr><tr><td>15:08</td><td>Pcicfg8_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>SATA 设备配置头空间 Byte15-08 的重配置使 能</td></tr><tr><td>07:00</td><td>Pcicfg8_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>SATA 设备配置头空间 Byte07-00 的重配置使 能</td></tr></table>

# 5.36PCICFGf_RECFG寄存器

PCICFGf_RECFG控制器用于使能DMA控制器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03850

表5-37PCICFGf_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfgf_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfgf_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfgf_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfgf_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfgf_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfgf_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfgf_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>DMA控制器配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfgf_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>DMA控制器配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.37PCICFG10_RECFG寄存器

PCICFG10_RECFG控制器用于使能VPU解码器的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03858

表5-38PCICFG10_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg10_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>VPU解码器配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg10_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>VPU解码器配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg10_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>VPU解码器配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg10_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>VPU解码器配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg10_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>VPU解码器配置头空间 Byte31-24 的重配置使能</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>23:16</td><td>Pcicfg10_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>VPU 解码器配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg10_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>VPU 解码器配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg10_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>VPU 解码器配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 5.38PCICFG11_RECFG寄存器

PCICFG11_RECFG控制器用于使能CAMERA模块的配置头空间的重配置功能，表示对应的字节是否可写。该寄存器主要用于调试。

地址：0x1fe03860

表5-39PCICFG11_RECFG配置寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:56</td><td>Pcicfg11_reconfig[63:56]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte63-56 的重配置使能</td></tr><tr><td>55:48</td><td>Pcicfg11_reconfig[55:48]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte55-48 的重配置使能</td></tr><tr><td>47:40</td><td>Pcicfg11_reconfig[47:40]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte47-40 的重配置使能</td></tr><tr><td>39:32</td><td>Pcicfg11_reconfig[39:32]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte39-32 的重配置使能</td></tr><tr><td>31:24</td><td>Pcicfg11_reconfig[31:24]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte31-24 的重配置使能</td></tr><tr><td>23:16</td><td>Pcicfg11_reconfig[23:16]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte23-16 的重配置使能</td></tr><tr><td>15:08</td><td>Pcicfg11_reconfig[15:08]</td><td>RW</td><td>0xff</td><td>CAMERA 模块配置头空间 Byte15-08 的重配置使能</td></tr><tr><td>07:00</td><td>Pcicfg11_reconfig[07:00]</td><td>RW</td><td>0xf0</td><td>CAMERA 模块配置头空间 Byte07-00 的重配置使能</td></tr></table>

# 6地址空间分配

6 地址空间分配龙芯2K1000的路由主要通过系统的两级交叉开关以及IO子网络组成。一级交叉开关采用固定路由；二级交叉开关可以对每个Master端口接收到的请求进行路由配置；IO子网络采用PCI总线的拓扑结构，通过对软件扫描设备并配置寄存器基地址的方式来访问，因此每个设备都有一个标准的PCI配置头。

龙芯2K1000的地址空间与PCI定义的地址空间形式相同，主要分为三类：

1. 配置空间：该地址空间用于访问各个IO设备的PCI配置头，其地址组成符合PCI配置访问的地址组织形式；2. IO空间：该地址空间用于访问PCI协议定义的IO地址空间。在2K1000中只有PCIE有这段地址空间，用于通过IO类型的请求访问PCIE控制器的下游设备。3. MEM空间：除了以上两种地址空间之外的所有地址空间为MEM空间。

全芯片的地址空间划分如下表所示：

表6-1芯片地址空间划分  

<table><tr><td>NAME</td><td>BASE</td><td>MASK</td><td>SIZE</td><td>备注</td></tr><tr><td>Memory</td><td>64&#x27;h0000_0000</td><td>64&#x27;hFFFF_FFFF_F000_0000</td><td>256M</td><td>内存空间</td></tr><tr><td>Memory mapped IO</td><td>64&#x27;h1000_0000</td><td>64&#x27;hFFFF_FFFF_F800_0000</td><td>128M</td><td>用于映射内部所使用的设备空间(BAR)</td></tr><tr><td>PCIE IO</td><td>64&#x27;h1800_0000</td><td>64&#x27;hFFFF_FFFF_FE00_0000</td><td>32M</td><td>用于映射 PCIE 控制器对外的 IO 空间</td></tr><tr><td>Type0</td><td>64&#x27;h1A00_0000</td><td>64&#x27;hFFFF_FFFF_F000_0000</td><td>16M</td><td>Type0 配置空间</td></tr><tr><td>Type1</td><td>64&#x27;h1B00_0000</td><td>64&#x27;hFFFF_FFFF_F000_0000</td><td>16M</td><td>Type1 配置空间</td></tr><tr><td>BOOT</td><td>64&#x27;h1C00_0000</td><td>64&#x27;hFFFF_FFFF_FFC0_0000</td><td>1M</td><td>启动空间，可映射至 SPI 和 LIO 上</td></tr><tr><td>LIO Memory</td><td>64&#x27;h1C00_0000</td><td>64&#x27;hFFFF_FFFF_F000_0000</td><td>55M</td><td>除去 BOOT 空间可配置之外，其它的部分映射至 LIO</td></tr><tr><td>Flash</td><td>64&#x27;h1FC0_0000</td><td>64&#x27;hFFFF_FFFF_FFF0_0000</td><td>1M</td><td>可映射至 SPI、NAND、SDIO 和 LIO 上</td></tr><tr><td>CONFIG</td><td>64&#x27;h1FE0_0000</td><td>64&#x27;hFFFF_FFFF_FFF0_0000</td><td>1M</td><td>芯片配置寄存器空间</td></tr><tr><td>SPI</td><td>64&#x27;h1FFF_0000</td><td>64&#x27;hFFFF_FFFF_FFFF_0000</td><td>64K</td><td>SPI 配置空间</td></tr><tr><td>Memory mapped IO</td><td>64&#x27;h4000_0000</td><td>64&#x27;hFFFF_FFFF_C000_0000</td><td>1G</td><td>PCIE MEM 空间</td></tr><tr><td>Memory</td><td>64&#x27;h8000_0000</td><td>64&#x27;hFFFF_FFFF_8000_0000</td><td>2G</td><td>内存空间</td></tr><tr><td>Memory</td><td>64&#x27;h1_0000_0000</td><td>64&#x27;hFFFF_FFFF_0000_0000</td><td>4G</td><td>内存空间</td></tr><tr><td>Memory</td><td>64&#x27;h2_0000_0000</td><td>64&#x27;hFFFF_FFFE_0000_0000</td><td>8G</td><td>内存空间</td></tr><tr><td>Memory mapped IO</td><td>64&#x27;h40_0000_0000</td><td>64&#x27;hFFFF_FFC0_0000_0000</td><td>256G</td><td>PCIE MEM 空间</td></tr><tr><td>Type0</td><td>64&#x27;hFE_0000_0000</td><td>64&#x27;hFFFF_FFFF_FFF00_0000</td><td>256M</td><td>Type0 配置空间</td></tr><tr><td>Type1</td><td>64&#x27;hFE_1000_0000</td><td>64&#x27;hFFFF_FFFF_FFF00_0000</td><td>256M</td><td>Type1 配置空间</td></tr></table>

<table><tr><td>Type0</td><td>64&#x27;hFE_2000_0000</td><td>64&#x27;hFFFF_FFFF_FF00_0000</td><td>256M</td><td>Type0 配置空间</td></tr><tr><td>Type1</td><td>64&#x27;hFE_3000_0000</td><td>64&#x27;hFFFF_FFFF_FF00_0000</td><td>256M</td><td>Type1 配置空间</td></tr></table>

# 6.1 一级交叉开关

一级交叉开关一共有3个主端口，分别连接CORE0、CORE1和IODMA。

一级开关的路由不可配置，具体的地址空间分配如下表所示。其中包含了32位和64位地址空间访问模式。

表6-2一级交叉开关路由规则  

<table><tr><td>地址空间</td><td>大小</td><td>设备/路由目的空间</td><td>备注</td></tr><tr><td>0x1000_0000 - 0x17FF_FFFF</td><td>128M</td><td>I/O 设备的配置寄存器空间</td><td>需注意该段地址空间为 MEM 类型。各个 IO 设备的 BAR 地址在该地址空间。</td></tr><tr><td>0x1800_0000 - 0x19FF_FFFF</td><td>32M</td><td>PCIE 的 I/O 空间</td><td>32 位模式</td></tr><tr><td>0x1A00_0000 - 0x1BFF_FFFF</td><td>32M</td><td>各个 I/O 设备的配置头空间</td><td>32 位模式</td></tr><tr><td>0x1C00_0000 - 0x1C0F_FFFF</td><td>1M</td><td>启动空间</td><td>BOOT</td></tr><tr><td>0x1FE0_0000 - 0x1FEF_FFFF</td><td>1M</td><td>Confbus</td><td>芯片配置空间</td></tr><tr><td>0x4000_0000 - 0x7FFF_FFFF</td><td>1G</td><td>32 位访问模式下 I/O 设备的 MEM 空间</td><td>32 位模式</td></tr><tr><td>0x40_0000_0000 - 0x4F_FFFF_FFFF</td><td>256G</td><td>64 位访问模式下 I/O 设备的 MEM 空间</td><td>64 位模式</td></tr><tr><td>0xFE_0000_0000 - 0xFE_FFFF_FFFF</td><td>4G</td><td>各个 I/O 设备的配置头空间</td><td>64 位模式</td></tr><tr><td>0xFD_FC00_0000 - 0xFD_FDDF_FFFF</td><td>32M</td><td>PCIE 的 I/O 空间</td><td>64 位模式</td></tr><tr><td>其他地址或上述空间若为 CACHED</td><td></td><td>Bit6 为 0，路由到 scache0； Bit6 为 1，路由到 scache1</td><td>由地址的第 6 位决定路由目的</td></tr></table>

# 6.2 二级交叉开关

龙芯2K1000的二级交叉开关连接两个二级Cache、内存控制器、启动模块（SPI或者LIO）以及IO子网络（Uncache访问路径），如下图所示：

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/d2b63eba5fe8b57bd17925466d39b2c2b77d5ba17f89eeeba9df15d43a3c2383.jpg)  
图6-1二级交叉开关地址路由示意图

地址窗口是供SCACHE0、SCACHE1和IODMA三个具有主功能的IP进行路由选择和地址转换而设置的。CPU和IODMA的访问都拥有8个地址窗口，可以完成目标地址空间的选择以及从源地址空间到目标地址空间的转换。每个地址窗口由BASE、MASK和MMAP三个64位寄存器组成，BASE以K字节对齐，MASK采用类似网络掩码高位为1的格式，MMAP的低三位是路由选择。在二级交叉开关处，标号与所述模块的对应关系如下表所示。

在二级交叉开关处，标号与所述模块的对应关系如下表所示。

表6-3二级交叉开关处标号与所述模块的对应关系  

<table><tr><td>标号</td><td>缺省值</td></tr><tr><td>0</td><td>DDR3 内存控制器</td></tr><tr><td>1</td><td>保留</td></tr><tr><td>2</td><td>启动模块（SPI 或 LIO）</td></tr><tr><td>3</td><td>保留</td></tr></table>

MMAP[4]表示允许取指，MMAP[5]表示允许块读，MMAP[7]表示窗口使能。

表6-4MMAP字段对应的该空间访问属性  

<table><tr><td>[4]</td><td>[5]</td><td>[7]</td></tr><tr><td>允许取指</td><td>允许块读</td><td>窗口使能</td></tr></table>

二级交叉开关的地址配置具有地址转换的功能。不需要维护CACHE一致性。

窗口命中公式：  $(\mathrm{IN\_ADDR\ & MASK}) = =\mathrm{BASE}$  新地址换算公式：OUT_ADDR  $=$  (IN_ADDR&\~MASK){MMAP[63:10],10'h0}地址窗口转换寄存器如下表。

表6-5二级交叉开关地址窗口转换寄存器表  

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td>0x1fe02000</td><td>CPU_WIN0_BASE</td><td>RW</td><td>cache 通路二级窗口 0 的 基地址</td><td>0x1FC0_0000</td></tr><tr><td>0x1fe02008</td><td>CPU_WIN1_BASE</td><td>RW</td><td>cache 通路二级窗口 1 的 基地址</td><td>0x1000_0000</td></tr><tr><td>0x1fe02010</td><td>CPU_WIN2_BASE</td><td>RW</td><td>cache 通路二级窗口 2 的 基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02018</td><td>CPU_WIN3_BASE</td><td>RW</td><td>cache 通路二级窗口 3 的 基地址</td><td>0x1_0000_0000</td></tr><tr><td>0x1fe02020</td><td>CPU_WIN4_BASE</td><td>RW</td><td>cache 通路二级窗口 4 的 基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02028</td><td>CPU_WIN5_BASE</td><td>RW</td><td>cache 通路二级窗口 5 的 基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02030</td><td>CPU_WIN6_BASE</td><td>RW</td><td>cache 通路二级窗口 6 的 基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02038</td><td>CPU_WIN7_BASE</td><td>RW</td><td>cache 通路二级窗口 7 的 基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02040</td><td>CPU_WIN0_MASK</td><td>RW</td><td>cache 通路二级窗口 0 的 掩码</td><td>0xFFFF_FFFF_FFF0_0000</td></tr><tr><td>0x1fe02048</td><td>CPU_WIN1_MASK</td><td>RW</td><td>cache 通路二级窗口 1 的 掩码</td><td>0xFFFF_FFFF_F000_0000</td></tr><tr><td>0x1fe02050</td><td>CPU_WIN2_MASK</td><td>RW</td><td>cache 通路二级窗口 2 的 掩码</td><td>0xFFFF_FFFF_F000_0000</td></tr></table>

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td>0x1fe02058</td><td>CPU_WIN3_MASK</td><td>RW</td><td>cache 通路二级窗口 3 的掩码</td><td>0xFFFF_FFFF_0000_0000</td></tr><tr><td>0x1fe02060</td><td>CPU_WIN4_MASK</td><td>RW</td><td>cache 通路二级窗口 4 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02068</td><td>CPU_WIN5_MASK</td><td>RW</td><td>cache 通路二级窗口 5 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02070</td><td>CPU_WIN6_MASK</td><td>RW</td><td>cache 通路二级窗口 6 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02078</td><td>CPU_WIN7_MASK</td><td>RW</td><td>cache 通路二级窗口 7 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02080</td><td>CPU_WIN0_MMAP</td><td>RW</td><td>cache 通路二级窗口 0 的新基地址</td><td>0x1FC0_00F2</td></tr><tr><td>0x1fe02088</td><td>CPU_WIN1_MMAP</td><td>RW</td><td>cache 通路二级窗口 1 的新基地址</td><td>0x1000_0082</td></tr><tr><td>0x1fe02090</td><td>CPU_WIN2_MMAP</td><td>RW</td><td>cache 通路二级窗口 2 的新基地址</td><td>0x0000_00F0</td></tr><tr><td>0x1fe02098</td><td>CPU_WIN3_MMAP</td><td>RW</td><td>cache 通路二级窗口 3 的新基地址</td><td>0x0000_00F0</td></tr><tr><td>0x1fe020a0</td><td>CPU_WIN4_MMAP</td><td>RW</td><td>cache 通路二级窗口 4 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe020a8</td><td>CPU_WIN5_MMAP</td><td>RW</td><td>cache 通路二级窗口 5 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe020b0</td><td>CPU_WIN6_MMAP</td><td>RW</td><td>cache 通路二级窗口 6 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe020b8</td><td>CPU_WIN7_MMAP</td><td>RW</td><td>cache 通路二级窗口 7 的新基地址</td><td>0x0000_0000</td></tr><tr><td></td><td></td><td></td><td></td><td></td></tr><tr><td>0x1fe02100</td><td>PCI_WIN0_BASE</td><td>RW</td><td>uncache 通路二级窗口 0 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02108</td><td>PCI_WIN1_BASE</td><td>RW</td><td>uncache 通路二级窗口 1 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02110</td><td>PCI_WIN2_BASE</td><td>RW</td><td>uncache 通路二级窗口 2 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02118</td><td>PCI_WIN3_BASE</td><td>RW</td><td>uncache 通路二级窗口 3 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02120</td><td>PCI_WIN4_BASE</td><td>RW</td><td>uncache 通路二级窗口 4 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02128</td><td>PCI_WIN5_BASE</td><td>RW</td><td>uncache 通路二级窗口 5 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02130</td><td>PCI_WIN6_BASE</td><td>RW</td><td>uncache 通路二级窗口 6 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02138</td><td>PCI_WIN7_BASE</td><td>RW</td><td>uncache 通路二级窗口 7 的基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02140</td><td>PCI_WIN0_MASK</td><td>RW</td><td>uncache 通路二级窗口 0 的掩码</td><td>0xFFFF_FFFF_F000_0000</td></tr><tr><td>0x1fe02148</td><td>PCI_WIN1_MASK</td><td>RW</td><td>uncache 通路二级窗口 1 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02150</td><td>PCI_WIN2_MASK</td><td>RW</td><td>uncache 通路二级窗口 2 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02158</td><td>PCI_WIN3_MASK</td><td>RW</td><td>uncache 通路二级窗口 3 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02160</td><td>PCI_WIN4_MASK</td><td>RW</td><td>uncache 通路二级窗口 4 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02168</td><td>PCI_WIN5_MASK</td><td>RW</td><td>uncache 通路二级窗口 5 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02170</td><td>PCI_WIN6_MASK</td><td>RW</td><td>uncache 通路二级窗口 6</td><td>0x0000_0000</td></tr></table>

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td></td><td></td><td></td><td>的掩码</td><td></td></tr><tr><td>0x1fe02178</td><td>PCI_WIN7_MASK</td><td>RW</td><td>uncache 通路二级窗口 7 的掩码</td><td>0x0000_0000</td></tr><tr><td>0x1fe02180</td><td>PCI_WIN0_MMAP</td><td>RW</td><td>uncache 通路二级窗口 0 的新基地址</td><td>0x0000_00F0</td></tr><tr><td>0x1fe02188</td><td>PCI_WIN1_MMAP</td><td>RW</td><td>uncache 通路二级窗口 1 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02190</td><td>PCI_WIN2_MMAP</td><td>RW</td><td>uncache 通路二级窗口 2 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe02198</td><td>PCI_WIN3_MMAP</td><td>RW</td><td>uncache 通路二级窗口 3 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe021a0</td><td>PCI_WIN4_MMAP</td><td>RW</td><td>uncache 通路二级窗口 4 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe021a8</td><td>PCI_WIN5_MMAP</td><td>RW</td><td>uncache 通路二级窗口 5 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe021b0</td><td>PCI_WIN6_MMAP</td><td>RW</td><td>uncache 通路二级窗口 6 的新基地址</td><td>0x0000_0000</td></tr><tr><td>0x1fe021b8</td><td>PCI_WIN7_MMAP</td><td>RW</td><td>uncache 通路二级窗口 7 的新基地址</td><td>0x0000_0000</td></tr></table>

根据缺省的寄存器配置，芯片启动后，CPU的0x1C00_0000- 0x1C0F_FFFF的地址区间（1M）映射到BOOT设备（SPI或LIO）存储空间的0x00000000- 0x000F_FFFF区间，CPU的0x10000000- 0x1FFFFFFF区间（256M）映射到BOOT设备的0x10000000- 0x1FFFFFFF区间，CPU的0x00000000- 0x0FFFFFFF的地址区间（256M）映射到DDR3的0x00000000- 0x0FFFFFFF的地址区间，CPU的0x100000000- 0x1FFFFFFF的地址区间（4G）映射到DDR3的0x00000000- 0x0FFFFFFF的地址区间。软件可以通过修改相应的配置寄存器实现新的地址空间路由和转换。

此外，当出现8个地址窗口都不命中时，由内存控制器模块返回数据。

# 6.3 IO互连网络

2K1000中的IO互连网络结构图如图6- 2所示。为了便于说明，这里把IO设备分成PCIE设备（包括PCIE0与PCIE1）和其他IO设备（除PCIE之外的所有其他设备）两部分。

IO互连网络主要由以下两部分通路组成：

1. CPU访问IO设备的通路，其地址路由形式主要包括：

- 配置访问：CPU通过配置请求访问IO设备的配置头，其地址组织形式在6.3.1节有介绍；- IO访问：IO访问用于访问PCIE控制器下游设备的IO地址空间，根据PCIE配置头的IO Base和IO Limit配置决定地址路由方式；- 内部寄存器访问：通过各个IO设备配置头的BAR地址访问IO设备的内部配置寄存器，MEM访问类型。- PCIE下游设备MEM访问：根据PCIE配置头的Memory Base和Memory Limit、Prefetchable Memory Base和Prefetchable Memory Limit进行地址路由，用于访问

PCIE下游设备的MEM空间。

2. IO DMA访问内存的通路，包括CACHE访问和UNCACHE访问。该部分地址路由方式在6.4节介绍。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/1bbd83bfc60b483847db38adc60f624c67a842e5d7b4aa1ea1d5570677707f2f.jpg)  
图6-2IO互连结构图

# 6.3.1 IO设备的配置空间

这里需要先区分配置头空间和设备寄存器空间两个概念。配置头空间指的是每个IO设备的配置头所在的地址空间，而设备寄存器空间指的是配置头中的BAR所指定的设备配置寄存器的地址空间。访问IO设备的设备寄存器空间需要先读取其配置头空间中的BAR地址作为其配置空间的起始地址，然后再对相应设备寄存器进行读写。

首先介绍64位模式下访问配置头空间的地址格式，龙芯2K1000中定义地址段0xFE_0000_0000－0xFE_1FFF_FFFF是CPU的64位配置地址空间，CPU通过这段地址访问各个设备的PCI配置头。由地址的[63:28]决定配置头类型（0xFE0是Type0，0xFE1是Type1，其他保留）；[23:16]在Type1类型配置头访问时表示总线号（BusNumber），Type0时保留；[15:11]表示设备号（DeviceNumber）；[10:8]表示功能号（FunctionNumber）；[27:24]和[7:0]组合起来表示偏移（offset），大小为4K。下图是CPU访问PCI配置头空间的64位地址格式。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/3eb7791a527cc34d380bcadac98860a1f717ba57f692d8a372a9faec0267d42c.jpg)  
图6-364位配置访问地址格式

为了保证系统的兼容性，还提供了32位地址模式下的配置地址格式，如下图所示。该模式下，地址偏移只有8位（即256B）。其中，地址的[31:24]决定配置头类型（0x1A是Type0，0x1B是Type1）；[23:16]在Type1类型配置头访问时表示总线号（BusNumber），

Type0 时保留；[15:11]表示设备号(Device Number)；[10:8]表示功能号(Function Number)；[7:0]表示偏移（offset），大小为256B。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/fea427f071fd43eeede59c6593d6f929a8ce2819b42d1167af566928f5204365.jpg)  
图6-432位配置访问地址格式

龙芯2K1000芯片中，具备配置头空间的设备包括：APB总线控制器、GMAC0/1控制器、USB控制器、GPU控制器、DC控制器、HDA控制器、SATA控制器、PCIE0/1控制器和DMA控制器。其中所有APB设备都作为一个整体挂在APB总线控制器上，由地址的[15:12]位进行译码，下文中将会做详细说明。各个设备的设备号、功能号以及配置头访问首地址如下表所示（该表只包括各个控制器本身的配置空间，不关注PCIE控制器的Type1访问等）。除了PCIE控制器之外，其他设备的配置头空间大小都只有256B。

表6-6各个设备的配置头访问对应关系  

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>APB</td><td>0x0</td><td>0x2</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_1000</td><td>-</td></tr><tr><td>GMAC0</td><td>0x0</td><td>0x3</td><td>0x0</td><td>0x7fff</td><td>0xFE_0000_1800</td><td>GMAC设备有2个功能，Func0是GMAC0</td></tr><tr><td>GMAC1</td><td>0x0</td><td>0x3</td><td>0x1</td><td>0x7fff</td><td>0xFE_0000_1900</td><td>GMAC设备有2个功能，Func1是GMAC1</td></tr><tr><td>USB-OTG</td><td>0x0</td><td>0x4</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_2000</td><td>USB设备共3个功能：Func0是OTG</td></tr><tr><td>USB-EHCI</td><td>0x0</td><td>0x4</td><td>0x1</td><td>0x7fff</td><td>0xFE_0000_2100</td><td>USB设备共3个功能：Func1是EHCI</td></tr><tr><td>USB-OHCI</td><td>0x0</td><td>0x4</td><td>0x2</td><td>0x7fff</td><td>0xFE_0000_2200</td><td>USB设备共3个功能：Func2是OHCI</td></tr><tr><td>GPU</td><td>0x0</td><td>0x5</td><td>0x0</td><td>0x3ffff</td><td>0xFE_0000_2800</td><td>-</td></tr><tr><td>DC</td><td>0x0</td><td>0x6</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_3000</td><td>-</td></tr><tr><td>HDA</td><td>0x0</td><td>0x7</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_3800</td><td>-</td></tr><tr><td>SATA</td><td>0x0</td><td>0x8</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_4000</td><td>-</td></tr><tr><td>PCIE0-Port0</td><td>0x0</td><td>0x9</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_4800</td><td rowspan="4">Func1和Type1的访问没有在此表中说明。当PCIE0控制器为X4模式时，只能访问设备号为0x9的控制器。</td></tr><tr><td>PCIE0-Port1</td><td>0x0</td><td>0xa</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_5000</td></tr><tr><td>PCIE0-Port2</td><td>0x0</td><td>0xb</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_5800</td></tr><tr><td>PCIE0-Port3</td><td>0x0</td><td>0xc</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_6000</td></tr><tr><td>PCIE1-Port0</td><td>0x0</td><td>0xd</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_6800</td><td rowspan="2">Func1和Type1的访问没有在此表中说明。当PCIE1控制器为X4模式时，只能访问设备号为0xd的控制器。</td></tr><tr><td>PCIE1-Port1</td><td>0x0</td><td>0xe</td><td>0x0</td><td>0xffff</td><td>0xFE_0000_7000</td></tr><tr><td>DMA</td><td>0x0</td><td>0xf</td><td>0x0</td><td>0xff</td><td>0xFE_0000_7800</td><td>-</td></tr></table>

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>N/A</td><td colspan="4">无效的配置头访问</td><td></td><td>Vendor ID为16hffff，表示没有有效设备</td></tr></table>

除PCIE外，所有的配置头空间都遵循PCI Type0类型的格式，如下表所示，但是每个设备的缺省值不尽相同，后文中将展开介绍。

表6-7Type0类型配置头  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">Device ID</td><td colspan="3">Vendor ID</td><td>00h</td><td></td></tr><tr><td colspan="4">Status</td><td colspan="3">Command</td><td>04h</td><td></td></tr><tr><td colspan="5">Class Code</td><td colspan="2">Revision ID</td><td>08h</td><td></td></tr><tr><td>BIST</td><td colspan="2">Header Type</td><td colspan="2">Lantency Timer</td><td colspan="2">Cache Line Size</td><td>0Ch</td><td></td></tr><tr><td colspan="7">Base Address 0</td><td>10h</td><td></td></tr><tr><td colspan="7">Base Address 1</td><td>14h</td><td></td></tr><tr><td colspan="7">Base Address 2</td><td>18h</td><td></td></tr><tr><td colspan="7">Base Address 3</td><td>1Ch</td><td></td></tr><tr><td colspan="7">Base Address 4</td><td>20h</td><td></td></tr><tr><td colspan="7">Base Address 5</td><td>24h</td><td></td></tr><tr><td colspan="7">CardBus CIS Pointer</td><td>28h</td><td></td></tr><tr><td colspan="3">Subsystem ID</td><td colspan="4">Subsystem Vendor ID</td><td>2Ch</td><td></td></tr><tr><td colspan="7">Expansion ROM Base Address</td><td>30h</td><td></td></tr><tr><td colspan="5">Reserved</td><td colspan="2">Capabilities Pointer</td><td>34h</td><td></td></tr><tr><td colspan="7">Reserved</td><td>38h</td><td></td></tr><tr><td>Max_Lat</td><td colspan="2">Min_Gnt</td><td colspan="2">Interrupt Pin</td><td colspan="2">Interrupt Line</td><td>3Ch</td><td></td></tr></table>

表6-8Type0的配置头寄存器  

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(除PCIE控制器)</td></tr><tr><td>Vendor ID
制造商 ID 号</td><td>0x0</td><td>RO</td><td>制造商 ID 号</td><td>缺省值都为 0x0014</td></tr><tr><td>Device ID
设备 ID 号</td><td>0x2</td><td>RO</td><td>设备 ID 号</td><td></td></tr><tr><td>Command
命令寄存器</td><td>0x4</td><td>RW</td><td>Bit0: I/O 访问使能
Bit1: 存储器访问使能</td><td>扫描配置地址后，需要使能相应设备的存</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(除PCIE控制器)</td></tr><tr><td></td><td></td><td></td><td>Bit2: 主设备使能
Bit3: 专用周期监视
Bit4: 存储器写与使失效使
能
Bit5: VGA画板侦测
Bit6: 奇偶校验错响应
Bit7: 等待周期控制
Bit8: SERR#使能
Bit9: 快速背靠背使能
其它: 保留</td><td>储器访问使能</td></tr><tr><td>Status
状态寄存器</td><td>0x6</td><td>RO/RW</td><td>Bit3-0: RO, 保留
Bit4: RO, 能力列表
Bit5: RO, 66MHz 能力
Bit6: 保留
Bit7: RO, 快速背靠背能力
Bit8: RW, 主设备数据奇偶
校验错
Bit10-9: RO, 设备选择定时
Bit11: RW, 发出目标失败
信号
Bit12: RW, 收到目标失败
Bit13: RW, 收到主设备失
败
Bit14: RW, 发出系统错信
号
Bit15: RW, 检测到奇偶错</td><td rowspan="2">缺省值都为0x0</td></tr><tr><td>Revision ID
版本索引</td><td>0x8</td><td>RO</td><td>设备版本号</td></tr><tr><td>Class Code
类代码</td><td>0x9</td><td>RO</td><td>Bit7-0: Prog.I/F 编程接口
Bit15-8: Subclass code, 子
类码
Bit23-16: Class code 类别码</td><td></td></tr><tr><td>Cache Line Size
缓存行大小</td><td>0xC</td><td>RO</td><td>龙芯2K1000的缓存行大小为
64Byte</td><td>缺省值都为0x10</td></tr><tr><td>Latency Timer
等待计时器</td><td>0xD</td><td>RW</td><td></td><td>保留</td></tr><tr><td>Header Type
配置头类型</td><td>0xE</td><td>RO</td><td>除PCIE控制器外, 都是
Type0。
另外bit7表示多功能属性:
1-多功能设备
0-单功能设备</td><td></td></tr><tr><td>BIST
内自建测试</td><td>0xF</td><td>RW</td><td>Bit3-0: 完成代码, 0-成功完
成, 非0-设备指定的错误
Bit5-4: 保留
Bit6: 起动BIST
Bit7: BIST 能力, 1-提供
BIST 功能, 0-不提供</td><td>保留</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(除PCIE控制器)</td></tr><tr><td>Base Address Register 0 0号基址寄存器</td><td>0x10</td><td>RW/RO</td><td>基地址寄存器 0-
Bit0: RO, 1-I/O 基地址寄存器, 0-MEM 基地址寄存器
I/O 基地址寄存器:
Bit1: 保留
Bit31-2: RO, 基地址单元
Bit63-32: 保留
MEM 基地址存储器:
Bit2-1: RO, MEM 基地址寄存器-译码器宽度单元, 00-32 位, 10-64 位
Bit3: RO, 预提取属性
Bit64-4: 基地址单元</td><td>只支持64位模式</td></tr><tr><td>Base Address Register 1 1号基址寄存器</td><td>0x18</td><td>RW</td><td>基址寄存器 1</td><td>只支持64位模式</td></tr><tr><td>Base Address Register 2 2号基址寄存器</td><td>0x20</td><td>RW</td><td>基址寄存器 2</td><td>只支持64位模式</td></tr><tr><td>Cardbus CIS Pointer CIS 卡总线指针</td><td>0x28</td><td>RW</td><td>CIS 卡总线指针</td><td>保留</td></tr><tr><td>Subsystem Vendor ID 子系统版本号</td><td>0x2C</td><td>RO</td><td>子系统供应商 ID 号</td><td>保留</td></tr><tr><td>Subsystem ID 子系统版本号</td><td>0x2D</td><td>RO</td><td>子系统版本 ID 号</td><td>保留</td></tr><tr><td>Expansion ROM Base Address 扩展 ROM 基地址</td><td>0x30</td><td>RW</td><td>扩展 ROM 基地址寄存器
Bit0: 地址译码使能
Bit10-1: 保留
Bit31-11: 用于指定 ROM 的起始地址</td><td>保留</td></tr><tr><td>Capabilities Pointer 扩展能力指针</td><td>0x34</td><td>RW</td><td>扩展指针</td><td>保留</td></tr><tr><td>Reserved</td><td>0x35</td><td></td><td></td><td>保留</td></tr><tr><td>Reserved</td><td>0x38</td><td></td><td></td><td>保留</td></tr><tr><td>Interrupt Line 中断线寄存器</td><td>0x3C</td><td>RW</td><td>中断线寄存器</td><td></td></tr><tr><td>Interrupt Pin 中断引脚寄存器</td><td>0x3D</td><td>RO</td><td>中断引脚寄存器</td><td>保留(除PCIE控制器)</td></tr><tr><td>Min Gnt 最小突发期时间</td><td>0x3E</td><td>RW</td><td></td><td>保留(除PCIE控制器)</td></tr><tr><td>Max Lat 获取PCI总线最大时间</td><td>0x3F</td><td>RW</td><td></td><td>保留(除PCIE控制器)</td></tr></table>

PCIE控制器的配置头为Type1类型，具体如下：

表6-9Type1类型配置头  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">Device ID</td><td colspan="4">Vendor ID</td><td>00h</td></tr><tr><td colspan="4">Status</td><td colspan="4">Command</td><td>04h</td></tr></table>

表6-10Type1的配置头寄存器  

<table><tr><td colspan="3">Class Code</td><td>Revision ID</td><td>08h</td></tr><tr><td>BIST</td><td>Header Type</td><td>Lantency Timer</td><td>Cache Line Size</td><td>0Ch</td></tr><tr><td colspan="4">Base Address 0</td><td>10h</td></tr><tr><td colspan="4">Base Address 1</td><td>14h</td></tr><tr><td>Secondary Lantency Timer</td><td>Subordinate Bus #</td><td>Secondary Bus #</td><td>Primary Bus #</td><td>18h</td></tr><tr><td colspan="2">Secondary Status</td><td>IO Limit</td><td>IO Base</td><td>1Ch</td></tr><tr><td colspan="2">(Non-Prefetchable) Memory Limit</td><td colspan="2">(Non-Prefetchable) Memory Base</td><td>20h</td></tr><tr><td colspan="2">PrefetchableMemory Limit</td><td colspan="2">PrefetchableMemory Base</td><td>24h</td></tr><tr><td colspan="4">Prefetchable Memory Base Upper 32 Bits</td><td>28h</td></tr><tr><td colspan="4">Prefetchable Memory Limit Upper 32 Bits</td><td>2Ch</td></tr><tr><td colspan="2">IO Limit
Upper 16 Bits</td><td colspan="2">IO Base
Upper 16 Bits</td><td>30h</td></tr><tr><td colspan="3">Reserved</td><td>Capabilities Pointer</td><td>34h</td></tr><tr><td colspan="4">Expansion ROM Base Address</td><td>38h</td></tr><tr><td colspan="2">Bridge Control</td><td>Interrupt Pin</td><td>Interrupt Line</td><td>3Ch</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(PCIE 控制器)</td></tr><tr><td>Vendor ID
制造商 ID 号</td><td>0x0</td><td>RO</td><td>制造商 ID 号</td><td>缺省值都为 0x0014</td></tr><tr><td>Device ID
设备 ID 号</td><td>0x2</td><td>RO</td><td>设备 ID 号</td><td></td></tr><tr><td>Command
命令寄存器</td><td>0x4</td><td>RW</td><td>Bit0: I/O 访问使能
Bit1: 存储器访问使能
Bit2: 主设备使能
Bit3: 专用周期监视
Bit4: 存储器写与使失效使能
Bit5: VGA 画板侦测
Bit6: 奇偶校验错响应
Bit7: 等待周期控制
Bit8: SERR#使能
Bit9: 快速背靠背使能
其它: 保留</td><td>扫描配置地址后,需要使能相应设备的存储器访问使能</td></tr><tr><td>Status
状态寄存器</td><td>0x6</td><td>RO/RW</td><td>Bit3-0: RO, 保留
Bit4: RO, 能力列表
Bit5: RO, 66MHz 能力
Bit6: 保留
Bit7: RO, 快速背靠背能力
Bit8: RW, 主设备数据奇偶校验错
Bit10-9: RO, 设备选择定时</td><td></td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(PCIE 控制器)</td></tr><tr><td></td><td></td><td></td><td>Bit11: RW, 发出目标失败信号
Bit12: RW, 收到目标失败
Bit13: RW, 收到主设备失败
Bit14: RW, 发出系统错信号
Bit15: RW, 检测到奇偶错</td><td>缺省值为 0x0</td></tr><tr><td>Revision ID
版本索引</td><td>0x8</td><td>RO</td><td>设备版本号</td><td>缺省值都为 0x0</td></tr><tr><td>Class Code
类代码</td><td>0x9</td><td>RO</td><td>Bit7-0: Prog.I/F 编程接口
Bit15-8: Subclass code, 子类码
Bit23-16: Class code 类别码</td><td>缺省值为 0x0b3000</td></tr><tr><td>Cache Line Size
缓存行大小</td><td>0xC</td><td>RO</td><td>缓存行大小</td><td>缺省值都为 0x00</td></tr><tr><td>Latency Timer
等待计时器</td><td>0xD</td><td>RW</td><td></td><td>保留</td></tr><tr><td>Header Type
配置头类型</td><td>0xE</td><td>RO</td><td>PCIE 控制器都是 Type1</td><td>缺省值为 0x01</td></tr><tr><td>BIST
内自建测试</td><td>0xF</td><td>RW</td><td>Bit3-0: 完成代码, 0-成功完成, 非 0-设备指定的错误
Bit5-4: 保留
Bit6: 起动 BIST
Bit7: BIST 能力, 1-提供
BIST 功能, 0-不提供</td><td>保留</td></tr><tr><td>Base Address Register 0</td><td>0x10</td><td>RW/RO</td><td>基地址寄存器 0-
Bit0: RO, 1-I/O 基地址寄存器, 0-MEM 基地址寄存器
I/O 基地址寄存器:
Bit1: 保留
Bit31-2: RO, 基地址单元
Bit63-32: 保留
MEM 基地址存储器:
Bit2-1: RO, MEM 基地址寄存器-译码器宽度单元, 00-32 位, 10-64 位
Bit3: RO, 预提取属性
Bit64-4: 基地址单元</td><td>64 位 MEM 空间</td></tr><tr><td>Primary Bus #</td><td>0x18</td><td>RW</td><td>Primary Bus 编号</td><td>默认为 0</td></tr><tr><td>Secondary Bus #</td><td>0x19</td><td>RW</td><td>Secondary Bus 编号</td><td>默认为 0</td></tr><tr><td>Subordinate Bus #</td><td>0x1A</td><td>RW</td><td>Subordinate Bus 编号</td><td>默认为 0</td></tr><tr><td>IO Base</td><td>0x1C</td><td>RW/RO</td><td>[3:0] RO, 0h-16bit,1h-32bit
[7:4] RW</td><td>默认为 0x1</td></tr><tr><td>IO Limit</td><td>0X1D</td><td>RW/RO</td><td>[3:0] RO, 0h-16bit,1h-32bit
[7:4] RW</td><td>默认为 0x1</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写</td><td>功能描述</td><td>说明
(PCIIE控制器)</td></tr><tr><td>Secondary Lantency Timer</td><td>0x1E</td><td>RO</td><td>Secondary Bus状态寄存器</td><td>默认为0</td></tr><tr><td>(Non-Prefetchable) Memory Base</td><td>0x20</td><td>RW/R0</td><td>[3:0] RO, must be 0
[15:4] RW</td><td>默认为0x0</td></tr><tr><td>(Non-Prefetchable) Limit</td><td>0x22</td><td>RW/R0</td><td>[3:0] RO, must be 0
[15:4] RW</td><td>默认为0x0</td></tr><tr><td>Prefetchable Memory Base</td><td>0x24</td><td>RW/R0</td><td>[3:0] RO, 0h-32bit, 1h-64bit
[15:4] RW</td><td>默认为0x1</td></tr><tr><td>Prefetchable Memory Limit</td><td>0x26</td><td>RW/R0</td><td>[3:0] RO, 0h-32bit, 1h-64bit
[15:4] RW</td><td>默认为0x1</td></tr><tr><td>Prefetchable Memory Base Upper 32bits</td><td>0x28</td><td>RW</td><td>可预取MEM地址空间高32位</td><td>默认为0</td></tr><tr><td>Prefetchable Memory Limit Upper 32bits</td><td>0x2C</td><td>RW</td><td>可预取MEM地址空间高32位</td><td>默认为0</td></tr><tr><td>IO Base Upper 16 Bits</td><td>0x30</td><td>RW</td><td>IO地址Base高16位</td><td>默认为0</td></tr><tr><td>IO Limit Upper 16 Bits</td><td>0x32</td><td>RW</td><td>IO地址Limit高16位</td><td>默认为0</td></tr><tr><td>Capabilities Pointer扩展能力指针</td><td>0x34</td><td>RW</td><td>扩展指针</td><td>默认为0x40</td></tr><tr><td>Expansion ROM Base Address扩展ROM基地址</td><td>0x38</td><td>RW</td><td>扩展ROM基地址寄存器
Bit0: 地址译码使能
Bit10-1: 保留
Bit31-11: 用于指定ROM的起始地址</td><td>保留</td></tr><tr><td>Interrupt Line中断线寄存器</td><td>0x3C</td><td>RW</td><td>中断线寄存器</td><td>默认为0xFF</td></tr><tr><td>Interrupt Pin中断引脚寄存器</td><td>0x3D</td><td>RO</td><td>中断引脚寄存器</td><td>默认为0x0</td></tr><tr><td>Bridge Control</td><td>0x3E</td><td>RW/RO</td><td>0(RW): Parity Error Response Enable
1(RW): SERR# Enable
2(RW): ISA Enable
3(RW): VGA Enable
4(RW): VGA 16bit Decode
5(RW): Master-Abort Mode
6(RW): Secondary Bus Reset
7(RW): Fast Back-to-Back Enable
8(RO): Primary Discard Timer
9: Secondary Discard Timer
10(RW1C): Discard Timer Status
11(RW): Discard Timer
SERR# Enable
15:12 Reserved</td><td>默认为0</td></tr></table>

# 6.3.2 APB配置头

APB总线控制器下挂载多个APB设备。该总线控制器作为一个设备具有相应的配置头空间。其配置头访问的基址为0xFE_0000_1000。

表6-11APB总线控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="3">16&#x27;h7a02</td><td colspan="4">16&#x27;h0014</td><td>00h</td><td></td></tr><tr><td colspan="3">保留</td><td colspan="2">保留</td><td colspan="2">3&#x27;h0</td><td>04h</td><td></td></tr></table>

<table><tr><td>8&#x27;h 08</td><td>8&#x27;h 80</td><td>8&#x27;h 0</td><td>保留</td><td>08h</td></tr><tr><td>保留</td><td>8&#x27;h 0</td><td>保留</td><td>8&#x27;h 10</td><td>0Ch</td></tr><tr><td colspan="3">24&#x27;h 0</td><td>8&#x27;h 4</td><td>10h</td></tr><tr><td colspan="3">32&#x27;h 0</td><td colspan="2">14h</td></tr><tr><td colspan="3">保留</td><td colspan="2">18h</td></tr><tr><td colspan="3">保留</td><td colspan="2">1Ch</td></tr><tr><td colspan="3">保留</td><td colspan="2">20h</td></tr><tr><td colspan="3">保留</td><td colspan="2">24h</td></tr><tr><td colspan="3">保留</td><td colspan="2">28h</td></tr><tr><td colspan="3">保留</td><td colspan="2">2Ch</td></tr><tr><td colspan="3">保留</td><td colspan="2">30h</td></tr><tr><td colspan="3">保留</td><td colspan="2">34h</td></tr><tr><td colspan="3">保留</td><td colspan="2">38h</td></tr><tr><td colspan="3">保留</td><td>0xFF</td><td>3Ch</td></tr></table>

# 6.3.3 GMAC0/1配置头

GMAC0控制器和GMAC1控制器的配置头缺省值一致。其配置头访问的基址分别为0xFE_0000_1800和0xFE_0000_1900。

表6-12GMAC0控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="3">16&#x27;h7a03</td><td colspan="4">16&#x27;h0014</td><td>00h</td><td></td></tr><tr><td colspan="3">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td><td></td></tr><tr><td>8&#x27;h02</td><td colspan="2">8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>08h</td><td></td></tr><tr><td>保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td><td></td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>14h</td><td></td></tr><tr><td colspan="7">保留</td><td>18h</td><td></td></tr><tr><td colspan="7">保留</td><td>1Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>20h</td><td></td></tr><tr><td colspan="7">保留</td><td>24h</td><td></td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr><tr><td colspan="7">保留</td><td>2Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>30h</td><td></td></tr><tr><td colspan="7">保留</td><td>34h</td><td></td></tr><tr><td colspan="7">保留</td><td>38h</td><td></td></tr><tr><td colspan="5">保留</td><td colspan="2">0xFF</td><td>3Ch</td><td></td></tr></table>

# 6.3.4 USBOTG配置头

该设备的配置头访问的基址为0xFE_0000_2000。

表6-13USB-OTG控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="3">16&#x27;h7a04</td><td colspan="4">16&#x27;h0014</td><td>00h</td><td></td></tr><tr><td colspan="3">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td><td></td></tr><tr><td>8&#x27;h0c</td><td colspan="2">8&#x27;h03</td><td colspan="2">8&#x27;h80</td><td colspan="2">保留</td><td>08h</td><td></td></tr><tr><td>保留</td><td colspan="2">8&#x27;h80</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td><td></td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>I4h</td><td></td></tr><tr><td colspan="7">保留</td><td>18h</td><td></td></tr><tr><td colspan="7">保留</td><td>1Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>Z0h</td><td></td></tr><tr><td colspan="7">保留</td><td>24h</td><td></td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr></table>

<table><tr><td colspan="2">保留</td><td>2Ch</td><td></td></tr><tr><td colspan="2">保留</td><td>30h</td><td></td></tr><tr><td colspan="2">保留</td><td>34h</td><td></td></tr><tr><td colspan="2">保留</td><td>38h</td><td></td></tr><tr><td colspan="2">保留</td><td>0xFF</td><td>3Ch</td></tr></table>

# 6.3.5 USBEHCI配置头

该设备的配置头访问的基址为0xFE_0000_2100。

表6-14USB-EHCI控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a14</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td colspan="2">8&#x27;h0c</td><td colspan="2">8&#x27;h03</td><td colspan="2">8&#x27;h20</td><td colspan="2">保留</td><td>08h</td></tr><tr><td colspan="2">保留</td><td colspan="2">8&#x27;h80</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td></tr><tr><td colspan="6">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td></tr><tr><td colspan="8">32&#x27;h0</td><td>14h</td></tr><tr><td colspan="8">保留</td><td>18h</td></tr><tr><td colspan="8">保留</td><td>1Ch</td></tr><tr><td colspan="8">保留</td><td>20h</td></tr><tr><td colspan="8">保留</td><td>24h</td></tr><tr><td colspan="8">保留</td><td>28h</td></tr><tr><td colspan="8">保留</td><td>2Ch</td></tr><tr><td colspan="8">保留</td><td>30h</td></tr><tr><td colspan="8">保留</td><td>34h</td></tr><tr><td colspan="8">保留</td><td>38h</td></tr><tr><td colspan="6">保留</td><td colspan="2">0xFF</td><td>3Ch</td></tr></table>

# 6.3.6 USBOHCI配置头

该设备的配置头访问的基址为0xFE_0000_2200。

表6-15USB-OHCI控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a24</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td colspan="2">8&#x27;h0c</td><td colspan="2">8&#x27;h03</td><td colspan="2">9&#x27;h10</td><td colspan="2">保留</td><td>08h</td></tr><tr><td colspan="2">保留</td><td colspan="2">8&#x27;h80</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td></tr><tr><td colspan="6">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h\</td></tr><tr><td colspan="8">32&#x27;h0</td><td>14h</td></tr><tr><td colspan="8">保留</td><td>18h</td></tr><tr><td colspan="8">保留</td><td></td></tr><tr><td colspan="8">保留</td><td>20h</td></tr><tr><td colspan="8">保留</td><td>24h</td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr><tr><td colspan="8">保留</td><td>2Ch</td></tr><tr><td colspan="8">保留</td><td>30h</td></tr><tr><td colspan="7">保留</td><td>34h</td><td></td></tr><tr><td colspan="8">保留</td><td>38h</td></tr><tr><td colspan="6">保留</td><td colspan="2">0xFF</td><td>3Ch</td></tr></table>

# 6.3.7 GPU配置头

该设备的配置头访问的基址为0xFE_0000_2800。

表6-16GPU控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a05</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td colspan="2">8&#x27;h 03</td><td colspan="2">8&#x27;h02</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>08h</td></tr><tr><td colspan="2">保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td></tr><tr><td colspan="6">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td></tr><tr><td colspan="8">32&#x27;h0</td><td>14h</td></tr><tr><td colspan="8">保留</td><td>18h</td></tr><tr><td colspan="8">保留</td><td>1Ch</td></tr><tr><td colspan="8">保留</td><td>20h</td></tr><tr><td colspan="8">保留</td><td>24h</td></tr><tr><td colspan="8">保留</td><td>28h</td></tr><tr><td colspan="8">保留</td><td>2Ch</td></tr><tr><td colspan="8">保留</td><td>30h</td></tr><tr><td colspan="8">保留</td><td>34h</td></tr><tr><td colspan="8">保留</td><td>38h</td></tr><tr><td colspan="6">保留</td><td colspan="2">0xFF</td><td>3Ch</td></tr></table>

# 6.3.8 DC配置头

该设备的配置头访问的基址为0xFE_0000_3000。

表6-17DC控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a06</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td colspan="2">8&#x27;h 03</td><td colspan="2">8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>08h</td></tr><tr><td colspan="2">保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td></tr><tr><td colspan="6">24&#x27;h0</td><td>8&#x27;h4&lt;lCEL&gt;</td><td>10h</td><td></td></tr><tr><td colspan="8">32&#x27;h0</td><td>14h</td></tr><tr><td colspan="8">保留</td><td>18h</td></tr><tr><td colspan="7">保留</td><td>1Ch</td><td></td></tr><tr><td colspan="8">保留</td><td>20h</td></tr><tr><td colspan="8">保留</td><td>24h</td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr><tr><td colspan="8">保留</td><td>2Ch</td></tr><tr><td colspan="8">保留</td><td>30h</td></tr><tr><td colspan="5">保留</td><td colspan="4">34h</td></tr><tr><td colspan="8">保留</td><td>38h</td></tr><tr><td colspan="8">保留</td><td>3Ch</td></tr></table>

# 6.3.9 HDA配置头

该设备的配置头访问的基址为0xFE_0000_3800。

表6-18HDA控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a07</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td colspan="2">8&#x27;h 04</td><td colspan="2">8&#x27;h03</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>08h</td></tr><tr><td colspan="2">保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td></tr><tr><td colspan="6">24&#x27;h0</td><td>8&#x27;h4&lt;l CEL&gt;</td><td>10h</td><td></td></tr><tr><td colspan="8">32&#x27;h0</td><td>14h</td></tr><tr><td colspan="8">保留</td><td>18h</td></tr><tr><td colspan="7">保留</td><td>1Ch</td><td></td></tr></table>

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>20h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>24h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>28h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>2Ch</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>30h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>34h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>38h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>0xFF</td><td>3Ch</td></tr></table>

# 6.3.10 SATA 配置头

该设备的配置头访问的基址为 0xFE_0000_4000。

表6-19SATA控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a08</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td>04h</td></tr><tr><td>8&#x27;h 01</td><td colspan="2">8&#x27;h06</td><td colspan="2">8&#x27;h01</td><td colspan="2">保留</td><td>08h</td><td></td></tr><tr><td>保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td><td></td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>14h</td><td></td></tr><tr><td colspan="7">保留</td><td>18h</td><td></td></tr><tr><td colspan="7">保留</td><td>1Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>20h</td><td></td></tr><tr><td colspan="7">保留</td><td>24h</td><td></td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr><tr><td colspan="7">保留</td><td>2Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>30h</td><td></td></tr><tr><td colspan="7">保留</td><td>34h</td><td></td></tr><tr><td colspan="7">保留</td><td>38h</td><td></td></tr><tr><td colspan="5">保留</td><td colspan="2">0xFF</td><td>3Ch</td><td></td></tr></table>

# 6.3.11PCIE配置头

龙芯2K1000中的两个PCIE共包含6个端口（Port），每个Port内都有一个TYPE1类型的配置头。其中

PCIE0Port0的配置头基址为0xFE_0000_4800；PCIE0Port1的配置头基址为0xFE_0000_5000；PCIE0Port2的配置头基址为0xFE_0000_5800；PCIE0Port3的配置头基址为0xFE_0000_6000；PCIE1Port0的配置头基址为0xFE_0000_6800；PCIE1Port1的配置头基址为0xFE_0000_7000；

下表列出PCIE0Port0的配置头缺省值，其他Port与Port0相比仅有DeviceID的差别，Port1/2/3的DeviceID都是  $16\mathrm{h}7\mathrm{a}09$  。同样的，PCIE1port0的DeviceID为  $16\mathrm{h}7\mathrm{a}19$  ，Port1的DeviceID为  $16\mathrm{h}7\mathrm{a}09$  。

# 表6-20PCIE0Port0的配置头缺省值

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a19</td><td colspan="3">16&#x27;h0014</td><td>00h</td><td></td></tr><tr><td colspan="4">16&#x27;h0010</td><td colspan="3">16&#x27;h0000</td><td>04h</td><td></td></tr><tr><td colspan="5">24&#x27;h0b0300</td><td colspan="2">8&#x27;h01</td><td>08h</td><td></td></tr><tr><td>8&#x27;h00</td><td colspan="2">8&#x27;h01</td><td colspan="2">8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td>0Ch</td><td></td></tr><tr><td colspan="7">32&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>14h</td><td></td></tr><tr><td>8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td colspan="2">8&#x27;h00</td><td>18h</td><td></td></tr><tr><td colspan="3">16&#x27;h0</td><td colspan="2">8&#x27;h01</td><td colspan="2">8&#x27;h01</td><td>1Ch</td><td></td></tr><tr><td colspan="3">16&#x27;h0</td><td colspan="4">16&#x27;h0</td><td>20h</td><td></td></tr><tr><td colspan="3">16&#x27;h0</td><td colspan="4">16&#x27;h0</td><td>24h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>28h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>2Ch</td><td></td></tr><tr><td colspan="3">32&#x27;h0</td><td colspan="4">32&#x27;h0</td><td>30h</td><td></td></tr><tr><td colspan="5">Reserved</td><td colspan="2">8&#x27;h40</td><td>34h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>38h</td><td></td></tr><tr><td colspan="3">16&#x27;h0</td><td colspan="2">8&#x27;h01</td><td colspan="2">0xFF</td><td>3Ch</td><td></td></tr></table>

# 6.3.12 DMA配置头

该设备的配置头访问的基址为0xFE_0000_7800。

表6-21DMA控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a0f</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="2">保留</td><td colspan="2">3&#x27;h0</td><td>04h</td></tr><tr><td>8&#x27;h 08</td><td colspan="2">8&#x27;h80</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>保留</td><td>08h</td></tr><tr><td>保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td><td></td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="7">32&#x27;h0</td><td>14h</td><td></td></tr><tr><td colspan="7">保留</td><td>18h</td><td></td></tr><tr><td colspan="7">保留</td><td>20h</td><td></td></tr><tr><td colspan="7">保留</td><td>24h</td><td></td></tr><tr><td colspan="7">保留</td><td>28h</td><td></td></tr><tr><td colspan="7">保留</td><td>2Ch</td><td></td></tr><tr><td colspan="7">保留</td><td>30h</td><td></td></tr><tr><td colspan="7">保留</td><td>34h</td><td></td></tr><tr><td colspan="7">保留</td><td>38h</td><td></td></tr><tr><td colspan="7">保留</td><td>0xFF</td><td>3Ch</td></tr></table>

# 6.3.13 VPU配置头

该设备的配置头访问的基址为0xFE_0000_8000。

表6-22VPU解码器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="4">16&#x27;h7a16</td><td colspan="4">16&#x27;h0014</td><td>00h</td></tr><tr><td colspan="4">保留</td><td colspan="2">保留</td><td colspan="2">3&#x27;h0</td><td>04h</td></tr><tr><td>8&#x27;h 04</td><td colspan="2">8&#x27;h80</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td>08h</td><td></td></tr><tr><td>保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td>0Ch</td><td></td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td>10h</td><td></td></tr><tr><td colspan="4">32&#x27;h0</td><td colspan="3"></td><td>14h</td><td></td></tr><tr><td colspan="7">保留</td><td>18h</td><td></td></tr></table>

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>1Ch</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>20h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>24h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>28h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>2Ch</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>30h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>34h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td></td><td>38h</td><td></td></tr><tr><td></td><td></td><td></td><td>保留</td><td></td><td></td><td>0xFF</td><td>3Ch</td><td></td></tr></table>

# 6.3.14CAMERA配置头

该设备的配置头访问的基址为0xFE_0000_8800。

表6-23CAMERA控制器的配置头缺省值  

<table><tr><td>31</td><td>24</td><td>23</td><td>16</td><td>15</td><td>8</td><td>7</td><td>0</td><td>ADDR</td></tr><tr><td colspan="3">16&#x27;h7a26</td><td colspan="4">16&#x27;h0014</td><td colspan="2">00h</td></tr><tr><td colspan="3">保留</td><td colspan="3">保留</td><td>3&#x27;h0</td><td colspan="2">04h</td></tr><tr><td>8&#x27;h 04</td><td colspan="2">8&#x27;h80</td><td colspan="2">8&#x27;h00</td><td colspan="2">保留</td><td colspan="2">08h</td></tr><tr><td>保留</td><td colspan="2">8&#x27;h0</td><td colspan="2">保留</td><td colspan="2">8&#x27;h10</td><td colspan="2">0Ch</td></tr><tr><td colspan="5">24&#x27;h0</td><td colspan="2">8&#x27;h4</td><td colspan="2">10h</td></tr><tr><td colspan="7">32&#x27;h0</td><td colspan="2">14h</td></tr><tr><td colspan="7">保留</td><td colspan="2">18h</td></tr><tr><td colspan="7">保留</td><td colspan="2">1Ch</td></tr><tr><td colspan="7">保留</td><td colspan="2">20h</td></tr><tr><td colspan="7">保留</td><td colspan="2">24h</td></tr><tr><td colspan="7">保留</td><td colspan="2">28h</td></tr><tr><td colspan="7">保留</td><td colspan="2">2Ch</td></tr><tr><td colspan="7">保留</td><td colspan="2">30h</td></tr><tr><td colspan="7">保留</td><td colspan="2">34h</td></tr><tr><td colspan="7">保留</td><td colspan="2">38h</td></tr><tr><td colspan="5">保留</td><td colspan="2">0xFF</td><td colspan="2">3Ch</td></tr></table>

# 6.3.15N/A处理

当配置头访问的总线号、设备号、功能号和地址偏移无效时，写入操作执行，但是无效。读取操作得到的数据为全1，表示该访问到无效设备。

<table><tr><td>N/A</td><td>位</td><td>读写</td><td>缺省值</td></tr><tr><td>N/A</td><td>63:0</td><td>RO</td><td>0xffff_fff_ffff_ffff</td></tr></table>

# 6.4 IODMA请求

IODMA请求的路由方式由访存属性（CACHE/UNCACHE类型，通过设备对应的*_coherent寄存器配置）以及IO互连网络的地址窗口共同决定。

PCIE设备（包括所有PCIE0和PCIE1下游设备）与其他IO设备（除PCIE之外的所有设备）的DMA访问各拥有8组地址窗口，可以完成目标地址空间的选择以及从源地址空间到目标地址空间的转换。每个地址窗口由BASE、MASK和MMAP三个64位寄存器组成，

BASE以K字节对齐，MASK采用类似网络掩码高位为1的格式。MMAP各字段含义如下表所示，MMAP[5]表示是否允许CACHE访问类型命中，MMAP[6]表示映射后访问类型(0为UNCACHE，1为CACHE)，MMAP[7]表示窗口使能。

表6-24MMAP字段对应的该空间访问属性  

<table><tr><td>[4]</td><td>[5]</td><td>[6]</td><td>[7]</td></tr><tr><td>保留</td><td>允许 cache 访问命中</td><td>映射后访问类型</td><td>窗口使能</td></tr></table>

MMAP[0]用于选择目标地址窗口，为0代表窗口0，为1代表窗口1。窗口0和窗口1的路由方式如图6- 2所示。需注意，PCIE设备与其他IO设备对于目标窗口0和目标窗口1的路由方式并不相同，PCIE设备的目标窗口0路由到一级交叉开关，目标窗口1路由到其他IO设备；其他IO设备的目标窗口0路由到一级交叉开关，目标窗口1路由到Uncache访存通路，直接通过二级交叉开关访问内存。

判断窗口是否命中需要同时满足以下两个条件：

1. (IN_ADDR & MASK) == BASE

2. \~IN_CACHE | MMAP[5]

其中IN_CACHE为DMA的访存属性，通过设备对应的*_coherent配置。如果DMA访问为CACHE类型，由MMAP[5]决定是否允许命中；如果DMA访问为UNCACHED类型，则不受MMAP[5]的控制。

IO互连网络的地址窗口还具有地址转换和访存类型转换的作用。

窗口命中情况下，新地址换算公式：

OUT_ADDR = (IN_ADDR & ~MASK) | {MMAP[63:10], 10'h0}

窗口命中情况下，新的访存类型由MMAP[6]指定。

所有窗口都不命中的情况下，默认路由到窗口0，同时不做地址转换和访存类型转换。地址窗口转换寄存器如下表。

表6-25IO设备DMA访存地址转换寄存器表  

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td>0x1fe02400</td><td>XBAR_WIN4_BASE0</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 0 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02408</td><td>XBAR_WIN4_BASE1</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 1 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02410</td><td>XBAR_WIN4_BASE2</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 2 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02418</td><td>XBAR_WIN4_BASE3</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 3 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02420</td><td>XBAR_WIN4_BASE4</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 4 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02428</td><td>XBAR_WIN4_BASE5</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 5 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02430</td><td>XBAR_WIN4_BASE6</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 6 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02438</td><td>XBAR_WIN4_BASE7</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 7 的基地址</td><td>0x0</td></tr></table>

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td>0x1fe02440</td><td>XBAR_WIN4_MASK0</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 0 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02448</td><td>XBAR_WIN4_MASK1</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 1 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02450</td><td>XBAR_WIN4_MASK2</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 2 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02458</td><td>XBAR_WIN4_MASK3</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 3 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02460</td><td>XBAR_WIN4_MASK4</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 4 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02468</td><td>XBAR_WIN4_MASK5</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 5 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02470</td><td>XBAR_WIN4_MASK6</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 6 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02478</td><td>XBAR_WIN4_MASK7</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 7 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02480</td><td>XBAR_WIN4_MMAP0</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 0 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02488</td><td>XBAR_WIN4_MMAP1</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 1 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02490</td><td>XBAR_WIN4_MMAP2</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 2 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02498</td><td>XBAR_WIN4_MMAP3</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 3 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe024a0</td><td>XBAR_WIN4_MMAP4</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 4 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe024a8</td><td>XBAR_WIN4_MMAP5</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 5 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe024b0</td><td>XBAR_WIN4_MMAP6</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 6 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe024b8</td><td>XBAR_WIN4_MMAP7</td><td>RW</td><td>除 PCIE 外的 IO 设备 DMA 访问窗口 7 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02500</td><td>XBAR_WIN5_BASE0</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 0 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02508</td><td>XBAR_WIN5_BASE1</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 1 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02510</td><td>XBAR_WIN5_BASE2</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 2 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02518</td><td>XBAR_WIN5_BASE3</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 3 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02520</td><td>XBAR_WIN5_BASE4</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 4 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02528</td><td>XBAR_WIN5_BASE5</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 5 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02530</td><td>XBAR_WIN5_BASE6</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 6 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02538</td><td>XBAR_WIN5_BASE7</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 7 的基地址</td><td>0x0</td></tr><tr><td>0x1fe02540</td><td>XBAR_WIN5_MASK0</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 0 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02548</td><td>XBAR_WIN5_MASK1</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 1 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02550</td><td>XBAR_WIN5_MASK2</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 2 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02558</td><td>XBAR_WIN5_MASK3</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 3 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02560</td><td>XBAR_WIN5_MASK4</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 4</td><td>0x0</td></tr></table>

<table><tr><td>地址</td><td>寄存器</td><td>R/W</td><td>描述</td><td>缺省值</td></tr><tr><td></td><td></td><td></td><td>的掩码</td><td></td></tr><tr><td>0x1fe02568</td><td>XBAR_WIN5_MASK5</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 5 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02570</td><td>XBAR_WIN5_MASK6</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 6 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02578</td><td>XBAR_WIN5_MASK7</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 7 的掩码</td><td>0x0</td></tr><tr><td>0x1fe02580</td><td>XBAR_WIN5_MMAP0</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 0 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02588</td><td>XBAR_WIN5_MMAP1</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 1 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02590</td><td>XBAR_WIN5_MMAP2</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 2 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe02598</td><td>XBAR_WIN5_MMAP3</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 3 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe025a0</td><td>XBAR_WIN5_MMAP4</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 4 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe025a8</td><td>XBAR_WIN5_MMAP5</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 5 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe025b0</td><td>XBAR_WIN5_MMAP6</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 6 的新基地址</td><td>0x0</td></tr><tr><td>0x1fe025b8</td><td>XBAR_WIN5_MMAP7</td><td>RW</td><td>PCIE 设备 DMA 访问窗口 7 的新基地址</td><td>0x0</td></tr></table>

以PCIE MSI中断的路由配置方法为例。因为系统分配给MSI的地址分别为0x1fe014b0和0x1fe014f0，这两个地址都需要通过uncache方式访问，所以可以配置以下窗口：

XBAR_WIN5_BASE0配置为0x1fe0_0000；XBAR_WIN5_MASK0配置为0xFFFF_FFFF_FFFF_0000；XBAR_WIN5_MMAP0配置为0x81；

# 6.5 APB设备路由

APB总线控制器下挂载了UART控制器、CAN控制器、I2C控制器、PWM控制器、RTC控制器、HPET控制器、NAND控制器、ACPI控制器、DES控制器、AES控制器、RSA控制器、RNG控制器、专用通信接口、SDIO控制器和I2S控制器。由访问地址的[15:12]来进行路由，具体如下表所示。

表6-26APB设备地址译码  

<table><tr><td>地址[15:12]</td><td>设备</td><td>备注</td></tr><tr><td>0x0</td><td>UART * 12 CAN * 2</td><td>用地址的[11:8]做下一级地址译码：
0x0: UART0
0x1: UART1
0x2: UART2
0x3: UART3
0x4: UART4
0x5: UART5
0x6: UART6
0x7: UART7
0x8: UART8
0x9: UART9
0xA: UART10</td></tr></table>

<table><tr><td></td><td></td><td>0xB: UART11
0xC: CAN0
0xD: CAN1
0xE: NULL
0xF: NULL</td></tr><tr><td>0x1</td><td>I2C * 2</td><td>用地址[11]做下一级地址译码:
0x0: I2C0
0x1: I2C1</td></tr><tr><td>0x2</td><td>PWM</td><td></td></tr><tr><td>0x3</td><td>Reserved</td><td></td></tr><tr><td>0x4</td><td>HPET</td><td></td></tr><tr><td>0x5</td><td>Reserved</td><td></td></tr><tr><td>0x6</td><td>NAND</td><td></td></tr><tr><td>0x7</td><td>ACPI/RTC</td><td>ACPI的偏移地址为 0x7000
RTC的偏移地址为 0x7800</td></tr><tr><td>0x8</td><td>DES</td><td></td></tr><tr><td>0x9</td><td>AES</td><td></td></tr><tr><td>0xA</td><td>RSA</td><td></td></tr><tr><td>0xB</td><td>RNG</td><td></td></tr><tr><td>0xC</td><td>SDIO</td><td></td></tr><tr><td>0xD</td><td>I2S</td><td></td></tr><tr><td>0xE</td><td>专用通信接口 E1</td><td></td></tr><tr><td>0xF</td><td>NULL</td><td></td></tr></table>

# 7处理器核间中断与通信

7处理器核间中断与通信龙芯2K1000为每个处理器核都实现了8个核间中断寄存器（IPI）以支持多核BIOS启动和操作系统运行时在处理器核之间进行中断和通信，其说明和地址见表7- 1到表7- 3。

表7-1处理器核间中断相关的寄存器及其功能描述  

<table><tr><td>名称</td><td>读写权限</td><td>描述</td></tr><tr><td>IPISR</td><td>R</td><td>32位状态寄存器，任何一位有被置1且对应位使能情况下，处理器核INT4中断线被置位。</td></tr><tr><td>IPIEN</td><td>RW</td><td>32位使能寄存器，控制对应中断位是否有效</td></tr><tr><td>IPiset</td><td>W</td><td>32位置位寄存器，往对应的位写1，则对应的STATUS寄存器位被置1</td></tr><tr><td>IPI_CLR</td><td>W</td><td>32位清除寄存器，往对应的位写1，则对应的STATUS寄存器位被清0</td></tr><tr><td>BUFO</td><td>RW</td><td>缓存寄存器，供启动时传递参数使用，按64或者32位的uncache方式进行访问。</td></tr><tr><td>BUF1</td><td>RW</td><td>缓存寄存器，供启动时传递参数使用，按64或者32位的uncache方式进行访问。</td></tr><tr><td>BUF2</td><td>RW</td><td>缓存寄存器，供启动时传递参数使用，按64或者32位的uncache方式进行访问。</td></tr><tr><td>BUF3</td><td>RW</td><td>缓存寄存器，供启动时传递参数使用，按64或者32位的uncache方式进行访问。</td></tr></table>

在龙芯2K1000与处理器核间中断相关的寄存器及其功能描述如下：

表7-20号处理器核核间中断与通信寄存器列表  

<table><tr><td>名称</td><td>地址</td><td>权限</td><td>描述</td></tr><tr><td>CORE0_IPISR</td><td>0x1fe01000</td><td>R</td><td>0号处理器核的IPI_Status寄存器</td></tr><tr><td>CORE0_IPIEN</td><td>0x1fe01004</td><td>RW</td><td>0号处理器核的IPI_Enalbe寄存器</td></tr><tr><td>CORE0_IPISET</td><td>0x1fe01008</td><td>W</td><td>0号处理器核的IPI_Set寄存器</td></tr><tr><td>CORE0_IPICLR</td><td>0x1fe0100c</td><td>W</td><td>0号处理器核的IPI_Clear寄存器</td></tr><tr><td>CORE0_BUFO</td><td>0x1fe01020</td><td>RW</td><td>0号处理器核的IPI_MailBox0寄存器</td></tr><tr><td>CORE0_BUFI</td><td>0x1fe01028</td><td>RW</td><td>0号处理器核的IPI_MailBox1寄存器</td></tr><tr><td>CORE0_BUF2</td><td>0x1fe01030</td><td>RW</td><td>0号处理器核的IPI_MailBox2寄存器</td></tr><tr><td>CORE0_BUF3</td><td>0x1fe01038</td><td>RW</td><td>0号处理器核的IPI_MailBox3寄存器</td></tr></table>

表7-31号处理器核的核间中断与通信寄存器列表  

<table><tr><td>名称</td><td>地址</td><td>权限</td><td>描述</td></tr><tr><td>CORE1_IPISR</td><td>0x1fe01100</td><td>R</td><td>1号处理器核的IPI_Status寄存器</td></tr><tr><td>CORE1_IPIEN</td><td>0x1fe01104</td><td>RW</td><td>1号处理器核的IPI_Enalbe寄存器</td></tr><tr><td>CORE1_IPISET</td><td>0x1fe01108</td><td>W</td><td>1号处理器核的IPI_Set寄存器</td></tr></table>

<table><tr><td>CORE1_IPICLR</td><td>0x1fe0110c</td><td>W</td><td>1 号处理器核的 IPL_Clear 寄存器</td></tr><tr><td>CORE1_BUF0</td><td>0x1fe01120</td><td>RW</td><td>1 号处理器核的 IPL-MailBox0 寄存器</td></tr><tr><td>CORE1_BUF1</td><td>0x1fe01128</td><td>RW</td><td>1 号处理器核的 IPL-MailBox1 寄存器</td></tr><tr><td>CORE1_BUF2</td><td>0x1fe01130</td><td>RW</td><td>1 号处理器核的 IPL-MailBox2 寄存器</td></tr><tr><td>CORE1_BUF3</td><td>0x1fe01138</td><td>RW</td><td>1 号处理器核的 IPL-MailBox3 寄存器</td></tr></table>

# 8 温度传感器

# 8.1 实时温度采样

8.1 实时温度采样龙芯2K1000内部集成一个温度传感器，可以通过0x1fe01510开始的采样寄存器进行观测，同时，可以使用灵活的高低温中断报警或者自动调频功能进行控制。温度传感器在采样寄存器的对应位如下（基地址为0x1fe01510）：

表8-1温度采样寄存器说明  

<table><tr><td>位域</td><td>字段名</td><td>访问</td><td>复位值</td><td>描述</td></tr><tr><td>0</td><td>Thsens_int_lo</td><td>RW</td><td></td><td>低温触发中断状态，写入任意值清除中断</td></tr><tr><td>1</td><td>Thsens_int_hi</td><td>RW</td><td></td><td>高温触发中断状态，写入任意值清除中断</td></tr><tr><td>3:2</td><td></td><td>R</td><td></td><td>保留</td></tr><tr><td>4</td><td>Thsens_overflow</td><td>R</td><td></td><td>温度传感器上溢（超过125℃）</td></tr><tr><td>39:32</td><td>Thsens_out</td><td>R</td><td></td><td>温度传感器0摄氏温度
结点温度=Thsens_out-100
温度范围-40度-125度</td></tr><tr><td>其它</td><td></td><td>R</td><td></td><td>保留</td></tr></table>

通过对控制寄存器的设置，可以实现超过预设温度中断、低于预设温度中断及高温自动降频功能。

# 8.2 高低温中断触发

对于高低温中断报警功能，分别有4组控制寄存器对其阈值进行设置。每组寄存器包含以下三个控制位：

GATE：设置高温或低温的阈值。当输入温度高于高温阈值或低于低温阈值时，将会产生中断；

EN：中断使能控制。置1之后该组寄存器的设置才有效；

SEL：输入温度选择。当前2K1000内部集成一个温度传感器，所以该位只可以配置成0。

高温中断控制寄存器中包含4组用于控制高温中断触发的设置位；低温中断控制寄存器中包含4组用于控制低温中断触发的设置位。另外还有一组寄存器用于显示中断状态，分别对应于高温中断和低温中断，对该寄存器进行任意写操作将清除中断状态。

这几个寄存器的具体描述如下：

表8-2高低温中断寄存器说明  

<table><tr><td>寄存器</td><td>地址</td><td>控制</td><td>说明</td></tr><tr><td>高温中断控制寄存器
Thsens_int_ctrl_Hi</td><td>0x1fe01500</td><td>RW</td><td>[7:0]: Hi_gate0: 高温阈值 0, 超过这个温度将产生中断
[8:8]: Hi_en0: 高温中断使能 1
[11:10]: Hi_Sel0: 选择高温中断 0 的温度传感器输入源
[23:16]: Hi_gate1: 高温阈值 1, 超过这个温度将产生中断
[24:24]: Hi_en1: 高温中断使能 1
[27:26]: Hi_Sel1: 选择高温中断 1 的温度传感器输入源
[39:32]: Hi_gate2: 高温阈值 2, 超过这个温度将产生中断
[40:40]: Hi_en2: 高温中断使能 2
[43:42]: Hi_Sel2: 选择高温中断 2 的温度传感器输入源
[55:48]: Hi_gate3: 高温阈值 3, 超过这个温度将产生中断
[56:56]: Hi_en3: 高温中断使能 3
[59:58]: Hi_Sel3: 选择高温中断 3 的温度传感器输入源</td></tr><tr><td>低温中断控制寄存器
Thsens_int_ctrl_LO</td><td>0x1fe01508</td><td>RW</td><td>[7:0]: Lo_gate0: 低温阈值 0, 低于这个温度将产生中断
[8:8]: Lo_en0: 低温中断使能 0
[11:10]: Lo_Sel0: 选择低温中断 0 的温度传感器输入源
[23:16]: Lo_gate1: 低温阈值 1, 低于这个温度将产生中断
[24:24]: Lo_en1: 低温中断使能 1
[27:26]: Lo_Sel1: 选择低温中断 1 的温度传感器输入源
[39:32]: Lo_gate2: 低温阈值 2, 低于这个温度将产生中断
[40:40]: Lo_en2: 低温中断使能 2
[43:42]: Lo_Sel2: 选择低温中断 2 的温度传感器输入源
[55:48]: Lo_gate3: 低温阈值 3, 低于这个温度将产生中断
[56:56]: Lo_en3: 低温中断使能 3
[59:58]: Lo_Sel3: 选择低温中断 3 的温度传感器输入源</td></tr><tr><td>中断状态寄存器
Thsens_int_status/clr</td><td>0x1fe01510</td><td>RW</td><td>中断状态寄存器，写任意值清除中断
[0]: 高温中断触发
[1]: 低温中断触发</td></tr></table>

# 8.3 高温自动降频设置

为了在高温环境中保证芯片的运行，可以设置令高温自动降频，使得芯片在超过预设范围时主动进行时钟分频，达到降低芯片翻转率的效果。

对于高温降频功能，有4组控制寄存器对其行为进行设置。每组寄存器包含以下四个控制位：

GATE：设置高温或低温的阈值。当输入温度高于高温阈值或低于低温阈值时，将触发分频操作；

EN：高温降频使能控制。置1之后该组寄存器的设置才有效；

SEL：输入温度选择。当前2K1000内部集成一个温度传感器，所以该位只可以配置成0。

FREQ：分频数。当触发分频操作时，将频率调整为当前时钟频率的FREQ/8倍。

表8-3高温降频控制寄存器说明  

<table><tr><td>寄存器</td><td>地址</td><td>控制</td><td>说明</td></tr><tr><td>高温降频控制寄存器
Thsens scale hi</td><td>0x1fe01520</td><td>RW</td><td>四组设置优先级由高到低
[7:0]: Scale_gate0: 高温阈值 0, 超过这个温度将降频
[8:8]: Scale_en0: 高温降频使能 0
[11:10]: Scale_Sel0: 选择高温降频 0 的温度传感器输入源
[14:12]: Scale_freq0: 降频时的分频值
[23:16]: Scale_gate1: 高温阈值 1, 超过这个温度将降频
[24:24]: Scale_en1: 高温降频使能 1
[27:26]: Scale_Sel1: 选择高温降频 1 的温度传感器输入源
[30:28]: Scale_freq1: 降频时的分频值
[39:32]: Scale_gate2: 高温阈值 2, 超过这个温度将降频
[40:40]: Scale_en2: 高温降频使能 2
[43:42]: Scale_Sel2: 选择高温降频 2 的温度传感器输入源
[46:44]: Scale_freq2: 降频时的分频值
[55:48]: Scale_gate3: 高温阈值 3, 超过这个温度将降频
[56:56]: Scale_en3: 高温降频使能 3
[59:58]: Scale_Sel3: 选择高温降频 3 的温度传感器输入源
[62:60]: Scale_freq3: 降频时的分频值</td></tr></table>

# 9 I/O 中断

龙芯 2K1000 芯片最多支持 64 个中断源，以统一方式进行管理，如下图所示，任意一个 IO 中断源可以被配置为是否使能、触发的方式、以及被路由的目标处理器核中断脚。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/1062977ba8d2a43dda8b4918405c120544a2395f452b22d969b1f33abdc1cb79.jpg)  
图 9-1 龙芯 2K1000 处理器中断路由示意图

中断相关配置寄存器都是以位的形式对相应的中断线进行控制，中断控制位连接及属性配置见表9- 1。中断使能（Enable）的配置有三个寄存器：Interset、Intenclr和Inten。Intenset设置中断使能，Intenset寄存器写1的位对应的中断被使能。Intenclr清除中断使能，Intenclr寄存器写1的位对应的中断被清除。Inten寄存器读取当前各中断使能的情况。脉冲形式的中断信号由Intedge配置寄存器来选择，写1表示脉冲触发，写0表示电平触发。中断处理程序可以通过Intenclr的相应位来清除脉冲记录，在中断被清除后，需要配置相应的Intenset才能采集到该中断的下一次脉冲触发。

# 9.1 中断触发类型

对于2K1000来说，dma控制器中断和PCIE MSI中断为脉冲触发类型，gpio中断根据需要可以配置成电平触发或者脉冲触发，其余中断均为电平触发类型。

# 9.2 中断分发模式

在龙芯2K1000中支持硬件中断负载均衡功能，使得中断可以在软件预设的几个处理器核上进行路由，分为四种模式：

1. 固定分发模式。2. 轮转分发模式。3. 忙碌分发模式。4. 空闲分发模式。

硬件负载均衡使能由寄存器Intbounce和Intauto共同控制。如果仅当Intbounce有效，而Intauto对应位为无效，则采用轮转分发模式；在Intbounce对应位有效的情况下，如果Intauto对应位也有效，则采用忙碌分发模式；在仅当Intauto有效时，则采用空闲分发模式。当Intbounce或Intauto对应位有效的情况下，Entry寄存器代表可用的处理器核。当Intbounce和Intauto对应位均无效的情况下，Entry寄存器中处理器核向量配置只能有一个处理器核，且该中断路由到该核上。需要注意到的是，一旦配置完Intbounce和Intauto后不应该在运行中途修改。

# 9.3 中断相关寄存器描述

表9-1中断控制寄存器属性  

<table><tr><td rowspan="2">位域</td><td colspan="8">访问属性/缺省值</td></tr><tr><td>Intedge</td><td>Intpd</td><td>Inten</td><td>Intenset</td><td>Intenclr</td><td>Intbounce</td><td>Intauto</td><td>中断源</td></tr><tr><td>0</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Uart00-uart03</td></tr><tr><td>1</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Uart04-uart07</td></tr><tr><td>2</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Uart08-uart11</td></tr><tr><td>3</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>专用通信接口</td></tr></table>

<table><tr><td>位域</td><td colspan="8">访问属性/缺省值</td></tr><tr><td>4</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Had_int</td></tr><tr><td>5</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>I2s_int</td></tr><tr><td>6</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Reserved</td></tr><tr><td>7</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Thsens_int</td></tr><tr><td>8</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>TOY_TICK</td></tr><tr><td>9</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>RTC_TICK</td></tr><tr><td>10</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>cam_int</td></tr><tr><td>11</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Reserved</td></tr><tr><td>12</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Gmac0_sbd_int</td></tr><tr><td>13</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Gmac0_pmt_int</td></tr><tr><td>14</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Gmac1_sbd_int</td></tr><tr><td>15</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Gmac1_pmt_int</td></tr><tr><td>16</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Can0_int</td></tr><tr><td>17</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Can1_int</td></tr><tr><td>18</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Spi_int</td></tr><tr><td>19</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Sata_int</td></tr><tr><td>20</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Nand_int</td></tr><tr><td>21</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Hpct_int</td></tr><tr><td>23:22</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>I2c_int</td></tr><tr><td>27:24</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Pwm_int</td></tr><tr><td>28</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Dc_int</td></tr><tr><td>29</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Gpu_int</td></tr><tr><td>30</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>vpu_int</td></tr><tr><td>31</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Sdio_int</td></tr><tr><td>35:32</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Pcie0_int</td></tr><tr><td>37:36</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Pcie1_int</td></tr><tr><td>38</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>hpet1_int</td></tr><tr><td>39</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>hpet2_int</td></tr><tr><td>43:40</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Toy_int</td></tr><tr><td>48:44</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Dma_int[4:0]</td></tr><tr><td>49</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Otg_int</td></tr><tr><td>50</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Ehci_int</td></tr><tr><td>51</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Ohci_int</td></tr><tr><td>54:52</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Rtc_int</td></tr><tr><td>55</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Rsa_int</td></tr><tr><td>56</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Aes_int</td></tr></table>

<table><tr><td>位域</td><td colspan="8">访问属性/缺省值</td></tr><tr><td>57</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>Des_int</td></tr><tr><td>58</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>GPIO_int_lo</td></tr><tr><td>59</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>GPIO_int_hi</td></tr><tr><td>63:60</td><td>RW/0</td><td>RW/0</td><td>R/0</td><td>W/0</td><td>W/0</td><td>RW/0</td><td>RW/0</td><td>GPIO_int</td></tr></table>

表9-2中断控制寄存器地址  

<table><tr><td>名称</td><td>地址偏移</td><td>访问属性</td><td>缺省值</td><td>描述</td></tr><tr><td>Intisr_0</td><td>0x1fe01420</td><td>RO</td><td>NA</td><td>低 32 位中断状态寄存器</td></tr><tr><td>Inten_0</td><td>0x1fe01424</td><td>RO</td><td>NA</td><td>低 32 位中断使能状态寄存器</td></tr><tr><td>Intenset_0</td><td>0x1fe01428</td><td>WO</td><td>NA</td><td>低 32 位设置使能寄存器</td></tr><tr><td>Intenclr_0</td><td>0x1fe0142c</td><td>WO</td><td>NA</td><td>低 32 位清除使能寄存器和脉冲触发的中断</td></tr><tr><td>Intpol_0</td><td>0x1fe01430</td><td>WR</td><td>0x0</td><td>中断极性控制：1 代表低电平，0 代表高电平</td></tr><tr><td>Intedge_0</td><td>0x1fe01434</td><td>WR</td><td>0x0</td><td>低 32 位触发方式寄存器（1：脉冲触发；0：电平触发）</td></tr><tr><td>Intbounce_0</td><td>0x1fe01438</td><td>WO</td><td>0x0</td><td>低 32 位中断分发模式控制，与 auto 合用。
{auto, bounce}:
2&#x27;b00-固定分发模式
2&#x27;b01-轮转分发模式
2&#x27;b10-空闲分发模式
2&#x27;b11-忙碌分发模式</td></tr><tr><td>Intauto_0</td><td>0x1fe0143c</td><td>WO</td><td>0x0</td><td>低 32 位中断分发模式控制，与 bounce 合用。
{auto, bounce}:
2&#x27;b00-固定分发模式
2&#x27;b01-轮转分发模式
2&#x27;b10-空闲分发模式
2&#x27;b11-忙碌分发模式</td></tr><tr><td>Intisr_1</td><td>0x1fe01460</td><td>RO</td><td>NA</td><td>高 32 位中断状态寄存器</td></tr><tr><td>Inten_1</td><td>0x1fe01464</td><td>RO</td><td>NA</td><td>高 32 位中断使能状态寄存器</td></tr><tr><td>Intenset_1</td><td>0x1fe01468</td><td>WO</td><td>NA</td><td>高 32 位设置使能寄存器</td></tr><tr><td>Intenclr_1</td><td>0x1fe0146c</td><td>WO</td><td>NA</td><td>高 32 位清除使能寄存器和脉冲触发的中断</td></tr><tr><td>Intpol_1</td><td>0x1fe01470</td><td>WR</td><td>0x0</td><td>中断极性控制：1 代表低电平，0 代表高电平</td></tr><tr><td>Intedge_1</td><td>0x1fe01474</td><td>WR</td><td>0x0</td><td>高 32 位触发方式寄存器（1：脉冲触发；0：电平触发）</td></tr><tr><td>Intbounce_1</td><td>0x1fe01478</td><td>WO</td><td>0x0</td><td>高 32 位中断分发模式控制，与 auto 合用。
{auto, bounce}:
2&#x27;b00-固定分发模式
2&#x27;b01-轮转分发模式
2&#x27;b10-空闲分发模式
2&#x27;b11-忙碌分发模式</td></tr><tr><td>Intauto_1</td><td>0x1fe01464</td><td>RO</td><td>0x0</td><td>高 32 位中断分发模式控制，与 bounce 合用。
{auto, bounce}:
2&#x27;b00-固定分发模式
2&#x27;b01-轮转分发模式
2&#x27;b10-空闲分发模式
2&#x27;b11-忙碌分发模式</td></tr><tr><td></td><td></td><td></td><td></td><td></td></tr></table>

<table><tr><td>名称</td><td>地址偏移</td><td>访问属性</td><td>缺省值</td><td>描述</td></tr><tr><td>CORE0_IPISR</td><td>0x1fe01000</td><td>RO</td><td>NA</td><td>0号处理器核的IPI_Status寄存器</td></tr><tr><td>CORE0_IPIEN</td><td>0x1fe01004</td><td>RW</td><td>0x0</td><td>0号处理器核的IPI_Enalbe寄存器</td></tr><tr><td>CORE0_IPISET</td><td>0x1fe01008</td><td>WO</td><td>NA</td><td>0号处理器核的IPI_Sel寄存器</td></tr><tr><td>CORE0_IPI_CLR</td><td>0x1fe0100c</td><td>WO</td><td>NA</td><td>0号处理器核的IPI_Clear寄存器</td></tr><tr><td>CORE0_INTISR0</td><td>0x1fe01010</td><td>RO</td><td>NA</td><td>路由给CORE0的低32位中断状态</td></tr><tr><td>CORE0_INTISR1</td><td>0x1fe01018</td><td>RO</td><td>NA</td><td>路由给CORE0的高32位中断状态</td></tr><tr><td>CORE0_BUFO</td><td>0x1fe01020</td><td>RW</td><td>0x0</td><td>0号处理器核的IPI_MailBox0寄存器</td></tr><tr><td>CORE0_BUFI</td><td>0x1fe01028</td><td>RW</td><td>0x0</td><td>0号处理器核的IPI_MailBox1寄存器</td></tr><tr><td>CORE0_BUF2</td><td>0x1fe01030</td><td>RW</td><td>0x0</td><td>0号处理器核的IPI_MailBox2寄存器</td></tr><tr><td>CORE0_BUF3</td><td>0x1fe01038</td><td>RW</td><td>0x0</td><td>0号处理器核的IPI_MailBox3寄存器</td></tr><tr><td>CORE0_INTISR0</td><td>0x1fe01040</td><td>RO</td><td>NA</td><td>路由给CORE0的低32位中断状态</td></tr><tr><td>CORE0_INTISR1</td><td>0x1fe01048</td><td>RO</td><td>NA</td><td>路由给CORE0的高32位中断状态</td></tr><tr><td>CORE1_IPISR</td><td>0x1fe01100</td><td>RO</td><td>NA</td><td>1号处理器核的IPI_Status寄存器</td></tr><tr><td>CORE1_IPIEN</td><td>0x1fe01104</td><td>RW</td><td>0x0</td><td>1号处理器核的IPI_Enalbe寄存器</td></tr><tr><td>CORE1_IPISET</td><td>0x1fe01108</td><td>WO</td><td>NA</td><td>1号处理器核的IPI_Sel寄存器</td></tr><tr><td>CORE1_IPI_CLR</td><td>0x1fe0110c</td><td>WO</td><td>NA</td><td>1号处理器核的IPI_Clear寄存器</td></tr><tr><td>CORE1_INTISR0</td><td>0x1fe01110</td><td>RO</td><td>NA</td><td>路由给CORE1的低32位中断状态</td></tr><tr><td>CORE1_INTISR1</td><td>0x1fe01118</td><td>RO</td><td>NA</td><td>路由给CORE1的高32位中断状态</td></tr><tr><td>CORE1_BUFO</td><td>0x1fe01120</td><td>RW</td><td>0x0</td><td>1号处理器核的IPI_MailBox0寄存器</td></tr><tr><td>CORE1_BUFI</td><td>0x1fe01128</td><td>RW</td><td>0x0</td><td>1号处理器核的IPI_MailBox1寄存器</td></tr><tr><td>CORE1_BUF2</td><td>0x1fe01130</td><td>RW</td><td>0x0</td><td>1号处理器核的IPI_MailBox2寄存器</td></tr><tr><td>CORE1_BUF3</td><td>0x1fe01138</td><td>RW</td><td>0x0</td><td>1号处理器核的IPI_MailBox3寄存器</td></tr><tr><td>CORE1_INTISR0</td><td>0x1fe01140</td><td>RO</td><td>NA</td><td>路由给CORE1的低32位中断状态</td></tr><tr><td>CORE1_INTISR1</td><td>0x1fe01148</td><td>RO</td><td>NA</td><td>路由给CORE1的高32位中断状态</td></tr></table>

龙芯2K1000中集成了2个处理器核，上述的64位中断源可以通过软件配置选择期望中断的目标处理器核。更进一步，中断源可以选择路由到处理器核中断INT0到INT3中的任意一个。64个I/O中断源中每一个都对应一个8位的路由控制器，其格式和地址如下表所示。路由寄存器采用向量的方式进行路由选择，如  $0\mathrm{x}42$  表示路由到1号处理器的INT2上。

表9-4中断路由寄存器地址  
表9-3中断路由寄存器的说明  

<table><tr><td>位域</td><td>说明</td></tr><tr><td>3:0</td><td>路由的处理器核向量号</td></tr><tr><td>7:4</td><td>路由的处理器核中断引脚向量号</td></tr></table>

<table><tr><td>名称</td><td>地址偏移</td><td>描述</td><td>名称</td><td>地址偏移</td><td>描述</td></tr></table>

<table><tr><td>名称</td><td>地址偏移</td><td>描述</td><td>名称</td><td>地址偏移</td><td>描述</td></tr><tr><td>Entry0</td><td>0x1fe0140 0</td><td>Uart00-03</td><td>Entry32</td><td>0x1fe0144 0</td><td>Pcie0_int0</td></tr><tr><td>Entry1</td><td>0x1fe0140 1</td><td>Uart04-07</td><td>Entry33</td><td>0x1fe0144 1</td><td>Pcie0_int1</td></tr><tr><td>Entry2</td><td>0x1fe0140 2</td><td>Uart08-11</td><td>Entry34</td><td>0x1fe0144 2</td><td>Pcie0_int2</td></tr><tr><td>Entry3</td><td>0x1fe0140 3</td><td>专用通信接口</td><td>Entry35</td><td>0x1fe0144 3</td><td>Pcie0_int3</td></tr><tr><td>Entry4</td><td>0x1fe0140 4</td><td>Hda_int</td><td>Entry36</td><td>0x1fe0144 4</td><td>Pcie1_int0</td></tr><tr><td>Entry5</td><td>0x1fe0140 5</td><td>I2s_int</td><td>Entry37</td><td>0x1fe0144 5</td><td>Pcie1_int1</td></tr><tr><td>Entry6</td><td>0x1fe0140 6</td><td>Reserved</td><td>Entry38</td><td>0x1fe0144 6</td><td>hpet1_int</td></tr><tr><td>Entry7</td><td>0x1fe0140 7</td><td>Thsens_int</td><td>Entry39</td><td>0x1fe0144 7</td><td>hpet2_int</td></tr><tr><td>Entry8</td><td>0x1fe0140 8</td><td>TOY_TICK</td><td>Entry40</td><td>0x1fe0144 8</td><td>Toy_int0</td></tr><tr><td>Entry9</td><td>0x1fe0140 9</td><td>RTC_TICK</td><td>Entry41</td><td>0x1fe0144 9</td><td>Toy_int1</td></tr><tr><td>Entry10</td><td>0x1fe0140a</td><td>cam</td><td>Entry42</td><td>0x1fe0144a</td><td>Toy_int2</td></tr><tr><td>Entry11</td><td>0x1fe0140 b</td><td>Reserved</td><td>Entry43</td><td>0x1fe0144 b</td><td>Toy_int3</td></tr><tr><td>Entry12</td><td>0x1fe0140c</td><td>Gmac0_sbd_int</td><td>Entry44</td><td>0x1fe0144c</td><td>Dma_int0</td></tr><tr><td>Entry13</td><td>0x1fe0140 d</td><td>Gmac0_pmt_int</td><td>Entry45</td><td>0x1fe0144 d</td><td>Dma_int1</td></tr><tr><td>Entry14</td><td>0x1fe0140e</td><td>Gmac1_sbd_int</td><td>Entry46</td><td>0x1fe0144e</td><td>Dma_int2</td></tr><tr><td>Entry15</td><td>0x1fe0140f</td><td>Gmac1_pmt_int</td><td>Entry47</td><td>0x1fe0144f</td><td>Dma_int3</td></tr><tr><td>Entry16</td><td>0x1fe0141 0</td><td>Can0_int</td><td>Entry48</td><td>0x1fe0145 0</td><td>Dma_int4</td></tr><tr><td>Entry17</td><td>0x1fe01411</td><td>Can1_int</td><td>Entry49</td><td>0x1fe0145 1</td><td>Otg_int</td></tr><tr><td>Entry18</td><td>0x1fe0141 2</td><td>Spi_int</td><td>Entry50</td><td>0x1fe0145 2</td><td>Ehci_int</td></tr><tr><td>Entry19</td><td>0x1fe0141 3</td><td>Sata_int</td><td>Entry51</td><td>0x1fe0145 3</td><td>Ohci_int</td></tr><tr><td>Entry20</td><td>0x1fe0141 4</td><td>Nand_int</td><td>Entry52</td><td>0x1fe0145 4</td><td>Rtc_int0</td></tr></table>

<table><tr><td>名称</td><td>地址偏移</td><td>描述</td><td>名称</td><td>地址偏移</td><td>描述</td></tr><tr><td>Entry21</td><td>0x1fe0141 5</td><td>Hpet_int</td><td>Entry53</td><td>0x1fe0145 5</td><td>Rtc_int1</td></tr><tr><td>Entry22</td><td>0x1fe0141 6</td><td>I2c_int0</td><td>Entry54</td><td>0x1fe0145 6</td><td>Rtc_int3</td></tr><tr><td>Entry23</td><td>0x1fe0141 7</td><td>I2c_int1</td><td>Entry55</td><td>0x1fe0145 7</td><td>Rsa_int</td></tr><tr><td>Entry24</td><td>0x1fe0141 8</td><td>Pwm_int0</td><td>Entry56</td><td>0x1fe0145 8</td><td>Aes_int</td></tr><tr><td>Entry25</td><td>0x1fe0141 9</td><td>Pwm_int1</td><td>Entry57</td><td>0x1fe0145 9</td><td>Des_int</td></tr><tr><td>Entry26</td><td>0x1fe0141a</td><td>Pwm_int2</td><td>Entry58</td><td>0x1fe0145a</td><td>Gpio_int_lo</td></tr><tr><td>Entry27</td><td>0x1fe0141b</td><td>Pwm_int3</td><td>Entry59</td><td>0x1fe0145b</td><td>Gpio_int_hi</td></tr><tr><td>Entry28</td><td>0x1fe0141c</td><td>Dc_int</td><td>Entry60</td><td>0x1fe0145c</td><td>Gpio_int0</td></tr><tr><td>Entry29</td><td>0x1fe0141d</td><td>Gpu_int</td><td>Entry61</td><td>0x1fe0145d</td><td>Gpio_int1</td></tr><tr><td>Entry30</td><td>0x1fe0141e</td><td>vpu</td><td>Entry62</td><td>0x1fe0145e</td><td>Gpio_int2</td></tr><tr><td>Entry31</td><td>0x1fe0141f</td><td>Sdio_int</td><td>Entry63</td><td>0x1fe0145f</td><td>Gpio_int3</td></tr></table>

# 9.4 GPIO中断

龙芯2K1000有60个GPIO引脚，具体请参考13.5节。这些GPIO引脚与中断引脚的对应关系如下：

表9-5GPIO中断  

<table><tr><td>GPIO引脚</td><td>中断引脚</td><td>说明</td></tr><tr><td>GPIO0</td><td>GPIO_int0</td><td rowspan="4">专用 GPIO引脚，与中断引脚一一对应，需设置中断使能，参考 13.4 节</td></tr><tr><td>GPIO1</td><td>GPIO_int1</td></tr><tr><td>GPIO2</td><td>GPIO_int2</td></tr><tr><td>GPIO3</td><td>GPIO_int3</td></tr><tr><td>GPIO[31:04]</td><td>GPIO_int_lo</td><td>GPIO31-GPIO4 复用中断引脚 GPIO_int_lo</td></tr><tr><td>GPIO[63:32]</td><td>GPIO_int_hi</td><td>GPIO63-GPIO32 复用中断引脚 GPIO_int_hi</td></tr></table>

# 9.5 MSI中断

龙芯2K1000的PCIE控制器支持MSI中断类型。当接收到MSI中断时，PCIE控制器把对应的中断请求转发到中断控制器。中断控制器将MSI中断的Message Data译码成中断

线的形式，与现有的中断引脚复用。

龙芯2K1000分配给MSI的地址有两个，分别对应寄存器INT MSI ADDR_0（0x1fe014b0）和INT MSI ADDR_1（0x1fe014f0），每个地址可以接受32个中断消息，分别与现有的中断引脚低32位和高32位复用。与MSI相关的寄存器如下表所示：

表9-6MSI中断相关寄存器  

<table><tr><td>0x1fe014a0</td><td>INT MSI_0</td><td>保留</td></tr><tr><td>0x1fe014a8</td><td>INT MSI_EN_0</td><td>保留</td></tr><tr><td>0x1fe014b0</td><td>INT MSI_ADDR_0</td><td>MSI地址 0</td></tr><tr><td>0x1fe014b4</td><td>INT MSI_TRIGGER_EN_0</td><td>32位 MSI 中断使能，每位对应一个写入 MSI 地址 0 的中断</td></tr><tr><td>0x1fe014e0</td><td>INT MSI_1</td><td>保留</td></tr><tr><td>0x1fe014e8</td><td>INT MSI_EN_1</td><td>保留</td></tr><tr><td>0x1fe014f0</td><td>INT MSI_ADDR_1</td><td>MSI地址 1</td></tr><tr><td>0x1fe014f4</td><td>INT MSI_TRIGGER_EN_1</td><td>32位 MSI 中断使能，每位对应一个写入 MSI 地址 1 的中断</td></tr></table>

在接收MSI中断之前需要先将对应的MSI中断使能位设为1。另外，MSI中断为脉冲触发类型。

MSI中断配置流程：

1. 配置设备的MSI相关capability，系统分配的MSI地址为INT MSI ADDR_0（0x1fe014b0）和INT MSI_ADDR_1（0x1fe014f0）；2. 配置中断触发类型INTEDGE寄存器，将MSI中断设置为脉冲触发，即对应位设为1；3. 配置对应的INT MSI TRIGGER_EN寄存器，即将对应的MSI中断使能位设为1；4. 配置对应的INTENSET寄存器，即将对应的中断使能位设为1；5. 读对应的中断状态INTISR寄存器，获取相应的中断信息；6. 读到对应的中断后，配置对应的INTENCLR寄存器，将其对应位设为1，清除中断；7. 再重新将对应的中断使能位(INTENSET寄存器)设为1。

# 9.6 硬件中断负载均衡功能举例

1. 轮转分发模式（bounce）  

<table><tr><td>步骤</td><td>处理器核向量配置</td><td>Bounce</td><td>Auto</td><td>触发中断</td><td>处理器核</td><td>清除中断</td><td>说明</td></tr><tr><td>1</td><td>Entry0[3:0] = 4&#x27;b0011
Entry1[3:0] = 4&#x27;b0010</td><td>32&#x27;b1</td><td>32&#x27;b0</td><td></td><td></td><td></td><td>Int0 的 bounce 有效</td></tr><tr><td>2</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>路由到配置中最右边的处理器核，即 Core0</td></tr><tr><td>3</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>4</td><td></td><td></td><td></td><td>Int1</td><td>Core1</td><td></td><td>使 Core1 上有未被处理的中断</td></tr><tr><td>5</td><td></td><td></td><td></td><td>Int0</td><td>Core1</td><td></td><td>仍旧路由到 Entry 配置的下一个处理器核，即 Core1</td></tr></table>

<table><tr><td>6</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>7</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>路由到 Entry 配置的下一个处理器核，即 Core0</td></tr><tr><td>8</td><td></td><td></td><td></td><td></td><td></td><td>Int0
Int1</td><td></td></tr></table>

2. 忙碌分发模式

<table><tr><td>步骤</td><td>处理器核向量配置</td><td>Bounce</td><td>Auto</td><td>触发中断</td><td>处理器核</td><td>清除中断</td><td>说明</td></tr><tr><td>1</td><td>Entry0[3:0] = 4&#x27;b0011
Entry1[3:0] = 4&#x27;b0001
Entry2[3:0] = 4&#x27;b0010</td><td>32&#x27;b1</td><td>32&#x27;b1</td><td></td><td></td><td></td><td>Int0 的 bounce 和 auto 有 效</td></tr><tr><td>2</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>路由到 Entry 配置中最右 边的处理器核，即 Core0</td></tr><tr><td>3</td><td></td><td></td><td></td><td>Int1</td><td>Core0</td><td></td><td>使 Core0 上有未被处理 的中断</td></tr><tr><td>4</td><td></td><td></td><td></td><td>Int2</td><td>Core1</td><td></td><td>使 Core1 上有未被处理 的中断</td></tr><tr><td>5</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>6</td><td></td><td></td><td></td><td>Int0</td><td>Core1</td><td></td><td>由于 Core0 上有未被处理 的中断，路由到 Entry 配置的下一个处理器核，即 core1。</td></tr><tr><td>7</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>8</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>由于 Core1 上有未被处理 的中断，路由到 Entry 配置的下一个处理器核，即 Core0</td></tr><tr><td>9</td><td></td><td></td><td></td><td></td><td></td><td>Int0
Int1</td><td></td></tr><tr><td>10</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>Core0 上没有未被处理的 中断，继续中断该核，即 Core0</td></tr><tr><td>11</td><td></td><td></td><td></td><td></td><td></td><td>Int0
Int2</td><td></td></tr></table>

3. 空闲分发模式

<table><tr><td>步骤</td><td>处理器核向量配置</td><td>Bounce</td><td>Auto</td><td>触发中断</td><td>处理器核</td><td>清除中断</td><td>说明</td></tr><tr><td>1</td><td>Entry0[3:0] = 4&#x27;b0011
Entry1[3:0] = 4&#x27;b0001
Entry2[3:0] = 4&#x27;b00010</td><td>32&#x27;b0</td><td>32&#x27;b1</td><td></td><td></td><td></td><td>Int0 的 auto 有效</td></tr><tr><td>2</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>路由到 Entry 配置中最右边的处理器核，即 Core0</td></tr><tr><td>3</td><td></td><td></td><td></td><td>Int2</td><td>Core1</td><td></td><td>使 Core1 上有未被处理的中断</td></tr><tr><td>4</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>5</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>由于 Core1 上有未被处理的中断，而 Core0 上没有未被处理的中断，所以路由到 Entry 配置的下一个空闲的处理器核，即 Core0。</td></tr><tr><td>6</td><td></td><td></td><td></td><td></td><td></td><td>Int0</td><td></td></tr><tr><td>7</td><td></td><td></td><td></td><td>Int1</td><td>Core0</td><td></td><td>使 Core0 上有未被处理的中断</td></tr><tr><td>8</td><td></td><td></td><td></td><td>Int0</td><td>Core0</td><td></td><td>由于 Entry 配置的核</td></tr></table>

<table><tr><td>步骤</td><td>处理器核向量配置</td><td>Bounce</td><td>Auto</td><td>触发中断</td><td>处理器核</td><td>清除中断</td><td>说明</td></tr><tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>（Core0和Core1）都有未被处理的中断，只能继续中断该核，即Core0</td></tr><tr><td>9</td><td></td><td></td><td></td><td></td><td></td><td>Int2 Int0</td><td></td></tr><tr><td>10</td><td></td><td></td><td></td><td>Int0</td><td>Core1</td><td></td><td>由于Core1空闲，所以路由到Entry配置的下一个空闲的处理器核，即Core1。</td></tr><tr><td>11</td><td></td><td></td><td></td><td></td><td></td><td>Int0 Int1</td><td></td></tr><tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr></table>

# 10 SPI控制器

10 SPI控制器串行外围设备接口SPI总线技术是多种微处理器、微控制器以及外围设备之间的一种全双工、同步、串行数据接口标准。

# 10.1访问地址

SPI控制器包括两个地址空间，分别如下：

表10-1SPI控制器地址空间分配  

<table><tr><td>起始地址</td><td>结束地址</td><td>名称</td><td>说明</td></tr><tr><td>0x1C00_0000</td><td>0x1C0F_FFFF</td><td>启动空间</td><td>当引脚设置为SPI启动时可访问，其它情况下不可访问</td></tr><tr><td>0x1FFF_0220</td><td>0x1FFF_022F</td><td>配置寄存器空间</td><td></td></tr></table>

# 10.2SPI控制器结构

10.2 SPI控制器结构本系统集成的SPI控制器仅可作为主控端，所连接的是从设备。对于软件而言，SPI控制器除了有若干IO寄存器外还有一段映射到SPI Flash的只读memory空间。如果将这段memory空间分配在  $0\mathrm{x}1\mathrm{c}000000$  ，复位后不需要软件干预就可以直接访问，从而支持处理器从SPIFlash启动。SPI的IO寄存器的基地址0x1fff0220。

# 10.3配置寄存器

表10-2SPI配置寄存器列表  

<table><tr><td>偏移</td><td>名称</td><td>描述</td></tr><tr><td>0</td><td>SPCR</td><td>控制寄存器</td></tr><tr><td>1</td><td>SPSR</td><td>状态寄存器</td></tr><tr><td>2</td><td>TxFIFO/RxFIFO</td><td>数据寄存器</td></tr><tr><td>3</td><td>SPER</td><td>外部寄存器</td></tr><tr><td>4</td><td>SFC_PARAM</td><td>参数控制寄存器</td></tr><tr><td>5</td><td>SFC_SOFTCS</td><td>片选控制寄存器</td></tr><tr><td>6</td><td>SFC_TIMING</td><td>时序控制寄存器</td></tr></table>

# 10.3.1控制寄存器（SPCR）

偏移地址：0x0

表10-3SPI控制寄存器（SPCR）  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7</td><td>spie</td><td>R/W</td><td>0</td><td>中断输出使能信号高有效</td></tr><tr><td>6</td><td>spe</td><td>R/W</td><td>0</td><td>系统工作使能信号高有效</td></tr></table>

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>5</td><td>-</td><td>-</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>mstr</td><td>-</td><td>1</td><td>master 模式选择位，此位一直保持 1</td></tr><tr><td>3</td><td>cpol</td><td>R/W</td><td>0</td><td>时钟极性位</td></tr><tr><td>2</td><td>cpha</td><td>R/W</td><td>0</td><td>时钟相位位 1 则相位相反，为 0 则相同</td></tr><tr><td>1:0</td><td>spr</td><td>R/W</td><td>0</td><td>sclk_o 分频设定，需要与 spcr 的 spre 一起使用</td></tr></table>

# 10.3.2 状态寄存器（SPSR)

偏移地址：0x1

表10-4SPI状态寄存器（SPSR)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7</td><td>spif</td><td>R/W</td><td>0</td><td>中断标志位 1 表示有中断申请，写 1 则清零</td></tr><tr><td>6</td><td>wcol</td><td>R/W</td><td>0</td><td>写寄存器溢出标志位为 1 表示已经溢出，写 1 则清零</td></tr><tr><td>5:4</td><td>-</td><td>-</td><td>0</td><td>保留</td></tr><tr><td>3</td><td>wffull</td><td>R</td><td>0</td><td>写寄存器满标志 1 表示已经满</td></tr><tr><td>2</td><td>wfempty</td><td>R</td><td>1</td><td>写寄存器空标志 1 表示空</td></tr><tr><td>1</td><td>rffull</td><td>R</td><td>0</td><td>读寄存器满标志 1 表示已经满</td></tr><tr><td>0</td><td>rfempty</td><td>R</td><td>1</td><td>读寄存器空标志 1 表示空</td></tr></table>

# 10.3.3 数据寄存器（TxFIFO/RxFIFO)

偏移地址：0x2

表10-5SPI数据寄存器（TxFIFO/RXFIFO)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7:0</td><td>TxFIFO
RxFIFO</td><td>W
R</td><td>-</td><td>数据发送端口
数据接收端口</td></tr></table>

# 10.3.4 外部寄存器（SPER)

偏移地址：0x3

表10-6SPI外部寄存器（SPER)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7:6</td><td>icnt</td><td>R/W</td><td>0</td><td>传输完多少个字节后发中断
00:1
01:2
10:3
11:4</td></tr><tr><td>5:3</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>2</td><td>mode</td><td>R/W</td><td>0</td><td>spi接口模式控制
0:采样与发送时机同时
1:采样与发送时机错开半周期</td></tr><tr><td>1:0</td><td>spre</td><td>R/W</td><td>0</td><td>与spr一起设定分频的比率</td></tr></table>

# 表10-7SPI分频系数

<table><tr><td>spre</td><td>00</td><td>00</td><td>00</td><td>00</td><td>01</td><td>01</td><td>01</td><td>01</td><td>10</td><td>10</td><td>10</td><td>10</td></tr><tr><td>spr</td><td>00</td><td>01</td><td>10</td><td>11</td><td>00</td><td>01</td><td>10</td><td>11</td><td>00</td><td>01</td><td>10</td><td>11</td></tr><tr><td>分频系数</td><td>2</td><td>4</td><td>16</td><td>32</td><td>8</td><td>64</td><td>128</td><td>256</td><td>512</td><td>1024</td><td>2048</td><td>4096</td></tr></table>

# 10.3.5 参数控制寄存器(SFC_PARAM)

偏移地址：0x4

表10-8SPI参数控制寄存器(SFC_PARAM)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7:4</td><td>clk_div</td><td>R/W</td><td>2</td><td>时钟分频数选择
分频系数与{spr, spr}组合相同</td></tr><tr><td>3</td><td>dual_io</td><td>R/W</td><td>0</td><td>双 I/O 模式，优先级高于快速读</td></tr><tr><td>2</td><td>fast_read</td><td>R/W</td><td>0</td><td>快速读模式</td></tr><tr><td>1</td><td>burst_en</td><td>R/W</td><td>0</td><td>SPI flash 支持连续地址读模式</td></tr><tr><td>0</td><td>memory_en</td><td>R/W</td><td>1</td><td>SPI flash 读使能，无效时 csn[0]可由软件控制。</td></tr></table>

# 10.3.6片选控制寄存器(SFCSOFTCS)

偏移地址：0x5

表10-9SPI片选控制寄存器(SFCSOFTCS)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7:4</td><td>csn</td><td>R/W</td><td>0</td><td>csn 引脚输出值</td></tr><tr><td>3:0</td><td>csen</td><td>R/W</td><td>0</td><td>为 1 时对应位的 csn 线由 7:4 位控制</td></tr></table>

# 10.3.7时序控制寄存器(SFC_TIMING)

偏移地址：0x6

表10-10SPI时序控制寄存器(SFC_TIMING)  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>7:3</td><td>-</td><td>-</td><td>-</td><td>保留</td></tr><tr><td>2</td><td>tFAST</td><td>R/W</td><td>0</td><td>SPI flash 读采样模式
0: 上沿采样，间隔半个 SPI 周期
1: 上沿采样，间隔一个 SPI 周期</td></tr><tr><td>1:0</td><td>tCSH</td><td>R/W</td><td>3</td><td>SPI Flash 的片选信号最短无堆时间，以分频后时钟周期 T 计算
00: 1T
01: 2T
10: 4T
11: 8T</td></tr></table>

# 10.4接口时序

# 10.4.1 SPI主控制器接口时序

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/8c6c0caf6589d51be17b6cda3931aa5bb0542abd50b7caedb6963dac0f03ba0c.jpg)  
图10-1SPI主控制器接口时序

# 10.4.2SPIFlash访问时序

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/38da6d693389cad0ca1ce7d7d1c70c997eecc92870512e644ada7cd7cd046156.jpg)  
图10-2SPIFlash标准读时序

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/1d8e23e92b7ff58b895c63ef43fb1ac4472c05e9d4f3ed842f91ab290649bc79.jpg)  
图10-3SPIFlash快速读时序

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/a1671c27c08383e35f2eaeffff2c3ad7364700995e8f53998b0444fb263093f4.jpg)  
图10-4 SPI Flash双向I/O读时序

# 10.5软件编程指南

10.5.1SPI主控制器的读写操作

# 模块初始化

$\bullet$  停止SPI控制器工作，对控制寄存器spcr的spe位写0 $\bullet$  重置状态寄存器spsr，对寄存器写入8b1100_0000 $\bullet$  设置外部寄存器sper，包括中断申请条件sper[7:6]和分频系数sper[1:0]，具体参考寄存器说明 $\bullet$  配置SPI时序，包括spcr的cpol、cpha和sper的mode位。mode为1时是标准SPI实现，为0时为兼容模式。 $\bullet$  配置中断使能，spcr的spie位 $\bullet$  启动SPI控制器，对控制寄存器spcr的spe位写1

# 模块的发送/传输操作

$\bullet$  往数据传输寄存器写入数据 $\bullet$  传输完成后从数据传输寄存器读出数据。由于发送和接收同时进行，即使SPI从设备没有发送有效数据也必须进行读出操作。

# 中断处理

$\bullet$  接收到中断申请 $\bullet$  读状态寄存器spsr的值，若spsr[2]为1则表示数据发送完成，若spsr[0]为1则表示已经接收数据 $\bullet$  读或写数据传输寄存器 $\bullet$  往状态寄存器spsr的spif位写1，清除控制器的中断申请

# 10.5.2硬件SPIFlash读

# 初始化

$\bullet$  将SFC_PARAM的memory_en位写1。当SPI被选为启动设备时此位复位为

1。

- 设置读参数(时钟分频、连续地址读、快速读、双 I/O、tCSH 等)。这些参数复位值均为最保守的值。

# $\bullet$  更改参数

如果所使用的 SPI Flash 支持更高的频率或者提供增强功能，修改相应参数可以大大加快 Flash 的访问速度。参数的修改不需要关闭 SPI Flash 读使能(memory_en)。具体参考寄存器说明。

# 10.5.3 混合访问 SPI Flash 和 SPI 主控制器

# $\bullet$  对 SPI Flash 进行读以外的访问

将 SPI Flash 读使能关闭后，软件就可直接控制  $\mathrm{csn}[0]$ ，并通过 SPI 主控制器访问 SPI 总线。这意味着在进行此操作时，不能从 SPI Flash 中取指。

除了读以外，SPI Flash 还实现了很多命令(如擦除、写入)，具体参见相关 Flash 的文档。

# 11 LocalIO 控制器

# 11.1访问地址及引脚复用

表11-1LocalIO地址空间分配  

<table><tr><td>起始地址</td><td>结束名称</td><td>名称</td><td>说明</td></tr><tr><td>0x1C00_0000</td><td>0x1COF_FFFF</td><td>启动空间</td><td>当引脚设置为 LIO 启动时可访问，其它情况下不可访问</td></tr><tr><td>0x1C10_0000</td><td>0x1FBF_FFFF</td><td rowspan="3">存储空间</td><td rowspan="3"></td></tr><tr><td>0x1FD0_0000</td><td>0x1FDF_FFFF</td></tr><tr><td>0x1FF0_0000</td><td>0x1FFF_FFFF</td></tr></table>

对于LocalIO模块，使用时要注意将对应的引脚设置为相应的功能。

与LocalIO相关的引脚设置寄存器为5.1节中的lio_sel。

# 11.2LocalIO控制器功能概述

LocalIO控制器提供了简单外设访问接口，主要用于连接系统启动ROM。它对外提供一个片选，具有可配置的数据位宽和访问延迟。其中wait参数指lord或liowr信号为低的周期数减一，读写时序可参考图10- 1，图10- 2。当数据位宽为16时，送出的地址由CPU物理地址右移一位得到。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/ee00ce3195661d03d4a8794a4a6e3193d3900833dc1adbd6f1d324ebdbc1a040.jpg)  
图11-1LocalIO读时序

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/3b5580dd2aebd91545819de0e886702005633d42ee648ce709b4bc508eca4d0c.jpg)  
图11-2LocalIO写时序

说明：

- 图中clock信号实际并不存在，只是为了方便时序描述- A_H代表地址bit23-bit29(8位模式)/bit24-bit30(16位模式)- A_M代表地址bit7-bit22(8位模式)/bit8-bit23(16位模式)- A_L代表低7位地址- 在big_mem设置为0时，第4拍波形不存在，第4拍之后的波形向前推一拍- LIO_WRN和LIO_RDN低有效时间与LIO clock_period_i设置有关：LIO clock_period_i设置为1时- 低电平持续8拍；LIO clock_period_i设置为2时- 低电平持续16拍；LIO clock_period_i设置为3/0时- 低电平持续32拍；- 一次CS有效期间可能出现多次读写操作，以上时序图仅作为一种示例。

# 12 DDR3控制器

12 DDR3 控制器龙芯 2K1000 处理器内部集成的内存控制器的设计遵守 DDR3 SDRAM 的行业标准（JESD79- 3）。所实现的所有内存读/写操作都遵守 JESD79- 3 的规定。

# 12.1访问地址

DDR3控制器包括两个地址空间，分别如下：

表12-1内存控制器地址空间分配  

<table><tr><td>起始地址</td><td>结束名称</td><td>名称</td><td>说明</td></tr><tr><td>0x0FF0_0000</td><td>0x0FFF_FFFF</td><td>配置空间</td><td>当 mc_default_reg = 1 时；
或当 mc_default_reg = 0
且 mc_disable_reg = 0 时，为配置空间。其它情况下为内存空间</td></tr><tr><td>其它</td><td></td><td>内存空间</td><td>使用 X2 的窗口配置路由至 DDR 的所有地址</td></tr></table>

具体的mc_default_reg和mc_disable_reg配置请参考表5-2通用配置寄存器0。

# 12.2DDR3SDRAM控制器功能概述

龙芯2K1000处理器支持最大4个CS（由4个片选信号实现，即两个双面内存条），一共含有19位的地址总线（即：16位的行列地址总线和3位的逻辑Bank总线）。

在具体选择使用不同内存芯片类型时，可以调整DDR3控制器参数设置进行支持。其中，支持的最大片选（CS_n）数为4，行地址（RAS_n）数为16，列地址（CAS_n）数为16，逻辑体地址（BANK_n）数为3。

CPU发送的内存请求物理地址可以根据控制器内部不同的配置进行多种不同的地址映射。

龙芯2K1000处理器中内存控制器具有如下特征：

1. 接口上命令、读写数据全流水操作2.内存命令合并、排序提高整体带宽3.配置寄存器读写端口，可以修改内存设备的基本参数4.内建动态延迟补偿电路（DCC），用于数据的可靠发送和接收5.支持133-533MHZ工作频率

# 12.3DDR3SDRAM读操作协议

DDR3SDRAM读操作的协议如图12- 1所示。在图中命令（Command，简称CMD）由RAS_n，CAS_n和WE_n，共三个信号组成。对于读操作，RAS_n=1，CAS_n=0，WE_n=1。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/7f3ada2f21f4ce03a3bdcc512936ca422bef23ab6bdd08f9a3b7d5bcc4d2fcf6.jpg)  
图12-1DDR3SDRAM读操作协议

上图中，Cas Latency  $(\mathrm{CL}) = 5$  ，Read Latency  $(\mathrm{RL}) = 5$  ，Burst Length  $= 8$

# 12.4DDR3SDRAM写操作协议

DDR3SDRAM写操作的协议如图12- 2所示。在图中命令CMD是由RAS_n，CAS_n和WE_n，共三个信号组成的。对于写操作，RAS_n=1，CAS_n=0，WE_n=0。另外，与读操作不同，写操作需要DQM来标识写操作的掩码，即需要写入的字节数。DQM与图中DQs信号同步。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/3f57584f6df68eae34007b3734baab2663b67357964f80a20eb901582d5c4d22.jpg)  
图12-2DDR3SDRAM写操作协议

上图中，Cas Latency  $(\mathrm{CL}) = 5$  ，Write Latency  $(\mathrm{WL}) = 5$  ，Burst Length  $= 8$

# 12.5DDR3控制器寄存器

DDR3控制器的配置寄存器内容如表12- 2所示

表12-2DDR3控制器配置寄存器  

<table><tr><td></td><td>63:56</td><td></td><td>55:48</td><td></td><td>47:40</td><td></td><td>39:32</td><td>31:24</td><td>23:16</td><td>15:8</td><td>7:0</td></tr><tr><td>dx000</td><td>Dll_value_0(RD)/Dll_adj_cnt/dll_sync_d</td><td colspan="5">Dll_value_ek(RD)</td><td colspan="2">Dll_init_done(RD)</td><td colspan="2">Version(RD)</td><td></td></tr><tr><td>dx008</td><td>Dll_value_4(RD)</td><td colspan="5">Dll_value_3(RD)</td><td colspan="2">Dll_value_2(RD)</td><td colspan="2">Dll_value_1(RD)/capability(RD)</td><td></td></tr><tr><td>dx010</td><td>Dll_value_3(RD)</td><td colspan="5">Dll_value_1(RD)</td><td colspan="2">Dll_value_0(RD)</td><td colspan="2">Dll_value_3(RD)</td><td></td></tr><tr><td>dx018</td><td>Dll_ck_3</td><td>Dll_ck_2</td><td>Dll_ck_1</td><td>Dll_ck_0</td><td colspan="2">Dll_increment</td><td colspan="2">Dll_start_point</td><td colspan="2">Dll_bypass</td><td>Init_start</td></tr><tr><td>dx020</td><td>Dq_oe_end_0</td><td>Dq_oe_begin_0</td><td>Dq_stop_edge_0</td><td>Dq_start_edge_0</td><td colspan="2">Rddata_delay_0</td><td colspan="2">Rddqds_1half_0</td><td colspan="2">Wrqds_1half_0</td><td>Wrqds_1half_0</td></tr><tr><td>dx028</td><td>Rd_oe_end_0</td><td>Rd_oe_begin_0</td><td>Rd_stop_edge_0</td><td>Rd_start_edge_0</td><td colspan="2">Dq_s_oe_end_0</td><td colspan="2">Dq_s_oe_begin_0</td><td colspan="2">Dq_s_stop_edge_0</td><td>Dq_s_start_edge_0</td></tr><tr><td>dx030</td><td>Enzi_end_0</td><td>Enzi_begin_0</td><td>Wrqck_sel_0</td><td>Wrqdk_ckdelay_0</td><td colspan="2">Odt_oe_end_0</td><td colspan="2">Odt_oe_begin_0</td><td colspan="2">Odt_stop_edge_0</td><td>Odt_start_edge_0</td></tr><tr><td>dx038</td><td>Enzi_stop_0</td><td>Enzi_start_0</td><td>Dll_oe_shorten_0</td><td>Dll_rrdqds_p_0</td><td colspan="2">Dll_rrdqds_p_0</td><td colspan="2">Dll_wrdqds_0</td><td colspan="2">Dll_wrdqds_0</td><td>Dll_gate_0</td></tr><tr><td>dx040</td><td>Dq_oe_end_1</td><td>Dq_oe_begin_1</td><td>Dq_stop_edge_1</td><td>Dq_start_edge_1</td><td colspan="2">Rddata_delay_1</td><td colspan="2">Rddqds_1half_1</td><td colspan="2">Wrqds_1half_1</td><td>Wrqds_1half_1</td></tr><tr><td>dx048</td><td>Rd_oe_end_1</td><td>Rd_oe_begin_1</td><td>Rd_stop_edge_1</td><td>Rd_start_edge_1</td><td colspan="2">Dq_s_oe_end_1</td><td colspan="2">Dq_s_oe_begin_1</td><td colspan="2">Dq_s_stop_edge_1</td><td>Dq_s_start_edge_1</td></tr><tr><td>dx050</td><td>Enzi_end_1</td><td>Enzi_begin_1</td><td>Wrqck_sel_1</td><td>Wrqdk_ckdelay_1</td><td colspan="2">Odt_oe_end_1</td><td colspan="2">Odt_oe_begin_1</td><td colspan="2">Odt_stop_edge_1</td><td>Odt_start_edge_1</td></tr></table>

<table><tr><td></td><td colspan="2">63:56</td><td colspan="2">55:48</td><td colspan="2">47:40</td><td colspan="2">39:32</td><td colspan="2">31:24</td><td colspan="2">23:16</td><td colspan="2">15:8</td><td colspan="2">7:0</td></tr><tr><td>0x058</td><td colspan="2">Enzi_stop_1</td><td colspan="2">Enzi_start_1</td><td colspan="2">Dll_oe_shorten_1</td><td colspan="2">Dll_dddqs_n_1</td><td colspan="2">Dll_dddqs_p_1</td><td colspan="2">Dll_wrds_1</td><td colspan="2">Dll_wrds_1</td><td colspan="2">Dll_gate_1</td></tr><tr><td>0x060</td><td colspan="2">Dq_oe_end_2</td><td colspan="2">Dq_oe_begin_2</td><td colspan="2">Dq_stop_edge_2</td><td colspan="2">Dq_start_edge_2</td><td colspan="2">Rddata_delay_2</td><td colspan="2">Rddqs_1half_2</td><td colspan="2">Wrds_1half_2</td><td colspan="2">Wrds_1half_2</td></tr><tr><td>0x068</td><td colspan="2">Rd_oe_end_2</td><td colspan="2">Rd_oe_begin_2</td><td colspan="2">Rd_stop_edge_2</td><td colspan="2">Rd_start_edge_2</td><td colspan="2">Dq_oe_end_2</td><td colspan="2">Dq_oe_begin_2</td><td colspan="2">Dq_stop_edge_2</td><td colspan="2">Dq_start_edge_2</td></tr><tr><td>0x070</td><td colspan="2">Enzi_end_2</td><td colspan="2">Enzi_begin_2</td><td colspan="2">Wrds_1half_2</td><td colspan="2">Wrds_1delay_2</td><td colspan="2">Odt_oe_end_2</td><td colspan="2">Odt_oe_begin_2</td><td colspan="2">Odt_stop_edge_2</td><td colspan="2">Odt_start_edge_2</td></tr><tr><td>0x078</td><td colspan="2">Enzi_stop_2</td><td colspan="2">Enzi_start_2</td><td colspan="2">Dll_oe_shorten_2</td><td colspan="2">Dll_dddqs_n_2</td><td colspan="2">Dll_dddqs_p_2</td><td colspan="2">Dll_wrds_2</td><td colspan="2">Dll_wrds_2</td><td colspan="2">Dll_gate_2</td></tr><tr><td>0x080</td><td colspan="2">Dq_oe_end_3</td><td colspan="2">Dq_oe_begin_3</td><td colspan="2">Dq_stop_edge_3</td><td colspan="2">Dq_start_edge_3</td><td colspan="2">Rddata_delay_3</td><td colspan="2">Rddqs_1half_3</td><td colspan="2">Wrds_1half_3</td><td colspan="2">Wrds_1half_3</td></tr><tr><td>0x088</td><td colspan="2">Rd_oe_end_3</td><td colspan="2">Rd_oe_begin_3</td><td colspan="2">Rd_stop_edge_3</td><td colspan="2">Rd_start_edge_3</td><td colspan="2">Dq_oe_end_3</td><td colspan="2">Dq_oe_begin_3</td><td colspan="2">Dq_stop_edge_3</td><td colspan="2">Dq_start_edge_3</td></tr><tr><td>0x090</td><td colspan="2">Enzi_end_3</td><td colspan="2">Enzi_begin_3</td><td colspan="2">Wrds_1half_3</td><td colspan="2">Wrds_1delay_3</td><td colspan="2">Odt_oe_end_3</td><td colspan="2">Odt_oe_begin_3</td><td colspan="2">Odt_stop_edge_3</td><td colspan="2">Odt_start_edge_3</td></tr><tr><td>0x098</td><td colspan="2">Enzi_stop_3</td><td colspan="2">Enzi_start_3</td><td colspan="2">Dll_oe_shorten_3</td><td colspan="2">Dll_dddqs_n_3</td><td colspan="2">Dll_dddqs_p_3</td><td colspan="2">Dll_wrds_3</td><td colspan="2">Dll_wrds_3</td><td colspan="2">Dll_gate_3</td></tr><tr><td>0x0A0</td><td colspan="2">Dq_oe_end_4</td><td colspan="2">Dq_oe_begin_4</td><td colspan="2">Dq_stop_edge_4</td><td colspan="2">Dq_start_edge_4</td><td colspan="2">Rddata_delay_4</td><td colspan="2">Rddqs_1half_4</td><td colspan="2">Wrds_1half_4</td><td colspan="2">Wrds_1half_4</td></tr><tr><td>0x0A8</td><td colspan="2">Rd_oe_end_4</td><td colspan="2">Rd_oe_begin_4</td><td colspan="2">Rd_stop_edge_4</td><td colspan="2">Rd_start_edge_4</td><td colspan="2">Dq_oe_end_4</td><td colspan="2">Dq_oe_begin_4</td><td colspan="2">Dq_stop_edge_4</td><td colspan="2">Dq_start_edge_4</td></tr><tr><td>0x0B0</td><td colspan="2">Enzi_end_4</td><td colspan="2">Enzi_begin_4</td><td colspan="2">Wrds_1half_4</td><td colspan="2">Wrds_1delay_4</td><td colspan="2">Odt_oe_end_4</td><td colspan="2">Odt_oe_begin_4</td><td colspan="2">Odt_stop_edge_4</td><td colspan="2">Odt_start_edge_4</td></tr><tr><td>0x0B8</td><td colspan="2">Enzi_stop_4</td><td colspan="2">Enzi_start_4</td><td colspan="2">Dll_oe_shorten_4</td><td colspan="2">Dll_dddqs_n_4</td><td colspan="2">Dll_dddqs_p_4</td><td colspan="2">Dll_wrds_4</td><td colspan="2">Dll_wrds_4</td><td colspan="2">Dll_gate_4</td></tr><tr><td>0x0C0</td><td colspan="2">Dq_oe_end_5</td><td colspan="2">Dq_oe_begin_5</td><td colspan="2">Dq_stop_edge_5</td><td colspan="2">Dq_start_edge_5</td><td colspan="2">Rddata_delay_5</td><td colspan="2">Rddqs_1half_5</td><td colspan="2">Wrds_1half_5</td><td colspan="2">Wrds_1half_5</td></tr><tr><td>0x0C8</td><td colspan="2">Rd_oe_end_5</td><td colspan="2">Rd_oe_begin_5</td><td colspan="2">Rd_stop_edge_5</td><td colspan="2">Rd_start_edge_5</td><td colspan="2">Dq_oe_end_5</td><td colspan="2">Dq_oe_begin_5</td><td colspan="2">Dq_stop_edge_5</td><td colspan="2">Dq_start_edge_5</td></tr><tr><td>0x0D0</td><td colspan="2">Enzi_end_5</td><td colspan="2">Enzi_begin_5</td><td colspan="2">Wrds_1half_5</td><td colspan="2">Wrds_1delay_5</td><td colspan="2">Odt_oe_end_5</td><td colspan="2">Odt_oe_begin_5</td><td colspan="2">Odt_stop_edge_5</td><td colspan="2">Odt_start_edge_5</td></tr><tr><td>0x0D8</td><td colspan="2">Enzi_stop_5</td><td colspan="2">Enzi_start_5</td><td colspan="2">Dll_oe_shorten_5</td><td colspan="2">Dll_dddqs_n_5</td><td colspan="2">Dll_dddqs_p_5</td><td colspan="2">Dll_wrds_5</td><td colspan="2">Dll_wrds_5</td><td colspan="2">Dll_gate_5</td></tr><tr><td>0x0E0</td><td colspan="2">Dq_oe_end_6</td><td colspan="2">Dq_oe_begin_6</td><td colspan="2">Dq_stop_edge_6</td><td colspan="2">Dq_start_edge_6</td><td colspan="2">Rddata_delay_6</td><td colspan="2">Rddqs_1half_6</td><td colspan="2">Wrds_1half_6</td><td colspan="2">Wrds_1half_6</td></tr><tr><td>0x0E8</td><td colspan="2">Rd_oe_end_6</td><td colspan="2">Rd_oe_begin_6</td><td colspan="2">Rd_stop_edge_6</td><td colspan="2">Rd_start_edge_6</td><td colspan="2">Dq_oe_end_6</td><td colspan="2">Dq_oe_begin_6</td><td colspan="2">Dq_stop_edge_6</td><td colspan="2">Dq_start_edge_6</td></tr><tr><td>0x0F0</td><td colspan="2">Enzi_end_6</td><td colspan="2">Enzi_begin_6</td><td colspan="2">Wrds_1half_6</td><td colspan="2">Wrds_1delay_6</td><td colspan="2">Odt_oe_end_6</td><td colspan="2">Odt_oe_begin_6</td><td colspan="2">Odt_stop_edge_6</td><td colspan="2">Odt_start_edge_6</td></tr><tr><td>0x0F8</td><td colspan="2">Enzi_stop_6</td><td colspan="2">Enzi_start_6</td><td colspan="2">Dll_oe_shorten_6</td><td colspan="2">Dll_dddqs_n_6</td><td colspan="2">Dll_dddqs_p_6</td><td colspan="2">Dll_wrds_6</td><td colspan="2">Dll_wrds_6</td><td colspan="2">Dll_gate_6</td></tr><tr><td>0x100</td><td colspan="2">Dq_oe_end_7</td><td colspan="2">Dq_oe_begin_7</td><td colspan="2">Dq_stop_edge_7</td><td colspan="2">Dq_start_edge_7</td><td colspan="2">Rddata_delay_7</td><td colspan="2">Rddqs_1half_7</td><td colspan="2">Wrds_1half_7</td><td colspan="2">Wrds_1half_7</td></tr><tr><td>0x108</td><td colspan="2">Rd_oe_end_7</td><td colspan="2">Rd_oe_begin_7</td><td colspan="2">Rd_stop_edge_7</td><td colspan="2">Rd_start_edge_7</td><td colspan="2">Dq_oe_end_7</td><td colspan="2">Dq_oe_begin_7</td><td colspan="2">Dq_stop_edge_7</td><td colspan="2">Dq_start_edge_7</td></tr><tr><td>0x110</td><td colspan="2">Enzi_end_7</td><td colspan="2">Enzi_begin_7</td><td colspan="2">Wrds_1half_7</td><td colspan="2">Wrds_1delay_7</td><td colspan="2">Odt_oe_end_7</td><td colspan="2">Odt_oe_begin_7</td><td colspan="2">Odt_stop_edge_7</td><td colspan="2">Odt_start_edge_7</td></tr><tr><td>0x118</td><td colspan="2">Enzi_stop_7</td><td colspan="2">Enzi_start_7</td><td colspan="2">Dll_oe_shorten_7</td><td colspan="2">Dll_dddqs_n_7</td><td colspan="2">Dll_dddqs_p_7</td><td colspan="2">Dll_wrds_7</td><td colspan="2">Dll_wrds_7</td><td colspan="2">Dll_gate_7</td></tr><tr><td>0x120</td><td colspan="2">Dq_oe_end_8</td><td colspan="2">Dq_oe_begin_8</td><td colspan="2">Dq_stop_edge_8</td><td colspan="2">Dq_start_edge_8</td><td colspan="2">Rddata_delay_8</td><td colspan="2">Rddqs_1half_8</td><td colspan="2">Wrds_1half_8</td><td colspan="2">Wrds_1half_8</td></tr><tr><td>0x128</td><td colspan="2">Rd_oe_end_8</td><td colspan="2">Rd_oe_begin_8</td><td colspan="2">Rd_stop_edge_8</td><td colspan="2">Rd_start_edge_8</td><td colspan="2">Dq_oe_end_8</td><td colspan="2">Dq_oe_begin_8</td><td colspan="2">Dq_stop_edge_8</td><td colspan="2">Dq_start_edge_8</td></tr><tr><td>0x130</td><td colspan="2">Enzi_end_8</td><td colspan="2">Enzi_begin_8</td><td colspan="2">Wrds_1half_8</td><td colspan="2">Wrds_1delay_8</td><td colspan="2">Odt_oe_end_8</td><td colspan="2">Odt_oe_begin_8</td><td colspan="2">Odt_stop_edge_8</td><td colspan="2">Odt_start_edge_8</td></tr><tr><td>0x138</td><td colspan="2">Enzi_stop_8</td><td colspan="2">Enzi_start_8</td><td colspan="2">Dll_oe_shorten_8</td><td colspan="2">Dll_dddqs_n_8</td><td colspan="2">Dll_dddqs_p_8</td><td colspan="2">Dll_wrds_8</td><td colspan="2">Dll_wrds_8</td><td colspan="2">Dll_gate_8</td></tr><tr><td>0x140</td><td colspan="2">Pad_ocd_clk</td><td colspan="2">Pad_ocd_ctl</td><td colspan="2">Pad_ocd_dqs</td><td colspan="2">Pad_ocd_dq</td><td colspan="2">Pad_enzi</td><td colspan="2">Pad_en_ctl</td><td colspan="2">Pad_en_ctl</td><td colspan="2">Pad_en_clk</td></tr><tr><td>0x148</td><td colspan="2">Pad_adj_code_dqs</td><td colspan="2">Pad_code_dq</td><td colspan="2">Pad_adj_code_dq</td><td colspan="2">Pad_code_dq</td><td colspan="2">Pad_adjInternal</td><td colspan="2">Pad_adj_code</td><td colspan="2">Pad_modez1v8</td><td colspan="2">Pad_modez1v8</td></tr><tr><td>0x150</td><td colspan="2"></td><td colspan="2">Pad_reset_po</td><td colspan="2">Pad_adj_code_clk</td><td colspan="2">Pad_code_1k</td><td colspan="2">Pad_adj_code_cmd</td><td colspan="2">Pad_code_cmd</td><td colspan="2">Pad_adj_code_addr</td><td colspan="2">Pad_code_addr</td></tr><tr><td>0x158</td><td colspan="2"></td><td colspan="2">Pad_comp_code_o</td><td colspan="2">Pad_comp_okn</td><td colspan="2">Pad_comp_code_i</td><td colspan="2">Pad_comp_mode</td><td colspan="2">Pad_comp_tm</td><td colspan="2">Pad_comp_pd</td><td colspan="2">Pad_comp_pd</td></tr><tr><td>0x160</td><td colspan="4">Rdinfo_empty(RD)</td><td colspan="4">Overflow(RD)</td><td colspan="2">Dram_init(RD)</td><td colspan="2">Rdinfo_valid</td><td colspan="2">Cmd_timming</td><td colspan="2">Ddr3_mode</td></tr><tr><td>0x168</td><td colspan="2">Ba_xor_row_offset</td><td colspan="2">AddrMirror</td><td colspan="2">Cmd_delay</td><td colspan="2">Burst_length</td><td colspan="2">Bank/Cs_resync</td><td colspan="2">Cs_zq</td><td colspan="2">Cs_mrs</td><td colspan="2">Cs_enable</td></tr><tr><td>0x170</td><td colspan="4">Odt_wr_cs_map</td><td colspan="2">Odt_wr_length</td><td colspan="2">Odt_wr_delay</td><td colspan="2">Odt_rd_cs_map</td><td colspan="2">Odt_rd_length</td><td colspan="2">Odt_rd_delay</td><td></td><td></td></tr><tr><td>0x178</td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td></tr><tr><td>0x180</td><td colspan="2">Lvl_resp_0(RD)</td><td colspan="2">Lvl_doe(RD)</td><td colspan="2">Lvl_ready(RD)</td><td colspan="2">Lvl_cs</td><td colspan="2">tLVL_DELAY</td><td colspan="2">Lvl_req(WR)</td><td colspan="2">Lvl_mode</td><td></td><td></td></tr><tr><td>0x188</td><td colspan="2">Lvl_resp_8(RD)</td><td colspan="2">Lvl_resp_7(RD)</td><td colspan="2">Lvl_resp_6(RD)</td><td colspan="2">Lvl_resp_5(RD)</td><td colspan="2">Lvl_resp_4(RD)</td><td colspan="2">Lvl_resp_3(RD)</td><td colspan="2">Lvl_resp_2(RD)</td><td colspan="2">Lvl_resp_1(RD)</td></tr><tr><td>0x190</td><td colspan="4">Cmd_a</td><td colspan="2">Cmd_ba</td><td colspan="2">Cmd_cmd</td><td colspan="2">Cmd_cs</td><td colspan="2">Status_cmd(RD)</td><td colspan="2">Cmd_req(WR)</td><td colspan="2">Command_mode</td></tr><tr><td>0x198</td><td colspan="2"></td><td colspan="2"></td><td colspan="2">Status_sref(RD)</td><td colspan="2">Srefresh_req</td><td colspan="2">Pre_all_done(RD)</td><td colspan="2">Pre_all_req(RD)</td><td colspan="2">Mrs_done(RD)</td><td colspan="2">Mrs_req(WR)</td></tr></table>

<table><tr><td></td><td>63:56</td><td>55:48</td><td>47:40</td><td>39:32</td><td>31:24</td><td>23:16</td><td>15:8</td><td>7:0</td></tr><tr><td>0x1A0</td><td>Mr_3_cs_0</td><td>Mr_2_cs_0</td><td>Mr_1_cs_0</td><td>Mr_0_cs_0</td><td></td><td></td><td></td><td></td></tr><tr><td>0x1A8</td><td>Mr_3_cs_1</td><td>Mr_2_cs_1</td><td>Mr_1_cs_1</td><td>Mr_0_cs_1</td><td></td><td></td><td></td><td></td></tr><tr><td>0x1B0</td><td>Mr_3_cs_2</td><td>Mr_2_cs_2</td><td>Mr_1_cs_2</td><td>Mr_0_cs_2</td><td></td><td></td><td></td><td></td></tr><tr><td>0x1B8</td><td>Mr_3_cs_3</td><td>Mr_2_cs_3</td><td>Mr_1_cs_3</td><td>Mr_0_cs_3</td><td></td><td></td><td></td><td></td></tr><tr><td>0x1C0</td><td>tRESET</td><td>tCKE</td><td>xPR</td><td>tMOD</td><td>tZQCL</td><td>tZA1/MID</td><td>tWLQSEN</td><td>tRDDATA</td></tr><tr><td>0x1C8</td><td>tFAW</td><td>tRRD</td><td>tRCD</td><td>tRP</td><td>tREF</td><td>tRFC</td><td>tZQCS</td><td>tZQperiod</td></tr><tr><td>0x1D0</td><td>tODTL</td><td>tXSRD</td><td>tPHY_RDLAT</td><td>tPHY_WRLAT</td><td>tRAS_max</td><td></td><td></td><td>tRAS_min</td></tr><tr><td>0x1D8</td><td>tXPDLL</td><td>tXP</td><td>tWR</td><td>tRTP</td><td>tRL</td><td>tWL</td><td>tCCD</td><td>tWTR</td></tr><tr><td>0x1E0</td><td>tW2R_diffCS</td><td>tW2W_diffCS</td><td>tR2P_sameBA</td><td>tW2P_sameBA</td><td>tR2R_sameBA</td><td>tR2W_sameBA</td><td>tW2R_sameBA</td><td>tW2W_sameBA</td></tr><tr><td>0x1E8</td><td>tR2R_diffCS</td><td>tR2W_diffCS</td><td>tR2P_sameCS</td><td>tW2P_sameCS</td><td>tR2R_sameCS</td><td>tR2W_sameCS</td><td>tW2R_sameCS</td><td>tW2W_sameCS</td></tr><tr><td>0x1F0</td><td>Power_up</td><td>Age_step</td><td>tCPDED</td><td>Cs_map</td><td>Bs_config</td><td>Nc</td><td>Pr_r2w</td><td>Placement_en</td></tr><tr><td>0x1F8</td><td>Hw_pd_3</td><td>Hw_pd_2</td><td>Hw_pd_1</td><td>Hw_pd_0</td><td>Credit_16</td><td>Credit_32</td><td>Credit_64</td><td>Selection_en</td></tr><tr><td>0x200</td><td>Cmdq_age_16</td><td colspan="2">Cmdq_age_32</td><td colspan="2">Cmdq_age_64</td><td>tCKESR</td><td>tRDDDEN</td><td></td></tr><tr><td>0x208</td><td>Wifio_age</td><td colspan="2">Rifio_age</td><td>Power_stat3</td><td>Power_stat2</td><td>Power_stat1</td><td>Power_stat0</td><td></td></tr><tr><td>0x210</td><td>Active_age</td><td>Cs_place_0</td><td>Addr_win_0</td><td>Cs_diff_0</td><td>Row_diff_0</td><td>Ba_diff_0</td><td>Col_diff_0</td><td></td></tr><tr><td>0x218</td><td>Fastpd_age</td><td>Cs_place_1</td><td>Addr_win_1</td><td>Cs_diff_1</td><td>Row_diff_1</td><td>Ba_diff_1</td><td>Col_diff_1</td><td></td></tr><tr><td>0x220</td><td>Slowpd_age</td><td>Cs_place_2</td><td>Addr_win_2</td><td>Cs_diff_2</td><td>Row_diff_2</td><td>Ba_diff_2</td><td>Col_diff_2</td><td></td></tr><tr><td>0x228</td><td>Selfref_age</td><td>Cs_place_3</td><td>Addr_win_3</td><td>Cs_diff_3</td><td>Row_diff_3</td><td>Ba_diff_3</td><td>Col_diff_3</td><td></td></tr><tr><td>0x230</td><td>Win_mask_0</td><td colspan="2"></td><td colspan="5">Win_base_0</td></tr><tr><td>0x238</td><td>Win_mask_1</td><td colspan="2"></td><td colspan="5">Win_base_1</td></tr><tr><td>0x240</td><td>Win_mask_2</td><td colspan="2"></td><td colspan="5">Win_base_2</td></tr><tr><td>0x248</td><td>Win_mask_3</td><td colspan="2"></td><td colspan="5">Win_base_3</td></tr><tr><td>0x250</td><td></td><td>Cmd_monitor</td><td colspan="2">Axi_monitor</td><td>Ecc_code(RD)</td><td>Ecc_enable</td><td>Int_vector</td><td>Int_enable</td></tr><tr><td>0x258</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr><tr><td>0x260</td><td colspan="8">Ecc_addr(RD)</td></tr><tr><td>0x268</td><td colspan="8">Ecc_data(RD)</td></tr><tr><td>0x270</td><td>Lpbk_ecc_mask(RD)</td><td colspan="2">Prbs_init</td><td>Lpbk_error(RD)</td><td>Prbs_23</td><td>Lpbk_start</td><td>Lpbk_en</td><td></td></tr><tr><td>0x278</td><td>Lpbk_ecc(RD)</td><td colspan="2">Lpbk_data_mask(RD)</td><td>Lpbk_correct(RD)</td><td>Lpbk_counter(RD)</td><td></td><td></td><td></td></tr><tr><td>0x280</td><td colspan="8">Lpbk_data_r(RD)</td></tr><tr><td>0x288</td><td colspan="8">Lpbk_data_f(RD)</td></tr><tr><td>0x290</td><td colspan="3">Axi0_bandwidth_w</td><td colspan="5">Axi0_bandwidth_r</td></tr><tr><td>0x298</td><td colspan="3">Axi0_latency_w</td><td colspan="5">Axi0_latency_r</td></tr><tr><td>0x2A0</td><td colspan="3">Axi1_bandwidth_w</td><td colspan="5">Axi1_bandwidth_r</td></tr><tr><td>0x2A8</td><td colspan="3">Axi1_latency_w</td><td colspan="5">Axi1_latency_r</td></tr><tr><td>0x2B0</td><td colspan="3">Axi2_bandwidth_w</td><td colspan="5">Axi2_bandwidth_r</td></tr><tr><td>0x2B8</td><td colspan="3">Axi2_latency_w</td><td colspan="5">Axi2_latency_r</td></tr><tr><td>0x2C0</td><td colspan="3">Axi3_bandwidth_w</td><td colspan="5">Axi3_bandwidth_r</td></tr><tr><td>0x2C8</td><td colspan="3">Axi3_latency_w</td><td colspan="5">Axi3_latency_r</td></tr><tr><td>0x2D0</td><td colspan="3">Axi4_bandwidth_w</td><td colspan="5">Axi4_bandwidth_r</td></tr><tr><td>0x2D8</td><td colspan="3">Axi4_latency_w</td><td colspan="5">Axi4_latency_r</td></tr><tr><td>0x2E0</td><td colspan="3">Cmdq0_bandwidth_w</td><td colspan="5">Cmdq0_bandwidth_r</td></tr></table>

<table><tr><td></td><td colspan="2">63:56</td><td colspan="2">55:48</td><td colspan="2">47:40</td><td colspan="2">39:32</td><td colspan="2">31:24</td><td colspan="2">23:16</td><td colspan="2">15:8</td><td colspan="2">7:0</td></tr><tr><td>0x2E8</td><td colspan="8">Cmdq0_latency_w</td><td colspan="8">Cmdq0_latency_r</td></tr><tr><td>0x2F0</td><td colspan="8">Cmdq1_bandwidth_w</td><td colspan="8">Cmdq1_bandwidth_r</td></tr><tr><td>0x2F8</td><td colspan="8">Cmdq1_latency_w</td><td colspan="8">Cmdq1_latency_r</td></tr><tr><td>0x300</td><td colspan="8">Cmdq2_bandwidth_w</td><td colspan="8">Cmdq2_bandwidth_r</td></tr><tr><td>0x308</td><td colspan="8">Cmdq2_latency_w</td><td colspan="8">Cmdq2_latency_r</td></tr><tr><td>0x310</td><td colspan="8">Cmdq3_bandwidth_w</td><td colspan="8">Cmdq3_bandwidth_r</td></tr><tr><td>0x318</td><td colspan="8">Cmdq3_latency_w</td><td colspan="8">Cmdq3_latency_r</td></tr><tr><td>0x320</td><td>tRESYNC_length</td><td colspan="2">tRESYNC_shift</td><td colspan="2">tRESYNC_max</td><td colspan="2">tRESYNC_min</td><td colspan="3">Pre_predict</td><td colspan="2">tX3</td><td colspan="3">tREP_low</td><td></td></tr><tr><td>0x328</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="3"></td><td colspan="2"></td><td colspan="3">tRESYNC_delay</td><td></td></tr><tr><td>0x330</td><td>Stat_en</td><td colspan="2">Rbuffer_max</td><td colspan="2">Retry</td><td colspan="2">Wr_pkg_num</td><td colspan="2">Rwq_rb</td><td colspan="2">Stb_en</td><td colspan="2">Addr_new</td><td colspan="3">tRDQidle</td></tr><tr><td>0x338</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2">Rd_fifo_depth</td><td colspan="9">Retry_cnt</td></tr><tr><td>0x340</td><td colspan="7">tREFretention</td><td colspan="2"></td><td colspan="2">Ref_num</td><td colspan="2">tREF_IDLE</td><td colspan="3">Ref_sch_en</td></tr><tr><td>0x348</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="3"></td></tr><tr><td>0x350</td><td colspan="16">Lpbk_data_en</td></tr><tr><td>0x358</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2">Lpbk_ecc_mask_en</td><td colspan="2">Lpbk_ecc_en</td><td colspan="3">Lpbk_data_mask_en</td></tr><tr><td>0x360</td><td></td><td colspan="2"></td><td colspan="2">Int_ecc_cnt Fatal</td><td colspan="2">Int_ecc_cnt_error</td><td colspan="2">Ecc_cnt_cs_3</td><td colspan="2">Ecc_cnt_cs_2</td><td colspan="2">Ecc_cnt_cs_1</td><td colspan="3">Ecc_cnt_cs_0</td></tr><tr><td>0x368</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="3"></td></tr><tr><td>0x370</td><td colspan="3">Prior_age3</td><td colspan="4">Prior_age2</td><td colspan="3">Prior_age1</td><td colspan="6">Prior_age_0</td></tr><tr><td>0x378</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2">No_dead_inorder</td><td colspan="3">Row_hit_place</td></tr><tr><td>0x380</td><td colspan="7">Zq_cnt_1</td><td colspan="9">Zq_cnt_0</td></tr><tr><td>0x388</td><td colspan="7">Zq_cnt_3</td><td colspan="9">Zq_cnt_2</td></tr><tr><td>0x390</td><td></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="2"></td><td colspan="3">Nc16_map</td></tr><tr><td>0x398</td><td colspan="7"></td><td colspan="9"></td></tr></table>

# 13 GPIO

13 GPIO龙芯2K1000共有60个GPIO引脚，4个为专用GPIO，其余56个与其他功能复用。下面具体介绍与GPIO相关的配置寄存器。

表13-1GPIO配置寄存器  

<table><tr><td>地址</td><td>名称</td><td>描述</td></tr><tr><td>0x1fe00500</td><td>GPIO0_OEN</td><td>GPIO 输出使能，低有效</td></tr><tr><td>0x1fe00508</td><td>GPIO1_OEN</td><td>保留</td></tr><tr><td>0x1fe00510</td><td>GPIO0_O</td><td>GPIO 输出值</td></tr><tr><td>0x1fe00518</td><td>GPIO1_O</td><td>保留</td></tr><tr><td>0x1fe00520</td><td>GPIO0_I</td><td>GPIO 输入值</td></tr><tr><td>0x1fe00528</td><td>GPIO1_I</td><td>保留</td></tr><tr><td>0x1fe00530</td><td>GPIO0_INTEN</td><td>GPIO 的低 64 位中断使能</td></tr><tr><td>0x1fe00538</td><td>GPIO1_INTEN</td><td>保留</td></tr></table>

# 13.1GPIO方向控制

地址：0x1fe00500

表13-2GPIO方向控制  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>63:0</td><td>GPIO0_IOEN</td><td>R/W</td><td>64&#x27;hffff_ffff_ffff_ffff</td><td>0为输出，1为输入</td></tr></table>

# 13.2GPIO输出设置

地址：0x1fe00510

表13-3GPIO输出设置  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>63:0</td><td>GPIO0_O</td><td>R/W</td><td>0</td><td>0 输出低电平，1 输出高电平</td></tr></table>

# 13.3GPIO输入采样

地址：0x1fe00520

表13-4GPIO输入采样  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>63:0</td><td>GPIO0_I</td><td>R</td><td>-</td><td>反映 GPIO 引脚的值</td></tr></table>

# 13.4GPIO中断使能

地址：0x1fe00530

表13-5GPIO中断使能  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>初值</td><td>描述</td></tr><tr><td>63:0</td><td>GPIO0_INTEN</td><td>R/W</td><td>0</td><td>中断使能位，每一位对应一个 GPIO 引脚</td></tr></table>

# 13.5GPIO复用关系

GPIO与其他功能的复用关系如下表所示：

表13-6GPIO复用关系  

<table><tr><td>GPIO编号</td><td>复用信号</td><td>备注</td></tr><tr><td>63</td><td>NAND_D7</td><td rowspan="20">默认为 GPIO 功能，使用 NAND 时需要设置 nand_sel 为 1，nand_sel 配置参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>62</td><td>NAND_D6</td></tr><tr><td>61</td><td>NAND_D5</td></tr><tr><td>60</td><td>NAND_D4</td></tr><tr><td>59</td><td>NAND_D3</td></tr><tr><td>58</td><td>NAND_D2</td></tr><tr><td>57</td><td>NAND_D1</td></tr><tr><td>56</td><td>NAND_D0</td></tr><tr><td>55</td><td>NAND_RDYn3</td></tr><tr><td>54</td><td>NAND_RDYn2</td></tr><tr><td>53</td><td>NAND_RDYn1</td></tr><tr><td>52</td><td>NAND_RDYn0</td></tr><tr><td>51</td><td>NAND_RDn</td></tr><tr><td>50</td><td>NAND_WRn</td></tr><tr><td>49</td><td>NAND_ALE</td></tr><tr><td>48</td><td>NAND_CLE</td></tr><tr><td>47</td><td>NAND_CEn3</td></tr><tr><td>46</td><td>NAND_CEn2</td></tr><tr><td>45</td><td>NAND_CEn1</td></tr><tr><td>44</td><td>NAND_CEn0</td></tr><tr><td>43</td><td>-</td><td>保留</td></tr><tr><td>42</td><td>-</td><td>保留</td></tr><tr><td>41</td><td>SDIO_CLK</td><td rowspan="6">默认为 GPIO 功能，使用 SDIO 时需要设置 sdio_sel[1]为 1，sdio_sel 配置参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>40</td><td>SDIO_CMD</td></tr><tr><td>39</td><td>SDIO_DATA3</td></tr><tr><td>38</td><td>SDIO_DATA2</td></tr><tr><td>37</td><td>SDIO_DATA1</td></tr><tr><td>36</td><td>SDIO_DATA0</td></tr><tr><td>35</td><td>CAN1_TX</td><td rowspan="2">默认为 GPIO 功能，使用 CAN 时需要设置 can_sel[1]为 1，can_sel 配置参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>34</td><td>CAN1_RX</td></tr><tr><td>33</td><td>CAN0_TX</td><td rowspan="2">默认为 GPIO 功能，使用 CAN 时需要设置 can_sel[1]为 1，can_sel 配置参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>32</td><td>CAN0_RX</td></tr><tr><td>31</td><td>-</td><td>保留</td></tr><tr><td>30</td><td>HDA_SDI2</td><td>默认为 GPIO 功能，使用 HDA 时需要设置 hda_sel 为 1，参</td></tr></table>

<table><tr><td>29</td><td>HDA_SD11</td><td rowspan="6">考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>28</td><td>HDA_SD10</td></tr><tr><td>27</td><td>HDA_SDO</td></tr><tr><td>26</td><td>HDA_RESETn</td></tr><tr><td>25</td><td>HDA_SYNC</td></tr><tr><td>24</td><td>HDA_BITCLK</td></tr><tr><td>23</td><td>PWM3</td><td>默认为 GPIO 功能，使用 PWM 时需要设置 pwm_sel[3]为 1，参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>22</td><td>PWM2</td><td>默认为 GPIO 功能，使用 PWM 时需要设置 pwm_sel[2]为 1，参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>21</td><td>PWM1</td><td>默认为 GPIO 功能，使用 PWM 时需要设置 pwm_sel[1]为 1，参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>20</td><td>PWM0</td><td>默认为 GPIO 功能，使用 PWM 时需要设置 pwm_sel[0]为 1，参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>19</td><td>I2C1_SDA</td><td rowspan="2">默认为 GPIO 功能，使用 I2C 时需要设置 i2c_sel[1]为 1，参考表 5-2 通用配置寄存器 0 设置。</td></tr><tr><td>18</td><td>I2C1_SCL</td></tr><tr><td>17</td><td>I2C0_SDA</td><td rowspan="2">默认为 GPIO 功能，使用 I2C 时需要设置 i2c_sel[1]为 1，参考表 5-2 通用配置寄存器 0 设置</td></tr><tr><td>16</td><td>I2C0_SCL</td></tr><tr><td>15</td><td>-</td><td>保留</td></tr><tr><td>14</td><td>SATA_LEDn</td><td rowspan="11">默认为 GPIO 功能，使用 SATA 时需要设置 sata_sel 为 1，参考表 5-2 通用配置寄存器 0 设置</td></tr><tr><td>13</td><td>GMAC1_TCTL</td></tr><tr><td>12</td><td>GMAC1_TXD3</td></tr><tr><td>11</td><td>GMAC1_TXD2</td></tr><tr><td>10</td><td>GMAC1_TXD1</td></tr><tr><td>9</td><td>GMAC1_TXD0</td></tr><tr><td>8</td><td>GMAC1_RCTL</td></tr><tr><td>7</td><td>GMAC1_RXD3</td></tr><tr><td>6</td><td>GMAC1_RXD2</td></tr><tr><td>5</td><td>GMAC1_RXD1</td></tr><tr><td>4</td><td>GMAC1_RXD0</td></tr><tr><td>3</td><td>无复用</td><td>专用 GPIO 引脚</td></tr><tr><td>2</td><td>无复用</td><td>专用 GPIO 引脚</td></tr><tr><td>1</td><td>无复用</td><td>专用 GPIO 引脚</td></tr><tr><td>0</td><td>无复用</td><td>专用 GPIO 引脚</td></tr></table>

# 14 APB 设备 (Dev2)

APB设备的配置空间基本信息如下：

表14-1APB配置访问信息  

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>APB</td><td>0x0</td><td>0x2</td><td>0x0</td><td>0xFFFF</td><td>0xFE_0000_1000</td><td>-</td></tr></table>

# 14.1内部设备地址路由

APB总线控制器下挂载了UART控制器、CAN控制器、I2C控制器、PWM控制器、RTC控制器、HPET控制器、NAND控制器、ACPI控制器、DES控制器、AES控制器、RSA控制器、RNG控制器、SDIO控制器和I2S控制器。由访问地址的[15:12]来进行路由，具体如下表所示。

表14-2APB设备地址译码  

<table><tr><td>地址[15:12]</td><td>设备</td><td>备注</td></tr><tr><td>0x0</td><td>UART * 12
CAN * 2</td><td>用地址的[11:8]做下一级地址译码：
0x0: UART0
0x1: UART1
0x2: UART2
0x3: UART3
0x4: UART4
0x5: UART5
0x6: UART6
0x7: UART7
0x8: UART8
0x9: UART9
0xA: UART10
0xB: UART11
0xC: CAN0
0xD: CAN1
0xE: NULL
0xF: NULL</td></tr><tr><td>0x1</td><td>I2C * 2</td><td>用地址[11]做下一级地址译码：
0x0: I2C0
0x1: I2C1</td></tr><tr><td>0x2</td><td>PWM * 4</td><td>用地址[7:4]做下一级地址译码：
0x0: PWM0
0x1: PWM1
0x2: PWM2
0x3: PWM3</td></tr><tr><td>0x3</td><td>Reserved</td><td></td></tr><tr><td>0x4</td><td>HPET</td><td></td></tr><tr><td>0x5</td><td>Reserved</td><td></td></tr><tr><td>0x6</td><td>NAND</td><td></td></tr></table>

<table><tr><td>0x7</td><td>ACPI/RTC</td><td>ACPI的偏移地址为 0x7000
RTC的偏移地址为 0x7800</td></tr><tr><td>0x8</td><td>DES</td><td></td></tr><tr><td>0x9</td><td>AES</td><td></td></tr><tr><td>0xA</td><td>RSA</td><td></td></tr><tr><td>0xB</td><td>RNG</td><td></td></tr><tr><td>0xC</td><td>SDIO</td><td></td></tr><tr><td>0xD</td><td>I2S</td><td></td></tr><tr><td>0xE</td><td>专用通信接口</td><td></td></tr><tr><td>0xF</td><td>NULL</td><td></td></tr></table>

后续章节将分别对APB总线的各个设备控制器进行介绍。

# 15 UART控制器

# 15.1概述

2K1000集成了12个UART控制器，通过APB总线与总线桥通信。UART控制器提供与MODEM或其他外部设备串行通信的功能，例如与另外一台计算机，以RS232为标准使用串行线路进行通信。该控制器在设计上能很好地兼容国际工业标准半导体设备16550A。

其中，UART0、UART3、UART4、UART5复用UART0接口；UART1、UART6、UART7、UART8复用UART1接口；UART2、UART9、UART10、UART11复用UART2接口。参见2.25节。

# 15.2访问地址及引脚复用

各个UART控制器内部寄存器的物理地址构成如下：

表15-1UART控制器物理地址构成  

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0</td><td>固定为 0</td></tr><tr><td>[11:08]</td><td>UART 号</td><td>0x0 - 0xB，分别表示各个 UART 号</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于各个UART，使用时要注意将对应的引脚设置为相应的功能。

与UART相关的引脚设置寄存器为5.2节中的uart1_sel、uart2_sel、uart0_enable、uart1_enable及uart2_enable。

# 15.3控制器结构

UART控制器有发送和接收模块（Transmitter and Receiver）、MODEM模块、中断仲裁模块（Interrupt Arbitrator）、和访问寄存器模块（Register Access Control），这些模块之间的关系见下图所示。主要模块功能及特征描述如下：

1）发送和接收模块：负责处理数据帧的发送和接收。发送模块是将FIFO发送队列中的数据按照设定的格式把并行数据转换为串行数据帧，并通过发送端口送出去。接收模块则监视接收端信号，一旦出现有效开始位，就进行接收，并实现将接收到的异步串行数据帧转换为并行数据，存入FIFO接收队列中，同时检查数据帧格式是否有错。UART的帧结构是通过行控制寄存器（LCR）设置的，发送和接收器的状态被保存在行状态寄存器（LSR）中

2）MODEM模块：MODEM控制寄存器（MCR）控制输出信号DTR和RTS的状态。MODEM控制模块监视输入信号DCD,CTS,DSR和RI的线路状态，并将这些信号的状态记

录在MODEM状态寄存器（MSR）的相对应位中。

3）中断仲裁模块：当任何一种中断条件被满足，并且在中断使能寄存器（IER）中相应位置1，那么UART的中断请求信号UAT_INT被置为有效状态。为了减少和外部软件的交互，UART把中断分为四个级别，并且在中断标识寄存器（IIR）中标识这些中断。四个级别的中断按优先级级别由高到低的排列顺序为，接收线路状态中断；接收数据准备好中断；传送拥有寄存器为空中断；MODEM状态中断。

4）访问寄存器模块：当UART模块被选中时，CPU可通过读或写操作访问被地址线选中的寄存器。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/96efa7eb847de6123ff9d6e1e3d996cbcc929c11945cf1ae0f1c938e405eb385.jpg)  
图15-1 UART控制器结构

# 15.4寄存器描述

# 15.4.1 数据寄存器（DAT）

中文名：数据传输寄存器寄存器位宽：[7：0]偏移量： 0x00复位值： 0x00

表15-2UART数据寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>Tx FIFO</td><td>8</td><td>RW</td><td>数据传输寄存器</td></tr></table>

# 15.4.2 中断使能寄存器（IER）

中文名：中断使能寄存器寄存器位宽：[7：0]

偏移量： 0x01 复位值： 0x00

表15-3UART中断使能寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:4</td><td>Reserved</td><td>4</td><td>RW</td><td>保留</td></tr><tr><td>3</td><td>IME</td><td>1</td><td>RW</td><td>Modem 状态中断使能 ‘0’ - 关闭 ‘1’ - 打开</td></tr><tr><td>2</td><td>ILE</td><td>1</td><td>RW</td><td>接收器线路状态中断使能 ‘0’ - 关闭 ‘1’ - 打开</td></tr><tr><td>1</td><td>ITxE</td><td>1</td><td>RW</td><td>传输保存寄存器为空中断使能 ‘0’ - 关闭 ‘1’ - 打开</td></tr><tr><td>0</td><td>IRxE</td><td>1</td><td>RW</td><td>接收有效数据中断使能 ‘0’ - 关闭 ‘1’ - 打开</td></tr></table>

# 15.4.3中断标识寄存器（IIR）

中文名：中断源寄存器 寄存器位宽：[7:0] 偏移量： 0x02 复位值： 0xc1

表15-4UART中断标识寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:4</td><td>Reserved</td><td>4</td><td>R</td><td>保留</td></tr><tr><td>3:1</td><td>II</td><td>3</td><td>R</td><td>中断源表示位，详见下表</td></tr><tr><td>0</td><td>INTp</td><td>1</td><td>R</td><td>中断表示位</td></tr></table>

中断控制功能表：

表15-5UART中断控制功能表  

<table><tr><td>Bit 3</td><td>Bit 2</td><td>Bit 1</td><td>优先级</td><td>中断类型</td><td>中断源</td><td>中断复位控制</td></tr><tr><td>0</td><td>1</td><td>1</td><td>1st</td><td>接收线路状态</td><td>奇偶、溢出或帧错误，或打断中断</td><td>读 LSR</td></tr><tr><td>0</td><td>1</td><td>0</td><td>2nd</td><td>接收到有效数据</td><td>FIFO 的字符个数达到 trigger 的水平</td><td>FIFO 的字符个数低于 trigger 的值</td></tr><tr><td>1</td><td>1</td><td>0</td><td>3rd</td><td>接收超时</td><td>在 FIFO 至少有一个字符，但在 4 个字符时间内没有任何操作，包括读和写操作</td><td>可接收 FIFO</td></tr><tr><td>0</td><td>0</td><td>1</td><td>3rd</td><td>传输保存寄存器为空</td><td>传输保存寄存器为空</td><td>写数据到 THR 或者多 IIR</td></tr><tr><td>0</td><td>0</td><td>0</td><td>4th</td><td>Modem 状态</td><td>CTS, DSR, RI or DCD</td><td>读 MSR</td></tr></table>

# 15.4.4FIFO控制寄存器（FCR）

中文名： FIFO控制寄存器 寄存器位宽：[7:0] 偏移量： 0x02

复位值： 0xc0

表15-6UART的FIFO控制寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:6</td><td>TL</td><td>2</td><td>W</td><td>接收 FIFO 提出中断申请的 trigger 值 ‘00’ - 1 字节 ‘01’ - 4 字节 ‘10’ - 8 字节 ‘11’ - 14 字节</td></tr><tr><td>5:3</td><td>Reserved</td><td>3</td><td>W</td><td>保留</td></tr><tr><td>2</td><td>Txset</td><td>1</td><td>W</td><td>‘1’ 清除发送 FIFO 的内容，复位其逻辑</td></tr><tr><td>1</td><td>Rxset</td><td>1</td><td>W</td><td>‘1’ 清除接收 FIFO 的内容，复位其逻辑</td></tr><tr><td>0</td><td>Reserved</td><td>1</td><td>W</td><td>保留</td></tr></table>

# 15.4.5线路控制寄存器（LCR）

中文名：线路控制寄存器寄存器位宽：[7：0]偏移量： 0x03复位值： 0x03

表15-7UART线路控制寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>dlab</td><td>1</td><td>RW</td><td>分频锁存器访问位
‘1’ - 访问操作分频锁存器
‘0’ - 访问操作正常寄存器</td></tr><tr><td>6</td><td>bcb</td><td>1</td><td>RW</td><td>打断控制位
‘1’ - 此时串口的输出被置为0(打断状态).
‘0’ - 正常操作</td></tr><tr><td>5</td><td>spb</td><td>1</td><td>RW</td><td>指定奇偶校验位
‘0’ - 不用指定奇偶校验位
‘1’ - 如果LCR[4]位是1则传输和检查奇偶校验位为0。
如果LCR[4]位是0则传输和检查奇偶校验位为1。</td></tr><tr><td>4</td><td>eps</td><td>1</td><td>RW</td><td>奇偶校验位选择
‘0’ - 在每个字符中有奇数个1（包括数据和奇偶校验位）
‘1’ - 在每个字符中有偶数个1</td></tr><tr><td>3</td><td>pe</td><td>1</td><td>RW</td><td>奇偶校验位使能
‘0’ - 没有奇偶校验位
‘1’ - 在输出时生成奇偶校验位，输入则判断奇偶校验位</td></tr><tr><td>2</td><td>sb</td><td>1</td><td>RW</td><td>定义生成停止位的位数
‘0’ - 1个停止位
‘1’ - 在5位字符长度时是1.5个停止位，其他长度是2个停止位</td></tr><tr><td>1:0</td><td>bec</td><td>2</td><td>RW</td><td>设定每个字符的位数
‘00’ - 5位
‘01’ - 6位
‘10’ - 7位
‘11’ - 8位</td></tr></table>

# 15.4.6MODEM控制寄存器（MCR）

中文名： Modem控制寄存器

寄存器位宽：[7：0]

偏移量： 0x04复位值： 0x00

表15-8UART的MODEM控制寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:5</td><td>Reserved</td><td>3</td><td>W</td><td>保留</td></tr><tr><td>4</td><td>Loop</td><td>1</td><td>W</td><td>回环模式控制位
‘0’-正常操作
‘1’-回环模式。在在回环模式中，TXD输出一直为1，输出移位寄存器直接连到输入移位寄存器中。其他连接如下。
DTR    DSR
RTS    CTS
Out1    RI
Out2    DCD</td></tr><tr><td>3</td><td>OUT2</td><td>1</td><td>W</td><td>在回环模式中连到DCD输入</td></tr><tr><td>2</td><td>OUT1</td><td>1</td><td>W</td><td>在回环模式中连到RI输入</td></tr><tr><td>1</td><td>RTSC</td><td>1</td><td>W</td><td>RTS信号控制位</td></tr><tr><td>0</td><td>DTRC</td><td>1</td><td>W</td><td>DTR信号控制位</td></tr></table>

# 15.4.7线路状态寄存器（LSR）

中文名：线路状态寄存器

寄存器位宽：[7:0]

偏移量： 0x05

复位值： 0x00

表15-9UART线路状态寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>ERROR</td><td>1</td><td>R</td><td>错误表示位
‘1’-至少有奇偶校验位错误，帧错误或打断中断的一个。
‘0’-没有错误</td></tr><tr><td>6</td><td>TE</td><td>1</td><td>R</td><td>传输为空表示位
‘1’-传输 FIFO 和传输移位寄存器都为空。给传输 FIFO 写数据时清零
‘0’-有数据</td></tr><tr><td>5</td><td>TFE</td><td>1</td><td>R</td><td>传输 FIFO 位空表示位
‘1’-当前传输 FIFO 为空，给传输 FIFO 写数据时清零
‘0’-有数据</td></tr><tr><td>4</td><td>BI</td><td>1</td><td>R</td><td>打断中断表示位
‘1’-接收到起始位+数据+奇偶位+停止位都是 0，即有打断中断
‘0’-没有打断</td></tr><tr><td>3</td><td>FE</td><td>1</td><td>R</td><td>帧错误表示位
‘1’-接收的数据没有停止位
‘0’-没有错误</td></tr><tr><td>2</td><td>PE</td><td>1</td><td>R</td><td>奇偶校验位错误表示位
‘1’-当前接收数据有奇偶错误
‘0’-没有奇偶错误</td></tr><tr><td>1</td><td>OE</td><td>1</td><td>R</td><td>数据溢出表示位
‘1’-有数据溢出</td></tr></table>

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td></td><td></td><td></td><td></td><td>‘0’-无溢出</td></tr><tr><td>0</td><td>DR</td><td>1</td><td>R</td><td>接收数据有效表示位
‘0’-在 FIFO 中无数据
‘1’-在 FIFO 中有数据</td></tr></table>

对这个寄存器进行读操作时，LSR[4:1]和LSR[7]被清零，LSR[6:5]在给传输FIFO写数据时清零，LSR[0]则对接收FIFO进行判断。

# 15.4.8MODEM状态寄存器（MSR）

中文名： Modem状态寄存器  寄存器位宽：[7:0]  偏移量： 0x06  复位值： 0x00

表15-10UART的MODEM状态寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>CDCD</td><td>1</td><td>R</td><td>DCD输入值的反，或者在回环模式中连到Out2</td></tr><tr><td>6</td><td>CRI</td><td>1</td><td>R</td><td>RI输入值的反，或者在回环模式中连到OUT1</td></tr><tr><td>5</td><td>CDSR</td><td>1</td><td>R</td><td>DSR输入值的反，或者在回环模式中连到DTR</td></tr><tr><td>4</td><td>CCTS</td><td>1</td><td>R</td><td>CTS输入值的反，或者在回环模式中连到RTS</td></tr><tr><td>3</td><td>DDCD</td><td>1</td><td>R</td><td>DDCD指示位</td></tr><tr><td>2</td><td>TERI</td><td>1</td><td>R</td><td>RI边沿检测。RI状态从低到高变化</td></tr><tr><td>1</td><td>DDSR</td><td>1</td><td>R</td><td>DDSR指示位</td></tr><tr><td>0</td><td>DCTS</td><td>1</td><td>R</td><td>DCTS指示位</td></tr></table>

# 15.4.9分频锁存器

中文名：分频锁存器1  寄存器位宽：[7:0]  偏移量： 0x00  复位值： 0x00

表15-11UART分频锁存器1  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>LSB</td><td>8</td><td>RW</td><td>存放分频锁存器的低8位</td></tr></table>

中文名：分频锁存器2  寄存器位宽：[7:0]  偏移量： 0x01  复位值： 0x00

表15-12UART分频锁存器2  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>MSB</td><td>8</td><td>RW</td><td>存放分频锁存器的高8位</td></tr></table>

# 16 CAN

16 CAN龙芯2K1000集成了两路CAN接口控制器。CAN总线是由发送数据线TX和接收数据线RX构成的串行总线，可发送和接收数据。器件与器件之间进行双向传送，最高传送速率1Mbps。

# 16.1访问地址及引脚复用

两个CAN控制器内部寄存器的物理地址构成如下：

表16-1CAN内部寄存器物理地址构成  

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0</td><td>固定为 0</td></tr><tr><td>[11:08]</td><td>CAN 编号</td><td>分别为 0xC 和 0xD</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于CAN模块，使用时要注意将对应的引脚设置为相应的功能。与CAN相关的引脚设置寄存器为5.1节中的can_sel。

# 16.2标准模式

地址区包括控制段和信息缓冲区，控制段在初始化载入是可被编程来配置通讯参数的，应发送的信息会被写入发送缓冲器，成功接收信息后，微控制器从接收缓冲器中读取接收的信息，然后释放空间以做下一步应用。

初始载入后，寄存器的验收代码，验收屏蔽，总线定时寄存器0和1以及输出控制就不能改变了。只有控制寄存器的复位位被置高时，才可以访问这些寄存器。在复位模式和工作模式两种不同的模式中，访问寄存器是不同的。当硬件复位或控制器掉线，状态寄存器的总线状态位时会自动进入复位模式。工作模式是通过置位控制寄存器的复位请求位激活的。

在标准模式下，CAN控制器将ID[10:3]的值和验收代码的值相同的消息包存入接收缓冲区。如果验收屏蔽码的某位为1，则验收码对应的位不参与对ID的检查。

表13-2CAN控制器标准模式下寄存器定义  

<table><tr><td rowspan="2">CAN地址</td><td rowspan="2">段</td><td colspan="2">工作模式</td><td colspan="2">复位模式</td></tr><tr><td>读</td><td>写</td><td>读</td><td>写</td></tr><tr><td>0</td><td>控制</td><td>控制</td><td>控制</td><td>控制</td><td>控制</td></tr><tr><td>1</td><td>FF</td><td>命令</td><td>FF</td><td>命令</td><td></td></tr><tr><td>2</td><td>状态</td><td>-</td><td>状态</td><td>-</td><td></td></tr><tr><td>3</td><td>FF</td><td>-</td><td>中断</td><td>-</td><td></td></tr><tr><td>4</td><td>FF</td><td>-</td><td>验收代码</td><td>验收代码</td><td></td></tr><tr><td>5</td><td>FF</td><td>-</td><td>验收屏蔽</td><td>验收屏蔽</td><td></td></tr><tr><td>6</td><td>FF</td><td>-</td><td>总线定时0</td><td>总线定时0</td><td></td></tr><tr><td>7</td><td>FF</td><td>-</td><td>总线定时1</td><td>总线定时1</td><td></td></tr><tr><td>8</td><td>保留</td><td>保留</td><td>保留</td><td>保留</td><td></td></tr><tr><td>9</td><td>控制</td><td>保留</td><td>保留</td><td>保留</td><td>保留</td></tr><tr><td>10</td><td>发送缓冲器</td><td>ID(10-3)</td><td>ID(10-3)</td><td>FF</td><td>-</td></tr><tr><td>11</td><td>ID(2-0), 
RTR,DLC</td><td>ID(2-0), 
RTR,DLC</td><td>FF</td><td>-</td><td></td></tr><tr><td>12</td><td>数据字节1</td><td>数据字节1</td><td>FF</td><td>-</td><td></td></tr><tr><td>13</td><td>数据字节2</td><td>数据字节2</td><td>FF</td><td>-</td><td></td></tr><tr><td>14</td><td>数据字节3</td><td>数据字节3</td><td>FF</td><td>-</td><td></td></tr><tr><td>15</td><td>数据字节4</td><td>数据字节4</td><td>FF</td><td>-</td><td></td></tr><tr><td>16</td><td>数据字节5</td><td>数据字节5</td><td>FF</td><td>-</td><td></td></tr><tr><td>17</td><td>数据字节6</td><td>数据字节6</td><td>FF</td><td>-</td><td></td></tr><tr><td>18</td><td>数据字节7</td><td>数据字节7</td><td>FF</td><td>-</td><td></td></tr><tr><td>19</td><td>数据字节8</td><td>数据字节8</td><td>FF</td><td>-</td><td></td></tr><tr><td>20</td><td>ID(10-3)</td><td>ID(10-3)</td><td>FF</td><td>-</td><td></td></tr><tr><td>21</td><td>ID(2-0), 
RTR,DLC</td><td>ID(2-0), 
RTR,DLC</td><td>FF</td><td>-</td><td></td></tr><tr><td>22</td><td>数据字节1</td><td>数据字节1</td><td>FF</td><td>-</td><td></td></tr><tr><td>23</td><td>数据字节2</td><td>数据字节2</td><td>FF</td><td>-</td><td></td></tr><tr><td>24</td><td>数据字节3</td><td>数据字节3</td><td>FF</td><td>-</td><td></td></tr><tr><td>25</td><td>数据字节4</td><td>数据字节4</td><td>FF</td><td>-</td><td></td></tr><tr><td>26</td><td>数据字节5</td><td>数据字节5</td><td>FF</td><td>-</td><td></td></tr><tr><td>27</td><td>数据字节6</td><td>数据字节6</td><td>FF</td><td>-</td><td></td></tr><tr><td>28</td><td>数据字节7</td><td>数据字节7</td><td>FF</td><td>-</td><td></td></tr><tr><td>29</td><td>数据字节8</td><td>数据字节8</td><td>FF</td><td>-</td><td></td></tr></table>

# 16.2.1 控制寄存器（CR）

中文名：控制寄存器  寄存器位宽：[7:0]  偏移量：0x00  复位值：0x01

读此位的值总是逻辑1。在硬启动或总线状态位设置为1（总线关闭）时，复位请求位被置为1。如果这些位被软件访问，其值将发生变化而且会影响内部时钟的下一个上升沿，在外部复位期间微控制器不能把复位请求位置为0。如果把复位请求位设为0，微控制器就

必须检查这一位以保证外部复位引脚不保持为低。复位请求位的变化是同内部分频时钟同步的。读复位请求位能够反映出这种同步状态。

复位请求位被设为0后控制器将会等待

a）一个总线空闲信号（11个弱势位），如果前一次复位请求是硬件复位或CPU初始复位。

b)128个总线空闲，如果前一次复位请求是CAN控制器在重新进入总线开启模式前初始化总线造成的。

表13-3CAN控制器标准模式下的控制寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 5</td><td>Reserve</td><td>3</td><td>-</td><td>保留</td></tr><tr><td>4</td><td>OIE</td><td>1</td><td>RW</td><td>溢出中断使能</td></tr><tr><td>3</td><td>EIE</td><td>1</td><td>RW</td><td>错误中断使能</td></tr><tr><td>2</td><td>TIE</td><td>1</td><td>RW</td><td>发送中断使能</td></tr><tr><td>1</td><td>RIE</td><td>1</td><td>RW</td><td>接收中断使能</td></tr><tr><td>0</td><td>RR</td><td>1</td><td>RW</td><td>复位请求</td></tr></table>

# 16.2.2 命令寄存器（CMR）

中文名：命令寄存器寄存器位宽：[7:0]偏移量： 0x01复位值： 0x00命令寄存器对微控制器来说是只写存储器如果去读这个地址返回值是0xff表13- 4CAN控制器标准模式下命令寄存器格式

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>EFF</td><td>1</td><td>W</td><td>扩展模式</td></tr><tr><td>6: 5</td><td>Reserve</td><td>2</td><td>-</td><td>保留</td></tr><tr><td>4</td><td>GTS</td><td>1</td><td>W</td><td>睡眠</td></tr><tr><td>3</td><td>CDO</td><td>1</td><td>W</td><td>清除数据溢出</td></tr><tr><td>2</td><td>RRB</td><td>1</td><td>W</td><td>释放接收缓冲器</td></tr><tr><td>1</td><td>AT</td><td>1</td><td>W</td><td>中止发送</td></tr><tr><td>0</td><td>TR</td><td>1</td><td>W</td><td>发送请求</td></tr></table>

# 16.2.3 状态寄存器（SR）

中文名：状态寄存器寄存器位宽：[7:0]偏移量： 0x02复位值： 0x00

# 表13-5CAN控制器标准模式下状态寄存器格式

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>BS</td><td>1</td><td>R</td><td>总线状态</td></tr><tr><td>6</td><td>ES</td><td>1</td><td>R</td><td>出错状态</td></tr><tr><td>5</td><td>TS</td><td>1</td><td>R</td><td>发送状态</td></tr><tr><td>4</td><td>RS</td><td>1</td><td>R</td><td>接收状态</td></tr><tr><td>3</td><td>TCS</td><td>1</td><td>R</td><td>发送完毕状态</td></tr><tr><td>2</td><td>TBS</td><td>1</td><td>R</td><td>发送缓存器状态</td></tr><tr><td>1</td><td>DOS</td><td>1</td><td>R</td><td>数据溢出状态</td></tr><tr><td>0</td><td>RBS</td><td>1</td><td>R</td><td>接收缓存器状态</td></tr></table>

# 16.2.4 中断寄存器（IR）

中文名：中断寄存器寄存器位宽：[7：0]偏移量： 0x03复位值： 0x00表13- 6CAN控制器标准模式下中断寄存器格式

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 5</td><td>Reserved</td><td>1</td><td>R</td><td>保留</td></tr><tr><td>4</td><td>WUI</td><td>1</td><td>R</td><td>唤醒中断</td></tr><tr><td>3</td><td>DOI</td><td>1</td><td>R</td><td>数据溢出中断</td></tr><tr><td>2</td><td>EI</td><td>1</td><td>R</td><td>错误中断</td></tr><tr><td>1</td><td>TI</td><td>1</td><td>R</td><td>发送中断</td></tr><tr><td>0</td><td>RI</td><td>1</td><td>R</td><td>接收中断</td></tr></table>

# 16.2.5 验收代码寄存器（ACR）

中文名：验收代码寄存器

寄存器位宽：[7：0]偏移量： 0x04复位值： 0x00

在复位情况下，该寄存器是可以读写的。

表16-7CAN验收代码寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>AC</td><td>8</td><td>RW</td><td>ID 验收代码</td></tr></table>

# 16.2.6 验收屏蔽寄存器（AMR）

中文名：验收屏蔽寄存器寄存器位宽：[7：0]偏移量： 0x05复位值： 0x00

验收代码位AC和信息识别码的高8位ID.10- ID.3相等且与验收屏蔽位AM的相应位相或为1时数据可以接收。在复位情况下，该寄存器是可以读写的。

表16-8CAN验收屏蔽寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>AM</td><td>8</td><td>RW</td><td>ID 屏蔽位</td></tr></table>

# 16.2.7 发送缓冲区列表

缓冲器是用来存储微控制器要CAN控制器发送的信息，它被分为描述符区和数据区。发送缓冲器的读/写只能由微控制器在工作模式下完成，在复位模式下读出的值总是0xff。

表13-9CAN控制器标准模式下发送缓冲区格式  

<table><tr><td>地址</td><td>区</td><td>名称</td><td>数据位</td></tr><tr><td>10</td><td rowspan="10">发送缓冲器</td><td>识别码字节 1</td><td>ID(10-3)</td></tr><tr><td>11</td><td>识别码字节 2</td><td>ID(2-0), RTR, DLC</td></tr><tr><td>12</td><td>TX 数据 1</td><td>TX 数据 1</td></tr><tr><td>13</td><td>TX 数据 2</td><td>TX 数据 2</td></tr><tr><td>14</td><td>TX 数据 3</td><td>TX 数据 3</td></tr><tr><td>15</td><td>TX 数据 4</td><td>TX 数据 4</td></tr><tr><td>16</td><td>TX 数据 5</td><td>TX 数据 5</td></tr><tr><td>17</td><td>TX 数据 6</td><td>TX 数据 6</td></tr><tr><td>18</td><td>TX 数据 7</td><td>TX 数据 7</td></tr><tr><td>19</td><td>TX 数据 8</td><td>TX 数据 8</td></tr></table>

# 16.2.8 接收缓冲区列表

接收缓冲区的配置和发送缓冲区的一样，只是地址变为20一29。

# 16.3扩展模式

在扩展模式下，允许使用11位ID的标准帧和29位ID的扩展帧（是标准帧还是扩展帧由TX帧信息的最高位IDE位确定）。

在扩展模式下，CAN控制器可以接收11位ID的标准帧也可以接收29位ID的扩展帧。在接收不同格式的帧的时候，验收代码  $0\sim$  验收代码3（code0～code3）所检查的内容有所不同。验收屏蔽码0~验收屏蔽码3（mask0～mask3）对应验收代码  $0\sim$  验收代码3。如果验收屏蔽码的某1位为1，则对应的验收代码的那一位就不参与对接收包的ID的检查。

在扩展模式下，CAN控制器可以对接收到的消息包进行单滤波也可以进行双滤波。在进行单滤波时，验收代码  $0\sim$  验收代码3仅对单一ID进行过滤。在进行双滤波时，验收代码  $0\sim$  验收代码3可以对两个不同的ID进行。CAN控制器只将ID符合滤波条件的消息包存入接收缓冲区。

对11位ID包的单滤波：

允许将 rdata0 和 rdata1 用来扩展对 ID 的验收长度

$(\mathrm{rx\_id}[10:3] = =\mathrm{code0})\& \& (\mathrm{rtr} = =\mathrm{code1}[4])\& \& (\mathrm{rx\_id}[2:0] = =\mathrm{code1}[7:5])\& \& (\mathrm{rx\_data0} = =\mathrm{code2})\& \& (\mathrm{rx\_data1} = =\mathrm{code3})$

对 11 位 ID 包的双滤波：

$(\mathrm{rx\_id}[10:3] = =\mathrm{code0})\& \& (\mathrm{rtr} = =\mathrm{code1}[4])\& \& (\mathrm{rx\_id}[2:0] = =\mathrm{code1}[7:5])\& \& (\mathrm{rx\_data0} = =\mathrm{code2})\& \& (\mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathbf{r}\& \mathfrak{r}\& \mathbf{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}\& \mathfrak{r}$  {code1[3:0],code3[3:0]})

或者

$(\mathrm{rx\_id}[10:3] = =\mathrm{code2})\& \& (\mathrm{rtr} = =\mathrm{code3}[4])\& \& (\mathrm{rx\_id}[2:0] = =\mathrm{code3}[7:5])$

对 29 位 ID 包的单滤波：

$(\mathrm{rx\_id}[28:21] = =\mathrm{code0})$  &&  $(\mathrm{rx\_id}[20:13] = =\mathrm{code1})\& \& (\mathrm{rx\_id}[12:5] = =\mathrm{code2})\& \& (\mathrm{rtr} = =\mathrm{code3}[2])\& \& (\mathrm{rx\_id}[4:0] = =\mathrm{code3}[7:3])$

对 29 位 ID 的双滤波：

$(\mathrm{rx\_id}[28:21] = =\mathrm{code0})$  &&  $(\mathrm{rx\_id}[20:13] = =\mathrm{code1})$

或者

$(\mathrm{rx\_id}[28:21] = =\mathrm{code2})$  &&  $(\mathrm{rx\_id}[20:13] = =\mathrm{code3})$

表13-10扩展模式下CAN控制器的地址列表  

<table><tr><td rowspan="2">CAN地址</td><td colspan="2">工作模式</td><td colspan="2">复位模式</td></tr><tr><td>读</td><td>写</td><td>读</td><td>写</td></tr><tr><td>0</td><td>控制</td><td>控制</td><td>控制</td><td>控制</td></tr><tr><td>1</td><td>0</td><td>命令</td><td>0</td><td>命令</td></tr><tr><td>2</td><td>状态</td><td>—</td><td>状态</td><td>—</td></tr><tr><td>3</td><td>中断</td><td>—</td><td>中断</td><td>—</td></tr><tr><td>4</td><td>中断使能</td><td>中断使能</td><td>中断使能</td><td>中断使能</td></tr><tr><td>5</td><td>—</td><td>—</td><td>验收屏蔽</td><td>验收屏蔽</td></tr><tr><td>6</td><td>总线定时 0</td><td>—</td><td>总线定时 0</td><td>总线定时 0</td></tr><tr><td>7</td><td>总线定时 1</td><td>—</td><td>总线定时 1</td><td>总线定时 1</td></tr><tr><td>8</td><td>保留</td><td>保留</td><td>保留</td><td>保留</td></tr><tr><td>9</td><td>保留</td><td>保留</td><td>保留</td><td>保留</td></tr><tr><td>10</td><td>保留</td><td>保留</td><td>保留</td><td>保留</td></tr><tr><td>11</td><td>仲裁丢失捕捉</td><td>—</td><td>仲裁丢失捕捉</td><td>—</td></tr><tr><td>12</td><td>错误代码捕捉</td><td>—</td><td>错误代码捕捉</td><td>—</td></tr><tr><td>13</td><td>错误警报限制</td><td>—</td><td>错误警报限制</td><td>—</td></tr><tr><td>14</td><td>RX 错误计数器</td><td>—</td><td>RX 错误计数器</td><td>—</td></tr><tr><td>15</td><td>TX 错误计数器</td><td>—</td><td>TX 错误计数器</td><td>—</td></tr><tr><td>16</td><td>RX 帧信息
{IDE,RTR,2&#x27;h0,DL C[3:0]}</td><td>TX 帧信息
{IDE,RTR,2&#x27;h0,D LC[3:0]}</td><td>验收代码 0
Id[28:21]（扩展帧）
Id[10:3](非扩展帧)</td><td>验收代码 0</td></tr><tr><td>17</td><td>RX_Id[28:21]（扩展帧）
RX_Id[10:3]（非</td><td>TX_Id[28:21]
（扩展帧）
TX_Id[10:3]（非</td><td>验收代码 1
Id[20:13]（扩展帧）</td><td>验收代码 1</td></tr></table>

<table><tr><td>CAN地</td><td colspan="2">工作模式</td><td colspan="2">复位模式</td></tr><tr><td></td><td>扩展帧)</td><td>扩展帧)</td><td>{Id[2:0],RTR,4&#x27; 
h0}(非扩展帧, 单滤波)
{Id[2:0],RTR,da 
ta0[7:4]}(非扩展帧, 双滤波)
{Id[2:0],RTR,4&#x27; 
h0]}(非扩展帧, 单滤波)</td><td></td></tr><tr><td>18</td><td>RX_Id[20:13]（扩展帧）
{RX_Id[2:0],5&#x27;h0}（非扩展帧）</td><td>TX_Id[20:13]（扩展帧）
{TX_Id[2:0],5&#x27;h0}（非扩展帧）</td><td>验收代码2
Id2[28:21]（扩展帧，双滤波）
Id[12:5]（扩展帧，单滤波）
Id2[10:3]（非扩展帧，双滤波）
Data0（非扩展帧，单滤波）</td><td>验收代码2</td></tr><tr><td>19</td><td>RX_Id[12:5]（扩展帧）
RX_data0(非扩展帧)</td><td>TX_Id[12:5]（扩展帧）
TX_data0(非扩展帧)</td><td>验收代码3
Id2[20:13]（扩展帧，双滤波）
{Id[4:0],RTR,2&#x27; 
h0}（扩展帧，单滤波）
{Id2[2:0],RTR,d 
ata0[3:0]}(非扩展帧，双滤波)
Data1（非扩展帧，单滤波）</td><td>验收代码3</td></tr><tr><td>20</td><td>{RX_id[4:0],3&#x27;h0}
（扩展帧）
RX_data1(非扩展帧)</td><td>{TX_id[4:0],3&#x27;h0}（扩展帧）
TX_data1(非扩展帧)</td><td>验收屏蔽0
（不判断为1的那位的id值）</td><td>验收屏蔽0</td></tr><tr><td>21</td><td>RX_data0(扩展帧)
RX_data2(非扩展帧)</td><td>TX_data0(扩展帧)
TX_data2(非扩展帧)</td><td>验收屏蔽1</td><td>验收屏蔽1</td></tr><tr><td>22</td><td>RX_data1(扩展帧)
RX_data3(非扩展帧)</td><td>TX_data1(扩展帧)
TX_data3(非扩展帧)</td><td>验收屏蔽2</td><td>验收屏蔽2</td></tr><tr><td>23</td><td>RX_data2(扩展帧)
RX_Data4(非扩展帧)</td><td>TX_data2(扩展帧)
TX_Data4(非扩展帧)</td><td>验收屏蔽3</td><td>验收屏蔽3</td></tr><tr><td>24</td><td>RX_data3(扩展帧)
RX_data5(非扩展帧)</td><td>TX_data3(扩展帧)
TX_data5(非扩展帧)</td><td>—</td><td>—</td></tr><tr><td>25</td><td>RX_data4(扩展帧)
RX_data6(非扩展帧)</td><td>TX_data4(扩展帧)
TX_data6(非扩展帧)</td><td>—</td><td>—</td></tr><tr><td>26</td><td>RX_data5(扩展帧)
RX_data7(非扩展帧)</td><td>TX_data5(扩展帧)
TX_data7(非扩展帧)</td><td>—</td><td>—</td></tr></table>

<table><tr><td>CAN 地</td><td colspan="2">工作模式</td><td colspan="2">复位模式</td></tr><tr><td></td><td></td><td>帧)</td><td></td><td></td></tr><tr><td>27</td><td>RX data6(扩展帧)</td><td>TX data6(扩展帧)</td><td>—</td><td>—</td></tr><tr><td>28</td><td>RX data7(扩展帧)</td><td>TX data7(扩展帧)</td><td>—</td><td>—</td></tr><tr><td>29</td><td>RX 信息计数器</td><td>—</td><td>RX 信息计数器</td><td>—</td></tr></table>

# 16.3.1 模式寄存器（MOD）

中文名：模式寄存器

寄存器位宽：[7:0]

偏移量： 0x00

复位值： 0x01

读此位的值总是逻辑1。在硬启动或总线状态位设置为1（总线关闭）时，复位请求位被置为1。如果这些位被软件访问，其值将发生变化而且会影响内部时钟的下一个上升沿，在外部复位期间微控制器不能把复位请求位置为0。如果把复位请求位设为0，微控制器就必须检查这一位以保证外部复位引脚不保持为低。复位请求位的变化是同内部分频时钟同步的。读复位请求位能够反映出这种同步状态。

复位请求位被设为0后控制器将会等待

a）一个总线空闲信号（11个弱势位），如果前一次复位请求是硬件复位或CPU初始复位。

b)128个总线空闲，如果前一次复位请求是CAN控制器在重新进入总线开启模式前初始化总线造成的。

表13-11CAN控制器扩展模式下的模式寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 5</td><td>Reserve</td><td>3</td><td>-</td><td>保留</td></tr><tr><td>4</td><td>SM</td><td>1</td><td>RW</td><td>睡眠模式</td></tr><tr><td>3</td><td>AFM</td><td>1</td><td>RW</td><td>单/双滤波模式(0为双滤波,仅复位模式可写)</td></tr><tr><td>2</td><td>STM</td><td>1</td><td>RW</td><td>正常工作模式(1为自测试模式,仅复位模式可写)</td></tr><tr><td>1</td><td>LOM</td><td>1</td><td>RW</td><td>只听模式(仅复位模式可写)</td></tr><tr><td>0</td><td>RM</td><td>1</td><td>RW</td><td>复位模式</td></tr></table>

# 16.3.2 命令寄存器（CMR）

中文名：命令寄存器

寄存器位宽：[7:0]

偏移量： 0x01

复位值： 0x00

命令寄存器对微控制器来说是只写存储器如果去读这个地址返回值是0xff

表13-12CAN控制器扩展模式下命令寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>EFF</td><td>1</td><td>W</td><td>扩展模式（仅在复位模式下可写）</td></tr><tr><td>6: 5</td><td>Reserve</td><td>2</td><td>-</td><td>保留</td></tr><tr><td>4</td><td>SRR</td><td>1</td><td>W</td><td>自接收请求（和TR不能同时为1）</td></tr><tr><td>3</td><td>CDO</td><td>1</td><td>W</td><td>清除数据溢出</td></tr><tr><td>2</td><td>RRB</td><td>1</td><td>W</td><td>释放接收缓冲器</td></tr><tr><td>1</td><td>AT</td><td>1</td><td>W</td><td>中止发送</td></tr><tr><td>0</td><td>TR</td><td>1</td><td>W</td><td>发送请求（和SRR不能同时为1）</td></tr></table>

# 16.3.3 状态寄存器（SR）

中文名：状态寄存器

寄存器位宽：[7:0]偏移量： 0x02复位值： 0x00

表13-13CAN控制器扩展模式下状态寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>BS</td><td>1</td><td>R</td><td>总线状态</td></tr><tr><td>6</td><td>ES</td><td>1</td><td>R</td><td>出错状态</td></tr><tr><td>5</td><td>TS</td><td>1</td><td>R</td><td>发送状态</td></tr><tr><td>4</td><td>RS</td><td>1</td><td>R</td><td>接收状态</td></tr><tr><td>3</td><td>TCS</td><td>1</td><td>R</td><td>发送完毕状态</td></tr><tr><td>2</td><td>TBS</td><td>1</td><td>R</td><td>发送缓存器状态</td></tr><tr><td>1</td><td>DOS</td><td>1</td><td>R</td><td>数据溢出状态</td></tr><tr><td>0</td><td>RBS</td><td>1</td><td>R</td><td>接收缓存器状态</td></tr></table>

# 16.3.4 中断寄存器（IR）

中文名：中断寄存器寄存器位宽：[7:0]偏移量： 0x03复位值： 0x00

表13-14CAN控制器扩展模式下中断寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>BEI</td><td>1</td><td>R</td><td>总线错误中断</td></tr><tr><td>6</td><td>ALI</td><td>1</td><td>R</td><td>仲裁丢失中断</td></tr><tr><td>5</td><td>EPI</td><td>1</td><td>R</td><td>错误消极中断</td></tr><tr><td>4</td><td>WUI</td><td>1</td><td>R</td><td>唤醒中断</td></tr><tr><td>3</td><td>DOI</td><td>1</td><td>R</td><td>数据溢出中断</td></tr><tr><td>2</td><td>EI</td><td>1</td><td>R</td><td>错误中断</td></tr><tr><td>1</td><td>TI</td><td>1</td><td>R</td><td>发送中断</td></tr><tr><td>0</td><td>RI</td><td>1</td><td>R</td><td>接收中断</td></tr></table>

# 16.3.5 中断使能寄存器（IER）

中文名：中断使能寄存器寄存器位宽：[7:0]偏移量： 0x04复位值： 0x00

表13-15CAN控制器扩展模式下中断使能寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>BEIE</td><td>1</td><td>RW</td><td>总线错误中断使能</td></tr><tr><td>6</td><td>ALIE</td><td>1</td><td>RW</td><td>仲裁丢失中断使能</td></tr><tr><td>5</td><td>EPIE</td><td>1</td><td>RW</td><td>错误消极中断使能</td></tr><tr><td>4</td><td>WUIE</td><td>1</td><td>RW</td><td>唤醒中断使能</td></tr><tr><td>3</td><td>DOIE</td><td>1</td><td>RW</td><td>数据溢出中断使能</td></tr><tr><td>2</td><td>EIE</td><td>1</td><td>RW</td><td>错误中断使能</td></tr><tr><td>1</td><td>TIE</td><td>1</td><td>RW</td><td>发送中断使能</td></tr><tr><td>0</td><td>RIE</td><td>1</td><td>RW</td><td>接收中断使能</td></tr></table>

# 16.3.6 仲裁丢失捕捉寄存器

中文名：仲裁丢失捕捉寄存器

寄存器位宽：[7:0]

偏移量： 0xB

复位值： 0x00

表13-16CAN控制器扩展模式下仲裁丢失捕捉寄存器格式  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 5</td><td>-</td><td>3</td><td>R</td><td>保留</td></tr><tr><td>4</td><td>BITNO4</td><td>1</td><td>R</td><td>第四位</td></tr><tr><td>3</td><td>BITNO3</td><td>1</td><td>R</td><td>第三位</td></tr><tr><td>2</td><td>BITNO2</td><td>1</td><td>R</td><td>第二位</td></tr><tr><td>1</td><td>BITNO1</td><td>1</td><td>R</td><td>第一位</td></tr><tr><td>0</td><td>BITNO0</td><td>1</td><td>R</td><td>第零位</td></tr></table>

<table><tr><td colspan="5">位</td><td rowspan="2">十进制值</td><td rowspan="2">功能</td></tr><tr><td>ALC.4</td><td>ALC.3</td><td>ALC.2</td><td>ALC.1</td><td>ALC.0</td></tr><tr><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>0</td><td>仲裁丢失在识别码的Jbit1</td></tr><tr><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>仲裁丢失在识别码的Jbit2</td></tr><tr><td>0</td><td>0</td><td>0</td><td>1</td><td>0</td><td>2</td><td>仲裁丢失在识别码的Jbit3</td></tr><tr><td>0</td><td>0</td><td>0</td><td>1</td><td>1</td><td>3</td><td>仲裁丢失在识别码的Jbit4</td></tr><tr><td>0</td><td>0</td><td>1</td><td>0</td><td>0</td><td>4</td><td>仲裁丢失在识别码的Jbit5</td></tr><tr><td>0</td><td>0</td><td>1</td><td>0</td><td>1</td><td>5</td><td>仲裁丢失在识别码的Jbit6</td></tr><tr><td>0</td><td>0</td><td>1</td><td>1</td><td>0</td><td>6</td><td>仲裁丢失在识别码的Jbit7</td></tr><tr><td>0</td><td>0</td><td>1</td><td>1</td><td>1</td><td>7</td><td>仲裁丢失在识别码的Jbit8</td></tr><tr><td>0</td><td>1</td><td>0</td><td>0</td><td>0</td><td>8</td><td>仲裁丢失在识别码的Jbit9</td></tr><tr><td>0</td><td>1</td><td>0</td><td>0</td><td>1</td><td>9</td><td>仲裁丢失在识别码的Jbit10</td></tr><tr><td>0</td><td>1</td><td>0</td><td>1</td><td>0</td><td>10</td><td>仲裁丢失在识别码的Jbit11</td></tr><tr><td>0</td><td>1</td><td>0</td><td>1</td><td>1</td><td>11</td><td>仲裁丢失在SRTR位</td></tr><tr><td>0</td><td>1</td><td>1</td><td>0</td><td>0</td><td>12</td><td>仲裁丢失在IDE位</td></tr><tr><td>0</td><td>1</td><td>1</td><td>0</td><td>1</td><td>13</td><td>仲裁丢失在识别码的Jbit12</td></tr><tr><td>0</td><td>1</td><td>1</td><td>1</td><td>0</td><td>14</td><td>仲裁丢失在识别码的Jbit13</td></tr><tr><td>0</td><td>1</td><td>1</td><td>1</td><td>1</td><td>15</td><td>仲裁丢失在识别码的Jbit14</td></tr><tr><td>1</td><td>0</td><td>0</td><td>0</td><td>0</td><td>16</td><td>仲裁丢失在识别码的Jbit15</td></tr><tr><td>1</td><td>0</td><td>0</td><td>0</td><td>1</td><td>17</td><td>仲裁丢失在识别码的Jbit16</td></tr><tr><td>1</td><td>0</td><td>0</td><td>1</td><td>0</td><td>18</td><td>仲裁丢失在识别码的Jbit17</td></tr><tr><td>1</td><td>0</td><td>0</td><td>1</td><td>1</td><td>19</td><td>仲裁丢失在识别码的Jbit18</td></tr><tr><td>1</td><td>0</td><td>1</td><td>0</td><td>0</td><td>20</td><td>仲裁丢失在识别码的Jbit19</td></tr><tr><td>1</td><td>0</td><td>1</td><td>0</td><td>1</td><td>21</td><td>仲裁丢失在识别码的Jbit20</td></tr><tr><td>1</td><td>0</td><td>1</td><td>1</td><td>0</td><td>22</td><td>仲裁丢失在识别码的Jbit21</td></tr><tr><td>1</td><td>0</td><td>1</td><td>1</td><td>1</td><td>23</td><td>仲裁丢失在识别码的Jbit22</td></tr><tr><td>1</td><td>1</td><td>0</td><td>0</td><td>0</td><td>24</td><td>仲裁丢失在识别码的Jbit23</td></tr><tr><td>1</td><td>1</td><td>0</td><td>0</td><td>1</td><td>25</td><td>仲裁丢失在识别码的Jbit24</td></tr><tr><td>1</td><td>1</td><td>0</td><td>1</td><td>0</td><td>26</td><td>仲裁丢失在识别码的Jbit25</td></tr><tr><td>1</td><td>1</td><td>0</td><td>1</td><td>1</td><td>27</td><td>仲裁丢失在识别码的Jbit26</td></tr><tr><td>1</td><td>1</td><td>1</td><td>0</td><td>0</td><td>28</td><td>仲裁丢失在识别码的Jbit27</td></tr><tr><td>1</td><td>1</td><td>1</td><td>0</td><td>1</td><td>29</td><td>仲裁丢失在识别码的Jbit28</td></tr><tr><td>1</td><td>1</td><td>1</td><td>1</td><td>0</td><td>30</td><td>仲裁丢失在识别码的Jbit29</td></tr><tr><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>31</td><td>仲裁丢失在RTR位</td></tr></table>

# 16.3.7 错误警报限制寄存器（EMLR）

中文名：错误警报限制寄存器

寄存器位宽：[7:0]

偏移量： 0xD

复位值： 0x60

表16-17CAN错误劲爆限制寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 0</td><td>EML</td><td>8</td><td>RW</td><td>错误警报阀值</td></tr></table>

# 16.3.8 RX错误计数寄存器（RXERR）

中文名： RX错误计数寄存器  寄存器位宽：[7:0]  偏移量： 0xE  复位值： 0x60

表16-18CAN的RX错误计数寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 0</td><td>RXERRR</td><td>8</td><td>R</td><td>接收错误计数</td></tr></table>

# 16.3.9 TX错误计数寄存器（TXERR）

中文名： TX错误计数寄存器  寄存器位宽：[7:0]  偏移量： 0xF  复位值： 0x60

表16-19CAN的TX错误计数寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 0</td><td>TXERRR</td><td>8</td><td>R</td><td>发送错误计数</td></tr></table>

# 16.3.10验收滤波器

在验收滤波器的帮助下，只有当接收信息中的识别位和验收滤波器预定义的值相等时，CAN控制器才允许将已接收信息存入RXFIFO。验收滤波器由验收代码寄存器和验收屏蔽寄存器定义。在模式寄存器中选择单滤波器模式或者双滤波器模式。具体的配置可以参考SJA1000的数据手册。

# 16.3.11RX信息计数寄存器（RMCR）

中文名： RX信息计数寄存器  寄存器位宽：[7:0]  偏移量： 0xD  复位值： 0x00

表16-20CAN的RX错信息计数寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 0</td><td>RMCR</td><td>8</td><td>R</td><td>接收的数据帧计数器</td></tr></table>

# 16.4公共寄存器

1bit time  $=$  internal_clock_time  $\ast ((\mathrm{BRP} + 1)\ast 2)\ast (1 + (\mathrm{TESG}2 + 1) + (\mathrm{TESG}1 + 1))$  16.4.1 总线定时寄存器0（BTR0）

中文名：总线定时寄存器

寄存器位宽：[7:0]

偏移量： 0x06复位值： 0x00

注：在复位模式是可以读写的，工作模式是只读的。

表16-21CAN总线定时寄存器0  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 6</td><td>SJW</td><td>8</td><td>RW</td><td>同步跳转宽度</td></tr><tr><td>5: 0</td><td>BRP</td><td>8</td><td>RW</td><td>波特率分频系数</td></tr></table>

# 16.4.2 总线定时寄存器1（BTR1）

中文名：总线定时寄存器1

寄存器位宽：[7:0]

偏移量： 0x07

复位值： 0x00

表16-22CAN总线定时寄存器1  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>SAM</td><td>1</td><td>RW</td><td>为1时三次采样，否则是一次采用</td></tr><tr><td>6: 4</td><td>TESG2</td><td>3</td><td>RW</td><td>一个bit中的时间段2的计数值</td></tr><tr><td>3: 0</td><td>TSEG1</td><td>4</td><td>RW</td><td>一个bit中的时间段1的计数值</td></tr></table>

# 16.4.3 输出控制寄存器（OCR）

中文名：输出控制寄存器

寄存器位宽：[7:0]

偏移量： 0x08

复位值： 0x00

表16-23CAN输出控制寄存器  

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7: 0</td><td>OCR</td><td>8</td><td>RW</td><td>保留（未使用）</td></tr></table>

# 17 I2C 控制器

# 17.1概述

17.1 概述本章给出 I2C 的详细描述和配置使用。本系统芯片集成了 I2C 接口，主要用于实现两个器件之间数据的交换。I2C 总线是由数据线 SDA 和时钟 SCL 构成的串行总线，可发送和接收数据。器件与器件之间进行双向传送，最高传送速率 400kbps。

# 17.2访问地址及引脚复用

两个I2C控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>1</td><td>固定为 1</td></tr><tr><td>[11]</td><td>I2C 编号</td><td>0x0 代表 I2C0；0x1 代表 I2C1</td></tr><tr><td>[10:03]</td><td>0</td><td>保留</td></tr><tr><td>[02:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于I2C模块，使用时要注意将对应的引脚设置为相应的功能。与I2C相关的引脚设置寄存器为5.1节中的i2c_sel。

# 17.3I2C控制器结构

I2C主控制器的结构，主要模块有，时钟发生器（ClockGenerator）、字节命令控制器（ByteCommandController）、位命令控制器（BitCommandcontroller）、数据移位寄存器（DataShiftRegister）。其余为APB总线接口和一些寄存器。

1）时钟发生器模块：产生分频时钟，同步位命令的工作。

2）字节命令控制器模块：将一个命令解释为按字节操作的时序，即把字节操作分解为位操作。

3）位命令控制器模块：进行实际数据的传输，以及位命令信号产生。

4) 数据移位寄存器模块：串行数据移位。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/517d592fe1a1de1f8d667effe4ed46240d5102f187ccbe30bab07775ec15f18e.jpg)  
图17-1 I2C主控制器结构

# 17.4 I2C控制器寄存器说明

本芯片集成了两个I2C控制器I2C- 0、I2C- 1。

17.4.1 分频锁存器低字节寄存器（PRERlo）

中文名：分频锁存器低字节寄存器寄存器位宽：[7：0]偏移量： 0x00复位值： 0xff

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>PRERlo</td><td>8</td><td>RW</td><td>存放分频锁存器的低 8 位</td></tr></table>

17.4.2 分频锁存器高字节寄存器（PRERhi）

中文名：分频锁存器高字节寄存器

寄存器位宽：[7：0]

偏移量： 0x01

复位值： 0xff

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>PRERhi</td><td>8</td><td>RW</td><td>存放分频锁存器的高 8 位</td></tr></table>

假设分频锁存器的值为prescale，从LPB总线PCLK时钟输入的频率为clock_a，SCL总线的输出频率为clock_s，则应满足如下关系：

Prcescale  $=$  clock_a/(4\*clock_s)- 1

# 17.4.3 控制寄存器（CTR）

中文名：控制寄存器

寄存器位宽：[7：0]偏移量： 0x02复位值： 0x00

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>EN</td><td>1</td><td>RW</td><td>模块工作使能位
为1正常工作模式，
0对分频寄存器进行操作</td></tr><tr><td>6</td><td>IEN</td><td>1</td><td>RW</td><td>中断使能位为1则打开中断</td></tr><tr><td>5:0</td><td>Reserved</td><td>6</td><td>RW</td><td>保留</td></tr></table>

# 17.4.4 发送数据寄存器（TXR）

中文名：发送寄存器寄存器位宽：[7：0]偏移量： 0x03复位值： 0x00

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:1</td><td>DATA</td><td>7</td><td>W</td><td>存放下个将要发送的字节</td></tr><tr><td>0</td><td>DRW</td><td>1</td><td>W</td><td>当数据传送时，该位保存的是数据的最低位；
当地址传送时，该位指示读写状态</td></tr></table>

# 17.4.5 接受数据寄存器（RXR）

中文名：接收寄存器寄存器位宽：[7：0]偏移量： 0x03复位值： 0x00

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7:0</td><td>RXR</td><td>8</td><td>R</td><td>存放最后一个接收到的字节</td></tr></table>

# 17.4.6 命令控制寄存器（CR）

中文名：命令寄存器寄存器位宽：[7：0]偏移量： 0x04复位值： 0x00

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>STA</td><td>1</td><td>W</td><td>产生 START 信号</td></tr><tr><td>6</td><td>STO</td><td>1</td><td>W</td><td>产生 STOP 信号</td></tr><tr><td>5</td><td>RD</td><td>1</td><td>W</td><td>产生读信号</td></tr><tr><td>4</td><td>WR</td><td>1</td><td>W</td><td>产生写信号</td></tr><tr><td>3</td><td>ACK</td><td>1</td><td>W</td><td>产生应答信号</td></tr></table>

<table><tr><td>2:1</td><td>Reserved</td><td>2</td><td>W</td><td>保留</td></tr><tr><td>0</td><td>IACK</td><td>1</td><td>W</td><td>产生中断应答信号</td></tr></table>

都是在I2C发送数据后硬件自动清零。对这些位读操作时候总是读回‘0’。bit3为1时表示此次传输结束时控制器不发送ack，反之结束时发送ack。

# 17.4.7 状态寄存器（SR）

中文名：状态寄存器  寄存器位宽：[7:0]  偏移量：0x04  复位值：0x00

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>7</td><td>RxACK</td><td>1</td><td>R</td><td>收到应答位
1 没收到应答位
0 收到应答位</td></tr><tr><td>6</td><td>Busy</td><td>1</td><td>R</td><td>I2c 总线忙标志位
1 总线在忙
0 总线空闲</td></tr><tr><td>5</td><td>AL</td><td>1</td><td>R</td><td>当 I2C 核失去 I2C 总线控制权时，该位置 1</td></tr><tr><td>4:2</td><td>Reserved</td><td>3</td><td>R</td><td>保留</td></tr><tr><td>1</td><td>TIP</td><td>1</td><td>R</td><td>指示传输的过程
1 表示正在传输数据
0 表示数据传输完毕</td></tr><tr><td>0</td><td>IF</td><td>1</td><td>R</td><td>中断标志位，一个数据传输完，或另外一个器件发起数据传输，该位置 1</td></tr></table>

# 18 PWM控制器

# 18.1概述

18.1概述2K1000芯片里实现了四路脉冲宽度调节/计数控制器，以下简称PWM。每一路PWM工作和控制方式完全相同。每路PWM有一路脉冲宽度输出信号和一路待测脉冲输入信号。系统时钟高达  $125\mathrm{MHz}$  。计数寄存器和参考寄存器均32位数据宽度。

# 18.2访问地址及引脚复用

PWM控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>2</td><td>固定为 2</td></tr><tr><td>[11:08]</td><td>0</td><td>保留</td></tr><tr><td>[07:04]</td><td>PWM 编号</td><td>取值 0x0-0x3 分别代表 PWM0-PWM3</td></tr><tr><td>[03:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于PWM模块，使用时要注意将对应的引脚设置为相应的功能。与PWM相关的引脚设置寄存器为5.1节中的pwm_sel。

# 18.3寄存器描述

每路控制器共有五个寄存器，具体描述如下：

表18-1PWM寄存器列表  

<table><tr><td>名称</td><td>地址</td><td>宽度</td><td>访问</td><td>说明</td></tr><tr><td>Low_buffer</td><td>Base + 0x4</td><td>32</td><td>R/W</td><td>低脉冲缓冲寄存器</td></tr><tr><td>Full_buffer</td><td>Base + 0x8</td><td>32</td><td>R/W</td><td>脉冲周期缓冲寄存器</td></tr><tr><td>CTRL</td><td>Base + 0xC</td><td>11</td><td>R/W</td><td>控制寄存器</td></tr></table>

表18-2PWM控制寄存器设置  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>复位值</td><td>说明</td></tr><tr><td>0</td><td>EN</td><td>R/W</td><td>0</td><td>计数器使能位
置1时：CNTR用来计数
置0时：CNTR停止计数（输出保持）</td></tr><tr><td>2: 1</td><td></td><td>Reserved</td><td>2 60</td><td>预留</td></tr><tr><td>3</td><td>OE</td><td>R/W</td><td>0</td><td>脉冲输出使能控制位，低有效
置0时：脉冲输出使能
置1时：脉冲输出屏蔽</td></tr><tr><td>4</td><td>SINGLE</td><td>R/W</td><td>0</td><td>单脉冲控制位
置1时：脉冲仅产生一次
置0时：脉冲持续产生</td></tr><tr><td>5</td><td>INTE</td><td>R/W</td><td>0</td><td>中断使能位
置1时：当full_pulse到1时送中断
置0时：不产生中断</td></tr></table>

<table><tr><td>6</td><td>INT</td><td>R/W</td><td>0</td><td>中断位
读操作：1表示有中断产生,0表示没有中断
写入1：清中断</td></tr><tr><td>7</td><td>RST</td><td>R/W</td><td>0</td><td>使得Low_level和full_pulse计数器重置
置1时：计数器重置（从buffer读，输出低电平）
置0时：计数器正常工作</td></tr><tr><td>8</td><td>CAPTE</td><td>R/W</td><td>0</td><td>测量脉冲使能
置1时：测量脉冲模式
置0时：非测量脉冲模式（一般而言则是脉冲输出模式）</td></tr><tr><td>9</td><td>INVERT</td><td>R/W</td><td>0</td><td>输出翻转使能
置1时：使脉冲在输出去发生信号翻转（周期以高电平开始）
置0时：使脉冲保持原始输出（周期以低电平开始）</td></tr><tr><td>10</td><td>DZONE</td><td>R/W</td><td>0</td><td>防死区功能使能
置1时：该计数模块需要启用防死区功能
置0时：该模块无需防死区功能</td></tr></table>

# 18.4功能说明

# 18.4.1脉宽调制功能

Low_buffer和Full_buffer寄存器可以由系统编程写入获得初始值。系统编程写入完毕后，模块内部的low_level和full_pulse寄存器分别从Low_buffer和Full_buffer缓冲寄存器中读取初值，之后在系统时钟驱动下不断自减（初始输出低电平）。当low_level寄存器到达1之后，输出变为高电平，此时full_pulse仍在自减。当full_pulse寄存器到达1之后，输出变为低电平，low_level和full_pulse又分别从Low_buffer和Full_buffer缓冲寄存器中读取初值，然后重新开始不断自减，控制器就产生连续不断的脉冲宽度输出。当full_pulse寄存器的值等于1的时候，可以配置产生一个中断，从而作为定时器使用。

例：如果要产生宽度为系统时钟周期50倍的高脉宽和90倍的低脉宽，在low_buffer中应该配置初始值90，在full_buffer寄存器中配置初始值  $(50 + 90) = 140$

值得说明的是，由于两个缓冲寄存器的写入有先后之分，在某些特殊的情况下（比如写入时刻刚好是旧脉冲结束时)会使得输出脉冲有异于预期。推荐的做法是在向缓冲寄存器写入新数前，将控制寄存器EN位写0，在写入新数之后再将EN位写1。值得说明的是，即使没有重写EN位，紊乱的脉冲输出最多只会维持一个周期。

如果对两个缓冲寄存器都写0，则输出为低电平；如果对low_buffer写0，对full_buffer写1，则输出高电平；如果写入Low_buffer的值不小于full_buffer，则输出低电平。但这三类数值都是不推荐的。

此外，缓冲寄存器的数值写入应当先于CTRL控制寄存器。

# 18.4.2脉冲测量功能

待测脉冲信号连在PWM输入信号接口上，在设置完CTRL控制寄存器后，在系统时钟的驱动下，Low_level和full_pulse寄存器开始不断自增。当检测到输入脉冲信号上跳变时，

将Low_level寄存器的值传送到low_buffer寄存器中；当检测到输入脉冲信号下跳变时，将full_pulse寄存器的值传送到full_buffer寄存器中，并将Low_level和full_pulse寄存器置1，重新开始计数。

例：如果要输入脉冲为系统时钟50倍的高脉宽和90倍的低脉宽，在low_buffer中最终读出的值为90，在full_buffer寄存器中读出的值为  $(50 + 90) = 140$

待测脉冲应当是周期信号，且脉冲周期不应超出32位计数器能计量的范围。

每次测量均是从下跳变开始，到下一个下跳变结束。由于测量及缓冲的需要，在连续测量两个脉冲周期后，low_buffer和full_buffer寄存器中存储的才是正确的脉冲参数。

若出现持续的周期超过0xFFFF_FFF9的脉冲，控制寄存器INT位会被置1，表示待测脉冲超出了计量范围。

# 18.4.3防死区功能

四路PWM都配备了防死区功能，可以防止四路脉冲输出同时发生跳变。

将四路模块分别标记为PWM_0、PWM_1、PWM_2、PWM_3，它们的优先级为  $0 > 1 > 2 > 3$  即若要同时产生跳变，在PWM_0跳变之后PWM_1才能跳变（低优先级的信号被“抹去”一个或多个系统时钟），依此类推。该优先级是固化的，不可配置。

一个典型的防死区示例如下（PWM_为未开防死区的输出，PWM_为打开防死区后的输出）：

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/cf3c43f53424f75f77d8821207d35a7f10590d6f6df25600ea0a2b07b367e4fd.jpg)  
图18-1防死区功能

# 19 HPET控制器

# 19.1概述

HPET（HighPrecisionEventTimer，高精度事件定时器）定义了一组新的定时器，这组定时器被操作系统使用，用来给线程调度，内核以及多媒体定时器服务器等产生中断。操作系统可以将不同的定时器分配给不同的应用程序使用。通过配置，每个定时器都能独立产生中断。

这组定时器由一个向上累加的主计时器（up- counter）以及一组比较器构成。这个计时器以固定的频率（125MHz）向上累加，因此当软件两次读取计时器的值时，除非遇到计时器溢出，否则第二次读取的值总是比第一次读取的值大。而每个定时器都包含一个match寄存器以及一个比较器。当match寄存器的值与主计时器相等时，那么定时器产生中断。部分定时器可产生周期性中断。

HPET模块包括一个主计数器（maincount）以及三个比较器（comparator），且他们的宽度都是32位。在这三个比较器中，有且仅有一个比较器支持周期性中断（periodic- capable）；这三个比较器都支持非周期性中断。

# 19.2访问地址

HPET控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>4</td><td>固定为 4</td></tr><tr><td>[01:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 19.3寄存器描述

下表列出了HPET的寄存器：  

<table><tr><td>寄存器偏移地址</td><td>寄存器</td><td>类型</td></tr><tr><td>000-007h</td><td>General Capabilities and ID Register</td><td>只读</td></tr><tr><td>008-00Fh</td><td>Reserved</td><td></td></tr><tr><td>010-017h</td><td>General Configuration Register</td><td>读/写</td></tr><tr><td>018-01Fh</td><td>Reserved</td><td>R/WC</td></tr><tr><td>020-027h</td><td>General Interrupt Status Register</td><td>R/W</td></tr><tr><td>028-0EFh</td><td>Reserved</td><td></td></tr><tr><td>0F0-0F7h</td><td>Main Counter Value Register</td><td>R/W</td></tr><tr><td>100-107h</td><td>Timer 0 Configuration and Capability Register</td><td>R/W</td></tr><tr><td>108-10Fh</td><td>Timer 0 Comparator Value Register</td><td>R/W</td></tr><tr><td>110-11Fh</td><td>Reserved</td><td></td></tr><tr><td>120-127h</td><td>Timer 1 Configuration and Capability Register</td><td>R/W</td></tr></table>

<table><tr><td>128-12Fh</td><td>Timer 1 Comparator Value Register</td><td>R/W</td></tr><tr><td>130-13Fh</td><td>Reserved</td><td></td></tr><tr><td>140-147h</td><td>Timer 2 Configuration and Capability Register</td><td>R/W</td></tr><tr><td>148-14Fh</td><td>Timer 2 Comparator Value Register</td><td>R/W</td></tr><tr><td>150-15Fh</td><td>Reserved</td><td></td></tr></table>

若系统在状态转换过程中需要保存这些寄存器的的值以便随后恢复，那么操作系统负责保存这些寄存器的值，硬件无需保存这些寄存器的值。因此当系统处于S3，S4，S5状态时，这些寄存器无需维持。

GeneralCapabilitiesandIDRegister  

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 32</td><td>COUNTER_CLK_PER IOD</td><td>Main Counter Tick Period: 这个域标示了主计时器的计时频率，以fs（10^-15s）为单位。这个值必须大于0，且小于或等于0x05F5E100（100ns，即10MHz）</td><td>RO</td></tr><tr><td>31: 16</td><td>VENDOR_ID</td><td></td><td>RO</td></tr><tr><td>15: 14</td><td>Reserved</td><td></td><td></td></tr><tr><td>13</td><td>COUNT_SIZE_CAP</td><td>Counter Size: 主计时器的宽度；
0: 32 bits
1: 64 bits</td><td>RO</td></tr><tr><td>12:8</td><td>NUM_TIM_CAP</td><td>Num of Timer: 定时器的个数；这个域的值指示最后一个定时器的编号，2K1000中有三个定时器，因此这个域的值是2。</td><td>RO</td></tr><tr><td>7:0</td><td>REV_ID</td><td>版本号：不可为0</td><td>RO</td></tr></table>

General Configuration Register  

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 1</td><td>Reserved</td><td></td><td></td></tr><tr><td>0</td><td>ENABLE_CNF</td><td>Overal Enable; 用来使能所有定时器产生中断。如果为 0，主计时器停止计时且所有的定时器都不再产生中断。
0：主计时器停止计时且所有的定时器都不再产生中断；
1：主计时器计时且允许定时器产生中断；</td><td>R/W</td></tr></table>

GeneralInterruptStatusRegister  

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 3</td><td>Reserved</td><td></td><td></td></tr><tr><td>2</td><td>T2_INT_STS</td><td>Timer 2 Interrupt Active:功能同 T0_INT_STS</td><td>R/WC</td></tr><tr><td>1</td><td>T1_INT_STS</td><td>Timer 1 Interrupt Active:功能同 T0_INT_STS</td><td>R/WC</td></tr><tr><td>0</td><td>T0_INT_STS</td><td>Timer 0 Interrupt Active:功能依赖于这个定时器的中断触发模式是电平触发还是边沿触发;</td><td>R/WC</td></tr></table>

<table><tr><td></td><td></td><td>如果是电平触发模式：
这位默认是0。当对应的定时器发生中断，那么有硬件将其置1.一旦被置位，软件往这位写1将会清空这位。往这位写0，则无意义。
如果边沿触发模式：
软件将忽略这位。软件通常往这位写0。</td><td></td></tr></table>

各个定时器的中断触发模式由各自Configuration and Capability 寄存器的Tn_TYPE_CNF 位确定。

Main Counter Value Register  

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 32</td><td>Reserved</td><td></td><td></td></tr><tr><td>31: 0</td><td>Main_Counter_Val</td><td>主计时器的值；只有当主计时器停止计时时，才允许修改这个寄存器的值。</td><td>R/W</td></tr></table>

# Timer N Configuration and Capabilities Register

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 9</td><td>Reserved</td><td></td><td></td></tr><tr><td>8</td><td>Tn_32MODE_CNF</td><td>Timer n 32-bit 模式 (N 为 0-2)。当定时器为 32 位时，这位为 0，且只读</td><td>RO</td></tr><tr><td>7</td><td>Reserved</td><td></td><td>RO</td></tr><tr><td>6</td><td>Tn_VAL_SET_CNF</td><td>Timer N Value Set (N 为 0-2)：只有能产生周期性中断的定时器才会使用这个域。通过对这位写 1，软件能直接修改周期性定期的累加器。软件无需对这位清 0 只有 Timer 0 能产生周期性中断，因此对 Timer0 来讲，这位是可读可写。而对于 Timer1，Timer2，这位默认为 0，且为只读。</td><td>R/W</td></tr><tr><td>5</td><td>Tn_SIZE_CAP</td><td>Timer N Size；Timer N 的宽度（N 为 0-2）。0：32 位宽。</td><td>RO</td></tr><tr><td>4</td><td>Tn_PER_INT_CAP</td><td>Timer N Periodic Interrupt Capable（N 为 0-2）：
1：定时器能产生周期性中断；
0：定时器不能产生周期性中断；</td><td>RO</td></tr><tr><td>3</td><td>Tn_TYPE_CNF</td><td>Timer N type（N 为 0-2）：
如果对应的 Tn_PER_INT_CAP 位为 0，那么这位为只读，且默认为 0。
若对应的 Tn_PER_INT_CAP 位为 1，那么这位可读可写。用作使能相应的定时器产生周期性中断。
1：使能定时器产生周期性中断</td><td>R/W</td></tr></table>

<table><tr><td></td><td></td><td>0：使能定时器产生非周期性中断</td><td></td></tr><tr><td>2</td><td>Tn_INT_ENB_CNF</td><td>Timer N interrupt Enable（N为0-2）：使能定时器产生中断</td><td>R/W</td></tr><tr><td>1</td><td>Tn_INT_TYPE_CNF</td><td>Timer N Interrupt Type（N为0-2）：
0：定时器的中断触发模式为边沿触发，这意味着对应的定时器将产生边沿触发中断。若另外的的中断产生，那么将产生另外的边沿。
1：定时器的中断触发模式为电平触发，这意味着对应的定时器将产生电平触发中断。这个中断将一直有效直到被软件清掉(General Interrupt Status Register)。</td><td></td></tr><tr><td>0</td><td>Reserved</td><td></td><td></td></tr></table>

# Timer N Comparator Value Register

<table><tr><td>位</td><td>名称</td><td>描述</td><td>读写特性</td></tr><tr><td>63: 32</td><td>Reserved</td><td></td><td></td></tr><tr><td>31: 0</td><td>Tn_Com_VAL</td><td>Tn_Comparator value（N为0-2）：定时器比较器的值；
当对应的定时器配置为非周期性模式时：
◆ 这个寄存器的值将与主计时器寄存器的值做比较；
◆ 若主计时器的值与比较器的值相等时，则产生定时中断（如果：对应的中断使能打开）。
◆ 比较器的值不会因为中断的产生而发生变化
若对应的定时器配置为周期性模式时：
• 当主计时器的值域比较器的值相等时，产生中断（如果对应的中断使能被打开）；
• 如果产生中断，那么比较器的值将累加最后一次软件写入比较器的值。比如当比较器的值被写入0x0123h，
• 那么当主计时器的值为0x123h时，产生中断；
• 比较器的值被硬件修改为0x246h；
• 当主计时器的值达到0x246h时，产生另外一个中断；
• 比较器的值被硬件修改为0x369h。
◆ 只要产生中断，那么比较器的值都会累加；直到比较器的值达到最大（0xffffffff），那么累加器的值将会继续累加。
比如当比较器的值是FFF0000h，而最后一次由软件写入比较器的值是20000。当中断发生后，比较器的值变为00010000h。</td><td>R/W</td></tr></table>

# 20 NAND 控制器

# 20.1 NAND 控制器结构描述

NAND FLASH 控制器最大支持单片 16GB FLASH 的容量，最大页大小 8KB，芯片最多支持 4 个片选和 4 个 RDY 信号，控制器支持 SLC 和 MLC 两种类型 FLASH 的操作。

# 20.2 访问地址及引脚复用

NAND 控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>6</td><td>固定为 6</td></tr><tr><td>[11]</td><td>0</td><td>保留</td></tr><tr><td>[10:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于 NAND 模块，使用时要注意将对应的引脚设置为相应的功能。与 NAND 相关的引脚设置寄存器为 5.1 节中的 nand_sel。

# 20.3 NAND 寄存器配置描述

NAND 内部的寄存器设置如下：

<table><tr><td>偏移地址</td><td>寄存器名称</td></tr><tr><td>0x00</td><td>NAND_CMD</td></tr><tr><td>0x04</td><td>ADDR_C</td></tr><tr><td>0x08</td><td>ADDR_R</td></tr><tr><td>0x0C</td><td>NAND_TIMING</td></tr><tr><td>0x10</td><td>ID_L</td></tr><tr><td>0x14</td><td>STATUS &amp;amp; ID_H</td></tr><tr><td>0x18</td><td>NAND_PARAMETER</td></tr><tr><td>0x1C</td><td>NAND_OP_NUM</td></tr><tr><td>0x20</td><td>CS_RDY_MAP</td></tr><tr><td>0x40</td><td>DMA access address</td></tr></table>

# 20.3.1 命令寄存器 NAND_CMD（偏移地址 0x00）

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31</td><td>DMA_REQ</td><td>R/-</td><td>非 ECC 模式下 NAND 发出 DMA 请求</td></tr><tr><td>30</td><td>ECC_DMA_R EQ</td><td>R/-</td><td>ECC 模式下 NAND 发出 DMA 请求</td></tr><tr><td>29:25</td><td>STATUS</td><td>R/-</td><td>内部状态机（供测试用）</td></tr><tr><td>24</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>23:20</td><td>NAND_CE</td><td>R/-</td><td>外部 NAND 芯片片选情况，四位分别对应片外四个片选，0 表示选</td></tr></table>

<table><tr><td></td><td></td><td></td><td>中</td></tr><tr><td>19: 16</td><td>NAND_RDY</td><td>R/-</td><td>外部 NAND 芯片 RDY 情况，对应关系和 NAND_CE 一致，1 表示准备好</td></tr><tr><td>15</td><td></td><td></td><td>Reserved</td></tr><tr><td>14</td><td>wait ecc</td><td>R/W</td><td>为 1 表示等待 ECC 读完成（用于 ECC 读）</td></tr><tr><td>13</td><td>INT_EN</td><td>R/W</td><td>NAND 中断使能信号，为 1 表示使能中断</td></tr><tr><td>12</td><td>RS_WR</td><td>R/W</td><td>为 1 表示写操作时候 ECC 功能开启</td></tr><tr><td>11</td><td>RS_RD</td><td>R/W</td><td>为 1 表示读操作时候 ECC 功能开启</td></tr><tr><td>10</td><td>done</td><td>R/W</td><td>为 1 表示操作完成，需要软件清零</td></tr><tr><td>9</td><td>Spare</td><td>R/W</td><td>为 1 表示操作发生在 NAND 的 SPARE 区</td></tr><tr><td>8</td><td>Main</td><td>R/W</td><td>为 1 表示操作发生在 NAND 的 MAIN 区</td></tr><tr><td>7</td><td>Read status</td><td>R/W</td><td>为 1 表示读 NAND 的状态操作</td></tr><tr><td>6</td><td>Reset</td><td>R/W</td><td>为 1 表示 Nand 复位操作</td></tr><tr><td>5</td><td>read id</td><td>R/W</td><td>为 1 表示读 ID 操作</td></tr><tr><td>4</td><td>blocks erase</td><td>R/W</td><td>连续擦除标志，缺省 0：1 有效，连续擦除块的数目由 nand_op_num 决定</td></tr><tr><td>3</td><td>erase operation</td><td>R/W</td><td>为 1 表示擦除操作</td></tr><tr><td>2</td><td>write operation</td><td>R/W</td><td>为 1 表示写操作</td></tr><tr><td>1</td><td>read operation</td><td>R/W</td><td>为 1 表示读操作</td></tr><tr><td>0</td><td>command valid</td><td>R/W</td><td>为 1 表示命令有效，操作完成后硬件自动清零</td></tr></table>

# 20.3.2 页内偏移地址寄存器 ADDR_C（偏移地址 0x04）

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:14</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>13:0</td><td>Nand_Col</td><td>R/W</td><td>读、写、擦除操作起始地址页内地址（必须以字对齐，为4的倍数），和页大小对应关系如下：
512Bytes：只需要填充[8:0]
2K：需要填充[11:0]，[11]表示 spare区，[10:0]表示页内偏移地址
4K：需要填充[12:0]，[12]表示 spare区，[11:0]表示页内偏移地址
8K：需要填充[13:0]，[13]表示 spare区，[12:0]表示页内偏移地址</td></tr></table>

# 20.3.3 页地址寄存器 ADDR_R（偏移地址 0x08）

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:25</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>24:0</td><td>Nand_Rew</td><td>R/W</td><td>读、写、擦除操作起始地址页地址，地址组成如下：
{片选，页数}
其中片选固定为2位，页数根据实际的单片颗粒容量确定，如1M页则为[19:0]，[21:20]用于选择4片中的第几片</td></tr></table>

# 20.3.4 时序寄存器 NAND_TIMING（偏移地址 0x0C）

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:16</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>15: 8</td><td>Hold cycle</td><td>R/W</td><td>NAND 命令有效需等待的周期数，缺省 4</td></tr><tr><td>7: 0</td><td>Wait cycle</td><td>R/W</td><td>NAND 一次读写所需总时钟周期数，缺省 18 ECC 模式下配置为 8&#x27; hb</td></tr></table>

# 20.3.5ID寄存器ID_L（偏移地址0x10)

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:0</td><td>ID[31:0]</td><td>R/-</td><td>ID[31:0]</td></tr></table>

# 20.3.6ID和状态寄存器STATUS&ID_H（偏移地址0x14)

<table><tr><td>位</td><td>位域名称</td><td>访问</td><td></td></tr><tr><td>31:24</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>23:16</td><td>STATUS</td><td>R/-</td><td>NAND 设备当前的读写完成状态</td></tr><tr><td>15:0</td><td>ID[47:32]</td><td>R/-</td><td>ID 高 16 位</td></tr></table>

# 20.3.7参数配置寄存器NAND_PARAMETER（偏移地址0x18)

<table><tr><td>位</td><td>位域名称</td><td>访问</td><td>描述</td></tr><tr><td>31:20</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>29:16</td><td>op_scope</td><td>R/W</td><td>每次能操作的范围，配置如下：
1. 操作 main 区，配置为一页的 main 区大小
2. 操作 spare 区，配置为一页的 spare 区大小
3. 操作 main 加 spare 区，配置为一页的 main 区加上 spare 区大小</td></tr><tr><td>15</td><td></td><td>R/-</td><td>Reserved</td></tr><tr><td>14:12</td><td>ID_number</td><td>R/W</td><td>ID 号的字节数</td></tr><tr><td>11:8</td><td>外部颗粒容量大小</td><td>R/W</td><td>0: 1Gb（2K 页）
1: 2Gb（2K 页）
2: 4Gb（2K 页）
3: 8Gb（2K 页）
4: 16Gb（4K 页）
5: 32Gb（8K 页）
6: 64Gb（8K 页）
7: 128Gb（8K 页）
9: 64Mb（512B 页）
a:128Mb（512B 页）
b:256Mb（512B 页）
c:512Mb（512B 页）
d:1Gb（512B 页）
(b)</td></tr><tr><td>7:0</td><td></td><td>R/-</td><td>Reserved</td></tr></table>

# 20.3.8操作数量寄存器NAND_OP_NUM（偏移地址0x1C)

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:0</td><td>NAND_OP_NUM</td><td>R/W</td><td>NAND 读写操作 Byte 数(非 ECC 模式下必须以字对齐，为 4 的倍数；ECC 模式下，配置成 527*N+2，其中 512*N 为 main 区大小；擦除为块数</td></tr></table>

# 20.3.9映射寄存器CS_RDY_MAP（偏移地址  $0\times 20$

NAND的4个CS由所访问的地址硬件自动生成，CS0/RDY0对应最低一块空间，CS1/RDY1对应次低一块空间，其它以此类推。如果需要调整外部芯片和NAND地址的关系，可通过设置本寄存器，对cs_rdy1/2/3进行重新映射。

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31:28</td><td>rdy3_sel</td><td>R/W</td><td>rdy3信号从芯片引脚到NAND控制器的映射
4&#x27;b0001:NAND_RDY[0]
4&#x27;b0010:NAND_RDY[1]
4&#x27;b0100:NAND_RDY[2]
4&#x27;b1000:NAND_RDY[3]</td></tr><tr><td>27:24</td><td>cs3_sel</td><td>R/W</td><td>cs3信号从NAND控制器到芯片引脚的映射
4&#x27;b0001:NAND_CS[0]
4&#x27;b0010:NAND_CS[1]
4&#x27;b0100:NAND_CS[2]
4&#x27;b1000:NAND_CS[3]</td></tr><tr><td>23:20</td><td>rdy2_sel</td><td>R/W</td><td>rdy2信号从芯片引脚到NAND控制器的映射
4&#x27;b0001:NAND_RDY[0]
4&#x27;b0010:NAND_RDY[1]
4&#x27;b0100:NAND_RDY[2]
4&#x27;b1000:NAND_RDY[3]</td></tr><tr><td>19:16</td><td>cs2_sel</td><td>R/W</td><td>cs2信号从NAND控制器到芯片引脚的映射
4&#x27;b0001:NAND_CS[0]
4&#x27;b0010:NAND_CS[1]
4&#x27;b0100:NAND_CS[2]
4&#x27;b1000:NAND_CS[3]</td></tr><tr><td>15:12</td><td>rdy1_sel</td><td>R/W</td><td>rdy1信号从芯片引脚到NAND控制器的映射
4&#x27;b0001:NAND_RDY[0]
4&#x27;b0010:NAND_RDY[1]
4&#x27;b0100:NAND_RDY[2]
4&#x27;b1000:NAND_RDY[3]</td></tr><tr><td>11:8</td><td>cs1_sel</td><td>R/W</td><td>cs1信号从NAND控制器到芯片引脚的映射
4&#x27;b0001:NAND_CS[0]
4&#x27;b0010:NAND_CS[1]
4&#x27;b0100:NAND_CS[2]
4&#x27;b1000:NAND_CS[3]</td></tr><tr><td>7:0</td><td></td><td>R/-</td><td>reserved</td></tr></table>

# 20.3.10DMA读写数据寄存器DMA_ADDRESS（偏移地址0x40)

<table><tr><td>位</td><td>位域名</td><td>读写</td><td>描述</td></tr><tr><td>31: 0</td><td>DMA_ADDRESS</td><td>R/W</td><td>NAND 读写 NAND flash 数据（ID/STATUS 除外）时候的访问地址，读/写地址相同，读写方向通过 DMA 配置实现</td></tr></table>

# 20.4NANDADDR说明

以2K页的NAND flash为例，定义如下：每一页的main区大小为2KB，spare区大小为64Bmain_op  $=$  NAND_CMD[8];spare_op  $=$  NAND_CMD[9];addr_in_page  $=$  {A11,A10. .A2,A1,A0}=ADDR_C

page_number  $=$  {...A30,A29,A28,A27. ..A13,A12}  $=$  ADDR_R

NAND flash的main区总容量计算公式为

容量  $= 2$  (ADDR_C- 1)  $\ast 2$  (ADDR_R)\*8bit  $= 2\mathrm{K}^{*}2$  (ADDR_R)\*8bit

NAND地址空间示例如下表：

<table><tr><td></td><td>I/O</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td></tr><tr><td>Column1</td><td>1st Cycle</td><td>A0</td><td>A1</td><td>A2</td><td>A3</td><td>A4</td><td>A5</td><td>A6</td><td>A7</td></tr><tr><td>Column2</td><td>2nd Cycle</td><td>A8</td><td>A9</td><td>A10</td><td>A11</td><td>*L</td><td>*L</td><td>*L</td><td>*L</td></tr><tr><td>Row1</td><td>3rd Cycle</td><td>A12</td><td>A13</td><td>A14</td><td>A15</td><td>A16</td><td>A17</td><td>A18</td><td>A19</td></tr><tr><td>Row2</td><td>4th Cycle</td><td>A20</td><td>A21</td><td>A22</td><td>A23</td><td>A24</td><td>A25</td><td>A26</td><td>A27</td></tr><tr><td>Row3</td><td>5th Cycle</td><td>A28</td><td>A29</td><td>A30</td><td>A31</td><td>A32</td><td>A33</td><td>...</td><td>...</td></tr></table>

(注：2K页的1Gb容量NANDflash对应的Row最大值为A27，发送地址给NANDflash时只用发Column1\~2和Row1\~2，不用发Row3。配置NAND参数时，注意不要配错型号，否则可能会读不出数据并且控制器会死等)

对系统板上NAND颗粒来说，如果仅仅操作spare区，  $\mathrm{Al1} = 1$  是唯一标志。所以软件配置内部寄存器时，需要配置A11和spare_op均为1(见Examples5），错误的示例见Examples2。

对系统板上NAND颗粒来说，如果仅仅操作main区，  $\mathrm{Al1} = 0$  是唯一标志.；所以软件配置内部寄存器时，需要配置A11和spare_op均为0（见Examples1），错误的示例见Examples4。

对系统板上NAND颗粒来说，如果操作main+spare区，A11可以为0（见Examples3）；也可以为1（见Examples6）。

Examples1：（非ECC模式下。NAND颗粒中一个page的数据只能位于  $0\mathrm{x}0\mathrm{- }0\mathrm{x}83\mathrm{f}$  ，第一个op表示读写开始的数据，接下来的op表示随后的读写数据；NO_op表示不能被本次NAND配置读写的数据）

(spare_op  $= 0$  &main_op  $= 0$  ) equal to (spare_op  $= 0$  &main_op  $= 1$  ); ADDR_C  $= 0\times 30$

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td></tr><tr><td>Page 1</td><td>op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td></tr><tr><td>Page 2</td><td>op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td></tr></table>

Examples2:

spare_op  $= 1$  &main_op  $= 0$  ; ADDR_C  $= 0\times 30$  （配置出错！！开始操作不在spare区，下图是可能的错误访问顺序）

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>op</td><td>op</td><td>op</td><td>Op</td><td>op</td><td>op</td></tr><tr><td>Page 1</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>Op</td><td>op</td><td>op</td></tr><tr><td>Page 2</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>Op</td><td>op</td><td>op</td></tr></table>

<table><tr><td>Page 3</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>Op</td><td>op</td><td>op</td></tr></table>

Examples3:

spare_op  $= 1$  &main_op  $= 1$  ; ADDR  $\mathrm{C} = 0\mathrm{x}30$

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr><tr><td>Page 1</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr><tr><td>Page 2</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr></table>

Examples4:

(spare_op  $= 0$  &main_op  $= 0$  ), (equal to spare_op  $= 0$  &main_op  $= 1$  ); ADDR_C  $= 0\mathrm{x}830$  : (配置出错！！开始操作在 spare 区，下图是可能的错误访问顺序)

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td></tr><tr><td>Page 1</td><td>NO_op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td></tr><tr><td>Page 2</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td></tr><tr><td>Page 3</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>NO_op</td><td>NO_op</td></tr></table>

Examples5:

spare_op  $= 1$  and main_op  $= 0$  ; ADDR_C  $= 0\mathrm{x}830$

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>op</td><td>op</td></tr><tr><td>Page 1</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>op</td><td>op</td><td>op</td></tr><tr><td>Page 2</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>op</td><td>op</td><td>op</td></tr></table>

Examples6:

spare_op  $= 1$  &main_op  $= 1$  ; ADDR_C  $= 0\mathrm{x}830$

<table><tr><td>Data in a page</td><td>0</td><td>0x30</td><td>...</td><td>0x7ff</td><td>0x800</td><td>0x830</td><td>0x83f</td></tr><tr><td>Page 0</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>NO_op</td><td>op</td><td>OP</td></tr><tr><td>Page 1</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr><tr><td>Page 2</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr><tr><td>Page 3</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td><td>op</td></tr></table>

512B页大小的NAND flash和2KB页大小配置类似，但在以下几个地方会有不同，需要注意：

每一页的main区大小为512B，spare区大小为16B。其中main区分为两个256B区，每个256B区通过A0~A7来寻址。读写操作时，通过发送命令  $0\mathrm{x}00$  、0x01和  $0\mathrm{x}50$  来选择是在哪个256B区或者spare区（软件不必关心，硬件自动选择，如配置NAND控制器写  $0\mathrm{x}100$  时，硬件会自动发送到高256B区）。

发送地址命令顺序如下：

<table><tr><td></td><td>I/O</td><td>0</td><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td></tr><tr><td>Column1</td><td>1st Cycle</td><td>A0</td><td>A1</td><td>A2</td><td>A3</td><td>A4</td><td>A5</td><td>A6</td><td>A7</td></tr><tr><td>Row1</td><td>2nd Cycle</td><td>A9</td><td>A10</td><td>A11</td><td>A12</td><td>A13</td><td>A14</td><td>A15</td><td>A16</td></tr></table>

<table><tr><td>Row2</td><td>3rd Cycle</td><td>A17</td><td>A18</td><td>A19</td><td>A20</td><td>A21</td><td>A22</td><td>A23</td><td>A24</td></tr><tr><td>Row3（注）</td><td>4th Cycle</td><td>A25</td><td>A26</td><td>°L</td><td>°L</td><td>°L</td><td>°L</td><td>°L</td><td>°L</td></tr></table>

（注：Nand flash容量为64Mb、128Mb和256Mb时，对应的最大列地址ADDR_R分别为A22、A23和A24，只用发送三次地址命令Column1和Row1\~2，不用发送Row3；容量为512Mb和1Gb时，需要发送Row3）

4K/8K页大小的配置和2K页配置一样，4K页的main区大小为4KB，spare区大小为128B；8K页的main区大小为8KB，spare区大小为640B。都需要发送五次地址命令。

# 20.5NAND-flash读写操作举例

命令寄存器的‘commandvalid’位不能与其他读写使能位同时置位，要先设置好要进行的操作，最后才置‘commandvalid’位。

例如：现在要读Main区的数据，那么操作分成以下两步：

a、先NAND  $\mathrm{CMD} = 0\mathrm{x}102$  b、再NAND  $\mathrm{CMD} = 0\mathrm{x}103$

# 20.6NANDECC说明

硬件集成ECC功能，ECC采用RS(527,512)方法进行编码和解码，即每512Byte的有效数据对应15Byte的ECC编码。在配置软件过程中需要注意以下几点：

1．每次读写NAND的时候，需要每次操作一个Page，即使原始数据不够一个Page;

2．原始数据存储在main区，ECC编码存储在spare区。ECC码从spare区的第三个字节开始存储，防止缺陷标记区defectarea被覆盖，该功能由硬件完成，软件只需按下文要求配置参数即可；

3．NAND的op_num参数与DMA控制器的操作数的关系在不同情况下处理方式不一样，具体见下文；

4．ECC操作和OOB操作可以分开，比如对一个页完成ECC读/写后可以对其OOB进行操作；

5. ECC模式下，不支持512B页大小。

在软件控制上，写操作与读操作流程有些差别，下面给出具体操作步骤：

写操作：

1．设置NAND_CMD[12]为1，表示接下来写操作为ECC写2．设置NAND_CMD[8]和NAND_CMD[9]为1，表示同时操作main区和spare区3．设置NAND_OP_NUM为（main区大小+main区数据对应的  $\mathrm{ECC} + 2$  一4.设置ADDR_C为0

5. 设置其他寄存器

6. 设置NAND_CMD[2]表示写操作

7. 设置NAND_CMD[0]开始写操作

8. 等待NAND_CMD[10]完成

以上每步的设置需要保证对应寄存器的其他位保持不变。对应的DMA控制器的操作数配置为main区大小/4。

读操作：

读操作比写操作复杂一些，其需要两个步骤来完成。第一个步骤先将spare区的ECC码读到NAND控制器的FIFO中暂存，具体步骤为：

1. 设置NAND_CMD[9]和NAND_CMD[11],表示接下来的操作在spare区进行，而且读回来的数据是为ECC解码做准备2. 设置NAND_OP_NUM为ECC码byte数3. 设置ADDR_C为main区大小4. 设置NAND_CMD[14]5. 设置其他寄存器6. 设置NAND_CMD[1]表示读操作7. 设置NAND_CMD[0]开始读操作8. 等待NAND_CMD[10]完成该步骤不需要DMA控制器，因此不必启动DMA控制器。第二个步骤，读main区数据，NAND控制器会同步将译码后的数据返回给DMA控制器，其步骤为：

1. 设置NAND_CMD[8],同时设置NAND_CMD[9]为无效，表示接下来操作在main区，同时需保证NAND_CMD[11]为12. 设置NAND_OP_NUM为main区大小3. 设置ADDR_C为04. 设置NAND_CMD[1]表示读操作5. 设置NAND_CMD[0]开始读操作6. 等待NAND_CMD[10]完成

该步骤中DMA控制器的操作数配置为main区大小/4。

需注意在整个读操作过程中保持NAND_CMD[11](rs_rd)为1，否则NAND控制器的FIFO会被复位。

可以在ECC操作完成后通过普通方式读回所有内容，包括原始数据和ECC校验增加的数据（此时配置操作数op_num和DMA相同）。

校验能力说明：最多可以纠错6个10bit单元，这些10bit单元内部出错的位数可以是

1- 10 个。10bit 单元指的是对于每个 512Byte 原始数据+15Byte ECC 码数据，从 bit0 开始把每个连续的 10bit 数据作为一个单元来看待。

# 20.7 支持 NAND 型号

本节列出经过验证的可支持的 NAND 型号，其它类型的 NAND 颗粒未经验证，不保证与本控制器兼容。

$\bullet$  镁光 L84A_64Gb_128Gb_256Gb_AsyncSync_NAND：可用于存储 $\bullet$  三星 K9F1G08U0C：可用于存储

# 21 电源管理模块

# 21.1概述

龙芯2K1000电源管理模块提供系统功耗管理实现机制。支持AdvancedConfigurationandPowerInterface，Version4.0a(ACPI)，提供相应的功耗管理功能。

系统休眠与唤醒，支持ACPI S3（待机到内存），ACPI S4（待机到硬盘），ACPIS5（软关机），并且支持电源失效检测和自动系统恢复。支持多种唤醒方式（USB，GMAC，电源开关等）- 动态性能功耗控制，支持处理器核DVFS控制，支持动态关闭媒体解码协处理器电源。- 系统时钟控制，模块时钟门控，多种方式调节频率。- 提供温度管理控制功能。支持3级报警机制。

# 21.2访问地址

电源管理模块内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>查找</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>7</td><td>固定为 7</td></tr><tr><td>[11]</td><td>ACPI/RTC</td><td>0x0 代表 ACPI; 0x1 代表 RTC</td></tr><tr><td>[10:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 21.3寄存器描述

本节介绍电源管理控制器相关寄存器，使用方法可参见下一节描述。寄存器电压域表示寄存器的该位所属电压域。寄存器属性简写包括：R/W（可读可写），RO（只读），R/WC（可读，写清除），WO（只写，读无效）

# PMCON_SOC:SOC General PM Configuration Register

<table><tr><td colspan="2">地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td colspan="2">0x00</td><td>SOC</td><td>R/W, RO</td></tr><tr><td>位域</td><td colspan="3">描述</td></tr><tr><td>25</td><td colspan="3">PWRBTN_LVI - RO
该位指示当前PWRBTN信号状态。</td></tr><tr><td>24</td><td colspan="3">PWRTYP - RO
该位指示供电模式
1: AC（适配器） 0: Battery（电池）</td></tr><tr><td>23:9</td><td colspan="3">保留</td></tr><tr><td>8:0</td><td colspan="3">TEMP_NOW - RO.
CPU内部温度传感器温度值，第8位为溢出位。</td></tr></table>

# PMCON_RESUME : RESUME General PM Configuration Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x04</td><td>RESUME</td><td>R/W, RO, R/WC</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:8</td><td>保留</td></tr><tr><td>7</td><td>USB_GMAC_OK - R/W
如果 RSMRSTn 有效过，该位为 0，表示 USB 和 GMAC 没有配置，不能唤醒系统。重新上电后由系统配置该位。</td></tr><tr><td>6</td><td>CITT_STS - R/WC
系统在 S0 状态时发生 temperature trip，系统进入 G2/S5 状态，该位为重新上电后系统检测记录事件状态</td></tr><tr><td>5</td><td>CTT_EN - R/W
使能 temperature trip 保护机制</td></tr><tr><td>4</td><td>LID_OPEN - RO
显示屏状态检测位
1：显示屏打开 0：显示屏合上</td></tr><tr><td>3</td><td>保留</td></tr><tr><td>2</td><td>SRS (System Reset Status) - R/WC
0：SYS_RESETn 没有被按下
1：SYS_RESETn 被按下过，系统重新复位后需检查此位并作出相应清除操作。</td></tr><tr><td>1</td><td>PWROK_FLR (PWROK_Failure) - R/WC
当系统在 S0 状态时，PWROK 信号变无效该位置 1，软件通过写 1 将该位清除。</td></tr><tr><td>0</td><td>DRAM_INIT - R/W
该位不影响硬件功能，PMON 在进行 DRAM 初始化前将该位置 1，结束 DRAM 初始化后将该位写 0，软件可通过此位检查 DRAM 初始化是否被打断过。</td></tr></table>

# PMCON_RTC : RTC General PM Configuration Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x08</td><td>RTC</td><td>R/W, R/WC</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:9</td><td>保留</td></tr><tr><td>8</td><td>WOL_EN - R/W enable wake on LAN don’t shut off SLP_LANn, use with WOL_BAT_EN
使能 wake on LAN，与 WOL_BAT_EN 共同使用</td></tr><tr><td>7</td><td>WOL_BAT_EN - R/W
如果系统使用电池供电，如果该位置 1，使能 WOL，如果该位置 0，不论 WOL_EN，WOL 无
效</td></tr><tr><td>6:5</td><td>S3_ASSERTION_WIDTH - R/W
这 2 bit 值代表 S3n 信号从有效到重新无效的最小时间间隔。
11:1s
10:125ms
01:1ms
00:60us</td></tr><tr><td>4:3</td><td>S4_ASSERTION_WIDTH - R/W
这 2 bit 值代表 S4n 信号从有效到重新无效的最小时间间隔。
11:4s
10:2s
01:1s
00:125us</td></tr><tr><td>2</td><td>S4_ASSERTION_EN - R/W
0: S4n 信号从有效到重新无效的间隔为几个 RTC 周期。
1: S4n 信号从有效到重新无效的间隔为 S4_ASSERTION_WIDTH 决定。</td></tr><tr><td>1</td><td>PWR_FLR (Power Failure) - R/WC.
该位在 RTC 域，只能被 RTC_RSTn 复位。
如果置 1 表示系统发生过电源失效（进入 G3 状态），即除 RTC 以外所有供电失效过（RSMRSTn</td></tr></table>

<table><tr><td></td><td>有效过），软件通过写1将该位清除。</td></tr><tr><td>0</td><td>AFTERG3_EN - R/W
该位决定系统进入G3状态后重新供电后的动作。
0：系统在供电恢复后将自动回复到S0状态。
1：系统将恢复到S5状态，如果发生电源失效时系统在S4状态，重新供电后系统恢复到S4状态。
该位会被power button override和thermal trip事件置1。</td></tr></table>

# PM1_STS:Power Management 1 Status Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x0C</td><td>RESUME/RTC/SOC</td><td>RAWC</td></tr></table>

<table><tr><td>位域</td><td>描述</td><td>电压域</td></tr><tr><td>31:16</td><td>Reserved</td><td></td></tr><tr><td>15</td><td>WAK STS (Wake Status) - R/WC
0: 软件写 1 将该位清除。
1: 如果系统从任何一个休眠状态被唤醒事件唤醒，硬件将该位置 1。</td><td>Resume</td></tr><tr><td>14</td><td>PCIEXP_WAKESTS - R/WC.
1: PCIE 唤醒事件发生
0: 软件写 1 将该位清除</td><td>Resume</td></tr><tr><td>13:12</td><td>Reserved</td><td></td></tr><tr><td>11</td><td>PRBTNOR_STS (Power Button Override Status) - R/WC
0: 软件写 1 将该位清除
1: 当 power button override 发生时，该位置 1，系统无条件进入 G2/S5 状态，同时将 AFTERG3_EN 位置 1。</td><td>RTC</td></tr><tr><td>10</td><td>RTC_STS (RTC Status) - R/WC
0: 软件写 1 将该位清除
1: 当 RTC 产生 alarm 时该位置 1。此外当 RTC_EN 有效时，该位产生唤醒事件。</td><td>Resume</td></tr><tr><td>9</td><td>Reserved</td><td></td></tr><tr><td>8</td><td>PWRBTN_STS (Power Button Status) - R/WC
0: 软件写 1 将该位清除。Thermal trip 会清除该位。
1: 当按下 PWRBTNn 保持 16ms 以上（4s 以下）时，该位会置 1。
在 S0 状态时，当 PWRBTN_EN 和 PWRBTN_STS 同时有效时，将产生中断。
在 S3-S5 任何休眠状态时，如果 PWRBTN_STS 置位，系统将恢复。</td><td>Resume</td></tr><tr><td>7:1</td><td>保留</td><td></td></tr><tr><td>0</td><td>TMROF_STS (PM Timer Overflow Status) - R/WC
0: 软件写 1 将该位清除
1: 当 24bit 计数器（周期 8ns）的最高位翻转时，该位置 1，该记时功能推荐使用 HPET 完成。</td><td>SOC</td></tr></table>

# PM1_EN:Power Management 1 Enable Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x10</td><td>RESUME/RTC/SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td><td>电压域</td></tr><tr><td>31:15</td><td>保留</td><td></td></tr><tr><td>14</td><td>PCIEXP_WAKE_DIS - R/W
当置位时，不产生PCIE唤醒事件，但是该位的值不影响PCIEXP_WAKE_STS的值。</td><td>resume</td></tr><tr><td>13:11</td><td>保留</td><td></td></tr><tr><td>10</td><td>RTC_EN (RTC Event Enable) - R/W
RTC 唤醒和中断使能</td><td>rtc</td></tr><tr><td>9</td><td>保留</td><td></td></tr><tr><td>8</td><td>PWRBTN_EN (Power Button Enable) - R/W.
PWRBTN 中断事件产生使能，该位不影响 PWRBTN 唤醒事件产生。</td><td>resume</td></tr><tr><td>7:1</td><td>保留</td><td></td></tr><tr><td>0</td><td>TMROF_EN (PM Timer Overflow Enable) - R/W.</td><td>SOC</td></tr></table>

<table><tr><td></td><td>如果该位置位，TMROF_STS将产生中断。</td><td></td></tr></table>

# PM1_CNT : Power Management 1 Control Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x14</td><td>RESUME/RTC/SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td><td>电压域</td></tr><tr><td>31:14</td><td>保留</td><td></td></tr><tr><td>13</td><td>SLP_EN(Sleep Enable) - R/W.
该位写1将会使系统进入SLP_TYP声明的休眠状态，进入相关休眠状态后该位自动恢复为0。</td><td>resume</td></tr><tr><td>12:10</td><td>SLP_TYP(Sleep Type) - R/W.
该3bit表示系统的休眠状态。
000: 表示S0状态。
001: Reserved.
010: Reserved.
011: Reserved.
100: Reserved.
101: Suspend-to-RAM.S3n信号有效，进入S3状态。
110: Suspend-to-Disk.S3n.S4n信号有效，进入S4状态。
111: Soft Off.S3n.S4n.S5n信号有效，进入S5状态。</td><td>rtc</td></tr><tr><td>9:1</td><td>Reserved</td><td></td></tr><tr><td>0</td><td>INT_EN - R/W
中断使能开关，使能电源管理控制器中断信号的产生。</td><td>SOC</td></tr></table>

# PM1_TMR : Power Management 1 Timer

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x18</td><td>SOC</td><td>RO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:24</td><td>保留</td></tr><tr><td>23:0</td><td>TMR_VAL(Timer Value) - RO.
计数器计数，周期8ns。当23位翻转时，置位TNROF_STS位。
推荐使用HPET。</td></tr></table>

# P_CNT : Processor Control Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x1C</td><td>SOC</td><td>R/W, RO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:5</td><td>保留</td></tr><tr><td>4</td><td>THTL_EN - R/W
当系统处于C0状态时，使能该位可以通过clock throttling对CPU时钟进行控制。</td></tr><tr><td>3:1</td><td>THTL_DTY - R/W
该3-bit确定throttling的百分比，throttling周期是1024APB时钟。
000: no throttling
001: 87.5% throttling
010: 75% throttling
011: 62.5% throttling
100: 50% throttling
101: 37.5% throttling
110: 25% throttling
111: 12.5% throttling</td></tr><tr><td>0</td><td>保留</td></tr></table>

# GPE0_STS : General Purpose Event0 Status Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x28</td><td>RESUME</td><td>R/WC</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:16</td><td>保留</td></tr><tr><td>15:10</td><td>USB[6:1]_STS - R/WC.
只有第10位有意义,15:11位暂无意义。
0:软件写1将该位清除。
1:当USB发生wake事件时这些位被置位,当USBx_EN位有效时,产生唤醒事件或中断。</td></tr><tr><td>9</td><td>保留</td></tr><tr><td>8</td><td>RISTS - R/WC.
0:软件写1将该位清除。
1:当RIn信号有效时被置位。</td></tr><tr><td>7</td><td>BATLOWSTS - R/WC.
0:软件写1将该位清除。
1:当BATLOWn信号有效时被置位
如果BATLOW_EN有效,BATLOWSTS将产生中断。该位不产生唤醒事件。</td></tr><tr><td>6</td><td>GMAC1STS - R/WC.
0:软件写1将该位清除。
1:当GMAC1发生wake事件时这些位被置位,当GMAC1_EN位有效时,产生唤醒事件或中断。</td></tr><tr><td>5</td><td>GMAC0STS - R/WC.
0:软件写1将该位清除。
1:当GMAC0发生wake事件时这些位被置位,当GMAC0_EN位有效时,产生唤醒事件或中断。</td></tr><tr><td>4</td><td>LIDSTS - R/WC.
0:软件写1将该位清除。
1:当LID_EN位有效时,产生唤醒事件。</td></tr><tr><td>3</td><td>CTWSTS - R/WC.
CPU thermal warming发生</td></tr><tr><td>2</td><td>CTASTS - R/WC.
CPU thermal alert发生</td></tr><tr><td>1</td><td>PWRSWITCHSTS - R/WC.
供电状态发生变化,PWRYP变化。该位会产生中断。</td></tr><tr><td>0</td><td>保留</td></tr></table>

# GPE0_EN:General Purpose Event0 Status Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x2C</td><td>RESUME/RTC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td><td>电压域</td></tr><tr><td>31:16</td><td>保留</td><td></td></tr><tr><td>15:10</td><td>USB[6:1]_EN - R/W.
0:无效
1:使能USBxSTS产生唤醒事件,当回到S0将产生中断信号。</td><td></td></tr><tr><td>9</td><td>保留</td><td></td></tr><tr><td>8</td><td>RI_EN - R/W.
0:无效
1:使能RInSTS产生唤醒事件,当回到S0将产生中断信号。</td><td>RTC</td></tr><tr><td>7</td><td>BATLOW_EN - R/W.
0:无效
1:使能BATLOWn产生中断事件。</td><td>RTC</td></tr><tr><td>6</td><td>GMAC1_EN - R/W.
0:无效
1:使能GMAC1STS产生唤醒事件,当回到S0将产生中断信号。</td><td>RTC</td></tr><tr><td>5</td><td>GMAC0_EN - R/W.
0:无效</td><td></td></tr></table>

<table><tr><td></td><td>1：使能 GMAC0_STS 产生唤醒事件，当回到 SO 将产生中断信号。</td><td></td></tr><tr><td>4</td><td>LID_EN - R/W.
0：无效
1：使能 LID_STS 产生唤醒事件，SO 状态时将产生中断信号。</td><td></td></tr><tr><td>3</td><td>CTW_EN - R/W
使能 CPU THERMAL WARNING 产生中断。</td><td></td></tr><tr><td>2</td><td>CTA_EN - R/W
使能 CPU THERMAL ALERT 产生中断。</td><td></td></tr><tr><td>1</td><td>PWRSWITCH_EN - R/W
使能 PWRSWITCH_STS 产生中断。</td><td></td></tr><tr><td>0</td><td>LID_POL - R/W
该位设置 LID 的极性。</td><td></td></tr></table>

# RST_CNT : Reset Control Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x30</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:2</td><td>保留</td></tr><tr><td>1</td><td>WD_EN - R/W
Watch dog 功能使能</td></tr><tr><td>0</td><td>OS_RST - R/W
软件写该位使系统复位。</td></tr></table>

# WD_SET : Watch Dog Set Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x34</td><td>SOC</td><td>WO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:1</td><td>保留</td></tr><tr><td>0</td><td>写该位将重填 watch dog 计数器</td></tr></table>

# WD_Timer : Watch Dog Timer Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x38</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:0</td><td>该寄存器的值为 watch dog 重填的值，复位后为全 1。</td></tr></table>

# THSENS_CNT : CPU Thermal Sensor Control Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x4C</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:24</td><td>WARNING_TMP - R/W
CPU 温度超过该值，如果被使能，将产生中断。如果温度位降低到该值以下，不会重复产生中断。
推荐操作：系统采取降低功耗操作。</td></tr><tr><td>23:16</td><td>ALERT_TMP - R/W
CPU 温度超过该值，如果被使能，将产生中断。如果温度位降低到该值以下，不会重复产生中断。
推荐操作：采取正常关闭系统操作</td></tr></table>

<table><tr><td rowspan="2">15:8</td><td>TRIP_TMP - R/W</td></tr><tr><td>CPU 温度超过该值，如果被使能，系统无条件进入 G2/S5 状态。</td></tr><tr><td rowspan="2">7:0</td><td>OFFSET - R/W</td></tr><tr><td>设置温度传感器的校正偏移（软件），最高位为符号位。</td></tr></table>

# GEN_RTC_1:General RTC Register 1

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x50</td><td>RTC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:0</td><td>RTC 通用寄存器</td></tr></table>

# GEN_RTC_2:General RTC Register 2

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x54</td><td>RTC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:0</td><td>RTC 通用寄存器</td></tr></table>

# DPM_CFG:Dynamic Power Management Configuration

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x400</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:7</td><td>保留</td></tr><tr><td>6</td><td>DPM_EN_GMAC1 - R/W
使能 GMAC1 的时钟电源管理</td></tr><tr><td>5</td><td>DPM_EN_GMAC0 - R/W
使能 GMAC0 的时钟电源管理</td></tr><tr><td>4</td><td>DPM_EN SATA - R/W
使能 SATA 的时钟电源管理</td></tr><tr><td>3</td><td>DPM_EN PCIe1 - R/W
使能 PCIE1 的时钟电源管理</td></tr><tr><td>2</td><td>DPM_EN PCIe0 - R/W
使能 PCIE0 的时钟电源管理</td></tr><tr><td>1</td><td>DPM_EN GPU - R/W
使能 GPU 的时钟电源管理</td></tr><tr><td>0</td><td>DPM_EN_DC - R/W
使能 DC 的时钟电源管理</td></tr></table>

# DPM_STS:Dynamic Power Management Status

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x404</td><td>SOC</td><td>RO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31</td><td>保留</td></tr><tr><td>30</td><td>DPM_RUNNING_GMAC1 - R/W
GMAC1 正在进行时钟电源状态转换</td></tr><tr><td>29</td><td>DPM_RUNNING_GMAC0 - R/W
GMAC0 正在进行时钟电源状态转换</td></tr><tr><td>28</td><td>DPM_RUNNING_SATA - R/W
SATA 正在进行时钟电源状态转换</td></tr><tr><td>27</td><td>DPM_RUNNING_PCIE1 - R/W
PCIE1 正在进行时钟电源状态转换</td></tr><tr><td>26</td><td>DPM_RUNNING_PCIE0 - R/W</td></tr></table>

<table><tr><td></td><td>PCIE0 正在进行时钟电源状态转换</td></tr><tr><td>25</td><td>DPM_RUNNING_GPUDC - R/W
GPU 和 DC 正在进行时钟电源状态转换</td></tr><tr><td>24</td><td>DPM_RUNNING_GPUDC - R/W
GPU 和 DC 正在进行时钟电源状态转换</td></tr><tr><td>23:14</td><td>保留</td></tr><tr><td>13:12</td><td>DPM_STS_GMAC1 - R/W
GMAC1 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>11:10</td><td>DPM_STS_GMAC0 - R/W
GMAC0 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>9:8</td><td>DPM_STS SATA - R/W
SATA 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>7:6</td><td>DPM_STS_PCIE1 - R/W
PCIE1 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>5:4</td><td>DPM_STS_PCIE0 - R/W
PCIE0 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>3:2</td><td>DPM_STS_GPU - R/W
GPU 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>1:0</td><td>DPM_STS_DC - R/W
DC 的时钟电源状态
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr></table>

# DPM_CNT : Dynamic Power Management Control

<table><tr><td>地址/偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x408</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:20</td><td>保留</td></tr><tr><td>19:18</td><td>DPM_PCIIE1_CG - RW
PCIIE1的两个PORT的独立clock gating使能</td></tr><tr><td>17:14</td><td>DPM_PCIIE0_CG - RW
PCIIE0的四个PORT的独立clock gating使能</td></tr><tr><td>13:12</td><td>DPM_TGT_GMAC1 - R/W
GMAC1的时钟电源控制
00: D0 ALL_ON 正常工作
01: D1 STP_CLK 停时钟
11: D3 ALL OFF 电源关断（保留，未实现）</td></tr><tr><td>11:10</td><td>DPM_TGT_GMAC0 - R/W</td></tr></table>

<table><tr><td></td><td>GMAC0的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr><tr><td>9:8</td><td>DPM_TGT_SATA - R/W
SATA的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr><tr><td>7:6</td><td>DPM_TGT_PCE1 - R/W
PCIE1的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr><tr><td>5:4</td><td>DPM_TGT_PCE0 - R/W
PCIE0的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr><tr><td>3:2</td><td>DPM_TGT_GPU - R/W
GPU的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr><tr><td>1:0</td><td>DPM_TGT_DC - R/W
DC的时钟电源状态
00: D0 ALL_ON正常工作
01: D1 STP_CLK停时钟
11: D3 ALL_OFF电源关断（保留，未实现）</td></tr></table>

# DVFS_CFG : Dynamic Voltage Frequency Scaling Config Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x410</td><td>SOC</td><td>R/W, RO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:28</td><td>保留</td></tr><tr><td>27:22</td><td>VID_DEFAULT-RO
上电时默认VID值</td></tr><tr><td>21:12</td><td>VOLT_ADJUST_TIME - R/W
该部分值表示DVFS电压转换时的延时值，该延时值期间可进行保护操作。
范围：0—1000us。</td></tr><tr><td>11:8</td><td>FREQ_ADJUST_TIME - R/W
该部分值表示DVFS电压转换时的延时值，单位为apb总线时钟周期</td></tr><tr><td>7:6</td><td>DVFS_CNT_1(DVFS Control 1) - R/W
DVFS转换时何时进行保护操作
00:不进行保护操作。
01:电压转换阶段进行保护操作。
10:频率变换是进行保护操作。
11:电压和频率转换阶段都进行保护操作。</td></tr><tr><td>5:4</td><td>DVFS_CNT_0(DVFS Control 2) - R/W
DVFS转换时保护操作类型
00:停时钟。
01:使用备份时钟。
10:保留。
11:对时钟设置不做操作。</td></tr><tr><td>3</td><td>保留</td></tr><tr><td>2</td><td>VID_VALID - R/W
使能电压控制</td></tr></table>

<table><tr><td>1</td><td>PWROK_MASK_EN - R/W
如果有效，在DVFS电压转换阶段时屏蔽PWROK信号</td></tr><tr><td>0</td><td>DVFS_EN - R/W
该位使能处理器核DVFS功能
0：无效DVFS使能 1：使能DVFS功能</td></tr></table>

# DVFS_STS: Dynamic Voltage Frequency Scaling Status Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x414</td><td>SOC</td><td>RO</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:23</td><td>保留</td></tr><tr><td>22:16</td><td>FEQ_STS-RO
CPU分频器分频值。</td></tr><tr><td>9:4</td><td>VID_STS - RO
当前VID值</td></tr><tr><td>3:1</td><td>保留</td></tr><tr><td>0</td><td>CPU_DVFS_STS(DVFS status) - RO.
0: DVFS控制器空闲，可进行DVFS操作
1: 系统正在进行DVFS转换。保留</td></tr></table>

# DVFS_CNT: Dynamic Voltage Frequency Scaling Control Register

<table><tr><td>地址偏移</td><td>电压域</td><td>属性</td></tr><tr><td>0x418</td><td>SOC</td><td>R/W</td></tr></table>

<table><tr><td>位域</td><td>描述</td></tr><tr><td>31:23</td><td>保留</td></tr><tr><td>22:16</td><td>FEQ_TGT - R/W
修改CPU分频器分频值。
22:update en
21:DIV enable
20:16 DIV number</td></tr><tr><td>15:10</td><td>保留</td></tr><tr><td>9:4</td><td>VID_TGT - R/W
设置目标电压值（目标VID）</td></tr><tr><td>3</td><td>保留</td></tr><tr><td>2</td><td>DVFS_POL - R/W
写1表示需提升性能，升压升频。
写0表示降低性能节约功耗，降压降频。</td></tr><tr><td>1</td><td>VID_UPDATE_EN R/W
如果不想改变电压VID值，将该位置0（即只进行频率转换）</td></tr><tr><td>0</td><td>DVFS_START - R/W
写1开始一个DVST转换。</td></tr></table>

# 22 RTC

# 22.1概述

实时时钟（RTC）单元可以在主板上电后进行配置，当主板断电后，该单元仍然运作，可以仅靠板上的电池供电就正常运行。RTC单元运行时电流仅几个微安。

RTC包含振荡器，结合外部32.768KHz晶体产生工作时钟。该时钟用于时间信息的维护以及产生各种定时和计数中断。

RTC模块中包含两个计数器，分别为TOY（TimeofYear）计数器和RTC计数器。其中TOY计数器按年月日时分秒计数，精度为以0.1秒；RTC计数器以32.768KHz时钟计数，宽度为32位。

# 22.2访问地址

电源管理模块内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>7</td><td>固定为 7</td></tr><tr><td>[11]</td><td>ACPI/RTC</td><td>0x0 代表 ACPI；0x1 代表 RTC</td></tr><tr><td>[10:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 22.3寄存器描述

RTC模块寄存器和ACPI模块位于同一地址空间内，其偏移为  $0\mathrm{x}800$  ，其基地址由BAR给定，所有寄存器位宽均为32位。

# 22.3.1寄存器地址列表

<table><tr><td>名称</td><td>偏移地址</td><td>位宽</td><td>RW</td><td>描述</td></tr><tr><td>sys_toytrim</td><td>0x20</td><td>32</td><td>RW</td><td>软件必须初始化为 0</td></tr><tr><td>sys_towrite0</td><td>0x24</td><td>32</td><td>W</td><td>TOY 低 32 位数值写入</td></tr><tr><td>sys_towrite1</td><td>0x28</td><td>32</td><td>W</td><td>TOY 高 32 位数值写入</td></tr><tr><td>sys_toread0</td><td>0x26</td><td>32</td><td>R</td><td>TOY 低 32 位数值读出</td></tr><tr><td>sys_toread1</td><td>0x30</td><td>32</td><td>R</td><td>TOY 高 32 位数值读出</td></tr><tr><td>sys_tomatch0</td><td>0x34</td><td>32</td><td>RW</td><td>TOY 定时中断 0</td></tr><tr><td>sys_tomatch1</td><td>0x38</td><td>32</td><td>RW</td><td>TOY 定时中断 1</td></tr><tr><td>sys_tomatch2</td><td>0x36</td><td>32</td><td>RW</td><td>TOY 定时中断 2</td></tr><tr><td>sys_rtcctrl</td><td>0x40</td><td>32</td><td>RW</td><td>TOY 和 RTC 控制寄存器</td></tr><tr><td>sys_rtctrim</td><td>0x60</td><td>32</td><td>RW</td><td>软件必须初始化</td></tr><tr><td>sys_rtcwrite0</td><td>0x64</td><td>32</td><td>W</td><td>RTC 定时计数写入</td></tr><tr><td>sys_rtcread0</td><td>0x68</td><td>32</td><td>R</td><td>RTC 定时计数读出</td></tr><tr><td>sys_rtcmatch0</td><td>0x6C</td><td>32</td><td>RW</td><td>RTC 时钟定时中断 0</td></tr><tr><td>sys_rtcmatch1</td><td>0x70</td><td>32</td><td>RW</td><td>RTC 时钟定时中断 1</td></tr><tr><td>sys_rtcmatch2</td><td>0x74</td><td>32</td><td>RW</td><td>RTC 时钟定时中断 2</td></tr></table>

# 22.3.2SYS_TOYWRITE0

中文名： TOY计数器低32位数值寄存器位宽：[31：0]偏移量： 0x24复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:26</td><td>TOY_MONTH</td><td>W</td><td></td><td>月，范围 1~12</td></tr><tr><td>25:21</td><td>TOY_DAY</td><td>W</td><td></td><td>日，范围 1~31</td></tr><tr><td>20:16</td><td>TOY_HOUR</td><td>W</td><td></td><td>小时，范围 0~23</td></tr><tr><td>15:10</td><td>TOY_MIN</td><td>W</td><td></td><td>分，范围 0~59</td></tr><tr><td>9:4</td><td>TOY_SEC</td><td>W</td><td></td><td>秒，范围 0~59</td></tr><tr><td>3:0</td><td>TOY_MILLSEC</td><td>W</td><td></td><td>0.1秒，范围 0~9</td></tr></table>

# 22.3.3SYS_TOYWRITE1

中文名： TOY计数器高32位数值寄存器位宽：[31：0]偏移量： 0x28复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:0</td><td>TOY_YEAR</td><td>W</td><td></td><td>年，范围 0～16383</td></tr></table>

# 22.3.4SYS_TOYREAD0

中文名： TOY计数器低32位数值寄存器位宽：[31：0]偏移量： 0x2C复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:26</td><td>TOY_MONTH</td><td>R</td><td></td><td>月，范围 1~12</td></tr><tr><td>25:21</td><td>TOY_DAY</td><td>R</td><td></td><td>日，范围 1~31</td></tr><tr><td>20:16</td><td>TOY_HOUR</td><td>R</td><td></td><td>小时，范围 0~23</td></tr><tr><td>15:10</td><td>TOY_MIN</td><td>R</td><td></td><td>分，范围 0~59</td></tr><tr><td>9:4</td><td>TOY_SEC</td><td>R</td><td></td><td>秒，范围 0~59</td></tr><tr><td>3:0</td><td>TOY_MILLSEC</td><td>R</td><td></td><td>0.1秒，范围 0~9</td></tr></table>

# 22.3.5SYS_TOYREAD1

中文名： TOY计数器高32位数值寄存器位宽：[31：0]偏移量： 0x30复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:0</td><td>TOY_YEAR</td><td>R</td><td></td><td>年，范围 0～16383</td></tr></table>

# 22.3.6SYS_TOYMATCH0/1/2

中文名： TOY计数器中断寄存器0/1/2寄存器位宽：[31：0]偏移量： 0x34/38/3C复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:26</td><td>YEAR</td><td>RW</td><td></td><td>年，范围 0～16383</td></tr><tr><td>25:22</td><td>MONTH</td><td>RW</td><td></td><td>月，范围 1～12</td></tr><tr><td>21:17</td><td>DAY</td><td>RW</td><td></td><td>日，范围 1～31</td></tr><tr><td>16:12</td><td>HOUR</td><td>RW</td><td></td><td>小时，范围 0～23</td></tr><tr><td>11:6</td><td>MIN</td><td>RW</td><td></td><td>分，范围 0～59</td></tr><tr><td>5:0</td><td>SEC</td><td>RW</td><td></td><td>秒，范围 0～59</td></tr></table>

# 22.3.7SYS_RTCCTRL

中文名： RTC定时器中断寄存器0/1/2寄存器位宽：[31：0]偏移量： 0x40

复位值：无

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:24</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>23</td><td>ERS</td><td>R</td><td>0</td><td>REN(bit13)写状态</td></tr><tr><td>22:21</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>20</td><td>RTS</td><td>R</td><td>0</td><td>Sys_rcttrim 写状态</td></tr><tr><td>19</td><td>RM2</td><td>R</td><td>0</td><td>Sys_rctmatch2 写状态</td></tr><tr><td>18</td><td>RM2</td><td>R</td><td>0</td><td>Sys_rctmatch2 写状态</td></tr><tr><td>17</td><td>RM0</td><td>R</td><td>0</td><td>Sys_rctmatch0 写状态</td></tr><tr><td>16</td><td>RS</td><td>R</td><td>0</td><td>Sys_rctwrite 写状态</td></tr><tr><td>15</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>14</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>13</td><td>REN</td><td>R/W</td><td>0</td><td>RTC 使能，高有效。需要初始化为 1</td></tr><tr><td>12</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>11</td><td>TEN</td><td>R/W</td><td>0</td><td>TOY 使能，高有效。需要初始化为 1</td></tr><tr><td>10</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>9</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>8</td><td>EO</td><td>R/W</td><td>0</td><td>0: 32.768k 晶振禁止；
1: 32.768k 晶振使能</td></tr><tr><td>7</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>6</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>5</td><td>32S</td><td>R</td><td>0</td><td>0: 32.768k 晶振不工作；
1: 32.768k 晶振正常工作。</td></tr></table>

<table><tr><td>4</td><td>保留</td><td>R</td><td>0</td><td>保留，置 0</td></tr><tr><td>3</td><td>TM2</td><td>R</td><td>0</td><td>Sys_toymatch2 写状态</td></tr><tr><td>2</td><td>TM1</td><td>R</td><td>0</td><td>Sys_toymatch1 写状态</td></tr><tr><td>1</td><td>TM0</td><td>R</td><td>0</td><td>Sys_toymatch0 写状态</td></tr><tr><td>0</td><td>TS</td><td>R</td><td>0</td><td>Sys_toywrite 写状态</td></tr></table>

# 22.3.8SYS_RTCWRITE

中文名： RTC计数器写入端口  寄存器位宽：[31:0]  偏移量： 0x64  复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:0</td><td>RTC</td><td>W</td><td></td><td></td></tr></table>

# 22.3.9SYS_RTCREAD

中文名：RTC计数器读出端口  寄存器位宽：[31:0]  偏移量： 0x68  复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:0</td><td>RTC</td><td>R</td><td></td><td></td></tr></table>

# 22.3.10SYS_RTCMATCH0/1/2

中文名： RTC定时器中断寄存器0/1/2

寄存器位宽：[31:0]  偏移量： 0x6C/70/74  复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>访问</td><td>缺省</td><td>描述</td></tr><tr><td>31:26</td><td>RTC</td><td>RW</td><td></td><td></td></tr></table>

# 23 加解密

# 23.1 DES

# 23.1.1 DES 功能概述

DES控制器采用32位的APB接口，支持使用DES/TDEA算法进行加/解密，并支持使用APB接口进行DMA操作。DES控制器采用了OpenCore的面积节约方案的DES3加/解密单元作为实现加/解密的运算单元。这个运算单元每16个时钟周期完成一次DES加/解密，每48个时钟周期完成一次TDEA加/解密。为了减少加/解密的等待时间，DES控制器使用2个时钟域分别处理APB接口的操作和进行DES加/解密计算（DES加解密使用的时钟的频率更高）。两个不同的时钟域通过2个4项的64bit位宽异步FIFO交换加/解密运算前后的数据。

DES控制器在使用前需要先配置密钥以及Command寄存器。控制器有3个使用64bit格式存储的密钥：Key0、Key1、Key2。其中，Key1、Key2仅在TDEA算法是需要进行配置。控制器不检查密钥中的校验位。待运算（加密还是解密由Command[1]的值确定）的数据通过Data_low和Data_high写入运算数据FIFO。DES运算模块从运算数据FIFO读取数据后进行加/解密运算迭代，迭代的结果被送入运算结果FIFO。通过APB接口查询Command[4]可以获知运算结果是否就绪。当Command[4]的值为0时，可以从APB接口通过Data_low和Data_high读取运算结果FIFO中的运算结果。

23.1.2DES访问地址：  

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>8</td><td>固定为 8</td></tr><tr><td>[11:06]</td><td>0</td><td>保留</td></tr><tr><td>[05:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 23.1.3DES寄存器描述

DES的寄存器列表及寄存器说明如下：

<table><tr><td>地址偏移</td><td>名称</td><td>说明</td></tr><tr><td>0x0</td><td>Key0_low</td><td>DES 密钥的低 32 位/TDEA 密钥 0 的低 32 位</td></tr><tr><td>0x4</td><td>Key0_high</td><td>DES 密钥的高 32 位/TDEA 密钥 0 的高 32 位</td></tr><tr><td>0x8</td><td>Key1_low</td><td>TDEA 密钥 1 的低 32 位</td></tr><tr><td>0xc</td><td>Key1_high</td><td>TDEA 密钥 1 的高 32 位</td></tr><tr><td>0x10</td><td>Key2_low</td><td>TDEA 密钥 2 的低 32 位</td></tr><tr><td>0x14</td><td>Key2_high</td><td>TDEA 密钥 2 的高 32 位</td></tr><tr><td>0x18</td><td>Data_low</td><td>非 DMA 模式（Command[2] = 1&#x27;b0）：待加/解密数据低 32 位的写入端口；加/解密后数据低 32 位的读出端口。</td></tr></table>

<table><tr><td></td><td></td><td>DMA 模式（Command[2] = 1&#x27;b1）：Data_low 和 Data_high 不做区分。先写入/读取的是数据的低 32 位，后写入/读取的是数据的高 32 位。</td></tr><tr><td>0x1c</td><td>Data_high</td><td>非 DMA 模式（Command[2] = 1&#x27;b0）：待加/解密数据高 32 位的写入端口；加/解密后数据高 32 位的读出端口。DMA 模式（Command[2] = 1&#x27;b1）：Data_low 和 Data_high 不做区分。先写入/读取的是数据的低 32 位，后写入/读取的是数据的高 32 位。</td></tr><tr><td>0x20</td><td>Command</td><td>命令和状态控制寄存器</td></tr><tr><td>0x24</td><td rowspan="3">Rev</td><td rowspan="3">保留</td></tr><tr><td>0x28</td></tr><tr><td>0x2c</td></tr></table>

Comand寄存器位域说明：

<table><tr><td>位域</td><td>复位值</td><td>名称</td><td>属性</td><td>说明</td></tr><tr><td>0</td><td>0</td><td>des3</td><td>RW</td><td>0：使用 DES 算法
1：使用 TDEA 算法</td></tr><tr><td>1</td><td>0</td><td>decrypt</td><td>RW</td><td>0：加密操作
1：解密操作</td></tr><tr><td>2</td><td>0</td><td>dma_start</td><td>RW</td><td>写入 1 启动 DMA 操作，写入 0 无影响
在 DMA 操作完成前此位保持为 1
当 DMA 操作完成时此位自动清零</td></tr><tr><td>3</td><td>0</td><td>dma_done</td><td>RW1C</td><td>当 DMA 操作完成时，此位置 1
向此位写入 1，则清零</td></tr><tr><td>4</td><td>1</td><td>dout_empty</td><td>RO</td><td>0：数据读 FIFO 非空
1：数据读 FIFO 为空</td></tr><tr><td>5</td><td>0</td><td>din_full</td><td>RO</td><td>0：数据写 FIFO 未满
1：数据写 FIFO 已满</td></tr><tr><td>7:6</td><td>0</td><td>Rev</td><td>RO</td><td>保留</td></tr><tr><td>31:8</td><td>0</td><td>dma_count</td><td>RW</td><td>需要进行 DMA 加/解密的 64 位数的个数</td></tr></table>

# 23.2 AES

# 23.2.1 AES功能概述

AES控制器采用32位的APB接口，支持使用128- bitKEY、192- bitKEY、256- bitKEY进行AES算法加/解密，并支持使用APB接口进行DMA操作。在使用DMA功能时，支持CTR模式进行加/解密。AES控制器有3个主要模块：KEY扩展模块、加密模块、解密模块。加/解密运算模块每11个时钟周期完成一次128- bitKEY的AES加/解密，每13个时钟周期完成一次192- bitKEY的AES加/解密，每15个时钟周期完成一次256- bitKEY的AES加/

解密。为了减少加/解密的等待时间，AES控制器使用2个时钟域分别处理APB接口的操作和进行AES加/解密计算（AES加解密使用的时钟的频率更高）。KEY扩展模块工作在速度较慢的APB时钟域。两个不同的时钟域通过2个4项的128bit位宽异步FIFO交换加/解密运算前后的数据。

AES控制器在使用前需要先配置密钥以及Command寄存器。AES的密钥、明文和密文均以小尾端的格式进行存储。控制器使用8个32bit寄存器以小尾端的格式存储密钥：Key0、Key1、Key2、key3、key4、key5、key6、key7。其中：Key4、Key5仅在192- bitKEY和256- bitKEY时需要进行配置；Key6、Key7仅在256- bitKEY时需要进行配置。待运算（加密还是解密由Command[1]的值确定）的数据通过4个32位寄存器地址（Data0、Data1、Data2、Data3）写入运算数据FIFO。AES运算模块从运算数据FIFO读取数据后进行加/解密运算迭代，迭代的结果被送入运算结果FIFO。通过APB接口查询Command[4]可以获知运算结果是否就绪。当Command[4]的值为0时，可以从APB接口通过Data0、Data1、Data2、Data3读取运算结果FIFO中的运算结果。

# 23.2.2 AES访问地址：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>9</td><td>固定为 9</td></tr><tr><td>[11:06]</td><td>0</td><td>保留</td></tr><tr><td>[05:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 23.2.3 AES寄存器描述

AES的寄存器列表及寄存器说明如下：

<table><tr><td>地址偏移</td><td>名称</td><td>说明</td></tr><tr><td>0x0</td><td>Key0</td><td>AES 密钥的 Key[31:0]，以小尾端形式存储的 AES 密钥的最低 32 位</td></tr><tr><td>0x4</td><td>Key1</td><td>AES 密钥的 Key[63:32]</td></tr><tr><td>0x8</td><td>Key2</td><td>AES 密钥的 Key[95:64]</td></tr><tr><td>0xc</td><td>Key3</td><td>AES 密钥的 Key[127:96]，在使用 128-bit Key 时为以小尾端形式存储的 AES 密钥的最高 32 位</td></tr><tr><td>0x10</td><td>Key4</td><td>AES 密钥的 Key[159:128]，仅在使用 192/256-bit 的 Key 时使用</td></tr><tr><td>0x14</td><td>Key5</td><td>AES 密钥的 Key[191:160]，仅在使用 192/256-bit 的 Key 时使用，在使用 192-bit Key 时为以小尾端形式存储的 AES 密钥的最高 32 位</td></tr><tr><td>0x18</td><td>Key6</td><td>AES 密钥的 Key[223:192]，仅在使用 256-bit 的 Key 时使用</td></tr><tr><td>0x1c</td><td>Key7</td><td>AES 密钥的 Key[255:224]，仅在使用 256-bit 的 Key 时使用，在使用 256-bit Key 时为以小尾端形式存储的 AES 密钥的最高 32 位</td></tr></table>

<table><tr><td>0x20</td><td>Data0</td><td>非 DMA模式(Command[2]=1&#x27;b0):待加/解密数据最低32位的写入端口和加/解密后数据最低32位的读出端口DMA模式(Command[2]=1&#x27;b1):Data0-Data3不做区分,数据按照从低到高的小尾端顺序写入或读出</td></tr><tr><td>0x24</td><td>Data1</td><td>非DMA模式(Command[2]=1&#x27;b0):待加/解密数据63～32位的写入端口和加/解密后数据高63～32位的读出端口DMA模式(Command[2]=1&#x27;b1):Data0-Data3不做区分,数据按照从低到高的小尾端顺序写入或读出</td></tr><tr><td>0x28</td><td>Data2</td><td>非DMA模式(Command[2]=1&#x27;b0):待加/解密数据95～64位的写入端口和加/解密后数据高95～64位的读出端口DMA模式(Command[2]=1&#x27;b1):Data0-Data3不做区分,数据按照从低到高的小尾端顺序写入或读出</td></tr><tr><td>0x2c</td><td>Data3</td><td>非DMA模式(Command[2]=1&#x27;b0):待加/解密数据的最高32位（127～96）的写入端口和加/解密后数据最高32位（127～96）的读出端口DMA模式(Command[2]=1&#x27;b1):Data0-Data3不做区分,数据按照从低到高的小尾端顺序写入或读出</td></tr><tr><td>0x30</td><td>Ctr_init_val</td><td>使用CTR模式时CTR计数器的初始值。要使此寄存器的值发生作用需要先配置此寄存器的值，再向command[0]写入0。</td></tr><tr><td>0x34</td><td>Command</td><td>命令和状态控制寄存器</td></tr><tr><td>0x38</td><td rowspan="2">Rev</td><td rowspan="2">保留</td></tr><tr><td>0x3c</td></tr></table>

Comand寄存器位域说明：

<table><tr><td>位域</td><td>复位值</td><td>名称</td><td>属性</td><td>说明</td></tr><tr><td>0</td><td>0</td><td>Ctrl_mode</td><td>RW</td><td>0：使用普通的 AES 算法进行加/解密
1：使用 CTR 模式进行 AES 算法的加/解密</td></tr><tr><td>1</td><td>0</td><td>decrypt</td><td>RW</td><td>0：加密操作
1：解密操作</td></tr><tr><td>2</td><td>0</td><td>dma_start</td><td>RW</td><td>写入 1 启动 DMA 操作，写入 0 无影响
在 DMA 操作完成前此位保持为 1
当 DMA 操作完成时此位自动清零</td></tr><tr><td>3</td><td>0</td><td>dma_done</td><td>RW1C</td><td>当 DMA 操作完成时，此位置 1，中断输出信号变为高电平
向此位写入 1，则清零，中断输出信号变为低电平</td></tr><tr><td>4</td><td>1</td><td>dout_empty</td><td>RO</td><td>0：数据读 FIFO 非空
1：数据读 FIFO 为空</td></tr></table>

<table><tr><td>5</td><td>0</td><td>din_full</td><td>RO</td><td>0: 数据写 FIFO 未满
1: 数据写 FIFO 已满</td></tr><tr><td>7:6</td><td>0</td><td>Kr_mode</td><td>RW</td><td>0: 128-bit KEY
1: 192-bit KEY
2: 256-bit KEY
3: 保留</td></tr><tr><td>31:8</td><td>0</td><td>dma_count</td><td>RW</td><td>需要进行 DMA 加/解密的 128 位数的个数</td></tr></table>

# 23.3 RSA

23.3.1RSA访问地址：  

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0xA</td><td>固定为 0xA</td></tr><tr><td>[11]</td><td>0</td><td>保留</td></tr><tr><td>[10:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 23.4 RNG

23.4.1 RNG访问地址：  

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0xB</td><td>固定为 0xB</td></tr><tr><td>[11:00]</td><td>0</td><td>保留</td></tr></table>

直接通过以上地址访问 RNG 会返回一个 32 位随机数。

# 24 SDIO 控制器

# 24.1功能概述

龙芯2K1000集成了一个SDIO控制器，用于SDMemory和SDIO卡的读写。SDIO控制器特性如下：

兼容SD存储卡规格（2.0版本）兼容SDIO卡规格（2.0版本）8字（32字节）数据发送/接收FIFO $\bullet$  扩展的256位SD卡状态寄存器8位预分频逻辑（频率  $\coloneqq$  系统时钟  $(\mathrm{p + 1})$  一DMA数据传输模式1位/4位（宽总线）的SD模式

# 24.2访问地址及引脚复用

SDIO控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0xC</td><td>固定为 0xC</td></tr><tr><td>[11:08]</td><td>0</td><td>保留</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于SDIO模块，使用时要注意将对应的引脚设置为相应的功能。

与SDIO相关的引脚设置寄存器为5.1节中的sdio_sel。

# 24.3SDIO协议概述

SDIO是一个串行通信方式，主设备和从设备通过消息传递来实现数据和状态的传输。如下图是一个写多块数据的示意框图，过程如下：

1. 主设备通过命令线发送写命令消息给从设备2.从设备接收完消息之后通过命令线发送应答消息给主设备3.主设备接收到正确的应答消息后，通过数据线发送一块数据(512KByte或者更多)给从设备，并且检测数据线忙状态4.从设备接收到正确的数据后会进入编程状态，此时将数据线置为忙状态，不再响应主设备的数据请求5.主设备检测到从设备编程完成，继续发送下一块数据。6.主设备发送完最后一块数据时，通过命令线发送停止命令给从设备，收到正确应答

之后完成这次多块写操作。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/792d3abd720ddc0caacc0a2f1b74264e386fcd81e5036ec5b32d544c869da96c.jpg)  
图24-1 SD卡多块写操作示意图

多块读操作的过程和多块写操作的过程类似（等补充说明）。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/9f5e0f006ca4156a9519d8c42c5e07a22cd67d43e46b4b3b3086a09f6005671f.jpg)  
图24-2 SD卡多块读操作示意图

不同的命令都有统一的格式。一般命令的格式如下表，1bit起始位，1bit传输方向位，6bit命令序号，32bit命令参数，7bit CRC检验位再加上1bit结束位。

<table><tr><td>Bit position</td><td>47</td><td>46</td><td>[45:40]</td><td>[39:8]</td><td>[7:1]</td><td>0</td></tr><tr><td>Width (bits)</td><td>1</td><td>1</td><td>6</td><td>32</td><td>7</td><td>1</td></tr><tr><td>Value</td><td>&#x27;0&#x27;</td><td>&#x27;1&#x27;</td><td>x</td><td>x</td><td>x</td><td>&#x27;1&#x27;</td></tr><tr><td>Description</td><td>start bit</td><td>transmission bit</td><td>command index</td><td>argument</td><td>CRC7</td><td>end bit</td></tr></table>

其中command index对应命令的序号，比如命令0，cmdindex为0，命令55，cmdindex为37。不同命令的参数可能不同，具体参考SD协议规范。

# 24.4寄存器描述

SDIO控制器的寄存器详细说明如下：

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_CON</td><td>0x00</td><td>R/W</td><td>SDIO 控制寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_CON</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>•</td><td>31:9</td><td>0x0</td><td></td></tr><tr><td>soft_rst</td><td>8</td><td>0x0</td><td>软件复位，整个模块复位。复位完成后硬件自动清零</td></tr></table>

<table><tr><td>Reserved</td><td>7:1</td><td>0x0</td><td></td></tr><tr><td>enclk</td><td>0</td><td>0x0</td><td>SD时钟输出使能</td></tr></table>

<table><tr><td>寄存器名称</td><td>地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_PRE</td><td>0x04</td><td>R/W</td><td>SDIO 预分频寄存器</td><td>0x1</td></tr></table>

<table><tr><td>SDI_PRE</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:8</td><td>0x0</td><td></td></tr><tr><td>Sdi_pre</td><td>7:0</td><td>0x1</td><td>SDIO 时钟预分频值，输出频率=PCLK/预分频值</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_CMD_ARG</td><td>0x08</td><td>R/W</td><td>SDIO 命令参数寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_CMD_ARG</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>sdi_cmd_arg</td><td>31:0</td><td>0x0</td><td>命令参数</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_CMD_CON</td><td>0x0c</td><td>R/W</td><td>SDIO 命令控制寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_CMD_ARG</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:18</td><td>0x0</td><td></td></tr><tr><td>func_num_abort</td><td>17:15</td><td>0x0</td><td>SDIO卡时中断的功能号，用于多块读写时，硬件自动发送停止命令。如果auto_stop_en为0，则此位无效</td></tr><tr><td>sdio_en</td><td>14</td><td>0x0</td><td>SDIO使能信号。用于多块读写时，硬件自动发送停止命令，为1时发送CMD52，为0是发送CMD12。如果auto_stop_en为0，则此位无效</td></tr><tr><td>check_on</td><td>13</td><td>0x0</td><td>是否检查CRC，为1时有效</td></tr><tr><td>Auto_stop_en</td><td>12</td><td>0x0</td><td>硬件自动发送停止命令，多块读写时，是否硬件自动发送停止命令，为1时有效</td></tr><tr><td>Reserved</td><td>11</td><td>0x0</td><td></td></tr><tr><td>long_rsp</td><td>10</td><td>0x0</td><td>是否为136位长响应，为1时表示长消息回复</td></tr><tr><td>Wait_rsp</td><td>9</td><td>0x0</td><td>决定是否主机等待相应，为1是表示等待消息回复</td></tr><tr><td>CMST</td><td>8</td><td>0x0</td><td>命令开始，置1时开始，命令结束后硬件自动清零</td></tr><tr><td>cmd_index</td><td>7:0</td><td>0x0</td><td>带开始2位的命令索引（共8位）</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_CMD_STA</td><td>0x10</td><td>RO</td><td>SDIO 命令状态寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_CMD_STA</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:13</td><td>0x0</td><td></td></tr><tr><td>cmd_sent_fin</td><td>14</td><td>0x0</td><td>命令发送完成（包含响应）标志位，为1表示命令发送完成及响应完成</td></tr><tr><td>auto_stop</td><td>13</td><td>0x0</td><td>硬件自动发送停止命令标志位，为1表示硬件自动发送停止命令，为0则没有</td></tr><tr><td>rsp_crc_err</td><td>12</td><td>0x0</td><td>响应CRC错误，接收到的响应CRC错误。为1时表示响应CRC错误，为0时未发现</td></tr><tr><td>cmd_end</td><td>11</td><td>0x0</td><td>命令发送完成（不关心响应）。为1时表示命令发送完成，为0时未完成。</td></tr><tr><td>cmd_tout</td><td>10</td><td>0x0</td><td>命令超时。命令响应超时（64个时钟周期），或者R1b类型的命令，忙等待超时，为1时表示响应超时，为0时未超时。</td></tr><tr><td>rsp_fin</td><td>9</td><td>0x0</td><td>响应结束，接收完成从设备的返回信息。为1时表示响应结束，为0时未完成。</td></tr><tr><td>cmd_on</td><td>8</td><td>0x0</td><td>命令传输标志位。为1时表示传输进行中，为0表示结束。</td></tr><tr><td>rsp_index</td><td>7:0</td><td>0x0</td><td>从设备返回的带开始2位的响应索引（共8位）</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_RSP0</td><td>0x14</td><td>RO</td><td>SDIO 命令响应寄存器 0</td><td>0x0</td></tr></table>

<table><tr><td>SDI_RESP0</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>sdi_resp0</td><td>31:0</td><td>0x0</td><td>卡状态[31:0]（短），卡状态[127:96]（长）长响应的配置间 sdi_cmd_conf[10]</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_RSP1</td><td>0x18</td><td>RO</td><td>SDIO 命令响应寄存器 1</td><td>0x0</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_RSP2</td><td>0x1c</td><td>RO</td><td>SDIO 命令响应寄存器 2</td><td>0x0</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_RSP2</td><td>0x1c</td><td>RO</td><td>SDIO 命令响应寄存器 2</td><td>0x0</td></tr></table>

<table><tr><td>SDI_RESP2</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>sd1_resp2</td><td>31:0</td><td>0x0</td><td>未使用（短），卡状态[63:32]（长）长响应的配置</td></tr></table>

<table><tr><td></td><td></td><td></td><td>间 sdi_cmd_con[10]</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_RSP3</td><td>0x20</td><td>RO</td><td>SDIO 命令响应寄存器 3</td><td>0x0</td></tr></table>

<table><tr><td>SDI_RESP3</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>sd_i_resp3</td><td>31:0</td><td>0x0</td><td>未使用（短），卡状态[31:0]（长）长响应的配置
间 sd_i_cmd_con[10]</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_DTIMER</td><td>0x24</td><td>R/W</td><td>SDIO 命令数据超时寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_DTIMER</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:24</td><td>0x0</td><td></td></tr><tr><td>sdidtimer</td><td>23:0</td><td>0x0</td><td>数据超时计数值，用分频后的时钟计数</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_BSIZE</td><td>0x28</td><td>R/W</td><td>SDIO 块大小寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_BSIZE</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:12</td><td>0x0</td><td></td></tr><tr><td>sdibsize</td><td>11:0</td><td>0x0</td><td>块大小值（0~4095）</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_DAT_CON</td><td>0x2c</td><td>R/W</td><td>SDIO 数据控制寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_DAT_CON</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:21</td><td>0x0</td><td></td></tr><tr><td>resume_rw</td><td>20</td><td>0x0</td><td>SDIO 挂起回复读写标志位。为 1 时，SDIO 挂起后恢复之前的写操作；为 0 时，恢复之前的读操作</td></tr><tr><td>IO_resume</td><td>19</td><td>0x0</td><td>SDIO 恢复请求。在 SDIO 设备进入挂起状态后，将此位写 1，并且 IO_suspend 位写 0 后，SDIO 设备恢复之前的操作。</td></tr><tr><td>IO_suspend</td><td>18</td><td>0x0</td><td>SDIO 挂起请求。写 1 后控制器会在合适的时机发送 CMD52 命令，通知 SDIO 设备进入挂起状态。恢复操作时需要将此位写 0。</td></tr><tr><td>RwaitReq</td><td>17</td><td>0x0</td><td>读等待请求。写 1 后控制器会在合适的时机将 DAT2 拉低，通知 SDIO 设备进入读等待状态。写</td></tr></table>

<table><tr><td></td><td></td><td></td><td>0 后恢复之前的读操作。</td></tr><tr><td>wide_mode</td><td>16</td><td>0x0</td><td>位宽选择位。为 1 表示 4 线模式，为 0 表示单线模式。</td></tr><tr><td>DMA_en</td><td>15</td><td>0x0</td><td>DMA 使能。为 1 时表示使能 DMA，为 0 表示禁止 DMA</td></tr><tr><td>DTST</td><td>14</td><td>0x0</td><td>数据传输开始，写 1 时数据传输开始，数据传输结束后硬件清零。</td></tr><tr><td>Reserved</td><td>13:12</td><td>0x0</td><td></td></tr><tr><td>Blk_num</td><td>11:0</td><td>0x0</td><td>读写操作的块数。</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_DAT_CNT</td><td>0x30</td><td>R/W</td><td>SDIO 数据计数寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_DAT_CNT</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:24</td><td>0x0</td><td></td></tr><tr><td>blk_num_cnt</td><td>23:12</td><td>0x0</td><td>当前传输块的字节数</td></tr><tr><td>blk_cnt</td><td>11:0</td><td>0x0</td><td>当前传输的块数</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_DAT_STA</td><td>0x34</td><td>R0</td><td>SDIO 数据状态寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_DAT_STA</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:17</td><td>0x0</td><td></td></tr><tr><td>suspend_on</td><td>16</td><td>0x0</td><td>为1时表示正在挂起状态</td></tr><tr><td>rst_suspend</td><td>15</td><td>0x0</td><td>为1表示正在挂起复位。用于SDIO设备挂起后，控制器复位FIFO和DMA请求</td></tr><tr><td>R1b_tout</td><td>14</td><td>0x0</td><td>为1表示R1b类型命令超时</td></tr><tr><td>data_start</td><td>13</td><td>0x0</td><td>为1表示数据传输开始</td></tr><tr><td>R1b_fin</td><td>12</td><td>0x0</td><td>检测到带busy状态的命令完成。当发送带busy状态的命令时，此位为0；当busy状态结束时变成1</td></tr><tr><td>auto_stop</td><td>11</td><td>0x0</td><td>为1时表示硬件正在自动发送停止命令</td></tr><tr><td>Reserved</td><td>10</td><td>0x0</td><td></td></tr><tr><td>r_wait_req</td><td>9</td><td>0x0</td><td>读等待发生。发送读等待请求信号到SDIO卡</td></tr><tr><td>SDIO_int</td><td>8</td><td>0x0</td><td>SDIO中断标志位。为1表示检测到中断</td></tr><tr><td>crc_sta</td><td>7</td><td>0x0</td><td>数据发送后，从设备返回CRC错误</td></tr><tr><td>dat_crc</td><td>6</td><td>0x0</td><td>数据接收CRC错误</td></tr><tr><td>dat_tout</td><td>5</td><td>0x0</td><td>数据传输超时。为1时表示数据超时。</td></tr><tr><td>dat_fin</td><td>4</td><td>0x0</td><td>数据传输结束标志位（比如编程时）。为1时标志忙结束</td></tr></table>

<table><tr><td>busy_fin</td><td>3</td><td>0x0</td><td>编程错误标志位（比如编程时）。为1时标志忙结束</td></tr><tr><td>prog_err</td><td>2</td><td>0x0</td><td>编程错误标志位，为1时表示编程错误</td></tr><tr><td>tx_dat_on</td><td>1</td><td>0x0</td><td>Tx数据发送中，为1时表示正在发送，为0时发送完成</td></tr><tr><td>rx_dat_on</td><td>0</td><td>0x0</td><td>Rx数据接收中，为1时表示正在接收，为0时发送完成。</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_FIFO_STA</td><td>0x38</td><td>RO</td><td>SDIO FIFO 状态寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_FIFO_STA</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:12</td><td>0x0</td><td></td></tr><tr><td>tx_full</td><td>11</td><td>0x0</td><td>Tx FIFO 满标志位</td></tr><tr><td>tx_empty</td><td>10</td><td>0x0</td><td>Tx FIFO 空标志位</td></tr><tr><td>Reserved</td><td>9</td><td>0x0</td><td></td></tr><tr><td>rx_full</td><td>8</td><td>0x0</td><td>Rx FIFO 满标志</td></tr><tr><td>rx_empty</td><td>7</td><td>0x0</td><td>Rx FIFO 空标志位</td></tr><tr><td>Reserved</td><td>6:0</td><td>0x0</td><td></td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_INT_MASK</td><td>0x3c</td><td>R/W</td><td>SDIO 中断寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_INT_MASK</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:10</td><td>0x0</td><td></td></tr><tr><td>R1b_fin_int</td><td>9</td><td>0x0</td><td>检测到 busy 结束中断，写 1 清零</td></tr><tr><td>rsp_crc_int</td><td>8</td><td>0x0</td><td>命令响应 CRC 错误中断，写 1 清零</td></tr><tr><td>cmd_tout_int</td><td>7</td><td>0x0</td><td>命令超时中断，写 1 清零</td></tr><tr><td>cmd_fin_int</td><td>6</td><td>0x0</td><td>发送完成中断，硬件清零</td></tr><tr><td>SDIO_int</td><td>5</td><td>0x0</td><td>检测到 SDIO 中断，写 1 清零</td></tr><tr><td>prog_err_int</td><td>4</td><td>0x0</td><td>SD 卡编程错误中断，写 1 清零</td></tr><tr><td>cre_stu_int</td><td>3</td><td>0x0</td><td>数据发送后从设备返回 CRC 错误中断，写 1 清零</td></tr><tr><td>dat_crc_int</td><td>2</td><td>0x0</td><td>数据接收 CRC 错误中断，写 1 清零</td></tr><tr><td>dat_tout_int</td><td>1</td><td>0x0</td><td>数据超时中断，写 1 清零</td></tr><tr><td>dat_fin_int</td><td>0</td><td>0x0</td><td>数据完成中断，硬件清零</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_DAT</td><td>0x40</td><td>RO</td><td>SDIO 命令数据寄存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_DAT</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>sd_i_dat</td><td>31:0</td><td>0x0</td><td>SDIO 控制器发送或者接收的数据（用于 DMA 操作）</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>SDI_INT_EN</td><td>0x04</td><td>R/W</td><td>SDIO 中断寄使能存器</td><td>0x0</td></tr></table>

<table><tr><td>SDI_INT_EN</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>Reserved</td><td>31:10</td><td>0x0</td><td></td></tr><tr><td>R1b_fin_int_en</td><td>9</td><td>0x0</td><td>Busy 结束中断使能，为 1 时有效</td></tr><tr><td>rsp_crc_int_en</td><td>8</td><td>0x0</td><td>命令响应 CRC 错误中断使能，为 1 时有效</td></tr><tr><td>cmd_tout_int_en</td><td>7</td><td>0x0</td><td>命令超时中断使能，为 1 时有效</td></tr><tr><td>cmd_fin_int_en</td><td>6</td><td>0x0</td><td>命令发送完成中断使能，为 1 时有效</td></tr><tr><td>SDIO_int_en</td><td>5</td><td>0x0</td><td>SDIO 中断使能，为 1 时有效</td></tr><tr><td>prog_err_int_en</td><td>4</td><td>0x0</td><td>SD 卡编程错误中断使能，为 1 时有效</td></tr><tr><td>crc_sta_int_en</td><td>3</td><td>0x0</td><td>数据发送后从设备返回 CRC 错误中断使能，为 1 时有效</td></tr><tr><td>dat_cec_int_en</td><td>2</td><td>0x0</td><td>数据接收 CRC 错误中断使能，为 1 时有效</td></tr><tr><td>dat_tout_int_en</td><td>1</td><td>0x0</td><td>数据超时中断使能，为 1 时有效</td></tr><tr><td>dat_fin_int_en</td><td>0</td><td>0x0</td><td>数据完成中断使能，为 1 时有效</td></tr></table>

# 24.5 软件编程指南

# 24.5.1 SD Memory 卡软件编程说明

SD Memory 卡要想正常工作，必须要先初始化。初始化的过程需要发送不同的命令序列来配置从设备。初始化的流程示意图如下：

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/0425cbc0f5454b97cd650e928b02b857cd7ab39e2c00980c6724c749f6655b56.jpg)  
图24-3 SD Memory 卡初始化流程示意图

初始化完成之后就可以正常工作了。

配置寄存器的流程如下：

1. 配置sdi_con，使能输出时钟

2. 配置sdi_pre，设置一个分频系数，如果时序不满足，可以设置输出反向时钟来调整时序。

3. 配置sdi_int_en，使能命令、数据完成及其他中断。

4. 按照上图初始化流程初始化控制器

发送命令的配置寄存器过程如下：

根据发送的命令，配置cmd_arg寄存器

配置cmd_con寄存器，发送命令

读sdi_int_msk寄存器，检查是否传输完成，是否有错误

如果需要，读sdi_rsp寄存器

初始化的流程如下：

$\mathrm{CMD0}\longrightarrow \mathrm{CMD8}\longrightarrow \mathrm{ACMD41}($  即  $\mathrm{CMD55}\longrightarrow \mathrm{CMD41})\longrightarrow \mathrm{CMD2}\longrightarrow \mathrm{CMD3}$

$\rightarrow \mathrm{CMD7}\rightarrow \mathrm{ACMD6}$  （用于配置是否用4bit数据线传输）

5. 进行数据操作之前需要配置Bsize寄存器，Dtimer寄存器

6. 数据的操作必须要配置DMA，配置dat_con寄存器并配置DMA（注：读操作时先配置DMA，写操作时先配置dat_con）

7. 读sdi_int_msk寄存器，检测是否传输完成，是否有错误。

8. 没有错误则完成一次数据传输，不需要软件发送停止命令

# 24.5.2SDIO卡软件编程说明

SDIO卡的初始化流程和SDmemory卡不同，其初始化流程如下：

1. 配置sdi_con，使能输出时钟。2.配置sdi_pre，设置一个分频系数，如果时序不满足，可以设置输出反向3.时钟来调整时序。4.配置sdi_int_en，使能命令、数据完成及其他中断。

5. 初始化流程如下：

发送命令的配置寄存器过程如下：

根据发送的命令，配置cmd_arg寄存器配置cmd_con寄存器，发送命令读sdi_int_msk寄存器，检查是否传输完成，是否有错误如果需要，读sdi_rsp寄存器

初始化的流程如下（对CCCR的操作）：

6. CMD52（复位）CMD5（等待上电完成）CMD3(获取RCA）CMD7（选择相应RCA的卡）CMD52（配置是否用4bit数据线传输）CMD52（配置读写数据的块大小）CMD52（打开IO中断使能）进行数据操作之前需要配置Bsize寄存器，Ditimer寄存器

7. 数据的操作必须要配置DMA，配置dat_con寄存器并配置DMA（注：读操作时先配置DMA，写操作时先配置dat_con）

8. 发送读写数据的命令时，如果需要硬件自动发送停止命令，需要配置auto_stop和sdio_en。读写数据时需要先读写支持的Function，配置相应的FBR的指针寄存器，再发送多块读写（CMD53）或者单块读写（CMD52）命令进行读写。

9. 读sdi_int_msk寄存器，检测是否传输完成，是否有错误。

10. 没有错误则完成一次数据传输，不需要软件发送停止命令

11. 如果检测到IO中断，控制器会置起响应的中断，但是不会停止当前的操作。

12. 对于读等待，控制器会在合适的时机将DAT2拉低，通知SDIO卡停止发送数据。所以如果当前正在传输数据过程中，控制器可能会在当前一块数据传输结束后，发出读等待信号，这时才不会接收下一块数据。

13. 对于挂起和恢复操作。有可能出现挂起嵌套情况，比如说操作1被操作2中断而挂起，然后操作2被操作3中断而挂起。挂起时中断的现场需要软件保存（如当前的读写标志位，当前传输的块数，地址等），进行入栈操作。恢复时需要软件再按相应的顺序出栈。

# 24.6支持SDIO型号

本节列出经过验证的可支持的SDIO卡型号，其它类型的SDIO卡未经验证，不保证与本控制器兼容。

Mem卡：Kingston SD- CO2GSDC/2GBIO卡（wifi）：maxwellsd8686

# 25 I2S 控制器

# 25.1概述

龙芯2K1000中I2S控制器，数据宽度是32位，支持DMA传输，支持多家公司的codec芯片。I2S控制器仅支持主模式，由I2S产生位时钟信号和左右声道选择时钟信号。I2S的功能特性包括：

1. 支持8、16、20、24、32位的音频数据采样位宽。2. 支持8、16、20、24、32位的左右声道处理字宽。3. 包含两个缓存FIFO，FIFO的缓存容量为8bytes。4. I2S的中断处理模式可配，在I2S的发送和接收中断功能都使能后，当两个通道的缓存fifo为满仍要写以及为空仍要读时，则向CPU发出中断信号。5. I2S可以为codec芯片提供系统时钟，时钟频率可配。

# 25.2访问地址及引脚复用

I2S控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[27:16]</td><td>BAR_BASE</td><td>Device 2 的基地址寄存器值</td></tr><tr><td>[15:12]</td><td>0xD</td><td>固定为 D</td></tr><tr><td>[11:08]</td><td>0</td><td>保留</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于I2S模块，使用时要注意将对应的引脚设置为相应的功能。与I2S相关的引脚设置寄存器为5.1节中的i2s_sel。

# 25.3接口协议

25.3接口协议I2S发送器和接收器的操作时序如图25- 1所示。发送器和接收器的传输操作时序要求都是在信道选择信号（WS）改变之后的第二个位时钟开始传输下一帧数据，数据先传MSB位，再传输LSB位。发送器和接收器能处理的字位宽可以不一致，若发送器所需发送的数据的位宽比系统所支持的数据位宽要短，则LSB补零发送，反之，则忽略部分LSB数据；同理，对接受器而言，当接收到的数据的位宽比自己能处理的位宽小时，则LSB补零接收，反之，则忽略接收部分LSB数据。所以，对收发的数据而言，MSB是具有固定的，而LSB则要取决于要收发的数据的字长已经系统所配置的字长位宽。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/282fcad5cb9cf43f08594de1def7e30bfbb41e2ef06feebbd04caf8412eac78d.jpg)  
图25-1 I2S传输协议

# 25.4专用寄存器

I2S包含5个寄存器，定义如下表所示

表25-1寄存器定义  

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td><td>复位值</td></tr><tr><td>IISVersion</td><td>0xd000</td><td>R/W</td><td>I2S标识寄存器</td><td>32&#x27;h0</td></tr><tr><td>IISConfig</td><td>0xd004</td><td>R/W</td><td>I2S配置寄存器</td><td>32&#x27;h0</td></tr><tr><td>IISControl</td><td>0xd008</td><td>R/W</td><td>I2S控制寄存器</td><td>32&#x27;h0</td></tr><tr><td>IISRxData</td><td>0xd00c</td><td>R/W</td><td>I2S接收数据寄存器（用于DMA接收数据）</td><td>32&#x27;h0</td></tr><tr><td>IISTxData</td><td>0xd010</td><td>R/W</td><td>I2S发送数据寄存器（用于DMA发送数据）</td><td>32&#x27;h0</td></tr></table>

接收标识寄存器允许主控机读取接收器的相关工作信息。它标识了IIS的地址位宽，数据位宽以及版本号等信息。

表25-2标识寄存器  

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td></tr><tr><td>IISVersion</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>ADRW</td><td>9:8</td><td>2&#x27;h0</td><td>地址总线宽度：
00：地址宽度8位
01：地址宽度16位
10：地址宽度32位
11：地址宽度64位</td></tr><tr><td>DATW</td><td>5:4</td><td>2&#x27;h0</td><td>数据宽度：
00：数据宽度8位
01：数据宽度16位
10：数据宽度32位
11：数据宽度64位</td></tr><tr><td>VER</td><td>3:0</td><td>4&#x27;h0</td><td>I2S版本号</td></tr></table>

接收配置寄存器是配置I2S的声道字长，音频数据的采样深度以及各个时钟的分频系数的。

表25-3配置寄存器  

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td></tr><tr><td>IISConfig</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>LR_LEN</td><td>31:24</td><td>&#x27;h0</td><td>左右声道处理的字长。</td></tr></table>

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td></tr><tr><td>RES_DEPTH</td><td>23:16</td><td>&#x27;h0</td><td>采样深度设置：
IIS 采样数据长度，有效范围为 8-32，如果发送或者接收到的数据宽度小于采样数据长度，则低位补 0；如果发送或者接收到的数据宽度大于采样数据长度，则低位忽略。</td></tr><tr><td>BCLK_RATIO</td><td>15:8</td><td>&#x27;h0</td><td>位时钟(BCLK)分频系数：
位时钟分频系数，分频数为总线时钟频率除以 2x(RATIO+1)</td></tr><tr><td>MCLK_RATIO</td><td>7:0</td><td>&#x27;h0</td><td>系统时钟(MCLK)分频系数，
系统时钟分频系数，分频数为总线时钟频率除以 2x(RATIO+1)，系统时钟作为 Codec 的sysclk</td></tr></table>

控制寄存器用于配置IIS的工作使能信号，缓存FIFO的存储状态以及中断相关信息状态。

表25-4控制寄存器  

<table><tr><td>寄存器名称</td><td>偏移地址</td><td>读/写(R/W)</td><td>功能描述</td></tr><tr><td>IISControl</td><td>位</td><td>缺省值</td><td>描述</td></tr><tr><td>MASTER</td><td>15</td><td>&#x27;h0</td><td>1：IIS 工作于主模式</td></tr><tr><td>MSB/LSB</td><td>14</td><td>&#x27;h0</td><td>1：高位在左端 0：高位在右端</td></tr><tr><td>RX_EN</td><td>13</td><td>&#x27;h0</td><td>控制器接收使能，为 1 时有效，开始接收数据</td></tr><tr><td>TX_EN</td><td>12</td><td>&#x27;h0</td><td>控制器发送使能，为 1 时有效，开始发送数据</td></tr><tr><td>RX_DMA_EN</td><td>11</td><td>&#x27;h0</td><td>DMA 接收使能，为 1 时有效</td></tr><tr><td>Reserved</td><td>10:8</td><td>&#x27;h0</td><td></td></tr><tr><td>TX_DMA_EN</td><td>7</td><td>&#x27;h0</td><td>DMA 发送使能，为 1 时有效</td></tr><tr><td>Reserved</td><td>6:2</td><td>&#x27;h0</td><td></td></tr><tr><td>RX_INT_EN</td><td>1</td><td>&#x27;h0</td><td>RX 中断使能，为 1 时使能中断，为 0 时禁止</td></tr><tr><td>TX_INT_EN</td><td>0</td><td>&#x27;h0</td><td>TX 中断使能，为 1 时使能中断，为 0 时禁止</td></tr></table>

# 25.5配置操作

I2S正常工作，需要先配置好CODEC芯片，然后配置I2S控制器的的配置寄存器和控制寄存器。

2K1000芯片通过I2S接口和CODEC芯片通信，CODEC芯片作为I2S总线上的从设备，详细地址、寄存器和配置方法请参考具体CODEC芯片的数据手册。配置完CODEC之后，需要对I2S控制器进行配置。可以将一些配置信息写入标志寄存器（I2SVersion），以供查询。先配置好I2SConfig寄存器，再配置I2SControl寄存器。

I2SConfig寄存器建议LR_LEN和RES_DEPTH配成一样，不至于在传输过程中丢数据或者空数据。BCK时钟根据配置CODEC的采样频率、采样深度和倍频系数来计算，计算公式如下：

$\mathrm{BCK} = 256\mathrm{fs}$  (或者512xfs)或者(768xfs)；(详细见CODEC手册推荐配置)

BCK_RATIO  $\equiv$  Freq_APB/(256xfs)/2- 1;

双通道的话，计算公式下：

BCK = RES_DEPTH H x 2 xfs;

BCK_RATIO = Freq_APB / (RES_DEPTH H x 2 x fs) / 2 - 1;

其中 fs 为配置的采样频率（即音频文件的码率）。发送和读取数据时，需要先配置好 DMA，再配置控制器，以免发生数据丢失。DMA 复用的配置方法见第 5.4 节。

MCK 时钟根据配置 CODEC 的采样频率和倍频系数来计算，计算公式如下：

$\mathrm{MCK} = 256\mathrm{xf}$  (或者512xfs）或者(768xfs)；(详细见CODEC手册推荐配置)

MCK_RATIO = Freq_APB / (256 x fs) / 2 - 1。

# 26 GMAC控制器 (Dev3)

26 GMAC控制器（Dev3）龙芯2K1000集成了两个GMAC控制器，即GMAC0和GMAC1，二者在逻辑结构上完全相同，分别为Device3的不同功能（Function）。其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>GMAC0</td><td>0x0</td><td>0x3</td><td>0x0</td><td>0x7FFF</td><td>0xFE_0000_1800</td><td></td></tr><tr><td>GMAC1</td><td>0x0</td><td>0x3</td><td>0x1</td><td>0x7FFF</td><td>0xFE_0000_1900</td><td></td></tr></table>

# 26.1访问地址及引脚复用

各个GMAC控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:15]</td><td>BAR_BASE</td><td>GMAC0为Device 3、FUNC 0的基地址寄存器值
GMAC1为Device 3、FUNC 1的基地址寄存器值</td></tr><tr><td>[14:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于GMAC1，使用时要注意将对应的引脚设置为相应的功能。

与GMAC1相关的引脚设置寄存器为5.1节中的gmac1_sel。

# 27 OTG 控制器（Dev 4, Fun 0）

# 27.1概述

2K1000的OTG支持特性如下：

27.1 概述2K1000 的 OTG 支持特性如下:- 支持 HNP 与 SRP 协议;- 内嵌 DMA，无需占用处理器带宽即可在 OTG 与外部存储之间移动数据;- 在 device 模式下，为高速设备（480Mbps）；- 在 host 模式下，仅能支持高速设备（480Mbps）；- 在 device 模式下，支持 6 个双向的 endpoint，其中仅有默认的 endpoint0 支持控制传输；- 在 device 模式下，最多同时支持 4 个 IN 方向的传输；- 在 host 模式下，支持 12 个 channel，且软件可配置每个 channel 的方向；- 在 host 模式下，支持 periodic OUT 传输；

# 27.2访问地址

27.2 访问地址2K1000 的 Device 4 包含 3 个功能（Function），分别为 OTG 控制器（Function0）、EHCI 控制器（Function1）和 OHCI 控制器（Function2），其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>OTG</td><td>0x0</td><td>0x4</td><td>0x0</td><td>0x3_FFFF</td><td>0xFE_0000_2000</td><td></td></tr><tr><td>EHCI</td><td>0x0</td><td>0x4</td><td>0x1</td><td>0x7FFF</td><td>0xFE_0000_2100</td><td></td></tr><tr><td>OHCI</td><td>0x0</td><td>0x4</td><td>0x2</td><td>0x7FFF</td><td>0xFE_0000_2200</td><td></td></tr></table>

OTG控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:18]</td><td>BAR_BASE</td><td>Device 4、FUNC 0 的基地址寄存器值</td></tr><tr><td>[17:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 28 USB 控制器（Dev 4, Fun 1/2）

# 28.1 总体概述

2K1000的USB主机端口特性如下：

兼容USBRev1.1、USBRev2.0协议兼容OHCIRev1.0、EHCIRev1.0协议支持LS（LowSpeed）、FS（FullSpeed）和HS（HighSpeed）的USB设备 $\bullet$  支持四个端口，每个端口都可挂LS、FS或HS设备

USB主机控制器模块包括一个支持高速设备的EHCI控制器，一个支持全速与低速设备的OHCI控制器。其中EHCI控制器处于主控地位，只有当挂上的设备是全速或低速设备时，才将控制权转交给OHCI控制器；当全速或低速设备拔掉时，控制权返回EHCI控制器。

同时USB控制器内部集成了AHB总线接口（与AMBA Specification Revision 2.0兼容），用来和内存/应用程序之间通信。USB控制器与外部的互联结构图如下图所示：

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/c8f4f8bd387ff74808639676a7ae0f14ce395931ac67a217423f58652700ecbe.jpg)  
图28-1 USB主机控制器模块图

# 28.2 访问地址

2K1000的Device4包含3个功能（Function），分别为OTG控制器(Function0)、EHCI控制器（Function1）和OHCI控制器（Function2），其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>OTG</td><td>0x0</td><td>0x4</td><td>0x0</td><td>0x3_FFFF</td><td>0xFE_0000_2000</td><td></td></tr></table>

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>EHCI</td><td>0x0</td><td>0x4</td><td>0x1</td><td>0x7FFF</td><td>0xFE_0000_2100</td><td></td></tr><tr><td>OHCI</td><td>0x0</td><td>0x4</td><td>0x2</td><td>0x7FFF</td><td>0xFE_0000_2200</td><td></td></tr></table>

EHCI控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:15]</td><td>BAR_BASE</td><td>Device 4、FUNC 1 的基地址寄存器值</td></tr><tr><td>[14:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

OHCI控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:15]</td><td>BAR_BASE</td><td>Device 4、FUNC 2 的基地址寄存器值</td></tr><tr><td>[14:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 29 图形处理器（Dev5）

# 29.1访问地址

2K1000的Device5为图形处理器GPU，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>GPU</td><td>0x0</td><td>0x5</td><td>0x0</td><td>0x3_FFFF</td><td>0xFE_0000_2800</td><td></td></tr></table>

GPU内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:18]</td><td>BAR_BASE</td><td>Device 5、FUNC 0 的基地址寄存器值</td></tr><tr><td>[17:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 30 显示控制器 (Dev 6)

# 30.1概述

显示控制器从内存中取帧缓冲和光标信息输出到外部显示接口上。龙芯2K1000的显示控制器支持的特性包括：

1) 双路DVO接口显示2) 每路显示最大支持至  $1920\mathrm{x}1080\@ 60\mathrm{Hz}$ 3) Monochrome、ARGB8888两种模式硬件光标4) RGB444、RGB555、RGB565、RGB888色深支持5) 输出抖动和伽马校正6) 可切换的双路线性帧缓冲7) 中断和软复位

# 30.2访问地址及引脚复用

2K1000的Device6为显示控制器(DC),其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>DC</td><td>0x0</td><td>0x6</td><td>0x0</td><td>0xFFFF</td><td>0xFE_0000_3000</td><td></td></tr></table>

DC控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:16]</td><td>BAR_BASE</td><td>Device 6、FUNC 0 的基地址寄存器值</td></tr><tr><td>[15:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于DC控制器，使用时要注意将对应的引脚设置为相应的功能。与DC相关的引脚设置寄存器为5.3节中的dvo0_sel、dvo1_sel。

# 31 HDA 控制器 (Dev 7)

# 31.1功能概述

31.1 功能概述HDA 控制器兼容 High Definition Audio Specification Revision 1.0x，主要的功能包括各种输入、输出流组合，对 48KHZ，和 44.1KHZ 的采样频率的支持，初始化序列，命令控制通道等。

# 31.2访问地址

2K1000的Device7为HDA，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>HDA</td><td>0x0</td><td>0x7</td><td>0x0</td><td>0xFFFF</td><td>0xFE_0000_3800</td><td></td></tr></table>

HDA控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:16]</td><td>BAR_BASE</td><td>Device 7、FUNC 0 的基地址寄存器值</td></tr><tr><td>[15:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

对于HDA控制器，使用时要注意将对应的引脚设置为相应的功能。

与HDA相关的引脚设置寄存器为5.1节中的hda_sel。

# 32 SATA 控制器 (Dev 8)

# 32.1 SATA 总体描述

SATA的特性包括：

支持SATA1代1.5Gbps和SATA2代3Gbps的传输 $\bullet$  兼容串行ATA2.6规范和AHCI1.1规范

# 32.2访问地址

2K1000的Device8为SATA控制器，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>SATA</td><td>0x0</td><td>0x8</td><td>0x0</td><td>0xFFFF</td><td>0xFE_0000_4000</td><td></td></tr></table>

SATA控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:16]</td><td>BAR_BASE</td><td>Device 8、FUNC 0 的基地址寄存器值</td></tr><tr><td>[15:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 32.3 SATA控制器内部寄存器描述

SATA的基地址是由SATA的BAR0给定，寄存器的定义和协议标准定义完全一致。

<table><tr><td>偏移地址</td><td>位宽</td><td>名称</td><td>描述</td></tr><tr><td>0x000</td><td>32</td><td>CAP</td><td>HBA 特性寄存器</td></tr><tr><td>0x004</td><td>32</td><td>GHC</td><td>全局 HBA 控制寄存器</td></tr><tr><td>0x008</td><td>32</td><td>IS</td><td>中断状态寄存器</td></tr><tr><td>0x00c</td><td>32</td><td>PI</td><td>端口寄存器</td></tr><tr><td>0x010</td><td>32</td><td>VS</td><td>AHCI 版本寄存器</td></tr><tr><td>0x014</td><td>32</td><td>CCC_CTL</td><td>命令完成合并控制寄存器</td></tr><tr><td>0x018</td><td>32</td><td>CCC_PORTS</td><td>命令完成合并端口寄存器</td></tr><tr><td>0x024</td><td>32</td><td>CAP2</td><td>HBA 特性扩展寄存器</td></tr><tr><td>0x0A0</td><td>32</td><td>BISTAFR</td><td>BIST 激活 FIS</td></tr><tr><td>0x0A4</td><td>32</td><td>BISTCR</td><td>BIST 控制寄存器</td></tr><tr><td>0x0A8</td><td>32</td><td>BISTCTR</td><td>BIST FIS 计数寄存器</td></tr><tr><td>0x0AC</td><td>32</td><td>BISTSR</td><td>BIST 状态寄存器</td></tr><tr><td>0x0B0</td><td>32</td><td>BISTDECR</td><td>BIST 双字错计数寄存器</td></tr><tr><td>0x0BC</td><td>32</td><td>OOBR</td><td>OOB 寄存器</td></tr><tr><td>0x0E0</td><td>32</td><td>TIMER1MS</td><td>1ms 计数寄存器</td></tr><tr><td>0x0E8</td><td>32</td><td>GPARAM1R</td><td>全局参数寄存器</td></tr></table>

<table><tr><td>0x0EC</td><td>32</td><td>GPARAM2R</td><td>全局参数寄存器 2</td></tr><tr><td>0x0F0</td><td>32</td><td>PPARAMR</td><td>端口参数寄存器</td></tr><tr><td>0x0F4</td><td>32</td><td>TESTR</td><td>测试寄存器</td></tr><tr><td>0x0F8</td><td>32</td><td>VERIONR</td><td>版本寄存器</td></tr><tr><td>0x0FC</td><td>32</td><td>IDR</td><td>ID 寄存器</td></tr><tr><td>0x100</td><td>32</td><td>P0_CLB</td><td>命令列表基地址低 32 位</td></tr><tr><td>0x104</td><td>32</td><td>P0_CLBU</td><td>命令列表基地址高 32 位</td></tr><tr><td>0x108</td><td>32</td><td>P0_FB</td><td>FIS 基地址低 32 位</td></tr><tr><td>0x10c</td><td>32</td><td>P0_FBU</td><td>FIS 基地址高 32 位</td></tr><tr><td>0x110</td><td>32</td><td>P0_IS</td><td>中断状态寄存器</td></tr><tr><td>0x114</td><td>32</td><td>P0_IE</td><td>中断使能寄存器</td></tr><tr><td>0x118</td><td>32</td><td>P0_CMD</td><td>命令寄存器</td></tr><tr><td>0x120</td><td>32</td><td>P0_TFD</td><td>任务文件数据寄存器</td></tr><tr><td>0x124</td><td>32</td><td>P0_SIG</td><td>签名寄存器</td></tr><tr><td>0x128</td><td>32</td><td>P0_SSTS</td><td>SATA 状态寄存器</td></tr><tr><td>0x12C</td><td>32</td><td>P0_SCTL</td><td>SATA 控制寄存器</td></tr><tr><td>0x130</td><td>32</td><td>P0_ERR</td><td>SATA 错误寄存器</td></tr><tr><td>0x134</td><td>32</td><td>P0_SACT</td><td>SATA 激活寄存器</td></tr><tr><td>0x138</td><td>32</td><td>P0_CI</td><td>命令发送寄存器</td></tr><tr><td>0x13C</td><td>32</td><td>P0_SNTF</td><td>SATA 命令通知寄存器</td></tr><tr><td>0x170</td><td>32</td><td>P0_DMACR</td><td>DMA 控制寄存器</td></tr><tr><td>0x178</td><td>32</td><td>P0_PHYCR</td><td>PHY 控制寄存器</td></tr><tr><td>0x17C</td><td>32</td><td>P0_PHYSR</td><td>PHY 状态寄存器</td></tr><tr><td>0x180</td><td>32</td><td>P1_CLB</td><td>命令列表基地址低 32 位</td></tr><tr><td>0x184</td><td>32</td><td>P1_CLBU</td><td>命令列表基地址高 32 位</td></tr><tr><td>0x188</td><td>32</td><td>P1_FB</td><td>FIS 基地址低 32 位</td></tr><tr><td>0x18c</td><td>32</td><td>P1_FBU</td><td>FIS 基地址高 32 位</td></tr><tr><td>0x190</td><td>32</td><td>P1_IS</td><td>中断状态寄存器</td></tr><tr><td>0x194</td><td>32</td><td>P1_IE</td><td>中断使能寄存器</td></tr><tr><td>0x108</td><td>32</td><td>P1_CMD</td><td>命令寄存器</td></tr><tr><td>0x1a0</td><td>32</td><td>P1_TFD</td><td>任务文件数据寄存器</td></tr><tr><td>0x1a4</td><td>32</td><td>P1_SIG</td><td>签名寄存器</td></tr><tr><td>0x1a8</td><td>32</td><td>P1_SSTS</td><td>SATA 状态寄存器</td></tr><tr><td>0x1aC</td><td>32</td><td>P1_SCTL</td><td>SATA 控制寄存器</td></tr><tr><td>0x1b0</td><td>32</td><td>P1_ERR</td><td>SATA 错误寄存器</td></tr><tr><td>0x1b4</td><td>32</td><td>P1_SACT</td><td>SATA 激活寄存器</td></tr><tr><td>0x1b8</td><td>32</td><td>P1_CI</td><td>命令发送寄存器</td></tr><tr><td>0x1bC</td><td>32</td><td>P1_SNTF</td><td>SATA 命令通知寄存器</td></tr><tr><td>0x1f0</td><td>32</td><td>P1_DMACR</td><td>DMA 控制寄存器</td></tr><tr><td>0x1f8</td><td>32</td><td>P1_PHYCR</td><td>PHY 控制寄存器</td></tr><tr><td>0x1fC</td><td>32</td><td>P1s_PHYSR</td><td>PHY 状态寄存器</td></tr></table>

# 33 PCIE 控制器（Dev 9/A/B/C/D/E）

# 33.1 总体结构

龙芯2K1000有两个PCIE控制器，其中一个PCIE控制器既可以作为一个X4的PCIE端口也可以作为4个独立的X1PCIE端口；另一个PCIE控制器既可以作为一个X4的PCIE端口也可以作为2个独立的X1PCIE端口，作为X1端口时，仅LANE0和LANE1可用，LANE2和LANE3不可用。

龙芯2K1000的PCIE控制器仅可以作为RC使用，不能作为EP。

龙芯2K1000的PCIE控制器结构如图33- 1所示。其中一个PCIE控制器包含  $0\sim 3$  号，共4个PCIE端口。0号端口可以以X4/X1的方式工作，  $1\sim 3$  号端口仅能以X1的方式工作。另一个PCIE控制器仅包含0和1号PCIE端口。每个PCIE端口均有自己独立的PCI地址空间。

![](https://cdn-mineru.openxlab.org.cn/result/2025-08-12/08212664-4bd6-438e-aa1a-6e76ae569ec1/af096a7d87fa03d4b84e6a43ba5eb440c22c5f8bda38fd3a984b82d9b3dc25c8.jpg)  
图33-1PCIE控制器结构

# 33.2 访问地址

PCIE控制器中每个端口有独立的配置头，其访问地址分别为：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>PCIE0 Port0</td><td>0x0</td><td>0x9</td><td>0x0/1</td><td>0xFFFF</td><td>0xFE_0000_4800</td><td rowspan="4">TYPE1类型配置头，仅有64位BAR0，用于访问内部寄存器。对功能1的访</td></tr><tr><td>PCIE0 Port1</td><td>0x0</td><td>0xA</td><td>0x0/1</td><td>0xFFFF</td><td>0xFE_0000_5000</td></tr><tr><td>PCIE0 Port2</td><td>0x0</td><td>0xB</td><td>0x0/1</td><td>0xFFFF</td><td>0xFE_0000_5800</td></tr><tr><td>PCIE0 Port3</td><td>0x0</td><td>0xC</td><td>0x0/1</td><td>0xFFFF</td><td>0xFE_0000_6000</td></tr></table>

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>PCIE1 Port0</td><td>0x0</td><td>0xD</td><td>0x0/1</td><td>0xFFF</td><td>0xFE_0000_6800</td><td rowspan="2">问用于修改一些只读寄存器。</td></tr><tr><td>PCIE1 Port1</td><td>0x0</td><td>0xE</td><td>0x0/1</td><td>0xFFF</td><td>0xFE_0000_7000</td></tr></table>

PCIE控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:12]</td><td>BAR_BASE</td><td>PCIE控制器内BAR0配置</td></tr><tr><td>[11:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 33.3地址空间划分

龙芯2K1000中的PCIE控制器内有标准的PCIE配置头，因此PCIE控制器的内部寄存器以及其下游设备的地址空间都通过其配置头的信息来管理。配置头中地址相关的寄存器在PCI设备扫描时确定。

因为2K1000的PCIE控制器仅可以工作在RC模式下，所以其配置头为TYPE1类型。

每个PCIE端口作为2K1000中的独立设备，每个端口都包含一个PCIE配置头。当PCIE工作在X4模式时，其他X1的端口软件不可见，只有当PCIE工作在X1模式时才可以访问其他X1端口。

对于每一个PCIE端口，其地址空间可以分为以下几部分：

配置头地址空间：该部分空间对应PCIE的配置头，通过配置请求来访问，最大8KB。其中低4KB通过将配置请求的func设为0来访问，用于访问标准配置头；高4KB通过将配置请求的func设为1来访问，用于改写标准配置头中的一些只读寄存器。

配置访问地址空间：该部分地址空间用于通过配置请求访问PCIE控制器的下游设备配置头信息。根据下游设备的Bus号，由PCIE控制器决定发送TYPE0类型还是TYPE1类型的配置访问。

以上两个地址空间的地址由配置地址空间基地址、BUS号、设备号、功能号以及寄存器偏移地址计算得出，访问以字为单位。

PCIE控制器内部寄存器空间：该部分地址空间用于访问PCIE控制器的内部寄存器。这些寄存器用于控制PCIE控制器的行为和特性，与PCIE配置头空间属于两个地址空间。该地址空间为MEM类型，64位地址空间，大小为4KB，基地址等于64位BAR0的值，该值在初始化时由PCI扫描软件分配。

MEM地址空间：该部分地址空间包含了PCIE控制器下游设备的所有MEM地址空间。对于32位地址空间，由PCIE配置头的memorybase和memorylimit决定；对于64位地址空间，由PCIE配置头的prefetchablememorybase（组合upper32bits）和prefetchablememorylimit（组合upper32bits）决定。该段地址空间由PCIE配置头的command寄存器bit1位来

使能控制。

IO地址空间：该部分地址空间包含了PCIE控制器下游设备的所有IO地址空间。由PCIE配置头的io base（组合upper16bits）和io limit（组合upper16bits）决定。该段地址空间由PCIE配置头的command寄存器bit0位来使能控制。

对于MEM地址空间和IO地址空间来说，如果在X1工作模式下，某个X1端口下游没有连接设备，通过设置command寄存器的bit0和bit1为0即可禁用其MEM和IO地址空间。

# 33.4软件编程指南

# 33.4.1 PCIE控制器使能

芯片配置寄存器中通用配置寄存器2（表5- 4）bit16和bit17控制两个PCIE控制器的使能。在使用对应PCIE控制器时需要先将其使能才能访问该控制器以及下游设备的所有地址空间，包括对控制器的配置访问。

# 33.4.2 PCIE配置头访问

基于前文对配置请求的介绍，对PCIE配置头的访问为Type0的访问，其格式为：

<table><tr><td rowspan="2">Type 0</td><td>39</td><td>32 31</td><td>28 27</td><td>24 23</td><td>16 15</td><td>11 10</td><td>8 7</td><td>0</td></tr><tr><td>FE0h</td><td>Offset[11:8]</td><td>Reserved</td><td>Device Number</td><td>Function Number</td><td>Offset[7:0]</td><td></td><td></td></tr></table>

PCIE各个Port的设备号（devicenumber）分别为0x9，0xa，0xb，0xc，0xd，0xe。根据设备号及所要访问的寄存器偏移地址(offset)即可得到对应寄存器的物理地址。功能号FunctionNumber为0时访问配置头内寄存器，功能号FunctionNumber为1时可以访问配置头空间内影子寄存器，用于改写配置头内的只读寄存器。

# 33.4.3 PCIE链路建立(Linkup)

链路建立流程如下：

(1）通过配置访问设置Gen2 Control Register寄存器中（0x80c）Directed Speed Change为1，PHY Tx Swing为0。注意配置地址格式，因为offset大于8位地址，需将高4位填到bit24- bit27；(2）通过配置访问配置好PCIE的BAR0寄存器；(3）根据BAR0中配的地址通过MEM访问设置PCIE控制器内部寄存器app_ltssm_enable(0x0)为1，开始link training过程；(4）等待内部寄存器Xmlh_ltssm_state(0xc)为0x11；(5）Linkup成功。

# 33.4.4 TYPE1类型配置访问

TYPE1类型配置请求地址格式如下：

<table><tr><td rowspan="2">Type 1</td><td>39</td><td>32 31</td><td>28 27</td><td>24 23</td><td>16 15</td><td>11 10</td><td>8 7</td><td>0</td></tr><tr><td>FE1h</td><td>Offset[11:8]</td><td>Bus Number</td><td>Device Number</td><td>Function Number</td><td>Offset[7:0]</td><td></td><td></td></tr></table>

发送TYPE1类型配置访问之前需要设置配置头的PrimaryBusNumber、SecondaryBusNumer和SubordinateBusNumber。然后直接按照TYPE1地址格式发送读写请求即可，PCIE控制器根据地址中的BusNumber与所配置的SecondaryBusNumer和SubordinateBusNumber决定发出TYPE0类型还是TYPE1类型的配置请求。

如果BusNumber  $\vDash$  SecondaryBusNumer，则发出TYPE0类型的配置请求。

如果BusNumber>SecondaryBusNuner并且BusNumber  $\epsilon =$  SubordinateBusNumber，则发出TYPE1类型的配置请求。

# 33.4.5 PCIEPHY配置方法

PCIEPHY内部有一些可配置的寄存器，对这些寄存器的访问通过读写芯片配置寄存器中PCIEPHY配置寄存器来实现，具体步骤如下：

对于写请求

1. 设置phy_cfg_disable为  $0x0$  
2. 设置所要访问的寄存器地址phy_cfg_addr3.将要写的数据写入phy_cfg_data4.设置寄存器phy_cfg_rw为1，开始写内部寄存器5.等待phy_cfg_done为16.写数据完成。对于读请求7.设置phy_cfg_disable为  $0x0$  8.设置所要访问的寄存器地址phy_cfg_addr9.设置寄存器phy_cfg_rw为0，开始从内部寄存器读数10.等待phy_cfg_done为111.从phy_cfg_data中读出数据。

# 33.5常用例程

33.5常用例程本节给出龙芯2K1000的PCIE控制器的常用例程。龙芯2K1000需要先执行下面示例中的pcie_link_init，再执行下面示例中的pcie_header_init。随后，龙芯2K1000可以通过cfg_device_read和cfg_device_write这两个函数对下游设备的PCIHeader进行初始化。

unsigned int tmp_var; unsigned int\* cfg0_base  $=$  0xfe00000000; unsigned int\* cfg1_base  $=$  0xfe10000000; unsigned int\* mem_base  $=$  0x9000000000000000;

void pcie_link_init(unsigned long bar,unsigned int dev_num, unsigned int func_num) { unsigned longpcie_header_base  $=$  cfg0_base| (dev_num  $< < 11$  ) (func_num  $< < 8$  unsigned longpcie_reg_base  $=$  mem_base  $^+$  bar; // set port logic register of port 0 // initiate speed change to PCIE Gen2 and set Tx to Low Swing tmp_var  $=$  \*(volatile unsigned int \*)(pcie_header_base  $+0\mathrm{x}80\mathrm{c}$  ); \*(volatile unsigned int \*)(pcie_header_base  $+0\mathrm{x}80\mathrm{c}) =$  (tmp_var | 0x20000)&0xffffbffff;

//start link training \*(volatile unsigned int \*)(pcie_reg_base)  $=$  0xff200c;

//wait link train end tmp_var  $=$  \*(volatile unsigned int \*)(pcie_reg_base  $+0\mathrm{xc}$  ); while((tmp_var&0x1f)  $! = 0\mathrm{x}11$  { tmp_var  $=$  \*(volatile unsigned int \*)(pcie_reg_base  $+0\mathrm{xc}$  ); } printf("now PCIE port O link is start up\n");

1

# void pcie_hot_reset(unsigned long bar)

1

unsigned longpcie_reg_base  $=$  mem_base  $^+$  bar; tmp_var  $=$  \*(volatile unsigned int \*)(pcie_reg_base); //enable soft reset \*(volatile unsigned int \*)(pcie_reg_base)  $=$  tmp_var |0x1000; //trigger hot reset \*(volatile unsigned int \*)(pcie_reg_base  $+0\mathrm{x}4) = 0\mathrm{x}4$  . 1

void pcie_header_init(unsigned long bar,unsigned int dev_num, unsigned int func_num, unsigned int io_base, unsigend int io_limit, unsigned int mem_base, unsigned int mem_limit, unsigned int pref_mem_base, unsigned int pref_mem_limit, unsigned pref_mem_base_upper32, unsigned int pref_mem_limit_upper32)

{//only used in RC mode: unsigned longpcie_header_base  $=$  cfg0_base| (dev_num  $< < 11$  ) (func_num  $< < 8$  unsigned longpcie_reg_base  $=$  mem_base  $^+$  bar; //set master enable, io enable, mem enable, perr enable, serr enable \*(volatile unsigned int \*)(pcie_header_base  $+0\mathrm{x}4) = 0\mathrm{x}147$  .

//clear master abort, serr and perr status //set IO space to be 16- bit address //set 64 KB IO space:  $0\mathrm{x}0000\sim 0\mathrm{x}$  ffff \*(volatile unsigned int \*)(pcie_header_base  $+0\mathrm{x}1\mathrm{c}) = 0\mathrm{x}1100\mathrm{f}0000$  .

//set IO limit and IO base up 16 bit address \*(volatile unsigned int \*)(pcie_header_base  $+0\mathrm{x}30) = 0\mathrm{x}0$  .

//set memory limit and memory base

*(volatile unsigned int*)(pcie_header_base + 0x20) = 0x17f00000;

//set prefetchable memory limit and base

*(volatile unsigned int*)(pcie_header_base + 0x24) = 0x7ff04000;

//set prefetchable base up 32 bit *(volatile unsigned int *(pcie_header_base + 0x28) = pref_mem_base, upper32; //set prefetchable limit up 32 bit *(volatile unsigned int*)(pcie_header_base + 0x2c) = pref_mem_limit_upper32;

//enable serf *(volatile unsigned int*)(pcie_header_base + 0x3c) = 0x20000;

//enable system error on correctalbe error, non- fatal error and fatal error //enable PME interrupt *(volatile unsigned int*)(pcie_header_base + 0x8c) = 0xf;

//enable correctalbe error, non- fatal error and fatal error *(volatile unsigned int*)(pcie_header_base + 0x12c) = 0x7;

//enable ASPM L0s and L1

*(volatile unsigned char*)(pcie_header_base + 0x80) = 0x3;

void cfg_device_read( unsigned int bus_num, unsigned int dev_num, unsigned int func_num, unsigned int reg_id, unsigned int * read_data

{ unsigned longpcie_header_base  $=$  cfg1_base| (bus_num  $< < 16$  )| (dev_num  $< < 11$  )| (func_num  $< < 8)$  \*(read_data)  $= \frac{3}{2}$  volatile unsigned int \*)(pcie_header_base  $^+$  (reg_id<<2)); }

void cfg_device_write( unsigned int bus_num, unsigned int dev_num, unsigned int func_num, unsigned int reg_id, unsigned int write_data

{ unsigned longpcie_header_base  $=$  cfg1_base| (bus_num  $< < 16$  )| (dev_num  $< < 11$  )| (func_num  $< < 8)$

*(volatile unsigned int*)(pcie_header_base + (reg_id<<2)) = write_data; }

# 34 DMA控制器 (Dev F)

# 34.1DMA控制器结构描述

龙芯2K1000中DMA用来实现内存与APB设备之间数据搬移，可以节省资源提高系统数据传输的效率。

DMA的传送数据的过程由三个阶段组成：

a）传送前的预处理：由CPU配置DMA描述符相关的寄存器。b）数据传送：在DMA控制器的控制下自动完成。c）传送结束处理：发送中断请求。

本DMA控制器是一个基于AXI总线的单通道、可配置的DMA控制器IP核，主要功能就是在芯片上集成了DMA功能，专门负责在内存与APB设备间搬运数据。本DMA控制器限定为以字（4Byte）为单位的数据搬运。

CPU通过一个通用寄存器（dma_order）向DMA发命令。DMA根据命令内容，从内存读取描述符启动DMA直接操作，或者将DMA状态写入内存，或者停止DMA。

dma_order寄存器为芯片配置寄存器，在5.25节有介绍，下表列出相同内容，方便查看。

表34-1DMAORDER寄存器  

<table><tr><td>位域</td><td>名称</td><td>访问</td><td>缺省值</td><td>描述</td></tr><tr><td>63:5</td><td>ask_addr</td><td>R/W</td><td>0</td><td>64位地址的高59位</td></tr><tr><td>5</td><td>-</td><td>-</td><td>0</td><td>保留</td></tr><tr><td>4</td><td>dma_stop</td><td>R/W</td><td>0</td><td>停止DMA操作。DMA控制器完成当前数据读写后停止。</td></tr><tr><td>3</td><td>dma_start</td><td>R/W</td><td>0</td><td>开始DMA操作。DMA控制器读取描述符地址(ask_addr)后将些位清零。</td></tr><tr><td>2</td><td>ask_valid</td><td>R/W</td><td>0</td><td>DMA工作寄存器写回到(ask_addr)所指向的内存，完成后清零。</td></tr><tr><td>1</td><td>axi_uncoherent</td><td>R/W</td><td>0</td><td>DMA访问地址非一致性使能，设置为1代表uncache访问，设置为0代表cache访问。</td></tr><tr><td>0</td><td>dma_64bit</td><td>R/W</td><td>0</td><td>DMA控制器64位地址支持</td></tr></table>

# 34.2访问地址

2K1000的DeviceF为DMA控制器，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>DMA</td><td>0x0</td><td>0xF</td><td>0x0</td><td>0xFF</td><td>0xFE_0000_7800</td><td></td></tr></table>

DMA控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:08]</td><td>BAR_BASE</td><td>Device F、FUNC 0 的基地址寄存器值</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 34.3 DMA控制器与APB设备的交互

2K1000中包含5个DMA控制器，使用DMA的APB设备包括NAND、I2S、SDIO以及加解密模块。其中NAND和SDIO各需要一个DMA控制器，而I2S、加密模块、解密模块各需要两个DMA控制器分别控制DMA写操作和DMA读操作。根据应用场景不同，需要通过芯片配置寄存器来设置APB设备使用哪个DMA控制器。

另外，2K1000的DMA控制器支持64位地址空间，这主要通过dma_order[0]来控制，当该位设置为1时表示DMA控制器工作在64位地址空间，反之为32位地址空间。在64位地址模式下，需要扩展DMA_ORDER_ADDR和DMA_SADDR为64位寄存器。

# 34.4 DMA描述符

# 34.4.1DMA_ORDER_ADDR_LOW

中文名：下一个描述符低位地址寄存器寄存器位宽：[31：0]偏移地址： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:1</td><td>dma_order_addr</td><td>31</td><td>R/W</td><td>存储器内部下一描述符地址寄存器（低 32 位）</td></tr><tr><td>0</td><td>Dma_order_en</td><td>1</td><td>R/W</td><td>描述符是否有效信号</td></tr></table>

说明：存储下一个DMA描述符的地址，dma_order_en是下个DMA描述符的使能位，如果该位为1表示下个描述符有效，该位为0表示下个描述符无效，不执行操作，地址16字节对齐。在配置DMA描述符时，该寄存器存放的是下个描述符的地址，执行完该次DMA操作后，通过判断dma_order_en信号确定是否开始下次DMA操作。在64位地址模式下，该寄存器存储低32位地址。

# 34.4.2DMA_SADDR

中文名：内存低位地址寄存器寄存器位宽：[31：0]偏移地址： 0x40复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_saddr</td><td>32</td><td>R/W</td><td>DMA 操作的内存地址（低 32 位）</td></tr></table>

说明：DMA操作分为：从内存中读数据，保存在DMA控制器的缓存中，由APB发请求来访问DMA缓存中的数据，该寄存器指定了读ddr3的地址；从APB设备读数据保存

在 DMA 缓存中，当 DMA 缓存中的字超过一定数目，就往内存中写，该寄存器指定了写内存的地址。在 64 位地址模式下，该寄存器存储低 32 位地址。

# 34.4.3 DMA_DADDR

中文名：设备地址寄存器

寄存器位宽：[31：0]

偏移地址： 0x8

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31</td><td></td><td>1</td><td>R/W</td><td>保留</td></tr><tr><td>30</td><td></td><td>1</td><td>R/W</td><td>保留</td></tr><tr><td>29:28</td><td></td><td>2</td><td>R/W</td><td>保留</td></tr><tr><td>27:0</td><td>dma_daddr</td><td>28</td><td>R/W</td><td>DMA 操作的 APB 设备地址</td></tr></table>

说明：从内存中读数据，保存在 DMA 控制器的缓存中，由 APB 发请求来访问 DMA 缓存中的数据，该寄存器指定了写 APB 设备的地址；从 APB 设备读数据保存在 DMA 缓存中，当 DMA 缓存中的字超过一定数目，就往内存中写，该寄存器指定了读 APB 设备的地址。使用 DMA 的 APB 设备都有对应的数据 buffer 地址，比如 NAND 控制器的数据 buffer 偏移地址为 0x40，默认的设备地址就可以配置成 0x1fe06040。

# 34.4.4 DMA_LENGTH

中文名：长度寄存器

寄存器位宽：[31：0]

偏移地址： 0xc

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_length</td><td>32</td><td>R/W</td><td>传输数据长度寄存器</td></tr></table>

说明：代表一块被搬运内容的长度，单位是字。当搬运完 length 长度的字之后，开始下个 step 即下一个循环。开始新的循环，则再次搬运 length 长度的数据。当 step 变为 1，单个 DMA 描述符操作结束，开始读下个描述符。

# 34.4.5 DMA_STEP_LENGTH

中文名：间隔长度寄存器

寄存器位宽：[31：0]

偏移地址： 0x10

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_step_length</td><td>32</td><td>R/W</td><td>数据传输间隔长度寄存器</td></tr></table>

说明：间隔长度说明两块被搬运内存数据块之间的长度，前一个 step 的结束地址与后一个 step 的开始地址之间的间隔。

# 34.4.6 DMA_STEP_TIMES

中文名：循环次数寄存器

寄存器位宽：[31:0]

偏移地址： 0x14

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_step_times</td><td>32</td><td>R/W</td><td>数据传输循环次数寄存器</td></tr></table>

说明：循环次数说明在一次DMA操作中需要搬运的块的数目。如果只想搬运一个连续的数据块，循环次数寄存器的值可以赋值为1。

# 34.4.7DMA_CMD

中文名：控制寄存器

寄存器位宽：[31:0]

偏移地址： 0x18

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>14:13</td><td>Dma_cmd</td><td>2</td><td>R/W</td><td>源、目的地址生成方式</td></tr><tr><td>12</td><td>dma_r_w</td><td>1</td><td>R/W</td><td>DMA 操作类型，“1”为读 ddr3 写设备，“0”为读设备写 ddr3</td></tr><tr><td>11:8</td><td>dma_write_state</td><td>4</td><td>R/W</td><td>DMA 写数据状态</td></tr><tr><td>7:4</td><td>dma_read_state</td><td>4</td><td>R/W</td><td>DMA 读数据状态</td></tr><tr><td>3</td><td>dma_trans_over</td><td>1</td><td>R/W</td><td>DMA 执行完被配置的所有描述符操作</td></tr><tr><td>2</td><td>dma_single_trans_over</td><td>1</td><td>R/W</td><td>DMA 执行完一次描述符操作</td></tr><tr><td>1</td><td>dma_int</td><td>1</td><td>R/W</td><td>DMA 中断信号</td></tr><tr><td>0</td><td>dma_int_mask</td><td>1</td><td>R/W</td><td>DMA 中断是否被屏蔽掉</td></tr></table>

说明：dma_single_trans_over  $= 1$  指一次DMA操作执行结束，此时length  $= 0$  且step_times  $= 1$  ，开始取下个DMA操作的描述符。下个DMA操作的描述符地址保存在DMA_ORDER_ADDR寄存器中，如果DMA_ORDER_ADDR寄存器中dma_order_en  $= 0$  ，则dma_trans_over  $= 1$  ，整个dma操作结束，没有新的描述符要读；如果dma_order_en  $= 1$  ，则dma_trans_over置为0，开始读下个dma描述符。dma_int为DMA的中断，如果没有中断屏蔽，在一次配置的DMA操作结束后发生中断。CPU处理完中断后可以直接将其置低，也可以等到DMA进行下次传输时自动置低。dma_int_mask为对应dma_int的中断屏蔽。dma_read_state说明了DMA当前的读状态。dma_write_state说明了DMA当前的写状态。

DMA写状态(WRITE_STATE[3:0])描述，DMA包括以下几个写状态：

<table><tr><td>Write_state</td><td>[3:0]</td><td>描述</td></tr><tr><td>Write_idle</td><td>4&#x27;h0</td><td>写状态正处于空闲状态</td></tr></table>

<table><tr><td>W_ddr_wait</td><td>4&#x27;h1</td><td>Dma 判断需要执行读设备写内存操作，并发起写内存请求，但是内存还没准备好响应请求，因此 dma 一直在等待内存的响应</td></tr><tr><td>Write_ddr</td><td>4&#x27;h2</td><td>内存接收了 dma 写请求，但是还没有执行完写操作</td></tr><tr><td>Write_ddr_end</td><td>4&#x27;h3</td><td>内存接收了 dma 写请求，并完成写操作，此时 dma 处于写内存操作完成状态</td></tr><tr><td>Write_dma_wait</td><td>4&#x27;h4</td><td>Dma 发出将 dma 状态寄存器写回内存的请求，等待内存接收请求</td></tr><tr><td>Write_dma</td><td>4&#x27;h5</td><td>内存接收写 dma 状态请求，但是操作还未完成</td></tr><tr><td>Write_dma_end</td><td>4&#x27;h6</td><td>内存完成写 dma 状态操作</td></tr><tr><td>Write_step_end</td><td>4&#x27;h7</td><td>Dma 完成一次 length 长度的操作（也就是说完成一个 step）</td></tr></table>

DMA读状态(READ_STATE[3:0])描述，DMA包括以下几个读状态：

<table><tr><td>Read_state</td><td>[3:0]</td><td>描述</td></tr><tr><td>Read_idle</td><td>4&#x27;h0</td><td>读状态正处于空闲状态</td></tr><tr><td>Read_ready</td><td>4&#x27;h1</td><td>接收到开始 dma 操作的 start 信号后，进入准备好状态，开始读描述符</td></tr><tr><td>Get_order</td><td>4&#x27;h2</td><td>向内存发出读描述符请求，等待内存应答</td></tr><tr><td>Read_order</td><td>4&#x27;h3</td><td>内存接收读描述符请求，正在执行读操作</td></tr><tr><td>Finish_order_end</td><td>4&#x27;h4</td><td>内存读完 dma 描述符</td></tr><tr><td>R_ddr_wait</td><td>4&#x27;h5</td><td>Dma 向内存发出读数据请求，等待内存应答</td></tr><tr><td>Read_ddr</td><td>4&#x27;h6</td><td>内存接收 dma 读数据请求，正在执行读数据操作</td></tr><tr><td>Read_ddr_end</td><td>4&#x27;h7</td><td>内存完成 dma 的一次读数据请求</td></tr><tr><td>Read_dev</td><td>4&#x27;h8</td><td>Dma 进入读设备状态</td></tr><tr><td>Read_dev_end</td><td>4&#x27;h9</td><td>设备返回读数据，结束此次读设备请求</td></tr><tr><td>Read_step_end</td><td>4&#x27;h10</td><td>结束一次 step 操作，step times 减 1</td></tr></table>

# 34.4.8DMA_ORDER_ADDR_HIGH

中文名：下一个描述符高位地址寄存器

寄存器位宽：[31：0]

偏移地址： 0x20

复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_order_addr</td><td>32</td><td>R/W</td><td>存储器内部下一个描述符地址寄存器(高 32 位)</td></tr></table>

# 34.4.9DMA_SADDR_HIGH

中文名：内存高位地址寄存器寄存器位宽：[31：0]偏移地址： 0x24复位值： 0x00000000

<table><tr><td>位域</td><td>位域名称</td><td>位宽</td><td>访问</td><td>描述</td></tr><tr><td>31:0</td><td>dma_saddr</td><td>32</td><td>R/W</td><td>DMA 操作的内存地址(高 32 位)</td></tr></table>

# 35 VPU 控制器 (Dev 16)

# 35.1 访问地址

2K1000 的 Device 16 为 VPU 控制器，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>VPU</td><td>0x0</td><td>0x10</td><td>0x0</td><td>0x1_FF</td><td>0xFE_0000_8000</td><td></td></tr></table>

VPU 内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:09]</td><td>BAR_BASE</td><td>Device 16、FUNC 0 的基地址寄存器值</td></tr><tr><td>[08:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>

# 36 CAMERA 接口控制器（Dev 17）

# 36.1功能概述

36.1 功能概述Camera Interface 模块是视频输入转换存储模块。实现了将 Camera 捕捉到的数据进行转换、并通过 DMA 存储到 memory 中。该模块支持 ITU- R BT 601/656 8- bit 模式，仅支持 YUV 格式输入。可以将 camera 产生的 YUV 三种颜色信号经打包成组后分别由 DMA 方式传输到 ESRAM 中的三片存储区。该设备支持图像的镜像和翻转，以便适应手持式设备在移动环境中图像的捕捉。可变的同步信号极性使得可以兼容各种摄像头外设。

# 36.2访问地址

2K1000的Device17为CAMERA接口控制器，其配置空间基本信息如下：

<table><tr><td>设备</td><td>总线号</td><td>设备号</td><td>功能号</td><td>配置空间掩码</td><td>配置头访问首地址（64位模式）</td><td>备注</td></tr><tr><td>CAMERA</td><td>0x0</td><td>0x11</td><td>0x0</td><td>0xFF</td><td>0xFE_0000_8800</td><td></td></tr></table>

CAMERA控制器内部寄存器的物理地址构成如下：

<table><tr><td>地址位</td><td>构成</td><td>备注</td></tr><tr><td>[63:08]</td><td>BAR_BASE</td><td>Device 11、FUNC 0 的基地址寄存器值</td></tr><tr><td>[07:00]</td><td>REG</td><td>内部寄存器地址</td></tr></table>