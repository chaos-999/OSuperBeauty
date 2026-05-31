#include "asm.h"

.section .text
.global _entry
_entry:


        li.d        $t0, 0x8000000000000001
        csrwr       $t0, LOONGARCH_CSR_DMWIN0
        li.d        $t0, (0x9000000000000001 | 1 << 4)
        csrwr       $t0, LOONGARCH_CSR_DMWIN1 
        // li.d        $t0, 0x0000000000000000
        // csrwr       $t0, LOONGARCH_CSR_DMWIN2
        // csrwr       $t0, LOONGARCH_CSR_DMWIN3
        csrwr       $t0, LOONGARCH_CSR_TLBRENTRY

        li.w	    $t0, 0xb0
	csrwr	    $t0, LOONGARCH_CSR_CRMD
        li.w        $t0, 0x00
        csrwr       $t0, LOONGARCH_CSR_PRMD
        li.w        $t0, 0x00
        csrwr       $t0, LOONGARCH_CSR_EUEN

	# set up a stack for C.
        # boot_stack_top is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = boot_stack_top + (hartid * 4096)

        la.global   $sp, boot_stack
        li.d        $a0, 1024*4
	csrrd       $a1, LOONGARCH_CSR_CPUID    # 获取CPUID
        addi.d      $tp, $a1, 0                 # tp = CPUID
        addi.d      $a1, $a1, 1                 # a1 = CPUID + 1
        mul.d       $a0, $a0, $a1               # a0 = 4096 * (CPUID + 1)
        add.d       $sp, $sp, $a0               # sp = boot_stack_top + 4096 * (CPUID + 1)
	# jump to main in main.c
        li.d        $t0, 0x1
        cpucfg      $t1, $t0                    # read the CFG
        bl          main
spin:
        b           spin

    .section .bss.stack
    .align 12
    .globl boot_stack
boot_stack:
    .space 4096 * 4 * 4
    .globl boot_stack_top
boot_stack_top:
