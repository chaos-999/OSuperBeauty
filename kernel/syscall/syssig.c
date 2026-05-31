#include "types.h"
#include "platform.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include <lock/lock.h>
#include <proc/proc.h>
#include "lib/sbi.h"
#include "time.h"
#include "mem.h"
#include "fs/vfs/file.h"
#include "fs/vfs_ext4_ext.h"
#include "fs/fcntl.h"

// rt_sigaction系统调用
uint64 sys_rt_sigaction(void) {
    int signum;
    uint64 act_addr, oldact_addr;

    argint(0, &signum);
    argaddr(1, &act_addr);
    argaddr(2, &oldact_addr);

    struct sigaction act, oldact;
    struct proc *p = myproc();

    // 复制用户空间的sigaction结构
    if (act_addr) {
        if (copyin(p->pagetable, (char *)&act, act_addr, sizeof(act)) < 0) {
            return -1;
        }
    }

    // 注册信号处理器
    if (sig_register(signum, act_addr ? &act : NULL, oldact_addr ? &oldact : NULL) < 0) {
        return -1;
    }

    // 返回旧的sigaction
    if (oldact_addr) {
        if (copyout(p->pagetable, oldact_addr, (char *)&oldact, sizeof(oldact)) < 0) {
            return -1;
        }
    }

    return 0;
}

// rt_sigprocmask系统调用
uint64 sys_rt_sigprocmask(void) {
    int how;
    uint64 set_addr, oldset_addr;
    uint64 sigsetsize;

    argint(0, &how);
    argaddr(1, &set_addr);
    argaddr(2, &oldset_addr);
    argaddr(3, &sigsetsize);

    // 检查sigset大小
    if (sigsetsize != sizeof(sigset_t)) {
        return -1;
    }

    sigset_t set, oldset;
    struct proc *p = myproc();

    // 复制用户空间的sigset
    if (set_addr) {
        if (copyin(p->pagetable, (char *)&set, set_addr, sizeof(set)) < 0) {
            return -1;
        }
    }

    // 修改信号屏蔽字
    if (sig_mask_change(how, set_addr ? &set : NULL, oldset_addr ? &oldset : NULL) < 0) {
        return -1;
    }

    // 返回旧的sigset
    if (oldset_addr) {
        if (copyout(p->pagetable, oldset_addr, (char *)&oldset, sizeof(oldset)) < 0) {
            return -1;
        }
    }

    return 0;
}

// rt_sigtimedwait系统调用（简化实现）
uint64 sys_rt_sigtimedwait(void) {
    // 暂时返回-ENOSYS
    return -38;  // ENOSYS
}

// rt_sigreturn系统调用
uint64 sys_rt_sigreturn(void) {
    sig_restore_context();
    return 0;
}

// kill系统调用
uint64 sys_kills(void) {
    int pid, sig;

    argint(0, &pid);
    argint(1, &sig);

    return send_signal(pid, sig);
}

// tkill系统调用
uint64 sys_tkill(void) {
    int tid, sig;

    argint(0, &tid);
    argint(1, &sig);

    return send_signal_thread(tid, sig);
}

// tgkill系统调用
uint64 sys_tgkill(void) {
    int tgid, tid, sig;

    argint(0, &tgid);
    argint(1, &tid);
    argint(2, &sig);

    // 在当前实现中，我们没有线程组概念
    // 简单检查tgid和tid是否一致
    if (tgid != tid) {
        return -1;
    }

    return send_signal_thread(tid, sig);
}