# VF2 board entry point  
# u-boot loads kernel at 0x80400000 and jumps here
# 完全跟随参考项目 xv6-vf2/kernel/entry_vf2.S 的实现

.section .text
.global _entry
_entry:
        # 跟随参考项目的栈设置
        add t0, a0, 1        # t0 = hartid + 1
        slli t0, t0, 14      # t0 = (hartid + 1) * 16384 (16KB per CPU)
        la sp, boot_stack    # 使用boot_stack
        add sp, sp, t0       # sp = boot_stack + offset
        # 清零BSS：使用链接脚本导出的 __bss_start/__bss_end
        la t1, __bss_start
        la t2, __bss_end
1:
        bgeu t1, t2, 2f
        sd zero, 0(t1)
        addi t1, t1, 8
        j 1b
2:
        call main            # 直接调用main

loop:
        j loop

# 跟随参考项目定义栈空间
.section .bss.stack
.align 12
.globl boot_stack
boot_stack:
        .space 4096 * 4 * 4  # 64KB total: 4 CPUs * 16KB each
.globl boot_stack_top
boot_stack_top: