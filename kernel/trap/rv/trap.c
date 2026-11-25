#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include <lock/lock.h>
#include <proc/proc.h>
#include "defs.h"
#include <lib/sbi.h>
#include <mem.h>

struct spinlock interrupt;
struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void trapinit(void) {
    initlock(&tickslock, "time");
    initlock(&interrupt, "interrupt");
    // 设置初始的时钟中断
    SBI_SET_TIMER(r_time() + 1000000);
}

struct vma *findvma(struct proc *p, uint64 va) {
    for (int i = 0; i < NVMA; i++) {
        struct vma *vv = &p->vmas[i];
        // if (vv->valid == 1) {
        //     printf("findvma: vma[%d] = %lx\n", i, vv->addr);
        // }
        if (vv->valid == 1 && va >= vv->addr && va < vv->addr + vv->length) {
            // printf("findvma: vma[%d] = %lx\n", i, vv->addr);
            return vv;
        }
    }
    return 0;
}

int vmatrylazytouch(uint64 va) {
    struct proc *p = myproc();
    struct vma *v = findvma(p, va);
    // printf("here1\n");
    if (v == 0) return 0;
    // printf("here2\n");

    void *pa = kalloc();
    if (pa == NULL) panic("vmalazytouch : kalloc");
    memset(pa, 0, PGSIZE);

    uint64 va_aligned = PGROUNDDOWN(va);

    // For file-backed mappings, read from file
    if (v->f != NULL) {
        int file_off = v->offset + PGROUNDDOWN(va - v->addr);
        int read_bytes = get_fops()->readat(v->f, (uint64)pa, PGSIZE, file_off);
        if (strcmp(p->name, "git") == 0) {
            // printf("DEBUG: vmatrylazytouch pid=%d va=%lx va_aligned=%lx file_off=%x read=%d\n",
            // p->pid, va, va_aligned, file_off, read_bytes);
        }
        if (read_bytes < 0) {
            kfree(pa);
            return 0;
        }
        if (read_bytes < PGSIZE) {
            // zero-fill tail explicitly to be safe (memset already zeroed, but keep note)
        }
    }
    // For anonymous mappings, memory is already zeroed

    int perm = PTE_U;
    if (v->prot & PROT_READ) perm |= PTE_R;
    if (v->prot & PROT_WRITE) perm |= PTE_W;
    if (v->prot & PROT_EXEC) perm |= PTE_X;

    if (mappages(p->pagetable, va_aligned, PGSIZE, (uint64)pa, perm) != 0) {
        kfree(pa);
        return 0;
    }

    if (strcmp(p->name, "git") == 0) {
        // printf("DEBUG: vmatrylazytouch MAP pid=%d va=%lx -> pa=%lx perm=0x%x\n", p->pid,
        // va_aligned, (uint64)pa, perm);
    }

    return 1;
}

// set up to take exceptions and traps while in the kernel.
void trapinithart(void) { w_stvec((uint64)kernelvec); }

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void) {
    int which_dev = 0;

    if ((r_sstatus() & SSTATUS_SPP) != 0) panic("usertrap: not from user mode");

    // send interrupts and exceptions to kerneltrap(),
    // since we're now in the kernel.
    w_stvec((uint64)kernelvec);

    struct proc *p = myproc();

    // save user program counter.
    p->trapframe->epc = r_sepc();

    if (r_scause() == 8) {
        // system call
        if (killed(p)) {
            exit(-1);
        }

        // sepc points to the ecall instruction,
        // but we want to return to the next instruction.
        p->trapframe->epc += 4;

        // an interrupt will change sepc, scause, and sstatus,
        // so enable only now that we're done with those registers.
        intr_on();

        syscall();
    } else if (r_scause() == 2) {
        setkilled(p);
    } else if (r_scause() == 13 || r_scause() == 15) {
        // Page fault: Load page fault (13) or Store/AMO page fault (15)
        uint64 va = r_stval();
        if (!vmatrylazytouch(va)) {
            printf("vma lazy touch failed\n");
            goto unexpected_scause;
        }
    } else if ((which_dev = devintr()) != 0) {
        // ok
    } else {
    unexpected_scause:
        printf("usertrap(): unexpected scause 0x%lx pid=%d\n", r_scause(), p->pid);
        printf("            sepc=0x%lx stval=0x%lx\n", r_sepc(), r_stval());
        setkilled(p);
    }

    if (killed(p)) {
        exit(-1);
    }

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2) yield();

    sig_deliver(p);

    usertrapret();
}

