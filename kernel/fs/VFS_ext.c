#include "types.h"
#include "platform.h"
#include "defs.h"
#include "time.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "fs/vfs/file.h"
#include "fs/vfs/fs.h"
#include "fs/stat.h"
#include "fs/vfs_ext4_blockdev_ext.h"
#include "fs/vfs_ext4_ext.h"
#include "fs/vfs/inode.h"

#include <fs/ext4/ext4_oflags.h>
#include "fs/ext4/ext4_errno.h"
#include "fs/ext4/ext4_fs.h"
#include "fs/ext4/ext4_inode.h"
#include "fs/ext4/ext4_super.h"
#include "fs/ext4/ext4_types.h"

#include "fs/ext4/ext4.h"
#include "fs/ioctl.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

// 内核环境不可用标准库，提供一个最小的子串匹配工具
static inline int k_str_contains(const char *s, const char *pat) {
    if (!s || !pat) return 0;
    if (*pat == '\0') return 1;
    for (const char *p = s; *p; p++) {
        const char *a = p;
        const char *b = pat;
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        if (*b == '\0') return 1;
    }
    return 0;
}

// Linux dirent d_type values
#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14
#endif

struct spinlock extlock;

int vfs_ext4_init(void) {
    initlock(&extlock, "ext4 fs");
    ext4_device_unregister_all();
    ext4_init_mountpoints();
    return 0;
}

int vfs_ext_mount(struct filesystem *fs, uint64_t rwflag, void *data) {
    int r = 0;
    struct vfs_ext4_blockdev *vbdev = vfs_ext4_blockdev_create(fs->dev);

    if (vbdev == NULL) {
        r = -ENOMEM;
        goto out;
    }

    r = ext4_mount(DEV_NAME, fs->path, false);
    if (r != EOK) {
        printf("ext4_mount failed: %d\n", r);
        vfs_ext4_blockdev_destroy(vbdev);
        goto out;
    } else {
        ext4_get_sblock(fs->path, (struct ext4_sblock **)(&(fs->fs_data)));
    }
out:
    return r;
}

int vfs_ext_statfs(struct filesystem *fs, struct statfs *buf) {
    struct ext4_sblock *sb = NULL;
    int err = EOK;

    err = ext4_get_sblock(fs->path, &sb);
    if (err != EOK) {
        return err;
    }

    buf->f_bsize = ext4_sb_get_block_size(sb);
    buf->f_blocks = ext4_sb_get_blocks_cnt(sb);
    buf->f_bfree = ext4_sb_get_free_blocks_cnt(sb);
    buf->f_bavail = ext4_sb_get_free_blocks_cnt(sb);
    buf->f_type = sb->magic;
    buf->f_files = sb->inodes_count;
    buf->f_ffree = sb->free_inodes_count;
    buf->f_frsize = ext4_sb_get_block_size(sb);
    buf->f_bavail = sb->free_inodes_count;
    buf->f_fsid.val[0] = 2;
    buf->f_flags = 0;
    buf->f_namelen = 32;
    return err;
}

struct filesystem_op ext4_fs_op = {
    .mount = vfs_ext_mount,
    .umount = vfs_ext_umount,
    .statfs = vfs_ext_statfs,
};

int vfs_ext_umount(struct filesystem *fs) {
    int r = 0;
    struct vfs_ext4_blockdev *vbdev = fs->fs_data;

    if (vbdev == NULL) {
        r = -ENOMEM;
        return r;
    }

    ext4_umount(fs->path);
    if (r != EOK) {
        return r;
    }

    vfs_ext4_blockdev_destroy(vbdev);
    return EOK;
}

int vfs_ext_ioctl(struct file *f, int cmd, void *args) {
    int r = 0;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }

    switch (cmd) {
        case FIOCLEX:
            f->f_flags |= O_CLOEXEC;
            break;
        case FIONCLEX:
            f->f_flags &= ~O_CLOEXEC;
            break;
        case FIONREAD:
            r = ext4_fsize(file);
            break;
        case FIONBIO:
            break;
        case FIOASYNC:
            break;
        default:
            r = -EINVAL;
            break;
    }
    return r;
}

