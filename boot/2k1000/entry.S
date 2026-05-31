#include "asm.h"

.section .head.text, "ax"
.global _entry
_entry:

        # 切换到自引用位置，确保后续 DMW 生效（参考 2k1000la）
        la.global   $t0, 0f
        jirl        $zero, $t0, 0
0:
        # DMW 配置（2k1000la 成功参数）：
        # DMWIN0: 0x9000_xxxx_xxxx_xxxx，Cacheable & IF 允许
        li.d        $t0, 0x9000000000000011
        csrwr       $t0, LOONGARCH_CSR_DMWIN0 
        # DMWIN1: 0x8000_xxxx_xxxx_xxxx，Uncached
        li.d        $t0, 0x8000000000000001
        csrwr       $t0, LOONGARCH_CSR_DMWIN1 
        li.d        $t0, 0x0000000000000000
        csrwr       $t0, LOONGARCH_CSR_DMWIN2
        csrwr       $t0, LOONGARCH_CSR_DMWIN3
        csrwr       $t0, LOONGARCH_CSR_TLBRENTRY 

        # 基本 CSR 初始化
        li.w        $t0, 0xb0             # PLV=0, IE=0, PG=1
        csrwr       $t0, LOONGARCH_CSR_CRMD
        invtlb      0x0, $zero, $zero
        li.w        $t0, 0x00
        csrwr       $t0, LOONGARCH_CSR_PRMD
        li.w        $t0, 0x00
        csrwr       $t0, LOONGARCH_CSR_EUEN

        # 设置每核栈
        la.global   $sp, boot_stack
        li.d        $a0, 1024*4
        csrrd       $a1, LOONGARCH_CSR_CPUID    # 获取CPUID
        addi.d      $tp, $a1, 0                 # tp = CPUID
        addi.d      $a1, $a1, 1                 # a1 = CPUID + 1
        mul.d       $a0, $a0, $a1               # a0 = 4096 * (CPUID + 1)
        add.d       $sp, $sp, $a0               # sp = boot_stack + 4096 * (CPUID + 1)

        # 仅在启动核清零 BSS，避免未初始化的全局/锁导致主路径卡住
        bnez        $tp, 2f
        # 使用 __bss_start/__bss_stop 覆盖整段 BSS（包含 COMMON），更保险
        la.global   $t0, __bss_start
        la.global   $t1, __bss_stop
        li.d        $t2, 0
1:
        bgeu        $t0, $t1, 2f
        st.d        $t2, $t0, 0
        addi.d      $t0, $t0, 8
        b           1b
2:

        # 跳转 2k1000 专属 main（极早期串口已由 C 层完成）
        bl          main_2k1000

1:
        b           1b

    .section .bss.stack
    .align 12
    .globl boot_stack
boot_stack:
    .space 4096 * 4 * 4
    .globl boot_stack_top
boot_stack_top:

    .section .head.text, "ax"
    .globl early_uart_banner
early_uart_banner:
    la.global  $a0, banner_str
    bl         early_uart_puts
    jr         $ra

    .section .rodata
banner_str:
    .asciz "[EARLY] entered entry.S before main\n"

    # 保留占位，便于必要时在极早期打印


