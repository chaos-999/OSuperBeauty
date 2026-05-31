#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "defs.h"
#include "lib/sbi.h"

void main();

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];

// entry.S jumps here in supervisor mode on stack0.
// a0 contains the hartid from OpenSBI
void start(unsigned long hartid) {
    // 通过OpenSBI的控制台功能输出，每个hart只输出一次
    SBI_PUTCHAR('H');
    SBI_PUTCHAR('e');
    SBI_PUTCHAR('l');
    SBI_PUTCHAR('l');
    SBI_PUTCHAR('o');
    SBI_PUTCHAR(' ');
    SBI_PUTCHAR('O');
    SBI_PUTCHAR('p');
    SBI_PUTCHAR('e');
    SBI_PUTCHAR('n');
    SBI_PUTCHAR('S');
    SBI_PUTCHAR('B');
    SBI_PUTCHAR('I');
    SBI_PUTCHAR(' ');
    SBI_PUTCHAR('h');
    SBI_PUTCHAR('a');
    SBI_PUTCHAR('r');
    SBI_PUTCHAR('t');
    SBI_PUTCHAR('i');
    SBI_PUTCHAR('d');
    SBI_PUTCHAR('=');
    // 输出hartid十进制值
    if (hartid == 0) {
        SBI_PUTCHAR('0');
    } else {
        // 简单实现十进制输出
        char digits[20];
        int idx = 0;
        unsigned long val = hartid;
        while (val > 0) {
            digits[idx++] = '0' + (val % 10);
            val /= 10;
        }
        while (idx > 0) {
            SBI_PUTCHAR(digits[--idx]);
        }
    }
    SBI_PUTCHAR('\r');
    SBI_PUTCHAR('\n');

    // Supervisor: disable paging for now.
    w_satp(0);

    // enable some supervisor interrupt
    w_sie(r_sie() | SIE_SEIE | SIE_STIE);

    // save hartid passed from OpenSBI
    w_tp(hartid);

    // enable supervisor mode deref user pointers.
    w_sstatus(r_sstatus() | SSTATUS_SUM);

    main();
}
