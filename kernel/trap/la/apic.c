#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "apic.h"

#include <dev/pci/pci.h>
//
// the loongarch 7A1000 I/O Interrupt Controller Registers.
//

void apic_init(void) {
#ifdef LA2K1000
    extern void iointc_2k1000_init(void);
    iointc_2k1000_init();
#else
    *(volatile uint64*)(LS7A_INT_MASK_REG) = ~((0x1UL << UART0_IRQ) | (0x1UL << PCIE_IRQ));
    *(volatile uint64*)(LS7A_INT_EDGE_REG) = (0x1UL << UART0_IRQ) | (0x1UL << PCIE_IRQ);
    *(volatile uint8*)(LS7A_INT_HTMSI_VEC_REG + UART0_IRQ) = UART0_IRQ;
    *(volatile uint8*)(LS7A_INT_HTMSI_VEC_REG + PCIE_IRQ) = PCIE_IRQ;
    *(volatile uint64*)(LS7A_INT_POL_REG) = 0x0UL;
#endif
}

// tell the apic we've served this IRQ.
void apic_complete(uint64 irq)

{
#ifdef LA2K1000
    // 2K1000 IOINTC：将 64 位 mask 拆分写入两个 32 位清除寄存器
    uint32 lo = (uint32)(irq & 0xFFFFFFFFu);
    uint32 hi = (uint32)((irq >> 32) & 0xFFFFFFFFu);
    if (lo) *(volatile uint32*)(LA2K1000_INTCLR_0) = lo;
    if (hi) *(volatile uint32*)(LA2K1000_INTCLR_1) = hi;
#else
    *(volatile uint64*)(LS7A_INT_CLEAR_REG) = (irq);
#endif
}
