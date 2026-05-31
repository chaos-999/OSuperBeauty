#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "defs.h"

#ifdef RISCV

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void plicinit(void) {
    // set desired IRQ priorities non-zero (otherwise disabled).
    *(uint32*)(PLIC + UART0_IRQ * 4) = 1;
    *(uint32*)(PLIC + VIRTIO0_IRQ * 4) = 1;
#ifdef VF2
    printf("[PLIC] PRIORITY set: uart=%d virtio=%d\n", UART0_IRQ, VIRTIO0_IRQ);
#endif
}

void plicinithart(void) {
    int hart = cpuid();

    // set enable bits for this hart's S-mode
    // for the uart and virtio disk.
#ifdef VF2
    // VF2: delay enabling UART interrupt until after FS mount; keep disabled here
    *(uint32*)PLIC_SENABLE(hart) = 0u;
    *(uint32*)(PLIC_SENABLE(hart) + 4) = 0u;
    uint32 sen_lo = *(uint32*)PLIC_SENABLE(hart);
    uint32 sen_hi = *(uint32*)(PLIC_SENABLE(hart) + 4);
    printf("[PLIC] SENABLE hart=%d lo=0x%x hi=0x%x (UART disabled at boot)\n", hart, sen_lo,
           sen_hi);
#else
    *(uint32*)PLIC_SENABLE(hart) = (1u << UART0_IRQ) | (1u << VIRTIO0_IRQ);
#endif

    // set this hart's S-mode priority threshold to 0.
    *(uint32*)PLIC_SPRIORITY(hart) = 0;
#ifdef VF2
    printf("[PLIC] SPRIORITY hart=%d set to 0\n", hart);
#endif
}

#ifdef VF2
// Enable UART interrupt later when system is stable
void plic_enable_uart_late(void) {
    int hart = cpuid();
#if (UART0_IRQ >= 32)
    *(uint32*)(PLIC_SENABLE(hart) + 4) |= (1u << (UART0_IRQ - 32));
#else
    *(uint32*)PLIC_SENABLE(hart) |= (1u << UART0_IRQ);
#endif
    uint32 sen_lo = *(uint32*)PLIC_SENABLE(hart);
    uint32 sen_hi = *(uint32*)(PLIC_SENABLE(hart) + 4);
    printf("[PLIC] LATE-ENABLE UART: hart=%d lo=0x%x hi=0x%x\n", hart, sen_lo, sen_hi);
}
#endif

// ask the PLIC what interrupt we should serve.
int plic_claim(void) {
    int hart = cpuid();
    int irq = *(uint32*)PLIC_SCLAIM(hart);
    return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq) {
    int hart = cpuid();
    *(uint32*)PLIC_SCLAIM(hart) = irq;
}

#endif  // RISCV
