#if defined(VF2) || defined(LA2K1000)

//
// ramdisk that uses embedded filesystem image
// 替代 virtio-blk 驱动，用于 VF2 上板
//

#include "types.h"
#include "platform.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "lock/lock.h"
#include "fs/vfs/fs.h"
#include "fs/buf.h"

// 由链接器引入的内嵌二进制镜像符号（通过 objcopy -I binary 生成）
extern unsigned char _binary_ramdisk_img_start[];
extern unsigned char _binary_ramdisk_img_end[];

#define RAMDISK_BYTES ((uint64)(_binary_ramdisk_img_end - _binary_ramdisk_img_start))
#define RAMDISK_SIZE (RAMDISK_BYTES / BSIZE)

void ramdisk_init(void) {
    // RAMDisk 不需要初始化硬件
    printf("ramdisk: using embedded filesystem image (%lu bytes, %lu blocks)\n",
           (unsigned long)RAMDISK_BYTES, (unsigned long)RAMDISK_SIZE);
}

void ramdisk_rw(struct buf *b, int write) {
    if (!holdingsleep(&b->lock)) panic("ramdisk_rw: buf not locked");
    if (b->blockno >= RAMDISK_SIZE) {
        printf("ramdisk_rw: block number %lu too big (max %lu)\n", (unsigned long)b->blockno,
               (unsigned long)RAMDISK_SIZE);
        panic("ramdisk_rw");
    }
    static int rd_dbg = 0, wr_dbg = 0;
    if (!write && rd_dbg < 8) {
        printf("ramdisk_rd: block=%lu\n", (unsigned long)b->blockno);
        rd_dbg++;
    }
    if (write && wr_dbg < 8) {
        printf("ramdisk_wr: block=%lu\n", (unsigned long)b->blockno);
        wr_dbg++;
    }
    int offset = b->blockno * BSIZE;
    char *addr = (char *)_binary_ramdisk_img_start + offset;
    if (write) {
        memmove(addr, b->data, BSIZE);
    } else {
        memmove(b->data, addr, BSIZE);
    }
}

#endif