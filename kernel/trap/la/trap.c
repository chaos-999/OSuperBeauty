#include <dev/pci/pci.h>

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "lock/lock.h"
#include "proc/proc.h"
#include "defs.h"
#include "apic.h"
#include "extioi.h"
#include "dev/virtio.h"
#include "mem.h"

struct spinlock interrupt;
struct spinlock tickslock;
uint ticks;

// in kernelvec.S, calls kerneltrap().
void kernelvec();
void uservec();
void handle_tlbr();
void handle_merr();
void userret(uint64, uint64);

extern int devintr();

void trapinit(void) {
    initlock(&tickslock, "time");
    uint32 ecfg = (0U << CSR_ECFG_VS_SHIFT) | HWI_VEC | TI_VEC;
    uint64 tcfg = 0x1000000UL | CSR_TCFG_EN | CSR_TCFG_PER;
    w_csr_ecfg(ecfg);
    w_csr_tcfg(tcfg);
    w_csr_eentry((uint64)kernelvec);
    w_csr_tlbrentry((uint64)handle_tlbr);
    w_csr_merrentry((uint64)handle_merr);
    intr_on();

#ifdef LA2K1000
    // 2K1000: 不修改 CRMD 寄存器的分页/齐位相关位，避免误置导致的 PPI/PG 切换问题
#endif
}

struct vma *findvma(struct proc *p, uint64 va) {
    for (int i = 0; i < NVMA; i++) {
        struct vma *vv = &p->vmas[i];
        if (vv->valid == 1 && va >= vv->addr && va < vv->addr + vv->length) {
            return vv;
        }
    }
    return 0;
}

int vmatrylazytouch(uint64 va) {
    struct proc *p = myproc();
    struct vma *v = findvma(p, va);
    // printf("before judge vma\n");
    if (v == 0) return 0;
    // printf("after judge vma\n");

    void *pa = kalloc();
    if (pa == NULL) panic("vmalazytouch : kalloc");
    memset(pa, 0, PGSIZE);

    uint64 va_aligned = PGROUNDDOWN(va);

    // For file-backed mappings, read one page on demand
    if (v->f != NULL) {
        int read_bytes =
            get_fops()->readat(v->f, (uint64)pa, PGSIZE, v->offset + PGROUNDDOWN(va - v->addr));
        if (read_bytes < 0) {
            kfree(pa);
            return 0;
        }
    }
    // For anonymous mappings, memory is already zeroed

    int perm;
#if defined(LOONGARCH) && defined(LA2K1000)
    // 2K1000：使用 LoongArch 用户位语义（PTE_PLV 而不是 PTE_U）
    perm = PTE_P | PTE_PLV | PTE_MAT | PTE_D;
    if (!(v->prot & PROT_READ)) perm |= PTE_NR;
    if (v->prot & PROT_WRITE) perm |= PTE_W;
    if (!(v->prot & PROT_EXEC)) perm |= PTE_NX;
#else
    perm = PTE_U | PTE_MAT | PTE_P | PTE_D;
    if (!(v->prot & PROT_READ)) perm |= PTE_NR;
    if (v->prot & PROT_WRITE) perm |= PTE_W;
    if (!(v->prot & PROT_EXEC)) perm |= PTE_NX;
#endif

    if (mappages(p->pagetable, va_aligned, PGSIZE, (uint64)pa, perm) != 0) {
        kfree(pa);
        return 0;
    }

    return 1;
}

