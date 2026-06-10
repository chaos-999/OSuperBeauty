#include "proc/proc.h"
#include "signal.h"
#include "memlayout.h"
#include "defs.h"

extern char sig_trampoline[];
// #ifdef RISCV
// extern char sig_handler[];
// #endif

extern struct proc proc[NPROC];

// 初始化进程的信号处理系统
void sig_init(struct proc *p) {
    p->sig_handlers = (struct sig_handlers *)kalloc();
    if (!p->sig_handlers) {
        panic("sig_init: out of memory");
    }

    memset(p->sig_handlers, 0, sizeof(struct sig_handlers));
    initlock(&p->sig_handlers->lock, "sighandlers");
    p->sig_handlers->refcount = 1;  // 初始引用计数为 1

    // 初始化默认处理器
    for (int i = 1; i <= MAX_SIGNALS; i++) {
        p->sig_handlers->handlers[i].sa_handler = SIG_DFL;
        sig_emptyset(&p->sig_handlers->handlers[i].sa_mask);
        p->sig_handlers->handlers[i].sa_flags = 0;
    }

    sig_emptyset(&p->sig_blocked);
    p->sig_pending = 0;
    p->sig_context = 0;
}

// 清理信号处理系统
void sig_cleanup(struct proc *p) {
    acquire(&p->sig_handlers->lock);
    p->sig_handlers->refcount--;
    int refcount = p->sig_handlers->refcount;
    release(&p->sig_handlers->lock);

    if (refcount == 0) {
        kfree(p->sig_handlers);
        p->sig_handlers = 0;
    }

    // 清理所有挂起的信号上下文
    struct sig_context *ctx = p->sig_context;
    while (ctx) {
        struct sig_context *next = ctx->prev;
        kfree(ctx);
        ctx = next;
    }
    p->sig_context = 0;
}

// 注册信号处理器
int sig_register(int signum, struct sigaction *act, struct sigaction *oldact) {
    struct proc *p = myproc();

    if (!sig_valid(signum)) {
        return -1;
    }

    // SIGKILL和SIGSTOP不能被捕获或忽略
    if (signum == SIGKILL || signum == SIGSTOP) {
        return -1;
    }

    acquire(&p->sig_handlers->lock);

    if (oldact) {
        *oldact = p->sig_handlers->handlers[signum];
    }

    if (act) {
        p->sig_handlers->handlers[signum] = *act;
        // 确保不能屏蔽SIGKILL和SIGSTOP
        sig_delset(&p->sig_handlers->handlers[signum].sa_mask, SIGKILL);
        sig_delset(&p->sig_handlers->handlers[signum].sa_mask, SIGSTOP);
    }

    release(&p->sig_handlers->lock);
    return 0;
}

// 修改信号屏蔽字
int sig_mask_change(int how, sigset_t *set, sigset_t *oldset) {
    struct proc *p = myproc();

    if (oldset) {
        *oldset = p->sig_blocked;
    }

    if (!set) {
        return 0;
    }

    switch (how) {
        case SIG_BLOCK:
            p->sig_blocked.__val[0] |= set->__val[0];
            break;
        case SIG_UNBLOCK:
            p->sig_blocked.__val[0] &= ~set->__val[0];
            break;
        case SIG_SETMASK:
            p->sig_blocked = *set;
            break;
        default:
            return -1;
    }

    // 确保SIGKILL和SIGSTOP永远不被屏蔽
    sig_delset(&p->sig_blocked, SIGKILL);
    sig_delset(&p->sig_blocked, SIGSTOP);

    return 0;
}

// 默认信号处理
void sig_default_action(struct proc *p, int signum) {
    switch (signum) {
        case SIGHUP:
        case SIGINT:
        case SIGQUIT:
        case SIGILL:
        case SIGTRAP:
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGKILL:
        case SIGSEGV:
        case SIGPIPE:
        case SIGALRM:
        case SIGTERM:
        case SIGUSR1:
        case SIGUSR2:
            // 终止进程
            p->killed = 1;
            break;

        case SIGCHLD:
            // 忽略子进程状态改变
            break;

        case SIGCONT:
            // 继续执行（如果之前被停止）
            break;

        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
            // 停止进程（暂不实现）
            break;

        default:
            // 其他信号默认忽略
            break;
    }
}

// 处理信号入口
void sig_handler_entry(struct proc *p, int signum, struct sigaction *sa) {
    // 分配信号上下文
    struct sig_context *ctx = (struct sig_context *)kalloc();
    if (!ctx) {
        p->killed = 1;
        return;
    }

    // 保存当前状态
    ctx->saved_tf = *p->trapframe;
    ctx->old_mask = p->sig_blocked;
    ctx->prev = p->sig_context;
    p->sig_context = ctx;

    // 设置新的信号屏蔽字
    p->sig_blocked.__val[0] |= sa->sa_mask.__val[0];
    sig_addset(&p->sig_blocked, signum);

    // 设置信号处理器执行环境
#ifdef RISCV
    p->trapframe->ra = SIG_TRAMPOLINE;
    p->trapframe->epc = (uint64)sa->sa_handler;
#elif defined(LOONGARCH)
    p->trapframe->ra = SIG_TRAMPOLINE;
    p->trapframe->era = (uint64)sa->sa_handler;
#endif

    p->trapframe->sp -= PGSIZE;  // 为信号处理器预留栈空间
    p->trapframe->a0 = signum;
}

// 投递信号
int sig_deliver(struct proc *p) {
    if (p->sig_pending == 0) {
        return 0;
    }

    acquire(&p->sig_handlers->lock);

    // 查找待处理的信号
    for (int sig = 1; sig <= MAX_SIGNALS; sig++) {
        if ((p->sig_pending & (1ULL << (sig - 1))) && !sig_ismember(&p->sig_blocked, sig)) {
            // 清除待处理标记
            p->sig_pending &= ~(1ULL << (sig - 1));

            struct sigaction *sa = &p->sig_handlers->handlers[sig];

            release(&p->sig_handlers->lock);

            if (sa->sa_handler == SIG_DFL) {
                sig_default_action(p, sig);
            } else if (sa->sa_handler != SIG_IGN) {
                sig_handler_entry(p, sig, sa);
                return 1;  // 信号已投递
            }

            acquire(&p->sig_handlers->lock);
        }
    }

    release(&p->sig_handlers->lock);
    return 0;
}

// 恢复信号处理前的上下文
void sig_restore_context(void) {
    struct proc *p = myproc();

    if (!p->sig_context) {
        p->killed = 1;
        return;
    }

    struct sig_context *ctx = p->sig_context;

    // 恢复原始状态
    *p->trapframe = ctx->saved_tf;
    p->sig_blocked = ctx->old_mask;

    // 释放上下文
    p->sig_context = ctx->prev;
    kfree(ctx);
}

// 发送信号到进程
int send_signal(int pid, int sig) {
    if (!sig_valid(sig)) {
        return -1;
    }

    struct proc *p;

    for (p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if (p->state != UNUSED && p->pid == pid) {
            p->sig_pending |= (1ULL << (sig - 1));
            release(&p->lock);
            return 0;
        }
        release(&p->lock);
    }

    return -1;
}

// 发送信号到线程
int send_signal_thread(int tid, int sig) {
    // 在当前实现中，线程ID就是进程ID
    return send_signal(tid, sig);
}