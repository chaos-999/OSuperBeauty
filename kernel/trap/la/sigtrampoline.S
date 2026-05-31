# Signal handling trampoline for LOONGARCH

    .section sigtrampsec
    .global sig_trampoline
    .align 12
    
sig_trampoline:
    # 调用sigreturn系统调用
    li.d $a7, 139  # SYS_rt_sigreturn
    syscall 0
    # 不应该返回
    break 0