//
// handle an interrupt, exception, or system call from user space.
// called from uservec.S
//
void usertrap(void) {
    int which_dev = 0;

    if ((r_csr_prmd() & PRMD_PPLV) == 0) panic("usertrap: not from user mode");

    // send interrupts and exceptions to kerneltrap(),
    // since we're now in the kernel.
    w_csr_eentry((uint64)kernelvec);

    struct proc *p = myproc();

    // save user program counter.
    p->trapframe->era = r_csr_era();

#ifdef LA2K1000
    uint32 estat_val = r_csr_estat();
    uint32 ecode_val = (estat_val & CSR_ESTAT_ECODE) >>
                       16;  // mmexport: 0x1 PIL, 0x2 PIS, 0x3 PIF, 0x8 ADEF/ADEM, 0xB SYS
    uint32 esubcode_val = (estat_val >> 22) & 0x1F;  // 当 ecode==0x8 时，0=ADEF, 1=ADEM

    if (ecode_val == 0xb) {
#else
    if (((r_csr_estat() & CSR_ESTAT_ECODE) >> 16) == 0xb) {
#endif
        // system call

        if (p->killed) exit(-1);

        // sepc points to the ecall instruction,
        // but we want to return to the next instruction.
        p->trapframe->era += 4;

        // an interrupt will change crmd & prmd registers,
        // so don't enable until done with those registers.
        intr_on();

        syscall();

    }
#ifdef LA2K1000
    else if (ecode_val == 0x1 || ecode_val == 0x2 || ecode_val == 0x3 ||
             (ecode_val == 0x8 && esubcode_val == 1)) {
        // PIL/PIS/PIF 或 ADEM：按缺页懒分配处理
        uint64 va = r_csr_badv();
        if (!vmatrylazytouch(va)) {
            printf("vma lazy touch failed (ecode=%x esub=%x)\n", ecode_val, esubcode_val);
            goto unexpected_scause;
        }
    } else if (ecode_val == 0x9) {
        // ALE：地址非对齐。最小软件模拟常见的 load/store，避免进程被误杀
        uint32 bad_instr = 0;
        uint64 era_u = r_csr_era();
        // 优先用 copyin 获取指令；失败则直接通过 walkaddr 获取内核可访问地址读取
        if (copyin(p->pagetable, (char *)&bad_instr, era_u, sizeof(uint32)) < 0) {
            uint64 pa_instr = walkaddr(p->pagetable, era_u);
            if (pa_instr) {
                bad_instr = *(volatile uint32 *)(pa_instr);
            } else {
                // 无法取指，退化处理：跳过本条指令
                p->trapframe->era += 4;
                goto ale_done;
            }
        }

        uint32 main_opcode = (bad_instr >> 26) & 0x3F;
        uint32 sub_opcode = 0;
        uint64 badv = r_csr_badv();

        // 寄存器索引
        uint32 rd_idx = (bad_instr >> 0) & 0x1F;   // load 目的寄存器 / store 形式仍占位
        uint32 rj_idx = (bad_instr >> 5) & 0x1F;   // 基址寄存器（本次未用到）
        uint32 rk_idx = (bad_instr >> 10) & 0x1F;  // store 源寄存器

        // 取 trapframe 中的寄存器指针表
        uint64 *trapregs[32] = {0};
        trapregs[1] = &p->trapframe->ra;
        trapregs[2] = &p->trapframe->tp;
        trapregs[3] = &p->trapframe->sp;
        trapregs[4] = &p->trapframe->a0;
        trapregs[5] = &p->trapframe->a1;
        trapregs[6] = &p->trapframe->a2;
        trapregs[7] = &p->trapframe->a3;
        trapregs[8] = &p->trapframe->a4;
        trapregs[9] = &p->trapframe->a5;
        trapregs[10] = &p->trapframe->a6;
        trapregs[11] = &p->trapframe->a7;
        trapregs[12] = &p->trapframe->t0;
        trapregs[13] = &p->trapframe->t1;
        trapregs[14] = &p->trapframe->t2;
        trapregs[15] = &p->trapframe->t3;
        trapregs[16] = &p->trapframe->t4;
        trapregs[17] = &p->trapframe->t5;
        trapregs[18] = &p->trapframe->t6;
        trapregs[19] = &p->trapframe->t7;
        trapregs[20] = &p->trapframe->t8;
        trapregs[21] = &p->trapframe->r21;
        trapregs[22] = &p->trapframe->fp;
        trapregs[23] = &p->trapframe->s0;
        trapregs[24] = &p->trapframe->s1;
        trapregs[25] = &p->trapframe->s2;
        trapregs[26] = &p->trapframe->s3;
        trapregs[27] = &p->trapframe->s4;
        trapregs[28] = &p->trapframe->s5;
        trapregs[29] = &p->trapframe->s6;
        trapregs[30] = &p->trapframe->s7;
        trapregs[31] = &p->trapframe->s8;

        int handled = 0;
        // 0b001001：按 2k1000la 的注释，支持 4/8 字节 load/store（LL/SC 空间除外）
        if (main_opcode == 0b001001) {
            sub_opcode = (bad_instr >> 24) & 0x3;
            int is_store = (sub_opcode & 0b01) ? 1 : 0;
            int byte_cnt = (sub_opcode & 0b10) ? 8 : 4;
            if ((!is_store && rd_idx < 32 && trapregs[rd_idx]) ||
                (is_store && rk_idx < 32 && trapregs[rk_idx])) {
                if (is_store) {
                    uint64 val = *trapregs[rk_idx];
                    if (copyout(p->pagetable, badv, (char *)&val, byte_cnt) == 0) handled = 1;
                } else {
                    uint64 val = 0;
                    if (copyin(p->pagetable, (char *)&val, badv, byte_cnt) == 0) {
                        *trapregs[rd_idx] = val;
                        handled = 1;
                    }
                }
            }
        }
        // 0b001010：基本 load/store，支持 1/2/4/8，含有符号扩展位
        else if (main_opcode == 0b001010) {
            sub_opcode = (bad_instr >> 22) & 0xF;
            int byte_sel = (sub_opcode & 0x3);
            int byte_cnt = (byte_sel == 0) ? 1 : (byte_sel == 1 ? 2 : (byte_sel == 2 ? 4 : 8));
            int is_store = (sub_opcode & (1 << 2)) ? 1 : 0;
            int is_unsigned = (sub_opcode & (1 << 3)) ? 1 : 0;
            if ((!is_store && rd_idx < 32 && trapregs[rd_idx]) ||
                (is_store && rk_idx < 32 && trapregs[rk_idx])) {
                if (is_store) {
                    uint64 val = *trapregs[rk_idx];
                    if (copyout(p->pagetable, badv, (char *)&val, byte_cnt) == 0) handled = 1;
                } else {
                    uint64 val = 0;
                    if (copyin(p->pagetable, (char *)&val, badv, byte_cnt) == 0) {
                        if (!is_unsigned) {
                            if (byte_cnt == 1 && (val & (1ULL << 7))) val |= 0xFFFFFFFFFFFFFF00ULL;
                            if (byte_cnt == 2 && (val & (1ULL << 15))) val |= 0xFFFFFFFFFFFF0000ULL;
                            if (byte_cnt == 4 && (val & (1ULL << 31))) val |= 0xFFFFFFFF00000000ULL;
                        }
                        *trapregs[rd_idx] = val;
                        handled = 1;
                    }
                }
            }
        }

        if (handled) {
            p->trapframe->era += 4;  // 前进到下一条指令
        } else {
            // 未覆盖的编码：降级策略，跳过该指令，避免误杀进程（记录一次日志）
            printf("ALE emulate failed: instr=%08x\n", bad_instr);
            p->trapframe->era += 4;
        }
    ale_done:;
    }
#else
    else if ((((r_csr_estat() & CSR_ESTAT_ECODE) >> 16) == 0x1 ||
              ((r_csr_estat() & CSR_ESTAT_ECODE) >> 16) == 0x2)) {
        // Page fault: Load/Store page invalid -> lazy alloc
        uint64 va = r_csr_badv();
        if (!vmatrylazytouch(va)) {
            printf("vma lazy touch failed\n");
            goto unexpected_scause;
        }
    }
#endif
    else if ((which_dev = devintr()) != 0) {
        // ok
    } else {
    unexpected_scause:
#ifdef LA2K1000
        printf("usertrap(): unexpected trapcause %x pid=%d\n", estat_val, p->pid);
        printf("            ecode=%x esub=%x era=%lx badv=%lx badi=%x\n", ecode_val, esubcode_val,
               r_csr_era(), r_csr_badv(), r_csr_badi());
#else
        printf("usertrap(): unexpected trapcause %x pid=%d\n", r_csr_estat(), p->pid);
        printf("            era=%ld badi=%x\n", r_csr_era(), r_csr_badi());
#endif
        p->killed = 1;
    }

    if (p->killed) exit(-1);

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2) yield();

    sig_deliver(p);

    usertrapret();
}