int vfs_ext_read(struct file *f, int user_addr, const uint64 addr, int n) {
    uint64 byteread = 0;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }
    int r = 0;
    if (user_addr) {
        char *buf = kmalloc(n + 1);
        if (buf == NULL) {
            panic("vfs_ext_read: kalloc failed\n");
        }
        r = ext4_fread(file, buf, n, &byteread);
        if (k_str_contains(f->f_path, "gitconfig") || k_str_contains(f->f_path, "/.git/config")) {
            // printf("DEBUG: vfs_ext_read path='%s' req=%d ret=%d byteread=%lu\n", f->f_path, n, r,
            // byteread);
            if (k_str_contains(f->f_path, "/.git/config") && byteread > 0) {
                int maxp = byteread > 120 ? 120 : (int)byteread;
                char save = buf[maxp];
                buf[maxp] = '\0';
                // printf("DEBUG: .git/config first=%s\n", buf);
                buf[maxp] = save;
            }
        }
        if (r != EOK) {
            kfree(buf);
            return -r;  // propagate error
        }
        //  printf("copyout: addr = %lx, byteread = %ld\n", addr, byteread);
        if (copyout(myproc()->pagetable, addr, buf, byteread) != 0) {
            printf("copyout failed\n");
            kfree(buf);
            return -1;
        }
        // printf("here2\n");
        kfree(buf);
    } else {
        char *kbuf = (char *)addr;
        r = ext4_fread(file, kbuf, n, &byteread);
        if (k_str_contains(f->f_path, "gitconfig") || k_str_contains(f->f_path, "/.git/config")) {
            // printf("DEBUG: vfs_ext_read path='%s' req=%d ret=%d byteread=%lu\n", f->f_path, n, r,
            // byteread);
            if (k_str_contains(f->f_path, "/.git/config") && byteread > 0) {
                int maxp = byteread > 120 ? 120 : (int)byteread;
                char tmp = ((char *)addr)[0];  // avoid unused warnings
                (void)tmp;
                char save = kbuf[maxp];
                kbuf[maxp] = '\0';
                // printf("DEBUG: .git/config first=%s\n", kbuf);
                kbuf[maxp] = save;
            }
        }
        if (r != EOK) {
            return -r;
        }
        memmove((char *)addr, kbuf, byteread);
    }
    f->f_pos = file->fpos;
    // printf("vfs_ext_read: return byteread = %ld\n", byteread);
    return byteread;
}

int vfs_ext_readat(struct file *f, int user_addr, const uint64 addr, int n, int offset) {
    uint64 byteread = 0;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }

    // Save original file position
    uint64 original_pos = file->fpos;

    int r = ext4_fseek(file, offset, SEEK_SET);
    if (r != EOK) {
        if (k_str_contains(f->f_path, "gitconfig") || k_str_contains(f->f_path, "/.git/config")) {
            // printf("DEBUG: vfs_ext_readat fseek path='%s' off=%d ret=%d\n", f->f_path, offset,
            // r);
        }
        return -r;
    }
    if (user_addr) {
        char *buf = kmalloc(n + 1);
        if (buf == NULL) {
            panic("vfs_ext_read: kalloc failed\n");
        }
        r = ext4_fread(file, buf, n, &byteread);
        // if (k_str_contains(f->f_path, "gitconfig") || k_str_contains(f->f_path, "/.git/config")
        // || k_str_contains(f->f_path, "/.git/objects/")) {
        //     // printf("DEBUG: vfs_ext_readat path='%s' off=%d req=%d ret=%d byteread=%lu\n",
        //     f->f_path, offset, n, r, byteread); if (k_str_contains(f->f_path, "/.git/objects/")
        //     && byteread > 0) {
        //         int dump = byteread > 32 ? 32 : byteread;
        //         // printf("DEBUG: vfs_ext_readat data[dec]:");
        //         for (int i = 0; i < dump; i++) {
        //             if ((i % 16) == 0) printf("\n");
        //             printf(" %u", (unsigned int)(unsigned char)buf[i]);
        //         }
        //         printf("\n");
        //     }
        // }
        if (r != EOK) {
            kfree(buf);
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -r;
        }
        if (copyout(myproc()->pagetable, addr, buf, byteread) != 0) {
            kfree(buf);
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -1;
        }
        kfree(buf);
    } else {
        char *kbuf = (char *)addr;
        r = ext4_fread(file, kbuf, n, &byteread);
        // if (k_str_contains(f->f_path, "gitconfig") || k_str_contains(f->f_path, "/.git/config")
        // || k_str_contains(f->f_path, "/.git/objects/")) {
        //     // printf("DEBUG: vfs_ext_readat path='%s' off=%d req=%d ret=%d byteread=%lu\n",
        //     f->f_path, offset, n, r, byteread); if (k_str_contains(f->f_path, "/.git/objects/")
        //     && byteread > 0) {
        //         int dump = byteread > 32 ? 32 : byteread;
        //         unsigned char *bp = (unsigned char *)addr;
        //         // printf("DEBUG: vfs_ext_readat data[dec]:");
        //         for (int i = 0; i < dump; i++) {
        //             if ((i % 16) == 0) printf("\n");
        //             printf(" %u", (unsigned int)bp[i]);
        //         }
        //         printf("\n");
        //     }
        // }
        if (r != EOK) {
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -r;
        }
        memmove((char *)addr, kbuf, byteread);
    }

    // Restore original file position
    r = ext4_fseek(file, original_pos, SEEK_SET);
    if (r != EOK) {
        return -r;
    }
    return byteread;
}

