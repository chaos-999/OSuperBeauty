#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "defs.h"
#include "fs/vfs/file.h"
#include "fs/vfs/fs.h"
#include "fs/vfs/inode.h"
#include "fs/vfs_ext4_ext.h"
#include "lib/sbi.h"

volatile static int started = 0;
volatile static int boothartid = -1;

// 跟随参考项目的hartid初始化方式
static inline void inithartid(unsigned long hartid) {
    // 直接使用hartid作为cpuid，不做映射
    asm volatile("mv tp, %0" : : "r"(hartid & 0x1));
}

void print_springos_banner(void) {
    printf_highlight("\033[95m",
                     "░▒▓███████▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒░▒▓███████▓▒░ ░▒▓██████▓▒░ "
                     "░▒▓██████▓▒░ ░▒▓███████▓▒░ \n");
    printf_highlight(
        "\033[95m",
        "░▒▓█▓▒░      "
        "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░        \n");
    printf_highlight("\033[95m",
                     "░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      "
                     "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░        \n");
    printf_highlight("\033[95m",
                     " ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒▒▓███▓▒░▒▓█▓▒"
                     "░░▒▓█▓▒░░▒▓██████▓▒░  \n");
    printf_highlight("\033[95m",
                     "       ░▒▓█▓▒░▒▓█▓▒░      "
                     "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░ \n");
    printf_highlight("\033[95m",
                     "       ░▒▓█▓▒░▒▓█▓▒░      "
                     "░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░ \n");
    printf_highlight("\033[95m",
                     "░▒▓███████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ "
                     "░▒▓██████▓▒░░▒▓███████▓▒░  \n");
    printf_highlight("\033[95m",
                     "                                                                             "
                     "                      \n");
}

// VF2专用main函数，直接从entry.S调用
// 跟随参考项目的函数签名
void main(unsigned long hartid, unsigned long dtb_pa) {
    // 跟随参考项目的初始化方式
    inithartid(hartid);

    SBI_PUTCHAR('A');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('B');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('C');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('D');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('E');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('F');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('G');
    SBI_PUTCHAR('\n');
    SBI_PUTCHAR('H');
    SBI_PUTCHAR('\n');

    // printf("SpringOS starting on VF2...\n");

    // 第一个hart成为引导核心
    if (boothartid == -1) {
        boothartid = hartid;

        // printf("hartid %ld is the boot core...\n", hartid);
        procinit();
        // 基础初始化
        consoleinit();
        printfinit();
        print_springos_banner();
        printf("\n");

        // 内存管理初始化
        kinit();
        printf("hartid %ld after kinit...\n", hartid);

        kvminit();
        printf("hartid %ld after kvminit...\n", hartid);

        kvminithart();
        printf("hartid %ld after kvminithart...\n", hartid);

        // 进程管理初始化

        // printf("hartid %ld after procinit...\n", hartid);

        // 中断和设备初始化
        futex_init();
        printf("hartid %ld after futex_init...\n", hartid);

        // 初始化trap框架（ticks/锁/定时器），再设置本核stvec
        trapinit();
        trapinithart();
        printf("hartid %ld after trapinithart...\n", hartid);

        plicinit();
        printf("hartid %ld after plicinit...\n", hartid);

        plicinithart();
        printf("hartid %ld after plicinithart...\n", hartid);
        // 开启S态中断
        extern void kernelvec(void);
        w_stvec((uint64)kernelvec);
        w_sstatus(r_sstatus() | SSTATUS_SIE);
        // 开启S态中断
        w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);

        // 文件系统初始化
        init_fs_table();
        binit();
        fileinit();
        inodeinit();

        vfs_ext4_init();
        ramdisk_init();  // VF2使用ramdisk

        // 创建第一个用户进程
        userinit();
        printf("hartid %ld after userinit...\n", hartid);

        // 启动标志
        __sync_synchronize();
        started = 1;

        printf("hartid %ld init done\n", hartid);
    } else {
        // 其他hart等待主hart完成初始化
        while (started == 0);
        __sync_synchronize();

        printf("hartid %ld continues\n", hartid);
        kvminithart();
        trapinithart();
        plicinithart();

        printf("hartid %ld init done\n", hartid);
    }

    printf("scheduler on hartid %ld\n", hartid);
    scheduler();
}