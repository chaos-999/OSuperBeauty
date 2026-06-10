#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "lock/lock.h"
#include "proc/proc.h"

#include "fs/vfs/fs.h"

#include "defs.h"
#include "mem.h"
#include "errno.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[];  // trampoline.S
extern char sig_trampoline[];

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void proc_mapstacks(pagetable_t kpgtbl) {
    struct proc *p;
#ifdef RISCV
    for (p = proc; p < &proc[NPROC]; p++) {
        char *pa = kmalloc(4 * PGSIZE);
        if (pa == 0) panic("kalloc");
        uint64 va = KSTACK((int)(p - proc));
        kvmmap(kpgtbl, va, (uint64)pa, 4 * PGSIZE, PTE_R | PTE_W);
    }
#elif defined(LOONGARCH)

    for (p = proc; p < &proc[NPROC]; p++) {
        char *pa = kalloc();
        if (pa == 0) panic("kalloc");
        uint64 va = KSTACK((int)(p - proc));
#ifdef LA2K1000
        if (mappages(kpgtbl, va, PGSIZE, (uint64)pa, PTE_P | PTE_W | PTE_MAT | PTE_D) != 0)
#else
        if (mappages(kpgtbl, va, PGSIZE, (uint64)pa, PTE_NX | PTE_P | PTE_W | PTE_MAT | PTE_D) != 0)
#endif
            panic("kvmmap");
    }
#endif
}