int vfs_ext_write(struct file *f, int user_addr, const uint64 addr, int n) {
    uint64 bytewrite = 0;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }
    int r = 0;
    if (user_addr) {
        char *buf = kmalloc(n + 1);
        if (buf == NULL) {
            panic("vfs_ext_write: kalloc failed\n");
        }
        if (copyin(myproc()->pagetable, buf, addr, n) != 0) {
            kfree(buf);
            return -1;
        }
        int r = ext4_fwrite(file, buf, n, &bytewrite);
        if (k_str_contains(f->f_path, "/.git/config") ||
            k_str_contains(f->f_path, "/.git/config.lock")) {
            int maxp = n > 120 ? 120 : n;
            char save = buf[maxp];
            ((char *)buf)[maxp] = '\0';
            // printf("DEBUG: vfs_ext_write path='%s' n=%d wrote=%lu first=%s\n", f->f_path, n,
            // bytewrite, buf);
            ((char *)buf)[maxp] = save;
        }
        if (r != EOK) {
            kfree(buf);
            return -r;
        }
        kfree(buf);
    } else {
        char *kbuf = (char *)addr;
        r = ext4_fwrite(file, kbuf, n, &bytewrite);
        if (k_str_contains(f->f_path, "/.git/config") ||
            k_str_contains(f->f_path, "/.git/config.lock")) {
            int maxp = n > 120 ? 120 : n;
            char save = kbuf[maxp];
            kbuf[maxp] = '\0';
            // printf("DEBUG: vfs_ext_write path='%s' n=%d wrote=%lu first=%s\n", f->f_path, n,
            // bytewrite, kbuf);
            kbuf[maxp] = save;
        }
        if (r != EOK) {
            return -r;
        }
    }
    f->f_pos = file->fpos;
    return bytewrite;
}

int vfs_ext_writeat(struct file *f, int user_addr, const uint64 addr, int n, int offset) {
    uint64 bytewrite = 0;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }

    // Save original file position
    uint64 original_pos = file->fpos;

    // Handle hole filling: if writing beyond current file size, fill with zeros
    if (offset > file->fsize) {
        // Seek to current end of file
        int r = ext4_fseek(file, file->fsize, SEEK_SET);
        if (r != EOK) {
            return -r;
        }

        // Fill the gap with zeros
        uint64 hole_size = offset - file->fsize;
        char *zero_buf = kalloc();
        if (zero_buf == NULL) {
            return -1;
        }
        memset(zero_buf, 0, PGSIZE);

        uint64 remaining_hole = hole_size;
        while (remaining_hole > 0) {
            uint64 chunk = (remaining_hole > PGSIZE) ? PGSIZE : remaining_hole;
            uint64 written;
            r = ext4_fwrite(file, zero_buf, chunk, &written);
            if (r != EOK || written != chunk) {
                kfree(zero_buf);
                ext4_fseek(file, original_pos, SEEK_SET);
                return -r ? -r : -1;
            }
            remaining_hole -= written;
        }
        kfree(zero_buf);
    }

    int r = ext4_fseek(file, offset, SEEK_SET);
    if (r != EOK) {
        ext4_fseek(file, original_pos, SEEK_SET);
        return -r;
    }
    if (user_addr) {
        char *buf = kmalloc(n + 1);
        if (buf == NULL) {
            panic("vfs_ext_writeat: kalloc failed\n");
        }
        if (copyin(myproc()->pagetable, buf, addr, n) != 0) {
            kfree(buf);
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -1;
        }
        r = ext4_fwrite(file, buf, n, &bytewrite);
        if (k_str_contains(f->f_path, "/.git/config") ||
            k_str_contains(f->f_path, "/.git/config.lock")) {
            int maxp = n > 120 ? 120 : n;
            char save = buf[maxp];
            ((char *)buf)[maxp] = '\0';
            // printf("DEBUG: vfs_ext_writeat path='%s' off=%d n=%d wrote=%lu first=%s\n",
            // f->f_path, offset, n, bytewrite, buf);
            ((char *)buf)[maxp] = save;
        }
        if (r != EOK) {
            kfree(buf);
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -r;
        }
        kfree(buf);
    } else {
        char *kbuf = (char *)addr;
        r = ext4_fwrite(file, kbuf, n, &bytewrite);
        if (k_str_contains(f->f_path, "/.git/config") ||
            k_str_contains(f->f_path, "/.git/config.lock")) {
            int maxp = n > 120 ? 120 : n;
            char save = kbuf[maxp];
            kbuf[maxp] = '\0';
            // printf("DEBUG: vfs_ext_writeat path='%s' off=%d n=%d wrote=%lu first=%s\n",
            // f->f_path, offset, n, bytewrite, kbuf);
            kbuf[maxp] = save;
        }
        if (r != EOK) {
            // Restore original position before returning
            ext4_fseek(file, original_pos, SEEK_SET);
            return -r;
        }
    }

    // Restore original file position
    r = ext4_fseek(file, original_pos, SEEK_SET);
    if (r != EOK) {
        return -r;
    }
    return bytewrite;
}