void trapframedump(void) {
    struct proc *p = myproc();
    printf("trapframe dump: ra=%ld\n", p->trapframe->ra);
    printf("trapframe dump: tp=%ld\n", p->trapframe->tp);
    printf("trapframe dump: sp=%ld\n", p->trapframe->sp);
    printf("trapframe dump: a0=%ld\n", p->trapframe->a0);
    printf("trapframe dump: a1=%ld\n", p->trapframe->a1);
    printf("trapframe dump: a2=%ld\n", p->trapframe->a2);
    printf("trapframe dump: a3=%ld\n", p->trapframe->a3);
    printf("trapframe dump: a4=%ld\n", p->trapframe->a4);
    printf("trapframe dump: a5=%ld\n", p->trapframe->a5);
    printf("trapframe dump: a6=%ld\n", p->trapframe->a6);
    printf("trapframe dump: a7=%ld\n", p->trapframe->a7);
    printf("trapframe dump: t0=%ld\n", p->trapframe->t0);
    printf("trapframe dump: t1=%ld\n", p->trapframe->t1);
    printf("trapframe dump: t2=%ld\n", p->trapframe->t2);
    printf("trapframe dump: t3=%ld\n", p->trapframe->t3);
    printf("trapframe dump: t4=%ld\n", p->trapframe->t4);
    printf("trapframe dump: t5=%ld\n", p->trapframe->t5);
    printf("trapframe dump: t6=%ld\n", p->trapframe->t6);
    printf("trapframe dump: t7=%ld\n", p->trapframe->t7);
    printf("trapframe dump: t8=%ld\n", p->trapframe->t8);
    printf("trapframe dump: r21=%ld\n", p->trapframe->r21);
    printf("trapframe dump: fp=%ld\n", p->trapframe->fp);
    printf("trapframe dump: s0=%ld\n", p->trapframe->s0);
    printf("trapframe dump: s1=%ld\n", p->trapframe->s1);
    printf("trapframe dump: s2=%ld\n", p->trapframe->s2);
    printf("trapframe dump: s3=%ld\n", p->trapframe->s3);
    printf("trapframe dump: s4=%ld\n", p->trapframe->s4);
    printf("trapframe dump: s5=%ld\n", p->trapframe->s5);
    printf("trapframe dump: s6=%ld\n", p->trapframe->s6);
    printf("trapframe dump: s7=%ld\n", p->trapframe->s7);
    printf("trapframe dump: s8=%ld\n", p->trapframe->s8);
    printf("trapframe dump: kernel_sp=%ld\n", p->trapframe->kernel_sp);
    printf("trapframe dump: kernel_trap=%ld\n", p->trapframe->kernel_trap);
    printf("trapframe dump: era=%ld\n", p->trapframe->era);
    printf("trapframe dump: kernel_hartid=%ld\n", p->trapframe->kernel_hartid);
    printf("trapframe dump: kernel_pgdl=%ld\n", p->trapframe->kernel_pgdl);
    return;
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

    // send syscalls, interrupts, and exceptions to uservec.S
    w_csr_eentry((uint64)uservec);

    // set up trapframe values that uservec will need when
    // the process next re-enters the kernel.
    p->trapframe->kernel_pgdl = r_csr_pgdl();      // kernel page table
    p->trapframe->kernel_sp = p->kstack + PGSIZE;  // process's kernel stack
    p->trapframe->kernel_trap = (uint64)usertrap;
    p->trapframe->kernel_hartid = r_tp();  // hartid for cpuid()

    // set up the registers that uservec.S's ertn will use
    // to get to user space.

    // Enable floating point unit for user mode - critical for glibc
    // Do this BEFORE setting privilege mode
    uint32 euen = r_csr_euen();
    euen |= CSR_EUEN_FPE;  // enable FPU
    w_csr_euen(euen);

    // set Previous Privilege mode to User (PLV=3) and enable PIE
    uint32 x = r_csr_prmd();
#ifdef LA2K1000
    x &= ~PRMD_PPLV;
    x |= (3U << 0);
    x |= PRMD_PIE;
#else
    x |= PRMD_PPLV;
    x |= PRMD_PIE;
#endif
    w_csr_prmd(x);

#ifdef LA2K1000
    // 2K1000: 保持 CRMD 不变，避免引入页表模式切换等副作用
#endif

    // set Exception Return Address to the saved user pc.
    // 若 era 为 0（未初始化），以 0 作为无效值会立即触发异常
#ifdef LA2K1000
    if (p->trapframe->era == 0) {
        // init 进程：入口设为 0（uvmfirst 在 0..2*PGSIZE ），紧接着的格式化进程会设置 sp
        p->trapframe->era = 0;
    }
#endif
    w_csr_era(p->trapframe->era);

    // tell uservec.S the user page table to switch to.

    volatile uint64 pgdl = VIRT2PHY((uint64)(p->pagetable));

    // jump to uservec.S at the top of memory, which
    // switches to the user page table, restores user registers,
    // and switches to user mode with ertn.
    userret(TRAPFRAME, pgdl);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap() {
    int which_dev = 0;
    uint64 era = r_csr_era();
    uint64 prmd = r_csr_prmd();

    if ((prmd & PRMD_PPLV) != 0) panic("kerneltrap: not from privilege0");
    if (intr_get() != 0) panic("kerneltrap: interrupts enabled");

    if ((which_dev = devintr()) == 0) {
#ifdef LA2K1000
        printf("estat %x\n", r_csr_estat());
        printf("era=%lx eentry=%lx badv=%lx badi=%x crmd=%x prmd=%x pgdl=%lx pgdh=%lx\n",
               r_csr_era(), r_csr_eentry(), r_csr_badv(), r_csr_badi(), r_csr_crmd(), r_csr_prmd(),
               r_csr_pgdl(), 0UL /* no r_csr_pgdh helper */);
        backtrace();
#else
        printf("estat %x\n", r_csr_estat());
        printf("era=%lx eentry=%lx\n", r_csr_era(), r_csr_eentry());
#endif
        panic("kerneltrap");
    }

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING) yield();

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's instruction.
    w_csr_era(era);
    w_csr_prmd(prmd);
}

