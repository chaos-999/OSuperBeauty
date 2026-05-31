#include "fs/vfs/fs.h"

#include <defs.h>

#include "types.h"
#include "platform.h"
#include "param.h"
#include "fs/vfs_ext4_ext.h"
#include "defs.h"
#include "fs/vfs/ops.h"

filesystem_t *fs_table[VFS_MAX_FS];
filesystem_op_t *fs_ops_table[VFS_MAX_FS] = {
    NULL,
    NULL,
    &ext4_fs_op,
    NULL,
};

filesystem_t ext4_fs;

struct spinlock fs_table_lock;

void init_fs_table(void) {
    initlock(&fs_table_lock, "fs_table_lock");
    for (int i = 0; i < VFS_MAX_FS; i++) {
        fs_table[i] = NULL;
    }
    printf("init_fs_table finished\n");
}

void fs_init(filesystem_t *fs, int dev, fs_t fs_type, const char *path) {
    acquire(&fs_table_lock);
    fs_table[fs_type] = fs;
    fs->dev = dev;
    fs->type = fs_type;
    fs->path = (char *)path;
    fs->fs_op = fs_ops_table[fs_type];
    release(&fs_table_lock);
    printf("fs_init done\n");
}

void filesystem_init(void) {
    fs_init(&ext4_fs, ROOTDEV, EXT4, "/");
    void *fs_sb = NULL;
    fs_mount(&ext4_fs, 0, fs_sb);
    struct file_vnode *cwd = &(myproc()->cwd);
    strcpy(cwd->path, "/");
    cwd->fs = &ext4_fs;
}

int fs_mount(filesystem_t *fs, uint64 rwflag, void *data) {
    if (fs->fs_op->mount) {
        int ret = fs->fs_op->mount(fs, rwflag, data);
        printf("fs_mount done: %d\n", ret);
        return ret;
    }
    return -1;
}

int fs_umount(filesystem_t *fs) { return 0; }

filesystem_t *get_fs_by_type(fs_t type) {
    if (fs_table[type]) {
        return fs_table[type];
    }
    return NULL;
}

filesystem_t *get_fs_by_mount_point(const char *mp) {
    for (int i = 0; i < VFS_MAX_FS; i++) {
        if (fs_table[i] && fs_table[i]->path && !strcmp(fs_table[i]->path, mp)) {
            return fs_table[i];
        }
    }
    return NULL;
}

struct filesystem *get_fs_from_path(const char *path) {
    char abs_path[MAXPATH] = {0};
    get_absolute_path(path, "/", abs_path);

    size_t len = strlen(abs_path);
    char *pstart = abs_path, *pend = abs_path + len - 1;
    while (pend > pstart) {
        if (*pend == '/') {
            *pend = '\0';
            filesystem_t *fs = get_fs_by_mount_point(pstart);
            if (fs) {
                return fs;
            }
        }
        pend--;
    }

    if (pend == pstart) {
        return get_fs_by_mount_point("/");
    }

    return NULL;
}