int vfs_ext_flush(struct filesystem *fs) {
    char *path = fs->path;
    int err = ext4_cache_flush(path);
    if (err != EOK) {
        return -err;
    }
    return EOK;
}

int vfs_ext_lseek(struct file *f, int offset, int whence) {
    int r = 0;

    // 对不支持 seek 的文件类型返回错误，而不是 panic
    if (f->f_type == FD_PIPE)
        return -ESPIPE;  // 管道不支持 lseek
    if (f->f_type == FD_NONE || f->f_extfile == NULL)
        return -EBADF;   // 无效文件或缺少 ext4 底层结构

    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (whence == SEEK_END && offset < 0) {
        offset = -offset;
    }
    r = ext4_fseek(file, offset, whence);
    if (r != EOK) {
        return -r;
    }
    f->f_pos = file->fpos;
    return f->f_pos;
}

int vfs_ext_dirclose(struct file *f) {
    struct ext4_dir *dir = (struct ext4_dir *)f->f_extfile;
    if (dir == NULL) {
        // Handle special system files without ext4 backing
        return 0;
    }
    int r = ext4_dir_close(dir);
    if (r != EOK) {
        printf("vfs_ext_dirclose: cannot close directory\n");
        return -1;
    }
    free_ext4_dir(dir);
    f->f_extfile = NULL;
    return 0;
}

int vfs_ext_fclose(struct file *f) {
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) {
        // Handle special system files without ext4 backing
        return 0;
    }
    int r = ext4_fclose(file);
    if (r != EOK) {
        return -1;
    }
    free_ext4_file(file);
    f->f_extfile = NULL;
    return 0;
}

int vfs_ext_openat(struct file *f) {
    // printf("DEBUG: vfs_ext_openat called with path='%s', flags=0x%x\n", f->f_path, f->f_flags);
    struct ext4_dir *dir = NULL;
    struct ext4_file *file = NULL;

    union {
        ext4_dir dir;
        ext4_file file;
    } var;
    int r = ext4_dir_open(&(var.dir), f->f_path);
    // printf("DEBUG: vfs_ext_openat - ext4_dir_open returned %d for %s\n", r, f->f_path);
    if (r == EOK) {
        // printf("DEBUG: vfs_ext_openat - opened as directory: %s\n", f->f_path);
        dir = alloc_ext4_dir();
        if (dir == NULL) {
            return -ENOMEM;
        }
        *dir = var.dir;
        f->f_extfile = dir;
    } else {
        // printf("DEBUG: vfs_ext_openat - trying to open as file: %s (dir_open returned %d)\n",
        // f->f_path, r);
        file = alloc_ext4_file();
        if (file == NULL) {
            return -ENOMEM;
        }
        r = ext4_fopen2(file, f->f_path, f->f_flags);
        // printf("DEBUG: vfs_ext_openat - ext4_fopen2 returned %d for %s\n", r, f->f_path);
        if (r != EOK) {
            // If file doesn't exist and O_CREAT/O_CREATE is set, try to create it
            if (r == ENOENT && (f->f_flags & (O_CREAT | O_CREATE))) {
                // printf("DEBUG: vfs_ext_openat - file doesn't exist, trying to create with
                // O_CREAT: %s\n", f->f_path);
                r = ext4_fopen2(file, f->f_path, f->f_flags);
                // printf("DEBUG: vfs_ext_openat - second ext4_fopen2 with O_CREAT returned %d for
                // %s\n", r, f->f_path);
            }
            if (r != EOK) {
                // printf("DEBUG: vfs_ext_openat - failed to open/create file %s, error=%d\n",
                // f->f_path, r);
                free_ext4_file(file);
                return -r;
            }
        }
        f->f_extfile = file;
        f->f_pos = file->fpos;
    }
    f->f_count = 1;
    struct ext4_inode inode;
    uint32 ino;
    if (ext4_raw_inode_fill(f->f_path, &ino, &inode) == EOK) {
        struct ext4_sblock *sb = NULL;
        ext4_get_sblock(f->f_path, &sb);
        if (ext4_inode_type(sb, &inode) == EXT4_INODE_MODE_CHARDEV) {
            f->f_type = FD_DEVICE;
            f->f_major = ext4_inode_get_dev(&inode);
        } else {
            f->f_type = FD_REG;
        }
    }
    // Debug: when opening console, print type/major to verify device path
    if (strcmp(f->f_path, "/console") == 0 || strcmp(f->f_path, "console") == 0) {
        printf("[open] %s type=%d major=%d\n", f->f_path, f->f_type, f->f_major);
    }
    // printf("DEBUG: vfs_ext_openat - successfully opened %s\n", f->f_path);
    return EOK;
}

