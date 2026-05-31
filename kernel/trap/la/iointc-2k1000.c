#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "extioi.h"
#include "board/2k1000.h"

// 通过 DMWIN1(0x8...) 非缓存直映访问片上 IOINTC 寄存器，避免 TLB 异常
static inline void writel(uint32 val, uint64 addr) {
    *(volatile uint32*)((addr) | DMWIN1_MASK) = val;
}
static inline uint32 readl(uint64 addr) { return *(volatile uint32*)((addr) | DMWIN1_MASK); }

// 将中断按电平触发/极性默认配置，并使能常用外设中断。
// 路由：全部路由到 CPU0（核 0）的 EXTIOI 入口。
void iointc_2k1000_init(void) {
    // 1) 默认极性为 0（高有效）
    writel(0x00000000u, LA2K1000_INTPOL_0);
    writel(0x00000000u, LA2K1000_INTPOL_1);

    // 2) 触发方式：UART0=电平，PCIe=脉冲
    //    注意：中断 0..31 使用 *_0 寄存器，32..63 使用 *_1 寄存器
    {
        // UART0：电平（清除对应位）
        uint32 e0 = readl(LA2K1000_INTEDGE_0);
        e0 &= ~(1u << LA2K1000_UART0_IRQ);
        writel(e0, LA2K1000_INTEDGE_0);

        // PCIe：脉冲（置位） — 用运行时变量避免编译期 1u<<32 的移位溢出告警
        uint32 pcie_irq = (uint32)LA2K1000_PCIE_IRQ;
        if (pcie_irq >= 32u) {
            uint32 e1 = readl(LA2K1000_INTEDGE_1);
            uint32 pcie_bit = 1u << (pcie_irq - 32u);
            e1 |= pcie_bit;
            writel(e1, LA2K1000_INTEDGE_1);
        } else {
            uint32 e0b = readl(LA2K1000_INTEDGE_0);
            uint32 pcie_bit = 1u << pcie_irq;
            e0b |= pcie_bit;
            writel(e0b, LA2K1000_INTEDGE_0);
        }
    }

    // 3) 路由：全部到 CPU0（ENTRY 每字节一路）
    writel(0x00000000u, LA2K1000_ENTRY32_0);
    writel(0x00000000u, LA2K1000_ENTRY32_1);
    writel(0x00000000u, LA2K1000_ENTRY32_2);
    writel(0x00000000u, LA2K1000_ENTRY32_3);
    writel(0x00000000u, LA2K1000_ENTRY32_4);

    // 4) 清状态并使能 UART0/PCIe（按 bank 拆分）
    // UART0 在低 32 位
    writel((1u << LA2K1000_UART0_IRQ), LA2K1000_INTCLR_0);
    writel((1u << LA2K1000_UART0_IRQ), LA2K1000_INTSI_0);

    // PCIe 可能在高 32 位（32）
    {
        uint32 pcie_irq = (uint32)LA2K1000_PCIE_IRQ;
        if (pcie_irq >= 32u) {
            uint32 bit = (1u << (pcie_irq - 32u));
            writel(bit, LA2K1000_INTCLR_1);
            writel(bit, LA2K1000_INTSI_1);
        } else {
            uint32 bit = (1u << pcie_irq);
            writel(bit, LA2K1000_INTCLR_0);
            writel(bit, LA2K1000_INTSI_0);
        }
    }
}

uint64 iointc_2k1000_pending(void) {
    uint64 hi = (uint64)readl(LA2K1000_INTISR_1);
    uint64 lo = (uint64)readl(LA2K1000_INTISR_0);
    return (hi << 32) | lo;
}