// initialize the proc table.
void procinit(void) {
    struct proc *p;
    struct cpu *c;

    // 显式初始化CPU数组，确保noff等字段为0
    for (c = cpus; c < &cpus[NCPU]; c++) {
        c->noff = 0;
        c->intena = 0;
        c->proc = 0;
    }

    initlock(&pid_lock, "nextpid");
    initlock(&wait_lock, "wait_lock");
    for (p = proc; p < &proc[NPROC]; p++) {
        initlock(&p->lock, "proc");
        p->state = UNUSED;
        p->kstack = KSTACK((int)(p - proc));
    }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid() {
    int id = r_tp();
    return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *mycpu(void) {
    int id = cpuid();
    struct cpu *c = &cpus[id];
    return c;
}

// Return the current struct proc *, or zero if none.
struct proc *myproc(void) {
    push_off();
    struct cpu *c = mycpu();
    struct proc *p = c->proc;
    pop_off();
    return p;
}

int allocpid() {
    int pid;

    acquire(&pid_lock);
    pid = nextpid;
    nextpid = nextpid + 1;
    release(&pid_lock);

    return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *allocproc(void) {
    struct proc *p;

    for (p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if (p->state == UNUSED) {
            goto found;
        } else {
            release(&p->lock);
        }
    }
    return 0;

found:
    p->pid = allocpid();
    p->tgid = p->pid; // Default: tgid == pid (single-threaded process)
    p->tid = p->pid;  // Default: tid == pid
    p->state = USED;
    p->clear_child_tid = 0;
    p->robust_list_head = 0;
    p->pt_shared = 0;
    p->uid = 0;
    p->euid = 0;
    p->suid = 0;
    p->gid = 0;
    p->egid = 0;
    p->sgid = 0;
    p->killed = 0;  // ensure known initial state
    p->exec_path[0] = '\0';
    for (int i = 0; i < NOFILE; i++) {
        p->ofile[i] = 0;
    }

    // signal init
    sig_init(p);

    // Allocate a trapframe page.
    if ((p->trapframe = (struct trapframe *)kalloc()) == 0) {
        freeproc(p);
        release(&p->lock);
        return 0;
    }
#ifdef LA2K1000
    // 避免第一次 userret 从未初始化的陷阱帧恢复出随机寄存器/era
    memset(p->trapframe, 0, PGSIZE);
#endif

    // An empty user page table.
    p->pagetable = proc_pagetable(p);
    if (p->pagetable == 0) {
        freeproc(p);
        release(&p->lock);
        return 0;
    }

    // Set up new context to start executing at forkret,
    // which returns to user space.
    memset(&p->context, 0, sizeof(p->context));
    p->context.ra = (uint64)forkret;
    p->context.sp = p->kstack + PGSIZE;

    // Initialize VMA array
    memset(&p->vmas, 0, NVMA * sizeof(struct vma));

    return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void freeproc(struct proc *p) {
    if (p->trapframe) kfree((void *)p->trapframe);
    p->trapframe = 0;

    // Clean up VMA mappings
    if (!p->pt_shared) {
        for (int i = 0; i < NVMA; i++) {
            struct vma *v = &p->vmas[i];
            if (v->valid) {
                vmaunmap(p->pagetable, v->addr, v->length, v);
                if (v->f) {
                    get_fops()->close(v->f);
                }
                v->valid = 0;
            }
        }
    }

    sig_cleanup(p);

    // flush any buffered console output (last line may not end with \n)
    console_flush(p);

    if (p->pagetable && !p->pt_shared)
        proc_freepagetable(p->pagetable, p->sz);
    p->pagetable = 0;
    p->sz = 0;
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->state = UNUSED;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t proc_pagetable(struct proc *p) {
    pagetable_t pagetable;

    // An empty page table.
    pagetable = uvmcreate();
    if (pagetable == 0) return 0;
#ifdef RISCV
    // map the trampoline code (for system call return)
    // at the highest user virtual address.
    // only the supervisor uses it, on the way
    // to/from user space, so not PTE_U.
    if (mappages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline, PTE_R | PTE_X) < 0) {
        uvmfree(pagetable, 0);
        return 0;
    }

    if (mappages(pagetable, SIG_TRAMPOLINE, PGSIZE, (uint64)sig_trampoline, PTE_R | PTE_X | PTE_U) <
        0) {
        uvmunmap(pagetable, TRAMPOLINE, 1, 0);
        uvmfree(pagetable, 0);
        return 0;
    }

    // map the trapframe page just below the trampoline page, for
    // trampoline.S.
    if (mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe), PTE_R | PTE_W) < 0) {
        uvmunmap(pagetable, TRAMPOLINE, 1, 0);
        uvmunmap(pagetable, SIG_TRAMPOLINE, 1, 0);
        uvmfree(pagetable, 0);
        return 0;
    }
#elif defined(LOONGARCH)
#ifdef LA2K1000
    if (mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe),
                 PTE_P | PTE_W | PTE_MAT | PTE_D | PTE_U) < 0) {
#else
    if (mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe),
                 PTE_NX | PTE_P | PTE_W | PTE_MAT | PTE_D) < 0) {
#endif
        uvmfree(pagetable, 0);
        return 0;
    }

    if (mappages(pagetable, SIG_TRAMPOLINE, PGSIZE, (uint64)sig_trampoline,
                 PTE_P | PTE_MAT | PTE_D | PTE_U) < 0) {
        printf("Fail to map sig_trampoline\n");
        uvmunmap(pagetable, TRAPFRAME, 1, 0);
        uvmfree(pagetable, 0);
        return 0;
    }