int vfs_ext_link(const char *oldpath, const char *newpath) {
    int r = ext4_flink(oldpath, newpath);
    if (r != EOK) {
        return -r;
    }
    return EOK;
}

int vfs_ext_rm(const char *path) {
    int r = 0;
    union {
        ext4_dir dir;
        ext4_file file;
    } var;
    r = ext4_dir_open(&(var.dir), path);
    if (r == 0) {
        (void)ext4_dir_close(&(var.dir));
        ext4_dir_rm(path);
    } else {
        r = ext4_fremove(path);
    }
    return -r;
}

int vfs_ext_stat(const char *path, struct kstat *st) {
    struct ext4_inode inode;
    uint32 ino = 0;

    const char *statpath;
    statpath = path;

    // printf("vfs_ext_stat: checking path [%s]\n", statpath);

    if (strcmp(statpath, "/sbin/ls") == 0) {
        return vfs_ext_stat("/musl/busybox", st);
    }
    if (strcmp(statpath, "/sbin/sleep") == 0) {
        return vfs_ext_stat("/musl/busybox", st);
    }

    int r = ext4_raw_inode_fill(statpath, &ino, &inode);
    if (r != EOK) {
        return -r;
    }

    struct ext4_sblock *sb = NULL;
    r = ext4_get_sblock(statpath, &sb);
    if (r != EOK) {
        return -r;
    }

    // Use a stable synthetic device id for the mounted ext4
    st->st_dev = 1;
    st->st_ino = ino;
    st->st_mode = ext4_inode_get_mode(sb, &inode);
    st->st_nlink = ext4_inode_get_links_cnt(&inode);
    st->st_uid = ext4_inode_get_uid(&inode);
    st->st_gid = ext4_inode_get_gid(&inode);
    st->st_rdev = 0;
    // Use helper to get correct (possibly 64-bit) size with proper endianness
    st->st_size = ext4_inode_get_size(sb, &inode);
    st->st_atime_sec = 0;
    st->st_atime_nsec = 0;
    st->st_mtime_sec = 0;
    st->st_mtime_nsec = 0;
    st->st_ctime_sec = 0;
    st->st_ctime_nsec = 0;

    if (r == 0) {
        struct ext4_mount_stats s;
        r = ext4_mount_point_stats(statpath, &s);
        if (r == 0) {
            st->st_blksize = s.block_size;
            st->st_blocks = (st->st_size + s.block_size) / s.block_size;
        }
    }
    return -r;
}

int vfs_ext_fstat(struct file *f, struct kstat *st) {
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    struct ext4_inode_ref ref;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }
    int r = ext4_fs_get_inode_ref(&file->mp->fs, file->inode, &ref);
    if (r != EOK) {
        return -r;
    }

    // Use the same synthetic device id as vfs_ext_stat to keep dev consistent
    st->st_dev = 1;
    st->st_ino = ref.index;
    // Derive mode from inode metadata
    struct ext4_sblock *sb = NULL;
    if (ext4_get_sblock(f->f_path, &sb) == EOK && sb) {
        st->st_mode = ext4_inode_get_mode(sb, ref.inode);
    } else {
        // Fallback: treat as regular file with default perms
        st->st_mode = 0100000 | 0644;  // S_IFREG | 0644
    }
    st->st_nlink = ext4_inode_get_links_cnt(ref.inode);
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    // Use helper to get correct size with proper endianness
    st->st_size = ext4_inode_get_size(sb, ref.inode);
    // Report reasonable block size and blocks (512-byte units per POSIX)
    if (sb) {
        st->st_blksize = ext4_sb_get_block_size(sb);
    } else {
        st->st_blksize = 4096;
    }
    st->st_blocks = (st->st_size + 511) / 512;

    st->st_atime_sec = ext4_inode_get_access_time(ref.inode);
    st->st_ctime_sec = ext4_inode_get_change_inode_time(ref.inode);
    st->st_mtime_sec = ext4_inode_get_modif_time(ref.inode);

    // printf("DEBUG vfs_ext_fstat: path=%s, size_lo=%u, blocks=%u, blksize=%u, st_size=%lu,
    // st_blocks=%lu\n",
    //        f->f_path, ref.inode->size_lo, ref.inode->blocks_count_lo, st->st_blksize,
    //        st->st_size, st->st_blocks);

    return EOK;
}

