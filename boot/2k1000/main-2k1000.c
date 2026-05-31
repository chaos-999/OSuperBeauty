#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "defs.h"
#include "fs/vfs/file.h"
#include "fs/vfs/fs.h"
#include "fs/vfs/inode.h"
#include "fs/vfs_ext4_ext.h"

void print_springos_banner2(void) {
    printf_highlight("\033[95m", "██████  ██▓███   ██▀███   ██▓ ███▄    █   ▄████  ▒█████    ██████ \n");
    printf_highlight("\033[95m", "▒██    ▒ ▓██░  ██▒▓██ ▒ ██▒▓██▒ ██ ▀█   █  ██▒ ▀█▒▒██▒  ██▒▒██    ▒ \n");
    printf_highlight("\033[95m", "░ ▓██▄   ▓██░ ██▓▒▓██ ░▄█ ▒▒██▒▓██  ▀█ ██▒▒██░▄▄▄░▒██░  ██▒░ ▓██▄   \n");
    printf_highlight("\033[95m", "  ▒   ██▒▒██▄█▓▒ ▒▒██▀▀█▄  ░██░▓██▒  ▐▌██▒░▓█  ██▓▒██   ██░  ▒   ██▒\n");
    printf_highlight("\033[95m", "▒██████▒▒▒██▒ ░  ░░██▓ ▒██▒░██░▒██░   ▓██░░▒▓███▀▒░ ████▓▒░▒██████▒▒\n");
    printf_highlight("\033[95m", "▒ ▒▓▒ ▒ ░▒▓▒░ ░  ░░ ▒▓ ░▒▓░░▓  ░ ▒░   ▒ ▒  ░▒   ▒ ░ ▒░▒░▒░ ▒ ▒▓▒ ▒ ░\n");
    printf_highlight("\033[95m", "░ ░▒  ░ ░░▒ ░       ░▒ ░ ▒░ ▒ ░░ ░░   ░ ▒░  ░   ░   ░ ▒ ▒░ ░ ░▒  ░ ░\n");
    printf_highlight("\033[95m", "░  ░  ░  ░░         ░░   ░  ▒ ░   ░   ░ ░ ░ ░   ░ ░ ░ ░ ▒  ░  ░  ░  \n");
    printf_highlight("\033[95m", "      ░              ░      ░           ░       ░     ░ ░        ░  \n");
    printf_highlight("\033[95m", "                                                                    \n");
}


// 2K1000 专属 main：最小闭环（串口/中断/内存/RAMDisk/用户进程）
void main_2k1000(void) {
    // 极早期直接输出，确认 C 入口
    extern void early_uart_puts(const char *);
    early_uart_puts("[EARLY] entered main_2k1000()\n");

    early_uart_puts("[M1] before consoleinit\n");
    consoleinit();   // 若串口尚未 ready，这里会接管之后的 printf
    early_uart_puts("[M2] after consoleinit\n");
    printfinit();
    early_uart_puts("[M3] after printfinit\n");
    printf("\n");
    print_springos_banner2();
    printf("\n");

    // 板内中断控制器 + EXTIOI
    early_uart_puts("[M4] before apic_init\n");
    apic_init();      // 在 LA2K1000 下路由到 iointc_2k1000_init()
    early_uart_puts("[M5] after apic_init\n");
    extioi_init();
    early_uart_puts("[M6] after extioi_init\n");

    // 内存/进程/陷入框架
    early_uart_puts("[M7] before kinit\n");
    kinit();          // 物理页分配器
    early_uart_puts("[M8] after kinit\n");
    kvminit();        // 内核页表
    early_uart_puts("[M9] after kvminit\n");
    kvminithart();    // 开启分页
    early_uart_puts("[M10] after kvminithart\n");
    procinit();
    early_uart_puts("[M11] after procinit\n");
    futex_init();
    early_uart_puts("[M12] after futex_init\n");

    // Trap 初始化
    early_uart_puts("[M13] before trapinit\n");
    trapinit();
    early_uart_puts("[M14] after trapinit\n");

    // 文件系统初始化顺序（与 VF2 对齐）：
    // 1) fs 表与缓存/文件表/索引表
    // 2) EXT4 子系统
    // 3) 设备（RAMDisk）注册
    // 4) 文件系统挂载（filesystem_init）
    early_uart_puts("[M15] before init_fs_table/binit/fileinit/inodeinit\n");
    init_fs_table();  // fs_table 初始化
    binit();          // buffer cache
    fileinit();       // file table
    inodeinit();      // inode table
    early_uart_puts("[M16] after init_fs_table/binit/fileinit/inodeinit\n");

    early_uart_puts("[M17] before vfs_ext4_init\n");
    vfs_ext4_init();
    early_uart_puts("[M18] after vfs_ext4_init\n");

    early_uart_puts("[M19] before ramdisk_init\n");
    ramdisk_init();   // 注册 RAMDisk 设备
    early_uart_puts("[M20] after ramdisk_init\n");

    // 创建第一个用户进程并进入调度
    early_uart_puts("[M21] before userinit\n");
    userinit();
    early_uart_puts("[M22] before scheduler\n");
    scheduler();
}