#endif
    return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz) {
#ifdef RISCV
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
#endif
    uvmunmap(pagetable, TRAPFRAME, 1, 0);
    uvmunmap(pagetable, SIG_TRAMPOLINE, 1, 0);

    uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
// uchar initcode[] = {
//   0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
//   0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
//   0x93, 0x08, 0x70, 0x03, 0x73, 0x00, 0x00, 0x00,
//   0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
//   0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
//   0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
//   0x00, 0x00, 0x00, 0x00
// };

extern uchar initcode_start[];
extern uchar initcode_end[];

// Set up first user process.
void userinit(void) {
#ifdef RISCV
    struct proc *p;

    p = allocproc();
    initproc = p;

    // allocate one user page and copy initcode's instructions
    // and data into it.
    uvmfirst(p->pagetable, initcode_start, initcode_end - initcode_start);
    p->sz = 3 * PGSIZE;

    // prepare for the very first "return" from kernel to user.
    p->trapframe->epc = 0;          // user program counter
    p->trapframe->sp = 3 * PGSIZE;  // user stack pointer

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd.fs = get_fs_by_type(EXT4);
    strcpy(p->cwd.path, "/");

    p->state = RUNNABLE;

    release(&p->lock);
#elif defined(LOONGARCH)
    struct proc *p;

    p = allocproc();
    initproc = p;

    // allocate one user page and copy initcode's instructions
    // and data into it.
    uvmfirst(p->pagetable, initcode_start, initcode_end - initcode_start);
    p->sz = 3 * PGSIZE;

    // prepare for the very first "return" from kernel to user.
#ifndef LA2K1000
    p->trapframe->era = 0;          // user program counter
    p->trapframe->sp = 3 * PGSIZE;  // user stack pointer
#else
    // 2K1000：确保 era 指向用户已映射页（从 0 开始），sp 指向用户栈顶
    p->trapframe->era = 0;
    p->trapframe->sp = 3 * PGSIZE;
#endif

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd.fs = get_fs_by_type(EXT4);
    strcpy(p->cwd.path, "/");

    p->state = RUNNABLE;

    release(&p->lock);
#endif
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n) {
    uint64 sz;
    struct proc *p = myproc();

    sz = p->sz;
    if (n > 0) {
#ifdef RISCV
        if ((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0) {
            return -1;
        }
#elif defined(LOONGARCH)
        if ((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_P | PTE_W | PTE_PLV | PTE_MAT | PTE_D)) ==
            0) {
            return -1;
        }
#endif
    } else if (n < 0) {
        sz = uvmdealloc(p->pagetable, sz, sz + n);
    }
    p->sz = sz;
    return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void) {
    int i, pid;
    struct proc *np;
    struct proc *p = myproc();

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // Copy user memory from parent to child.
    if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0) {
        freeproc(np);
        release(&np->lock);
        return -1;
    }
    np->sz = p->sz;

    // copy saved user registers.
    *(np->trapframe) = *(p->trapframe);

    // Cause fork to return 0 in the child.
    np->trapframe->a0 = 0;

    np->clear_child_tid = 0;
    np->robust_list_head = 0;
    np->uid = p->uid;
    np->euid = p->euid;
    np->suid = p->suid;
    np->gid = p->gid;
    np->egid = p->egid;
    np->sgid = p->sgid;

    // increment reference counts on open file descriptors.
    for (i = 0; i < NOFILE; i++)
        if (p->ofile[i]) np->ofile[i] = get_fops()->dup(p->ofile[i]);

    // Copy VMA regions
    for (i = 0; i < NVMA; i++) {
        if (p->vmas[i].valid) {
            np->vmas[i] = p->vmas[i];
            if (p->vmas[i].f != NULL) {
                get_fops()->dup(p->vmas[i].f);
            }
        }
    }

    np->cwd.fs = p->cwd.fs;
    strcpy(np->cwd.path, p->cwd.path);
    strcpy(np->exec_path, p->exec_path);

    strcpy(np->name, p->name);

    pid = np->pid;

    release(&np->lock);

    acquire(&wait_lock);
    np->parent = p;
    release(&wait_lock);

    acquire(&np->lock);
    np->state = RUNNABLE;
    release(&np->lock);

    return pid;
}

