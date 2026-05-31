#pragma once
// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0
// 10001000 -- virtio disk
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

// qemu puts UART registers here in physical memory.

#ifdef VF2
// VisionFive2 board memory layout
#define UART0 0x10000000L
// On VF2, UART0 interrupt source ID is 32 on the PLIC
#define UART0_IRQ 32

// virtio mmio interface (same as QEMU virt machine)
#define VIRTIO0 0x10001000
// Keep virtio on standard source if present; adjust if board uses different ID
#define VIRTIO0_IRQ 1

#define CLOCK_IRQ 5

// core local interruptor (CLINT), which contains the timer.
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8 * (hartid))
#define CLINT_MTIME (CLINT + 0xBFF8)

// Platform-level interrupt controller (PLIC)
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
// VF2 S-mode context windows (align with reference project layout)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + ((hart) - 1) * 0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2100 + ((hart) - 1) * 0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + ((hart) - 1) * 0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x202000 + ((hart) - 1) * 0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + ((hart) - 1) * 0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x202004 + ((hart) - 1) * 0x2000)

// VF2 kernel base address (loaded by u-boot at 0x80400000)
#define KERNBASE 0x80400000
#define PHYSTOP 0x88000000

#define TRAMPOLINE (MAXVA - PGSIZE)
#define USTACK_PAGE 32
#define USTACK (MAXVA - 512 * 10 * PGSIZE - USTACK_PAGE * PGSIZE)
#define USTACK_GURAD_PAGE (USTACK - PGSIZE)
#define USTACK_TOP (USTACK + USTACK_PAGE * PGSIZE)
#define KSTACK(p) (TRAMPOLINE - ((p) + 1) * 4 * PGSIZE)
#define SIG_TRAMPOLINE (TRAMPOLINE - PGSIZE)
#define TRAPFRAME (SIG_TRAMPOLINE - PGSIZE)
#define MMAPEND TRAPFRAME

#elif defined(RISCV)

#define UART0 0x10000000L
#define UART0_IRQ 10

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

#define CLOCK_IRQ 5

// core local interruptor (CLINT), which contains the timer.
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8 * (hartid))
#define CLINT_MTIME (CLINT + 0xBFF8)  // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart) * 0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart) * 0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart) * 0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart) * 0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart) * 0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart) * 0x2000)

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80000000
#define PHYSTOP 0x88000000

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAXVA - PGSIZE)

#define USTACK_PAGE 32
#define USTACK (MAXVA - 512 * 10 * PGSIZE - USTACK_PAGE * PGSIZE)
#define USTACK_GURAD_PAGE (USTACK - PGSIZE)
#define USTACK_TOP (USTACK + USTACK_PAGE * PGSIZE)
// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAMPOLINE - ((p) + 1) * 4 * PGSIZE)

#define SIG_TRAMPOLINE (TRAMPOLINE - PGSIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (SIG_TRAMPOLINE - PGSIZE)

#define MMAPEND TRAPFRAME

#elif defined(LOONGARCH)
// Physical memory layout (LoongArch)

// LoongArch direct-map windows
#define DMWIN_MASK 0x9000000000000000
#define DMWIN1_MASK 0x8000000000000000
#define VIRT_DMWIN_MASK 0xf000000000000000

#define VIRT2PHY(addr) ((addr) & ~VIRT_DMWIN_MASK)

#ifdef LA2K1000
// ===================== 2K1000 板级映射（39位地址空间高位使用） =====================
#include "board/2k1000.h"

// UART 与中断编号（IOINTC 提供 UART0=2，定时器仍用 5）
// 早期阶段优先走 DMWIN1(0x8...) 非缓存直映，避免 0x9... 触发 TLB 异常
#define UART0 (LA2K1000_UART0_BASE | DMWIN1_MASK)
#define UART0_IRQ LA2K1000_UART0_IRQ
#define CLOCK_IRQ 5

// 物理内存直映区（教学 OS 约定）
#define PHYSBASE (0x90000000UL | DMWIN_MASK)
#define PHYSTOP (PHYSBASE + 512 * 1024 * 1024)

// 39位地址空间最大值 (2^39 - 1)
#define MAXVA_39BIT (1ULL << 39)

// 用户/内核栈与陷阱帧布局（2K1000：启用39位高位使用）
#define TRAPFRAME (MAXVA_39BIT - PGSIZE)     // 39位地址空间顶部下方一页
#define SIG_TRAMPOLINE (TRAPFRAME - PGSIZE)  // 信号跳板页

#define USTACK_PAGE 32
#define USTACK_TOP (SIG_TRAMPOLINE)                 // 栈顶紧贴信号跳板页下方
#define USTACK (USTACK_TOP - USTACK_PAGE * PGSIZE)  // 用户栈空间
#define USTACK_GURAD_PAGE (USTACK - PGSIZE)         // 栈保护页

#define MMAPEND (USTACK)  // mmap 空间位于栈之下

// 每进程内核栈（使用高位地址空间）
#define KSTACK(p) (TRAPFRAME - ((p) + 1) * 2 * PGSIZE)

// 便捷宏
#define PA2VA(pa) ((pa) & (~(DMWIN_MASK)))

#else

// ===================== LoongArch 默认（QEMU/LS7A） =====================

// qemu puts UART registers here in virtual memory.
#define UART0 (0x1fe001e0UL | DMWIN_MASK)
#define UART0_IRQ 2

#define CLOCK_IRQ 5

/* ============== LS7A registers =============== */
#define LS7A_PCH_REG_BASE (0x10000000UL | DMWIN_MASK)

#define LS7A_INT_MASK_REG LS7A_PCH_REG_BASE + 0x020
#define LS7A_INT_EDGE_REG LS7A_PCH_REG_BASE + 0x060
#define LS7A_INT_CLEAR_REG LS7A_PCH_REG_BASE + 0x080
#define LS7A_INT_HTMSI_VEC_REG LS7A_PCH_REG_BASE + 0x200
#define LS7A_INT_STATUS_REG LS7A_PCH_REG_BASE + 0x3a0
#define LS7A_INT_POL_REG LS7A_PCH_REG_BASE + 0x3e0

// the kernel expects there to be RAM for use by user pages from PHYSBASE to PHYSTOP
#define PHYSBASE (0x90000000UL | DMWIN_MASK)
#define PHYSTOP (PHYSBASE + 512 * 1024 * 1024)

#define USTACK_PAGE 32
#define USTACK (MAXVA - 512 * 10 * PGSIZE - USTACK_PAGE * PGSIZE)
#define USTACK_GURAD_PAGE (USTACK - PGSIZE)
#define USTACK_TOP (USTACK + USTACK_PAGE * PGSIZE)

// map kernel stacks beneath the trampframe, each surrounded by invalid guard pages.
#define KSTACK(p) (TRAPFRAME - ((p) + 1) * 2 * PGSIZE)

// User memory layout（见注释）
#define TRAPFRAME (MAXVA - PGSIZE)
#define SIG_TRAMPOLINE (TRAPFRAME - PGSIZE)

#define MMAPEND SIG_TRAMPOLINE

#define PA2VA(pa) ((pa) & (~(DMWIN_MASK)))

#endif  // LA2K1000

#endif