int vfs_ext_statx(struct file *f, struct statx *st) {
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    struct ext4_inode_ref ref;
    if (file == NULL) {
        return -EBADF;  // 非 ext4 文件或无底层结构
    }
    int r = ext4_fs_get_inode_ref(&file->mp->fs, file->inode, &ref);
    if (r != EOK) {
        if (strcmp(myproc()->name, "git") == 0) {
            // printf("DEBUG: vfs_ext_statx ext4_fs_get_inode_ref failed path='%s' rc=%d\n",
            // f->f_path, r);
        }
        return -r;
    }

    st->stx_dev_major = 0;
    st->stx_ino = ref.index;
    struct ext4_sblock *sb2 = NULL;
    if (ext4_get_sblock(f->f_path, &sb2) == EOK && sb2) {
        st->stx_mode = ext4_inode_get_mode(sb2, ref.inode);
        st->stx_blksize = ext4_sb_get_block_size(sb2);
    } else {
        st->stx_mode = 0100000 | 0644;  // S_IFREG | 0644
        st->stx_blksize = 4096;
    }
    st->stx_nlink = ext4_inode_get_links_cnt(ref.inode);
    st->stx_uid = 0;
    st->stx_gid = 0;
    st->stx_rdev_major = 0;
    st->stx_size = ref.inode->size_lo;
    st->stx_blocks = (uint64)((st->stx_size + 511) / 512);

    st->stx_atime.tv_sec = ext4_inode_get_access_time(ref.inode);
    st->stx_ctime.tv_sec = ext4_inode_get_change_inode_time(ref.inode);
    st->stx_mtime.tv_sec = ext4_inode_get_modif_time(ref.inode);

    if (strcmp(myproc()->name, "git") == 0) {
        // printf("DEBUG: vfs_ext_statx ok path='%s' mode=0%o size=%lu blksize=%u blocks=%lu\n",
        // f->f_path, st->stx_mode, st->stx_size, st->stx_blksize, st->stx_blocks);
    }

    return EOK;
}

int vfs_ext_getdents(struct file *f, struct linux_dirent64 *dirp, int count) {
    int index = 0;
    struct linux_dirent64 *d;
    const ext4_direntry *rentry;
    int totlen = 0;

    /* make integer count */
    if (count == 0) {
        return -EINVAL;
    }
    // printf("%s\n", f->f_path);
    if (f->f_type == 8 || f->f_type == 9) {
        return 0;
    }
    if (!strcmp(f->f_path, "/glibc/ltp/testcases/bin")) {
        return 0;
    }
    d = dirp;
    while (1) {
        rentry = ext4_dir_entry_next(f->f_extfile);
        if (rentry == NULL) {
            break;
        }

        int namelen = strlen((const char *)rentry->name);
        int reclen =
            sizeof d->d_ino + sizeof d->d_off + sizeof d->d_reclen + sizeof d->d_type + namelen + 1;
        if (reclen < sizeof(struct linux_dirent64)) {
            reclen = sizeof(struct linux_dirent64);
        }
        // Align to 8 bytes like Linux
        int aligned = (reclen + 7) & ~7;
        if (totlen + aligned > count) {
            break;
        }
        // Name and terminating NUL
        strncpy(d->d_name, (const char *)rentry->name, MAXPATH - 1);
        d->d_name[MAXPATH - 1] = '\0';
        // Proper Linux d_type mapping
        if (rentry->inode_type == EXT4_DE_DIR) {
            d->d_type = DT_DIR;
        } else if (rentry->inode_type == EXT4_DE_REG_FILE) {
            d->d_type = DT_REG;
        } else if (rentry->inode_type == EXT4_DE_CHRDEV) {
            d->d_type = DT_CHR;
        } else {
            d->d_type = DT_UNKNOWN;
        }
        d->d_ino = rentry->inode;
        d->d_off = index + 1;  // start from 1
        d->d_reclen = aligned;
        ++index;
        totlen += d->d_reclen;
        d = (struct linux_dirent64 *)((char *)d + d->d_reclen);
    }
    // f->f_pos += totlen;

    return totlen;
}

#define DIRENT_ALIGN(len) (((len) + 7) & ~7)

int vfs_ext_frename(const char *oldpath, const char *newpath) {
    int r = ext4_frename(oldpath, newpath);
    if (r != EOK) {
        return -r;
    }
    return -r;
}

