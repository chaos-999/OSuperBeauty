#pragma once

#include "types.h"
#include "fs/ext4/ext4_blockdev.h"

struct vfs_ext4_blockdev {
    int dev;
    struct ext4_blockdev bd;
    uint8 ph_bbuf[4096];
};

#define DEV_NAME "virtio_disk"

struct vfs_ext4_blockdev *vfs_ext4_blockdev_create(int dev);
int vfs_ext4_blockdev_destroy(struct vfs_ext4_blockdev *bdev);
struct vfs_ext4_blockdev *vfs_ext4_blockdev_from_bd(struct ext4_blockdev *bd);