int clone(unsigned long flags, void *stack, uint64 ptid, unsigned long tls, uint64 ctid) {
    int i, pid;
    struct proc *np;
    struct proc *p = myproc();

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // Copy user memory from parent to child.
    if (flags & CLONE_VM) {
        proc_freepagetable(np->pagetable, np->sz);
        np->pagetable = p->pagetable;
        np->sz = p->sz;
        np->pt_shared = 1;
    } else {
        if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0) {
            freeproc(np);
            release(&np->lock);
            return -1;
        }
        np->sz = p->sz;
    }

    // Copy saved user registers.
    *(np->trapframe) = *(p->trapframe);

    // Cause clone to return 0 in the child.
    np->trapframe->a0 = 0;

    // Set new stack pointer if specified
    if (stack != 0) {
        np->trapframe->sp = (uint64)stack;
    }

    if (flags & CLONE_SETTLS) np->trapframe->tp = tls;

    // CLONE_THREAD: child shares PID (same thread group), gets unique TID
    if (flags & CLONE_THREAD) {
        np->tgid = p->tgid;
        np->tid = allocpid();  // Unique thread ID
    }

    // CLONE_SIGHAND: share signal handler table (increment refcount)
    if (flags & CLONE_SIGHAND) {
        acquire(&p->sig_handlers->lock);
        p->sig_handlers->refcount++;
        release(&p->sig_handlers->lock);
        np->sig_handlers = p->sig_handlers;  // 共享指针，不复制
    }

    // CLONE_CHILD_CLEARTID: store ctid for exit-time clearing
    if (flags & CLONE_CHILD_CLEARTID) {
        np->clear_child_tid = ctid;
    }

    // CLONE_PARENT_SETTID: write child ID to ptid (use tid for threads, pid for processes)
    if (flags & CLONE_PARENT_SETTID) {
        int child_id = (flags & CLONE_THREAD) ? np->tid : np->pid;
        copyout(p->pagetable, ptid, (char *)&child_id, sizeof(int));
    }

    // Handle file descriptors based on flags
    if (!(flags & CLONE_FILES)) {
        // Default behavior - duplicate file descriptors
        for (i = 0; i < NOFILE; i++) {
            if (p->ofile[i]) {
                np->ofile[i] = get_fops()->dup(p->ofile[i]);
            }
        }
    } else {
        // Share file descriptor table - just copy pointers
        for (i = 0; i < NOFILE; i++) {
            np->ofile[i] = p->ofile[i];
            if (p->ofile[i]) {
                // Increment reference count using dup instead of hold
                get_fops()->dup(p->ofile[i]);
            }
        }
    }

    for (i = 0; i < NVMA; ++i) {
        if (p->vmas[i].valid) {
            if (!(flags & CLONE_VM)) {
                memmove(&np->vmas[i], &p->vmas[i], sizeof(p->vmas[i]));
                if (p->vmas[i].f != NULL) {
                    get_fops()->dup(p->vmas[i].f);
                }
            } else {
                np->vmas[i] = p->vmas[i];
                if (p->vmas[i].f != NULL) {
                    get_fops()->dup(p->vmas[i].f);
                }
            }
        }
    }

    // Handle filesystem info based on flags
    if (!(flags & CLONE_FS)) {
        np->cwd.fs = p->cwd.fs;
        strcpy(np->cwd.path, p->cwd.path);
    } else {
        np->cwd = p->cwd;
    }

    strcpy(np->name, p->name);
    strcpy(np->exec_path, p->exec_path);



    pid = np->pid;

    release(&np->lock);

    acquire(&wait_lock);
    if (flags & CLONE_PARENT) {
        np->parent = p->parent;
    } else {
        np->parent = p;
    }
    release(&wait_lock);

    acquire(&np->lock);
    np->state = RUNNABLE;
    release(&np->lock);

    return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p) {
    struct proc *pp;

    for (pp = proc; pp < &proc[NPROC]; pp++) {
        if (pp->parent == p) {
            pp->parent = initproc;
            wakeup(initproc);
        }
    }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status) {
    struct proc *p = myproc();

    if (p == initproc) panic("init exiting");

    // 区分线程退出与主线程/进程退出
    // CLONE_THREAD 创建的线程：tid != tgid，只清理线程私有资源
    // 主线程（或单线程进程）：tid == tgid，清理全部进程资源
    int is_thread = (p->tid != p->tgid);

    if (!is_thread) {
        // === 主线程/进程退出：清理全部资源 ===

        // Close all open files.
        for (int fd = 0; fd < NOFILE; fd++) {
            if (p->ofile[fd]) {
                struct file *f = p->ofile[fd];
                get_fops()->close(f);
                p->ofile[fd] = 0;
            }
        }

        // 如果页表不是共享的（CLONE_VM），清理 VMA
        if (!p->pt_shared) {
            for (int i = 0; i < NVMA; i++) {
                struct vma *v = &p->vmas[i];
                if (v->valid) {
                    vmaunmap(p->pagetable, v->addr, v->length, v);
                    if (v->f) {
                        get_fops()->close(v->f);
                    }
                    v->valid = 0;
                }
            }
        }

        memset(&(p->cwd), 0, sizeof(p->cwd));
    }
    // 线程退出：不关闭文件/VMA/cwd（与其他线程共享）

    acquire(&wait_lock);

    if (!is_thread) {
        // Give any children to init.
        reparent(p);
    }

    // Parent might be sleeping in wait().
    wakeup(p->parent);

    // Send SIGCHLD to parent (ignored by default, caught by sigtimedwait)
    if (p->parent && p->parent->state != UNUSED) {
        p->parent->sig_pending |= (1ULL << (SIGCHLD - 1));
        // Wake ticks so sigtimedwait sleep loop immediately re-checks zombies
        extern uint ticks;
        wakeup(&ticks);
    }

    acquire(&p->lock);

    // 清除 clear_child_tid 并 futex_wake（通知 pthread_join 等待者）
    if (p->clear_child_tid) {
        int clear = 0;
        copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
        // 唤醒等在 ctid 上的 futex 等待者（pthread_join 用）
        extern uint64 futex_wake(uint64 uaddr, int nr_wake);
        futex_wake(p->clear_child_tid, 1);
        p->clear_child_tid = 0;
    }

    p->xstate = status;
    p->state = ZOMBIE;

    release(&wait_lock);

    // Jump into the scheduler, never to return.
    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr) {
    struct proc *pp;
    int havekids, pid;
    struct proc *p = myproc();

    acquire(&wait_lock);

    for (;;) {
        // Scan through table looking for exited children.
        havekids = 0;
        for (pp = proc; pp < &proc[NPROC]; pp++) {
            if (pp->parent == p) {
                // make sure the child isn't still in exit() or swtch().
                acquire(&pp->lock);

                havekids = 1;
                if (pp->state == ZOMBIE) {
                    // Found one.
                    pid = pp->pid;
                    if (addr != 0 &&
                        copyout(p->pagetable, addr, (char *)&pp->xstate, sizeof(pp->xstate)) < 0) {
                        release(&pp->lock);
                        release(&wait_lock);
                        return -1;
                    }
                    freeproc(pp);
                    release(&pp->lock);
                    release(&wait_lock);
                    return pid;
                }
                release(&pp->lock);
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || killed(p)) {
            release(&wait_lock);
            return -ECHILD;
        }

        // Wait for a child to exit.
        sleep(p, &wait_lock);  // DOC: wait-sleep
    }
}

// Wait for a specific child process to exit, or any child if pid == -1
// Return the pid of the exited child, or -1 on error
int waitpid(int target_pid, uint64 wstatus_addr, int options) {
    struct proc *pp;
    int havekids, found_pid;
    struct proc *p = myproc();

    // Handle special cases for pid values
    if (target_pid < -1) {
        // In Linux, pid < -1 means wait for any child in process group |pid|
        // For simplicity, we'll just treat it as wait for any child
        target_pid = -1;
    }

    acquire(&wait_lock);

    for (;;) {
        // Scan through table looking for matching children
        havekids = 0;
        found_pid = -1;

        for (pp = proc; pp < &proc[NPROC]; pp++) {
            if (pp->parent == p) {
                havekids = 1;

                // Check if this child matches our criteria
                int matches = 0;
                if (target_pid == -1) {
                    // Wait for any child
                    matches = 1;
                } else if (target_pid > 0) {
                    // Wait for specific child PID
                    matches = (pp->pid == target_pid);
                }

                if (matches) {
                    acquire(&pp->lock);

                    if (pp->state == ZOMBIE) {
                        // Found a matching zombie child
                        found_pid = pp->pid;

                        // Copy exit status to user space if requested
                        if (wstatus_addr != 0) {
                            if (copyout(p->pagetable, wstatus_addr, (char *)&pp->xstate,
                                        sizeof(pp->xstate)) < 0) {
                                release(&pp->lock);
                                release(&wait_lock);
                                return -1;
                            }
                        }

                        // Clean up the child process
                        freeproc(pp);
                        release(&pp->lock);
                        release(&wait_lock);

                        return found_pid;
                    }

                    release(&pp->lock);
                }
            }
        }

        // No point waiting if we don't have any children
        if (!havekids) {
            release(&wait_lock);
            return -ECHILD;
        }

        // If waiting for a specific PID, verify it exists as our child
        if (target_pid > 0) {
            int child_exists = 0;
            for (pp = proc; pp < &proc[NPROC]; pp++) {
                if (pp->parent == p && pp->pid == target_pid && pp->state != ZOMBIE) {
                    child_exists = 1;
                    break;
                }
            }
            if (!child_exists) {
                release(&wait_lock);
                return -ECHILD;  // No such living child
            }
        }

        // Check if we've been killed while waiting
        if (killed(p)) {
            release(&wait_lock);
            return -1;
        }

        // Wait for a child to exit
        sleep(p, &wait_lock);
    }
}

// Wait for a specific child process to exit (wait4 implementation)
// Based on the provided reference implementation
int wait4(int pid, uint64 wstatus_addr, int options) {
    struct proc *pp;
    int havekids, found_pid;
    struct proc *p = myproc();

    acquire(&wait_lock);

    for (;;) {
        havekids = 0;
        found_pid = 0;

        for (pp = proc; pp < &proc[NPROC]; pp++) {
            if (pp->parent == p) {
                acquire(&pp->lock);
                havekids = 1;

                if (pid != -1 && pp->pid != pid) {
                    release(&pp->lock);
                    continue;
                }
                if ((options & WNOHANG) && pp->state != ZOMBIE) {
                    found_pid = 0;
                    release(&pp->lock);
                    continue;
                }

                if (pp->state == ZOMBIE) {
                    found_pid = pp->pid;
                    // 不修改 pp->xstate，用局部变量打包退出状态
                    // Linux wait4 约定：内核返回原始 xstate，用户空间 WEXITSTATUS 提取
                    int status_word = (pp->xstate & 0xff) << 8;
                    if (wstatus_addr != 0 && copyout(p->pagetable, wstatus_addr,
                                                     (char *)&status_word, sizeof(status_word)) < 0) {
                        release(&pp->lock);
                        release(&wait_lock);
                        return -1;
                    }
                    freeproc(pp);
                    release(&pp->lock);
                    release(&wait_lock);
                    return found_pid;
                }
                release(&pp->lock);
            }
        }

        if (!havekids) {
            release(&wait_lock);
            return -ECHILD;
        }

        // If WNOHANG is set and no children are ready, return 0
        if (options & WNOHANG) {
            release(&wait_lock);
            return 0;
        }

        if (killed(p)) {
            release(&wait_lock);
            return -1;
        }

        sleep(p, &wait_lock);
    }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void) {
    struct proc *p;
    struct cpu *c = mycpu();
    extern void early_uart_puts(const char *);

    c->proc = 0;
    for (;;) {
        // Avoid deadlock by ensuring that devices can interrupt.
        intr_on();

        for (p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if (p->state == RUNNABLE) {
                // Switch to chosen process.  It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                p->state = RUNNING;
                c->proc = p;
#ifdef LA2K1000
                // Quiet 2K1000: avoid stray UART chars during context switch
#endif
                swtch(&c->context, &p->context);

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
            }
            release(&p->lock);
        }
    }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
    int intena;
    struct proc *p = myproc();
    if (!holding(&p->lock)) panic("sched p->lock");
    if (mycpu()->noff != 1) panic("sched locks");
    if (p->state == RUNNING) panic("sched running");
    if (intr_get()) panic("sched interruptible");

    intena = mycpu()->intena;
    swtch(&p->context, &mycpu()->context);
    mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
    struct proc *p = myproc();
    acquire(&p->lock);
    p->state = RUNNABLE;
    sched();
    release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void) {
    static int first = 1;
    extern void early_uart_puts(const char *);

#ifdef LA2K1000
    // Quiet 2K1000: avoid stray UART chars at fork return
#endif

    // Still holding p->lock from scheduler.
    release(&myproc()->lock);

#ifndef LA2K1000
    if (first) {
        // File system initialization must be run in the context of a
        // regular process (e.g., because it calls sleep), and thus cannot
        // be run from main().
        first = 0;
        filesystem_init();
    }
#else
    // LA2K1000: 同样在首次进入进程上下文时完成文件系统挂载与 cwd 初始化
    if (first) {
        first = 0;
        filesystem_init();
    }
#endif

#ifdef LA2K1000
    // Quiet 2K1000: avoid stray UART chars before usertrapret
#endif
    usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk) {
    struct proc *p = myproc();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.
    acquire(&p->lock);  // DOC: sleeplock1
    release(lk);

    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;

    sched();

    // Tidy up.
    p->chan = 0;

    // Reacquire original lock.
    release(&p->lock);
    acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void *chan) {
    struct proc *p;

    for (p = proc; p < &proc[NPROC]; p++) {
        if (p != myproc()) {
            acquire(&p->lock);
            if (p->state == SLEEPING && p->chan == chan) {
                p->state = RUNNABLE;
            }
            release(&p->lock);
        }
    }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid) {
    struct proc *p;

    for (p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if (p->pid == pid) {
            p->killed = 1;
            if (p->state == SLEEPING) {
                // Wake process from sleep().
                p->state = RUNNABLE;
            }
            release(&p->lock);
            return 0;
        }
        release(&p->lock);
    }
    return -1;
}

void setkilled(struct proc *p) {
    acquire(&p->lock);
    p->killed = 1;
    release(&p->lock);
}

int killed(struct proc *p) {
    int k;

    acquire(&p->lock);
    k = p->killed;
    release(&p->lock);
    return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len) {
    struct proc *p = myproc();
    if (user_dst) {
        return copyout(p->pagetable, dst, src, len);
    } else {
        memmove((char *)dst, src, len);
        return 0;
    }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len) {
    struct proc *p = myproc();
    if (user_src) {
        return copyin(p->pagetable, dst, src, len);
    } else {
        memmove(dst, (char *)src, len);
        return 0;
    }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void) {
    static char *states[] = {[UNUSED] "unused",   [USED] "used",      [SLEEPING] "sleep ",
                             [RUNNABLE] "runble", [RUNNING] "run   ", [ZOMBIE] "zombie"};
    struct proc *p;
    char *state;

    printf("\n");
    for (p = proc; p < &proc[NPROC]; p++) {
        if (p->state == UNUSED) continue;
        if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";
        printf("%d %s %s", p->pid, state, p->name);
        printf("\n");
    }
}

uint64 countprocs(void) {
    struct proc *p;
    uint64 count = 0;

    for (p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if (p->state != UNUSED) {
            count++;
        }
        release(&p->lock);
    }

    return count;
}