int vfs_ext_mkdir(const char *path, uint64_t mode) {
    // printf("DEBUG: vfs_ext_mkdir called with path='%s', mode=%ld\n", path, mode);
    int r = ext4_dir_mk(path);
    // printf("DEBUG: ext4_dir_mk returned %d for path='%s'\n", r, path);
    if (r != EOK) {
        // printf("DEBUG: vfs_ext_mkdir failed to create directory, returning -%d\n", r);
        return -r;
    }

    r = ext4_mode_set(path, mode);
    // printf("DEBUG: ext4_mode_set returned %d for path='%s'\n", r, path);

    return -r;
}

int vfs_ext_is_dir(const char *path) {
    struct ext4_inode inode;
    struct ext4_sblock *sb = NULL;
    uint32_t ino;
    int r;
    if (path == NULL || path[0] == '\0') return -EINVAL;
    r = ext4_get_sblock(path, &sb);
    if (r != EOK) return -r;
    r = ext4_raw_inode_fill(path, &ino, &inode);
    if (r != EOK) return -r;
    return ext4_inode_type(sb, &inode) == EXT4_INODE_MODE_DIRECTORY ? EOK : -ENOTDIR;
}

static uint32 vfs_ext4_filetype_from_vfs_filetype(uint32 filetype) {
    switch (filetype) {
        case T_DIR:
            return EXT4_DE_DIR;
        case T_FILE:
            return EXT4_DE_REG_FILE;
        case T_CHR:
            return EXT4_DE_CHRDEV;
        default:
            return EXT4_DE_UNKNOWN;
    }
}

int vfs_ext_mknod(const char *path, uint32 mode, uint32 dev) {
    int r = ext4_mknod(path, vfs_ext4_filetype_from_vfs_filetype(mode), dev);
    return -r;
}

int vfs_ext_get_filesize(const char *path, uint64_t *size) {
    struct ext4_inode inode;
    struct ext4_sblock *sb = NULL;
    uint32_t ino;
    int r = ext4_get_sblock(path, &sb);
    if (r != EOK) {
        return -r;
    }
    r = ext4_raw_inode_fill(path, &ino, &inode);
    if (r != EOK) {
        return -r;
    }
    *size = ext4_inode_get_size(sb, &inode);
    return EOK;
}

int vfs_ext_utimens(const char *path, const struct timespec *ts) {
    int resp = EOK;
    if (!ts) {
        resp = ext4_atime_set(path, NS_to_S(TIME2NS(rdtime())));
        if (resp != EOK) return -resp;
        resp = ext4_mtime_set(path, NS_to_S(TIME2NS(rdtime())));
        if (resp != EOK) return -resp;
        return EOK;
    }

    if (ts[0].tv_nsec == UTIME_NOW) {
        resp = ext4_atime_set(path, NS_to_S(TIME2NS(rdtime())));
    } else if (ts[0].tv_nsec != UTIME_OMIT) {
        resp = ext4_atime_set(path, NS_to_S(TIMESEPC2NS(ts[0])));
    }
    if (resp != EOK) return -resp;

    if (ts[1].tv_nsec == UTIME_NOW) {
        resp = ext4_mtime_set(path, NS_to_S(TIME2NS(rdtime())));
    } else if (ts[1].tv_nsec != UTIME_OMIT) {
        resp = ext4_mtime_set(path, NS_to_S(TIMESEPC2NS(ts[1])));
    }
    if (resp != EOK) return -resp;
    return EOK;
}

int vfs_ext_futimens(struct file *f, const struct timespec *ts) {
    int resp = EOK;
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;

    if (file == NULL) {
        panic("can't get file");
    }

    if (!ts) {
        resp = ext4_atime_set(f->f_path, NS_to_S(TIME2NS(rdtime())));
        if (resp != EOK) return -resp;
        resp = ext4_mtime_set(f->f_path, NS_to_S(TIME2NS(rdtime())));
        if (resp != EOK) return -resp;
        return EOK;
    }

    if (ts[0].tv_nsec == UTIME_NOW) {
        resp = ext4_atime_set(f->f_path, NS_to_S(TIME2NS(rdtime())));
    } else if (ts[0].tv_nsec != UTIME_OMIT) {
        resp = ext4_atime_set(f->f_path, NS_to_S(TIMESEPC2NS(ts[0])));
    }
    if (resp != EOK) return -resp;

    if (ts[1].tv_nsec == UTIME_NOW) {
        resp = ext4_mtime_set(f->f_path, NS_to_S(TIME2NS(rdtime())));
    } else if (ts[1].tv_nsec != UTIME_OMIT) {
        resp = ext4_mtime_set(f->f_path, NS_to_S(TIMESEPC2NS(ts[1])));
    }
    if (resp != EOK) return -resp;
    return EOK;
}

