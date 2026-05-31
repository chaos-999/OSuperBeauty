#include "fs/vfs_ext4_blockdev_ext.h"

#include "types.h"
#include "platform.h"
#include "defs.h"
#include "lib/list.h"
#include "lock/lock.h"
#include "fs/vfs/fs.h"
#include "fs/buf.h"
#include "dev/virtio.h"
#include "fs/ext4/ext4.h"
#include "fs/ext4/ext4_blockdev.h"
#include "fs/ext4/ext4_errno.h"
#include "fs/ext4/misc/queue.h"
#include "memlayout.h"
#ifdef VF2
// Binary-embedded ramdisk symbols provided by objcopy in the final link
extern unsigned char _binary_ramdisk_img_start[];
extern unsigned char _binary_ramdisk_img_end[];
#define RD_START _binary_ramdisk_img_start
#define RD_END _binary_ramdisk_img_end
#define RD_LEN ((uint64)(RD_END - RD_START))
#endif

static int blockdev_lock(struct ext4_blockdev *bdev);
static int blockdev_unlock(struct ext4_blockdev *bdev);
static int blockdev_open(struct ext4_blockdev *bdev);
static int blockdev_read(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id, uint32_t blk_cnt);
static int blockdev_write(struct ext4_blockdev *bdev, const void *buf, uint64_t blk_id,
                          uint32_t blk_cnt);
static int blockdev_close(struct ext4_blockdev *bdev);

struct ext4_blockdev_iface biface;
struct vfs_ext4_blockdev bvbdev;

struct vfs_ext4_blockdev *vfs_ext4_blockdev_from_bd(struct ext4_blockdev *bdev) {
    if (bdev == NULL) {
        return NULL;
    }
    return container_of(bdev, struct vfs_ext4_blockdev, bd);
}

static int vfs_ext4_blockdev_init(struct vfs_ext4_blockdev *vbdev, int dev) {
    uint8_t *ph_bbuf = NULL;
    struct ext4_blockdev *bd = NULL;
    struct ext4_blockdev_iface *iface = &biface;

    if (vbdev) {
        vbdev->dev = dev;

        bd = &vbdev->bd;
        bd->bdif = iface;
        bd->part_offset = 0;
        // Use the real RAMDisk size on VF2 to avoid OOB accesses and wrong
        // block count reporting. For non-VF2, fallback to large size elsewhere.
        bd->part_size = 4ULL * 1024 * 1024 * 1024;  // 4GB to match sdcard-rv.img size
        ph_bbuf = &vbdev->ph_bbuf[0];

        iface->lock = blockdev_lock;
        iface->unlock = blockdev_unlock;
        iface->open = blockdev_open;
        iface->bread = blockdev_read;
        iface->bwrite = blockdev_write;
        iface->close = blockdev_close;

        iface->ph_bsize = BSIZE;
        iface->ph_bbuf = ph_bbuf;
        iface->ph_bcnt = bd->part_size / (uint64)bd->bdif->ph_bsize;
        iface->ph_refctr = 0;  // ensure known initial state

        printf("vfs_ext4_blockdev_init: ph_bsize=%d, ph_bcnt=%ld\n", iface->ph_bsize,
               iface->ph_bcnt);
    }
    return EOK;
}

struct vfs_ext4_blockdev *vfs_ext4_blockdev_create(int dev) {
    struct vfs_ext4_blockdev *vbdev = &bvbdev;
    if (vbdev == NULL) {
        return NULL;
    }
    int r = vfs_ext4_blockdev_init(vbdev, dev);
    if (r != EOK) {
        printf("vfs_ext4_blockdev_init failed: %d\n", r);
        return NULL;
    }
    r = ext4_device_register(&vbdev->bd, DEV_NAME);
    if (r != EOK) {
        printf("ext4_device_register failed: %d\n", r);
        return NULL;
    }
    printf("ext4_device_register ok for '%s'\n", DEV_NAME);
    return vbdev;
}

int vfs_ext4_blockdev_destroy(struct vfs_ext4_blockdev *vbdev) {
    if (vbdev == NULL) return -EINVAL;
    return EOK;
}

static int blockdev_lock(struct ext4_blockdev *bdev) { return EOK; }

static int blockdev_unlock(struct ext4_blockdev *bdev) { return EOK; }

static int blockdev_open(struct ext4_blockdev *bdev) {
    // printf("blockdev_open called\n");
    if (bdev && bdev->bdif) {
        bdev->bdif->ph_refctr = 1;  // mark device open
    }
    return EOK;
}

static int blockdev_close(struct ext4_blockdev *bdev) {
    // printf("blockdev_close called\n");
    if (bdev && bdev->bdif) {
        bdev->bdif->ph_refctr = 0;  // mark device closed
    }
    return EOK;
}

static int blockdev_read(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id, uint32_t blk_cnt) {
    static int debug_reads = 0;
    if (debug_reads < 8) {
        // printf("blockdev_read: blk_id=%lu cnt=%u\n", (unsigned long)blk_id, blk_cnt);
        debug_reads++;
    }
#ifdef VF2
    uint64_t offset = blk_id * (uint64)BSIZE;
    uint64_t total = (uint64)blk_cnt * (uint64)BSIZE;
    if (offset + total > (uint64)RD_LEN) {
        return EINVAL;
    }
    memmove(buf, RD_START + offset, total);
    return EOK;
#else
    uint64 buf_ptr = (uint64)buf;
    for (int i = 0; i < blk_cnt; i++) {
        struct buf *b = bread(0, blk_id + i);
#ifdef LA2K1000
        // 若目标缓冲不是内核直映地址，走用户拷贝路径
        if (((buf_ptr & VIRT_DMWIN_MASK) == 0) && myproc() != 0) {
            char *kpage = kalloc();
            if (!kpage) {
                brelse(b);
                return EINVAL;
            }
            memmove(kpage, b->data, BSIZE);
            if (copyout(myproc()->pagetable, buf_ptr, kpage, BSIZE) != 0) {
                kfree(kpage);
                brelse(b);
                return EFAULT;
            }
            kfree(kpage);
        } else {
            memmove((void *)buf_ptr, b->data, BSIZE);
        }
#else
        memmove((void *)buf_ptr, b->data, BSIZE);
#endif
        buf_ptr += BSIZE;
        brelse(b);
    }
    return EOK;
#endif
}

static int blockdev_write(struct ext4_blockdev *bdev, const void *buf, uint64_t blk_id,
                          uint32_t blk_cnt) {
    static int debug_writes = 0;
    if (debug_writes < 8) {
        // printf("blockdev_write: blk_id=%lu cnt=%u\n", (unsigned long)blk_id, blk_cnt);
        debug_writes++;
    }
#ifdef VF2
    uint64_t offset = blk_id * (uint64)BSIZE;
    uint64_t total = (uint64)blk_cnt * (uint64)BSIZE;
    if (offset + total > (uint64)RD_LEN) {
        return EINVAL;
    }
    memmove(RD_START + offset, buf, total);
    return EOK;
#else
    uint64 buf_ptr = (uint64)buf;
    for (int i = 0; i < blk_cnt; i++) {
        struct buf *b = bget(0, blk_id + i);
        memmove(b->data, (void *)buf_ptr, BSIZE);
        bwrite(b);
        buf_ptr += BSIZE;
        brelse(b);
    }
    return EOK;
#endif
}