//
// return to user space
//
void usertrapret(void) {
    struct proc *p = myproc();

    // we're about to switch the destination of traps from
    // kerneltrap() to usertrap(), so turn off interrupts until
    // we're back in user space, where usertrap() is correct.
    intr_off();

    // send syscalls, interrupts, and exceptions to uservec in trampoline.S
    uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
    w_stvec(trampoline_uservec);

    // set up trapframe values that uservec will need when
    // the process next traps into the kernel.
    p->trapframe->kernel_satp = r_satp();          // kernel page table
    p->trapframe->kernel_sp = p->kstack + PGSIZE;  // process's kernel stack
    p->trapframe->kernel_trap = (uint64)usertrap;
    p->trapframe->kernel_hartid = r_tp();  // hartid for cpuid()

    // set up the registers that trampoline.S's sret will use
    // to get to user space.

    // set S Previous Privilege mode to User.
    unsigned long x = r_sstatus();
    x &= ~SSTATUS_SPP;  // clear SPP to 0 for user mode
    x |= SSTATUS_SPIE;  // enable interrupts in user mode
    w_sstatus(x);

    // set S Exception Program Counter to the saved user pc.
    w_sepc(p->trapframe->epc);

    // tell trampoline.S the user page table to switch to.
    uint64 satp = MAKE_SATP(p->pagetable);

    // jump to userret in trampoline.S at the top of memory, which
    // switches to the user page table, restores user registers,
    // and switches to user mode with sret.
    uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
    ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap() {
    int which_dev = 0;
    uint64 sepc = r_sepc();
    uint64 sstatus = r_sstatus();
    uint64 scause = r_scause();

    if ((sstatus & SSTATUS_SPP) == 0) panic("kerneltrap: not from supervisor mode");
    if (intr_get() != 0) panic("kerneltrap: interrupts enabled");

    if ((which_dev = devintr()) == 0) {
        // interrupt or trap from an unknown source
        printf("scause=0x%lx sepc=0x%lx stval=0x%lx\n", scause, r_sepc(), r_stval());
        panic("kerneltrap");
    }

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING) yield();

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}

void clockintr() {
    acquire(&tickslock);
    // printf("clockintr\n");
    ticks++;
    wakeup(&ticks);
    release(&tickslock);

    // 使用SBI调用设置下一次时钟中断
    // 1000000是约0.1秒
    SBI_SET_TIMER(r_time() + 100000);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.

volatile uint64 clock_counter = 0;
volatile uint64 virtio_counter = 0;
volatile uint64 uart_counter = 0;

inline uint64 get_clock() {
    // printf("clock_counter = %lu\n", clock_counter);
    return clock_counter;
}

inline uint64 get_virtio() { return virtio_counter; }

inline uint64 get_uart() { return uart_counter; }

void counter(int irq) {
    acquire(&interrupt);
    switch (irq) {
        case CLOCK_IRQ:  // 5
            clock_counter++;
            break;
        case VIRTIO0_IRQ:  // 1
            virtio_counter++;
            break;
        case UART0_IRQ:  // 10
            uart_counter++;
            break;
    }
    release(&interrupt);
    return;
}

int devintr() {
    uint64 scause = r_scause();

    if (scause == 0x8000000000000009L) {
        // this is a supervisor external interrupt, via PLIC.

        // irq indicates which device interrupted.
        int irq = plic_claim();

        if (irq == UART0_IRQ) {
            counter(irq);
            // Optional: minimal debug once
            static int dbg_uart_irq_once = 0;
            if (!dbg_uart_irq_once) {
                printf("[IRQ] UART0_IRQ received\n");
                dbg_uart_irq_once = 1;
            }
            uartintr();
        } else if (irq == VIRTIO0_IRQ) {
            counter(irq);
            virtio_disk_intr();
        } else if (irq) {
            printf("unexpected interrupt irq=%d\n", irq);
        }

        // the PLIC allows each device to raise at most one
        // interrupt at a time; tell the PLIC the device is
        // now allowed to interrupt again.
        if (irq) plic_complete(irq);

#ifdef VF2
        // Extra debug: on no-irq case, peek pending bits to help locate source
        if (!irq) {
            uint32 pend_lo = *(uint32 *)(PLIC + 0x1000);
            uint32 pend_hi = *(uint32 *)(PLIC + 0x1004);
            printf("[PLIC] pending lo=0x%x hi=0x%x\n", pend_lo, pend_hi);
        }
#endif

        return 1;
    } else if (scause == 0x8000000000000005L) {
        // timer interrupt.
        counter(CLOCK_IRQ);
        clockintr();
        return 2;
#ifdef VF2
    } else if (scause == 0x8000000000000001L) {
        // VF2: software interrupt (SSIP) — 在部分平台上用于投递定时事件
        // 将其等同处理为“时钟中断”，以驱动 ticks/sleep
        // 清除 SSIP pending 位
        w_sip(r_sip() & ~2UL);
        counter(CLOCK_IRQ);
        clockintr();
        return 2;
#endif
    } else {
        return 0;
    }
}