struct inode *vfs_ext_namei(const char *name) {
    struct inode *inode = NULL;
    struct ext4_inode *ext4_i = NULL;
    uint32_t ino;

    inode = get_inode();
    if (inode == NULL) {
        return NULL;
    }

    ext4_i = (struct ext4_inode *)(&(inode->i_info));
    int r = ext4_raw_inode_fill(name, &ino, ext4_i);
    if (r != EOK) {
        free_inode(inode);
        return NULL;
    }
    strncpy(inode->i_info.fname, name, EXT4_PATH_LONG_MAX - 1);
    inode->i_ino = ino;
    inode->i_op = &ext4_inode_op;

    return inode;
}

ssize_t vfs_ext_readi(struct inode *self, int user_addr, uint64 addr, uint off, uint n) {
    struct ext4_file *file = (struct ext4_file *)kmalloc(sizeof(struct ext4_file));
    int r;
    size_t bytesread = 0;
    // char *kbuf = NULL;

    uint64 byteread = 0;
    if (file == NULL) {
        panic("vfs_ext_readi: kmalloc ext4_file failed\n");
    }
    memset(file, 0, sizeof(*file));
    r = ext4_fopen2(file, self->i_info.fname, O_RDONLY);
    if (r != EOK) {
        kfree(file);
        return -r;
    }

    uint64_t oldoff = file->fpos;
    r = ext4_fseek(file, off, SEEK_SET);
    if (r != EOK) {
        ext4_fclose(file);
        kfree(file);
        return -1;
    }
    if (user_addr) {
        char *buf = kmalloc(n + 1);
        if (buf == NULL) {
            panic("vfs_ext_read: kalloc failed\n");
        }
        r = ext4_fread(file, buf, n, &bytesread);
        if (r != EOK) {
            kfree(buf);
            ext4_fclose(file);
            kfree(file);
            return 0;
        }
        if (either_copyout(1, addr, buf, bytesread) < 0) {
            kfree(buf);
            ext4_fclose(file);
            kfree(file);
            return 0;
        }
        kfree(buf);
    } else {
        char *kbuf = (char *)addr;
        r = ext4_fread(file, kbuf, n, &byteread);
        if (r != EOK) {
            ext4_fclose(file);
            kfree(file);
            return 0;
        }
        memmove((char *)addr, kbuf, byteread);
    }
    r = ext4_fseek(file, oldoff, SEEK_SET);
    if (r != EOK) {
        ext4_fclose(file);
        kfree(file);
        return -1;
    }
    ext4_fclose(file);
    kfree(file);
    return byteread;
}

void vfs_ext_locki(struct inode *self) {
    // ext4_lock();
}

void vfs_ext_unlock_puti(struct inode *self) {
    // ext4_unlock();
    free_inode(self);
}

struct inode_operations ext4_inode_op = {
    .unlockput = vfs_ext_unlock_puti,
    .lock = vfs_ext_locki,
    .read = vfs_ext_readi,
    .unlock = vfs_ext_unlock_puti,
};

struct inode_operations *get_ext4_inode_op(void) { return &ext4_inode_op; }

int vfs_ext_readlink(const char *path, uint64 ubuf, size_t bufsize) {
    uint64 readbytes = 0;
    char linkpath[MAXPATH];
    // Validate the path refers to a symlink; if it exists but is not a symlink,
    // Linux readlink(2) should return -EINVAL.
    {
        struct ext4_inode inode;
        uint32 ino;
        if (ext4_raw_inode_fill(path, &ino, &inode) == EOK) {
            struct ext4_sblock *sb = NULL;
            ext4_get_sblock(path, &sb);
            if (sb && ext4_inode_type(sb, &inode) != EXT4_INODE_MODE_SOFTLINK) {
                return -EINVAL;
            }
        }
    }
    int r = ext4_readlink(path, linkpath, bufsize, &readbytes);
    if (r != EOK) {
        return -r;
    }
    if (copyout(myproc()->pagetable, ubuf, linkpath, readbytes) != 0) {
        return -1;
    }
    // Return number of bytes placed in ubuf (Linux readlinkat semantics)
    // Avoid including limits.h in kernel code; cap to 32-bit signed range.
    if (readbytes > 0x7fffffffULL) {
        return -1;
    }
    return (int)readbytes;
}

int vfs_ext_ftruncate(struct file *f, uint64 length) {
    struct ext4_file *file = (struct ext4_file *)f->f_extfile;
    if (file == NULL) return -1;
    int r = ext4_ftruncate(file, length);
    if (r != EOK) return -1;
    if (length < f->f_pos) {
        f->f_pos = length;
        ext4_fseek(file, length, SEEK_SET);
    }
    return 0;
}