void machine_trap() { panic("machine error"); }

void clockintr() {
    acquire(&tickslock);
    ticks++;
    wakeup(&ticks);
    release(&tickslock);
}

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
        case PCIE_IRQ:  // 32
            virtio_counter++;
            break;
        case UART0_IRQ:  // 2
            uart_counter++;
            break;
    }
    release(&interrupt);
    return;
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr() {
    uint32 estat = r_csr_estat();
    uint32 ecfg = r_csr_ecfg();

    if (estat & ecfg & HWI_VEC) {
        // this is a hardware interrupt, via IOCR.

        // irq indicates which device interrupted.
        uint64 irq = extioi_claim();
        if (irq & (1UL << UART0_IRQ)) {
            counter(UART0_IRQ);
            uartintr();

            // tell the apic the device is
            // now allowed to interrupt again.

            extioi_complete(1UL << UART0_IRQ);
        } else if (irq & (1UL << PCIE_IRQ)) {
            counter(PCIE_IRQ);
            virtio_disk_intr();
        } else if (irq) {
            printf("unexpected interrupt irq=%ld\n", irq);

            apic_complete(irq);
            extioi_complete(irq);
        }

        return 1;
    } else if (estat & ecfg & TI_VEC) {
        // timer interrupt,

        if (cpuid() == 0) {
            counter(CLOCK_IRQ);
            clockintr();
        }

        // acknowledge the timer interrupt by clearing
        // the TI bit in TICLR.
        w_csr_ticlr(r_csr_ticlr() | CSR_TICLR_CLR);

        return 2;
    } else {
        return 0;
    }
}
