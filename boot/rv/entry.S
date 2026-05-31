        # qemu -kernel loads the kernel at 0x80200000 via OpenSBI
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80200000.
.section .text
.global _entry
_entry:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li t0, 1024*4
        # OpenSBI已经在a0寄存器中传递了hartid
        # 不再使用 csrr a1, mhartid
        addi t1, a0, 1
        mul t0, t0, t1
        add sp, sp, t0
        # 保持a0不变，作为hartid参数传递给start
        call start
spin:
        j spin
