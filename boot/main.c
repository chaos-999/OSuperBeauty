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
volatile static int boot_hart = -1;  // 用于存储引导核心的HART ID

void print_osuperbeauty_banner(void) {
    printf_highlight("\033[91m", " _____   ____                                   __                                __ \n");
    printf_highlight("\033[91m", "/\\  __`\\/\\  _`\\                                /\\ \\                              /\\ \\__ \n");
    printf_highlight("\033[91m", "\\ \\ \\/\\ \\ \\,\\L\\_\\  __  __  _____      __   _ __\\ \\ \\____     __     __     __  __\\ \\ ,_\\  __  __ \n");
    printf_highlight("\033[91m", " \\ \\ \\ \\ \\/_\\__ \\ /\\ \\/\\ \\/\\ '__`\\  /'__`\\/\\`'__\\ \\ '__`\\  /'__`\\ /'__`\\  /\\ \\/\\ \\\\ \\ \\/ /\\ \\/\\ \\ \n");
    printf_highlight("\033[91m", "  \\ \\ \\_\\ \\/\\ \\L\\ \\ \\ \\_\\ \\ \\ \\L\\ \\/\\  __/\\ \\ \\/ \\ \\ \\L\\ \\/\\  __//\\ \\L\\.\\_\\ \\ \\_\\ \\\\ \\ \\_\\ \\ \\_\\ \\ \n");
    printf_highlight("\033[91m", "   \\ \\_____\\ `\\____\\ \\____/\\ \\ ,__/\\ \\____\\\\ \\_\\  \\ \\_,__/\\ \\____\\ \\__/.\\_\\\\ \\____/ \\ \\__\\\\/`____ \\ \n");
    printf_highlight("\033[91m", "    \\/_____/\\/_____/\\/___/  \\ \\ \\/  \\/____/ \\/_/   \\/___/  \\/____/\\/__/\\/_/ \\/___/   \\/__/ `/___/> \\ \n");
    printf_highlight("\033[91m", "                             \\ \\_\\                                                            /\\___/ \n");
    printf_highlight("\033[91m", "                              \\/_/                                                            \\/__/ \n");
    printf_highlight("\033[91m", "                                                                                                   \n");
    printf_highlight("\033[91m", "                                                                                                   \n");
}

void print_osuperbeauty_banner2(void) {
    printf_highlight("\033[91m", " _____   ____                                   __                                __ \n");
    printf_highlight("\033[91m", "/\\  __`\\/\\  _`\\                                /\\ \\                              /\\ \\__ \n");
    printf_highlight("\033[91m", "\\ \\ \\/\\ \\ \\,\\L\\_\\  __  __  _____      __   _ __\\ \\ \\____     __     __     __  __\\ \\ ,_\\  __  __ \n");
    printf_highlight("\033[91m", " \\ \\ \\ \\ \\/_\\__ \\ /\\ \\/\\ \\/\\ '__`\\  /'__`\\/\\`'__\\ \\ '__`\\  /'__`\\ /'__`\\  /\\ \\/\\ \\\\ \\ \\/ /\\ \\/\\ \\ \n");
    printf_highlight("\033[91m", "  \\ \\ \\_\\ \\/\\ \\L\\ \\ \\ \\_\\ \\ \\ \\L\\ \\/\\  __/\\ \\ \\/ \\ \\ \\L\\ \\/\\  __//\\ \\L\\.\\_\\ \\ \\_\\ \\\\ \\ \\_\\ \\ \\_\\ \\ \n");
    printf_highlight("\033[91m", "   \\ \\_____\\ `\\____\\ \\____/\\ \\ ,__/\\ \\____\\\\ \\_\\  \\ \\_,__/\\ \\____\\ \\__/.\\_\\\\ \\____/ \\ \\__\\\\/`____ \\ \n");
    printf_highlight("\033[91m", "    \\/_____/\\/_____/\\/___/  \\ \\ \\/  \\/____/ \\/_/   \\/___/  \\/____/\\/__/\\/_/ \\/___/   \\/__/ `/___/> \\ \n");
    printf_highlight("\033[91m", "                             \\ \\_\\                                                            /\\___/ \n");
    printf_highlight("\033[91m", "                              \\/_/                                                            \\/__/ \n");
    printf_highlight("\033[91m", "                                                                                                   \n");
    printf_highlight("\033[91m", "                                                                                                   \n");
}

// start() jumps here in supervisor mode on all CPUs.
void main() {
    int hartid = cpuid();

    // 第一个进入main函数的hart将成为引导核心
    // 使用原子操作设置boot_hart
    if (__sync_bool_compare_and_swap(&boot_hart, -1, hartid)) {
#ifdef RISCV
        // 这是引导核心
        consoleinit();
        printfinit();
        printf("\n");
        print_osuperbeauty_banner();
        printf("\n");
        kinit();         // physical page allocator
        kvminit();       // create kernel page table
        kvminithart();   // turn on paging
        procinit();      // process table
        futex_init();    // futex synchronization
        trapinit();      // trap vectors
        trapinithart();  // install kernel trap vector
        plicinit();      // set up interrupt controller
        plicinithart();  // ask PLIC for device interrupts

        init_fs_table();  // fs_table init
        binit();          // buffer cache
        fileinit();       // file table
        inodeinit();      // inode table

        vfs_ext4_init();     // 初始化lwext4
        virtio_disk_init();  // emulated hard disk
        userinit();  // first user process
        __sync_synchronize();

        // 启动其他CPU核心，跳过引导核心自身
        for (int i = 0; i < NCPU; i++) {
            if (i != hartid) {
                // 使用SBI调用启动其他核心，起始地址是0x80200000
                SBI_HART_START(i, 0x80200000, 0);
            }
        }

        __sync_synchronize();
        started = 1;
#elif defined(LOONGARCH)
        consoleinit();
        printfinit();
        printf("\n");
        print_osuperbeauty_banner2();
        printf("\n");

        virtio_probe();

        apic_init();    // set up LS7A1000 interrupt controller
        extioi_init();  // extended I/O interrupt controller
        trapinit();     // trap vectors

        kinit();        // physical page allocator
        kvminit();      // create kernel page table
        kvminithart();  // turn on paging
        procinit();     // process table
        futex_init();   // futex synchronization

        init_fs_table();  // fs_table init
        binit();          // buffer cache
        fileinit();       // file table
        inodeinit();      // inode table

        vfs_ext4_init();     // 初始化lwext4
        virtio_disk_init();  // emulated hard disk

        userinit();  // first user process
        __sync_synchronize();
        started = 1;
#endif
    } else {
#ifdef RISCV
        // 从CPU仍然需要等待启动标志，确保主CPU已完成初始化
        while (started == 0);
        __sync_synchronize();
        printf("hart %d starting\n", hartid);
        kvminithart();   // turn on paging
        trapinithart();  // install kernel trap vector
        plicinithart();  // ask PLIC for device interrupts
#endif
    }

    scheduler();
}
