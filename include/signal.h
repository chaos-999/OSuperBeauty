#pragma once
#include "types.h"
#include "lib/list.h"

#define MAX_SIGNALS 64

// 信号定义
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL SIGIO
#define SIGPWR 30
#define SIGSYS 31
#define SIGUNUSED 31

// sigprocmask操作
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

typedef void (*sig_handler_t)(int);

#define SIG_DFL ((sig_handler_t)0)
#define SIG_IGN ((sig_handler_t)1)
#define SIG_ERR ((sig_handler_t) - 1)

// 信号集结构 - 修改成员名以避免与宏参数冲突
typedef struct {
    uint64 __val[1];  // 使用__val避免命名冲突
} sigset_t;

struct sigaction {
    sig_handler_t sa_handler;
    sigset_t sa_mask;
    int sa_flags;
    void *sa_restorer;  // 为了兼容性添加
};

// 信号上下文保存结构
struct sig_context {
    struct trapframe saved_tf;
    sigset_t old_mask;
    struct sig_context *prev;
};

// 进程的信号处理器表
struct sig_handlers {
    struct sigaction handlers[MAX_SIGNALS + 1];
    struct spinlock lock;  // 添加锁保护
    int refcount;         // 引用计数，支持共享
};

// 信号操作宏 - 修改以使用新的成员名
#define SIGSET_WORD(sig) ((sig - 1) / 64)
#define SIGSET_BIT(sig) ((sig - 1) % 64)
#define sig_ismember(set, signo) ((set)->__val[SIGSET_WORD(signo)] & (1ULL << SIGSET_BIT(signo)))
#define sig_addset(set, signo) ((set)->__val[SIGSET_WORD(signo)] |= (1ULL << SIGSET_BIT(signo)))
#define sig_delset(set, signo) ((set)->__val[SIGSET_WORD(signo)] &= ~(1ULL << SIGSET_BIT(signo)))
#define sig_emptyset(set) ((set)->__val[0] = 0)
#define sig_fillset(set) ((set)->__val[0] = ~0ULL)

#define sig_valid(sig) ((sig) >= 1 && (sig) <= MAX_SIGNALS)
#define sig_blockable(sig) ((sig) != SIGKILL && (sig) != SIGSTOP)

struct proc;

// 函数声明
void sig_init(struct proc *p);
void sig_cleanup(struct proc *p);
int sig_deliver(struct proc *p);
void sig_handler_entry(struct proc *p, int signum, struct sigaction *sa);
void sig_default_action(struct proc *p, int signum);
void sig_restore_context(void);
int sig_register(int signum, struct sigaction *act, struct sigaction *oldact);
int sig_mask_change(int how, sigset_t *set, sigset_t *oldset);
int send_signal(int pid, int sig);
int send_signal_thread(int tid, int sig);