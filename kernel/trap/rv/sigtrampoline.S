# Signal handling trampoline for RISCV

    .section sigtrampsec
    .global sig_trampoline
    .align 4
    
sig_trampoline:
    # 调用sigreturn系统调用
    li a7, 139
    ecall
    # 不应该返回到这里
    unimp