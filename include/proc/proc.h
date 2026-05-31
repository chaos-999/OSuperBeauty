#pragma once

#include "fs/vfs/file.h"
#include "time.h"

#define S_ISUID 04000  // set-user-ID bit
#define S_ISGID 02000  // set-group-ID bit

// Saved registers for kernel context switches.
#ifdef RISCV
struct context {
    uint64 ra;
    uint64 sp;

    // callee-saved
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};
#elif defined(LOONGARCH)
struct context {
    uint64 ra;
    uint64 sp;

    // callee-saved
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 fp;
};
#endif

// Per-CPU state.
struct cpu {
    struct proc *proc;       // The process running on this cpu, or null.
    struct context context;  // swtch() here to enter scheduler().
    int noff;                // Depth of push_off() nesting.
    int intena;              // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
#ifdef RISCV
struct trapframe {
    /*   0 */ uint64 kernel_satp;    // kernel page table
    /*   8 */ uint64 kernel_sp;      // top of process's kernel stack
    /*  16 */ uint64 kernel_trap;    // usertrap()
    /*  24 */ uint64 epc;            // saved user program counter
    /*  32 */ uint64 kernel_hartid;  // saved kernel tp
    /*  40 */ uint64 ra;
    /*  48 */ uint64 sp;
    /*  56 */ uint64 gp;
    /*  64 */ uint64 tp;
    /*  72 */ uint64 t0;
    /*  80 */ uint64 t1;
    /*  88 */ uint64 t2;
    /*  96 */ uint64 s0;
    /* 104 */ uint64 s1;
    /* 112 */ uint64 a0;
    /* 120 */ uint64 a1;
    /* 128 */ uint64 a2;
    /* 136 */ uint64 a3;
    /* 144 */ uint64 a4;
    /* 152 */ uint64 a5;
    /* 160 */ uint64 a6;
    /* 168 */ uint64 a7;
    /* 176 */ uint64 s2;
    /* 184 */ uint64 s3;
    /* 192 */ uint64 s4;
    /* 200 */ uint64 s5;
    /* 208 */ uint64 s6;
    /* 216 */ uint64 s7;
    /* 224 */ uint64 s8;
    /* 232 */ uint64 s9;
    /* 240 */ uint64 s10;
    /* 248 */ uint64 s11;
    /* 256 */ uint64 t3;
    /* 264 */ uint64 t4;
    /* 272 */ uint64 t5;
    /* 280 */ uint64 t6;
};
#elif defined(LOONGARCH)
struct trapframe {
    /*   0 */ uint64 ra;
    /*   8 */ uint64 tp;
    /*  16 */ uint64 sp;
    /*  24 */ uint64 a0;
    /*  32 */ uint64 a1;
    /*  40 */ uint64 a2;
    /*  48 */ uint64 a3;
    /*  56 */ uint64 a4;
    /*  64 */ uint64 a5;
    /*  72 */ uint64 a6;
    /*  80 */ uint64 a7;
    /*  88 */ uint64 t0;
    /*  96 */ uint64 t1;
    /* 104 */ uint64 t2;
    /* 112 */ uint64 t3;
    /* 120 */ uint64 t4;
    /* 128 */ uint64 t5;
    /* 136 */ uint64 t6;
    /* 144 */ uint64 t7;
    /* 152 */ uint64 t8;
    /* 160 */ uint64 r21;
    /* 168 */ uint64 fp;
    /* 176 */ uint64 s0;
    /* 184 */ uint64 s1;
    /* 192 */ uint64 s2;
    /* 200 */ uint64 s3;
    /* 208 */ uint64 s4;
    /* 216 */ uint64 s5;
    /* 224 */ uint64 s6;
    /* 232 */ uint64 s7;
    /* 240 */ uint64 s8;
    /* 248 */ uint64 kernel_sp;      // top of process's kernel stack
    /* 256 */ uint64 kernel_trap;    // usertrap()
    /* 264 */ uint64 era;            // saved user program counter
    /* 272 */ uint64 kernel_hartid;  // saved kernel tp
    /* 280 */ uint64 kernel_pgdl;    // saved kernel pagetable
};
#endif

struct sysinfo {
    uint64 uptime;   // System uptime
    uint64 freeram;  // Free memory
    uint64 procs;    // Number of processes
};

#include "signal.h"

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Virtual Memory Area for mmap
struct vma {
    uint64 addr;     // start address
    uint64 length;   // length in bytes
    int prot;        // protection flags
    int flags;       // mmap flags
    struct file *f;  // mapped file (NULL for anonymous)
    uint64 offset;   // offset in file
    int valid;       // 1 if this VMA is valid, 0 if free
};

// Per-process state
struct proc {
    struct spinlock lock;

    // p->lock must be held when using these:
    enum procstate state;  // Process state
    void *chan;            // If non-zero, sleeping on chan
    int killed;            // If non-zero, have been killed
    int xstate;            // Exit status to be returned to parent's wait
    int pid;               // Process ID

    // wait_lock must be held when using this:
    struct proc *parent;  // Parent process

    // these are private to the process, so p->lock need not be held.
    uint64 kstack;                // Virtual address of kernel stack
    uint64 sz;                    // Size of process memory (bytes)
    pagetable_t pagetable;        // User page table
    struct trapframe *trapframe;  // data page for trampoline.S
    struct context context;       // swtch() here to run process
    struct file *ofile[NOFILE];   // Open files
    struct file_vnode cwd;        // Current directory
    char name[16];                // Process name (debugging)
    struct vma vmas[NVMA];        // Virtual Memory Areas for mmap
    uint64 clear_child_tid;       // Clear child tid
    uint64 robust_list_head;      // Robust futex list head
    int uid;                      // Real user ID
    int euid;                     // Effective user ID
    int suid;                     // Saved set-user-ID
    int gid;                      // Real group ID
    int egid;                     // Effective group ID
    int sgid;                     // Saved set-group-ID

    // Signal related
    sigset_t sig_blocked;               // Signal mask
    uint64 sig_pending;                 // Pending signal bitmap
    struct sig_handlers *sig_handlers;  // Signal handler table
    struct sig_context *sig_context;    // Signal handling context stack
};
