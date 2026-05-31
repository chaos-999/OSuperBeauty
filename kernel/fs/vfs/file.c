#include "types.h"
#include "platform.h"
#include "param.h"
#include "defs.h"
#include "lock/lock.h"
#include "fs/stat.h"
#include "fs/vfs/fs.h"
#include "fs/vfs/file.h"
#include "memlayout.h"
#include <fs/ext4/ext4_oflags.h>

#include "proc/proc.h"
#include "fs/vfs_ext4_ext.h"

#ifdef LOONGARCH
#include "dev/pci/pci.h"
#endif

struct devsw devsw[NDEV];
struct {
    struct spinlock lock;
    struct file file[NFILE];
} ftable;

struct {
    struct spinlock lock;
    struct ext4_dir dir[NFILE];
    int valid[NFILE];
} ext4_dir_table;

struct {
    struct spinlock lock;
    struct ext4_file file[NFILE];
    int valid[NFILE];
} ext4_file_table;

/* Allocate a file structure. */
struct file *filealloc(void) {
    struct file *f;

    acquire(&ftable.lock);
    for (f = ftable.file; f < ftable.file + NFILE; f++) {
        if (f->f_count == 0) {
            f->f_count = 1;
            release(&ftable.lock);
            return f;
        }
    }
    release(&ftable.lock);
    return 0;
}

/* Increment ref count for file f. */
struct file *filedup(struct file *f) {
    if (f == NULL) {
        return NULL;
    }
    acquire(&ftable.lock);
    if (f->f_count < 1) panic("filedup");
    f->f_count++;
    release(&ftable.lock);
    return f;
}

/* Close file f.  (Decrement ref count, close when reaches 0.) */
void fileclose(struct file *f) {
    struct file ff;

    acquire(&ftable.lock);
    if (f->f_count < 1) panic("fileclose");
    if (--f->f_count > 0) {
        release(&ftable.lock);
        return;
    }
    ff = *f;
    f->f_count = 0;
    f->f_type = FD_NONE;
    release(&ftable.lock);

    if (ff.f_type == FD_PIPE) {
        pipeclose(ff.f_pipe, get_fops()->writable(&ff));
    } else if (ff.f_type == FD_REG || ff.f_type == FD_DEVICE) {
        /*
         *file中需要得到filesystem的类型
         *但是这里暂时只支持EXT4
         */
        if (vfs_ext_is_dir(ff.f_path) == 0) {
            vfs_ext_dirclose(&ff);
        } else {
            vfs_ext_fclose(&ff);
        }
        if (ff.removed) {
            vfs_ext_rm(ff.f_path);
            ff.removed = 0;
        }
    }
}

/* Get metadata about file f. */
/* addr is a user virtual address, pointing to a struct stat. */
int filestat(struct file *f, uint64 addr) {
    struct proc *p = myproc();
    struct kstat st;
    // Special system files without ext4 backing
    if (f->f_extfile == NULL) {
        memset(&st, 0, sizeof(st));
        st.st_dev = 0;
        st.st_ino = 1;
        st.st_mode = 0x81A4;  // S_IFREG | 0644
        st.st_nlink = 1;
        st.st_uid = 0;
        st.st_gid = 0;
        st.st_rdev = 0;
        st.st_size = 0;
        st.st_blksize = 4096;
        st.st_blocks = 1;
        st.st_atime_sec = 0;
        st.st_ctime_sec = 0;
        st.st_mtime_sec = 0;
        if (copyout(p->pagetable, addr, (char *)(&st), sizeof(st)) < 0) return -1;
        return 0;
    }
    // Use path-based stat so it works for both files and directories
    if (vfs_ext_stat(f->f_path, &st) < 0) {
        return -1;
    }
    if (copyout(p->pagetable, addr, (char *)(&st), sizeof(st)) < 0) return -1;
    return 0;
}

int filestatx(struct file *f, uint64 addr) {
    struct proc *p = myproc();
    struct statx sx;
    struct kstat ks;
    // Special system files without ext4 backing
    if (f->f_extfile == NULL) {
        memset(&sx, 0, sizeof(sx));
        sx.stx_blksize = 4096;
        sx.stx_nlink = 1;
        sx.stx_mode = 0x81A4;  // S_IFREG | 0644
        sx.stx_ino = 1;
        sx.stx_size = 0;
        sx.stx_blocks = 1;
        if (copyout(p->pagetable, addr, (char *)(&sx), sizeof(sx)) < 0) return -1;
        return 0;
    }
    // Use path-based stat so directories are handled
    if (vfs_ext_stat(f->f_path, &ks) < 0) {
        return -1;
    }
    memset(&sx, 0, sizeof(sx));
    sx.stx_blksize = ks.st_blksize;
    sx.stx_nlink = ks.st_nlink;
    sx.stx_uid = ks.st_uid;
    sx.stx_gid = ks.st_gid;
    sx.stx_mode = (uint16)ks.st_mode;
    sx.stx_ino = ks.st_ino;
    sx.stx_size = ks.st_size;
    sx.stx_blocks = ks.st_blocks;
    sx.stx_atime.tv_sec = ks.st_atime_sec;
    sx.stx_atime.tv_nsec = (uint32)ks.st_atime_nsec;
    sx.stx_ctime.tv_sec = ks.st_ctime_sec;
    sx.stx_ctime.tv_nsec = (uint32)ks.st_ctime_nsec;
    sx.stx_mtime.tv_sec = ks.st_mtime_sec;
    sx.stx_mtime.tv_nsec = (uint32)ks.st_mtime_nsec;
    if (copyout(p->pagetable, addr, (char *)(&sx), sizeof(sx)) < 0) return -1;
    return 0;
}

int read_interrupt(struct file *f, uint64 addr, int n) {
    char buf[1024];
    int len = 0;
    acquire(&interrupt);
    uint64 virtio_counter = get_virtio();
    uint64 clock_counter = get_clock();
    uint64 uart_counter = get_uart();
    release(&interrupt);

#ifdef RISCV
    if (virtio_counter > 0) {
        // printf("get_virtio() = %lu\n", get_virtio());
        len +=
            snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", VIRTIO0_IRQ, virtio_counter);
    }
    if (clock_counter > 0) {
        // printf("get_clock() = %lu\n", get_clock());
        len += snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", CLOCK_IRQ, clock_counter);
    }
    if (uart_counter > 0) {
        // printf("get_uart() = %lu\n", get_uart());
        len += snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", UART0_IRQ, uart_counter);
    }
#elif defined(LOONGARCH)
    if (uart_counter > 0) {
        // printf("get_uart() = %lu\n", get_uart());
        len += snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", UART0_IRQ, uart_counter);
    }
    if (clock_counter > 0) {
        // printf("get_clock() = %lu\n", get_clock());
        len += snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", CLOCK_IRQ, clock_counter);
    }
    if (virtio_counter > 0) {
        // printf("get_virtio() = %lu\n", get_virtio());
        len += snprintf(buf + len, sizeof(buf) - len, "%d:        %lu\n", PCIE_IRQ, virtio_counter);
    }
#endif

    if (f->f_pos >= len) return 0;  // EOF
    int remain = len - f->f_pos;
    int tocopy = (remain < n) ? remain : n;
    if (copyout(myproc()->pagetable, addr, buf + f->f_pos, tocopy) < 0) return -1;
    f->f_pos += tocopy;
    return tocopy;
}

/* Read from file f. */
/* addr is a user virtual address. */
int fileread(struct file *f, uint64 addr, int n) {
    int r = 0;

#ifdef LA2K1000
    if (f == NULL) {
        printf("[DBG] fileread: NULL file* addr=%lx n=%d\n", addr, n);
        return -1;
    }
#endif

    // printf("fileread addr = %lx, n = %d\n", addr, n);

    if (get_fops()->readable(f) == 0) return -1;

    if (f->f_type == FD_INTERRUPT) {
        return read_interrupt(f, addr, n);
    }

    if (f->f_type == FD_PIPE) {
        r = piperead(f->f_pipe, addr, n);
    } else if (f->f_type == FD_DEVICE) {
        if (f->f_major < 0 || f->f_major >= NDEV || !devsw[f->f_major].read) return -1;
        r = devsw[f->f_major].read(1, addr, n);
    } else if (f->f_type == FD_REG) {
        // printf("here1\n");
        r = vfs_ext_read(f, 1, addr, n);
    } else if (f->f_type == 9) {
        char a = 0;
        copyout(myproc()->pagetable, addr, (char *)&a, sizeof(char));
        return 0;
    } else if (f->f_type == 8) {
        return 0;
    } else {
        panic("fileread");
    }
    // printf("fileread done r = %d\n", r);

    return r;
}

/* Write to file f. */
/* addr is a user virtual address. */
int filewrite(struct file *f, uint64 addr, int n) {
    int ret = 0;

#ifdef LA2K1000
    if (f == NULL) {
        printf("[DBG] filewrite: NULL file* addr=%lx n=%d\n", addr, n);
        return -1;
    }
#endif
    if (get_fops()->writable(f) == 0) return -1;

    if (f->f_type == FD_INTERRUPT) return -1;

    if (f->f_type == FD_PIPE) {
        ret = pipewrite(f->f_pipe, addr, n);
    } else if (f->f_type == FD_DEVICE) {
        // printf("filewrite: device write, major=%d, NDEV=%d\n", f->f_major, NDEV);
        if (f->f_major < 0 || f->f_major >= NDEV) {
            // printf("filewrite: invalid major number %d\n", f->f_major);
            return -1;
        }
        if (!devsw[f->f_major].write) {
            // printf("filewrite: no write function for device %d\n", f->f_major);
            return -1;
        }
        // printf("filewrite: calling device write function\n");
        ret = devsw[f->f_major].write(1, addr, n);
        // printf("filewrite: device write returned %d\n", ret);
    } else if (f->f_type == FD_REG) {
        ret = vfs_ext_write(f, 1, addr, n);
    } else {
        panic("filewrite");
    }

    return ret;
}

int filewriteat(struct file *f, uint64 addr, int n, uint64 offset) {
    int r = 0;

    if (get_fops()->writable(f) == 0) return -1;

    if (f->f_type == FD_REG) {
        r = vfs_ext_writeat(f, 0, addr, n, offset);
    } else {
        return -1;
    }

    return r;
}

int filereadat(struct file *f, uint64 addr, int n, uint64 offset) {
    int r = 0;

    if (get_fops()->readable(f) == 0) return -1;
    if (f->f_type == FD_REG) {
        r = vfs_ext_readat(f, 0, addr, n, offset);
    }
    return r;
}

char filereadable(struct file *f) {
    char readable = !(f->f_flags & O_WRONLY);
    return readable;
}

char filewriteable(struct file *f) {
    char writeable = (f->f_flags & O_WRONLY) || (f->f_flags & O_RDWR);
    return writeable;
}

struct file_operations file_ops = {
    .dup = &filedup,
    .close = &fileclose,
    .read = &fileread,
    .readat = &filereadat,
    .write = &filewrite,
    .writeat = &filewriteat,
    .fstat = &filestat,
    .statx = &filestatx,
    .writable = &filewriteable,
    .readable = &filereadable,
};

struct file_operations *get_fops() { return &file_ops; }

void fileinit(void) {
    initlock(&ftable.lock, "ftable");
    initlock(&ext4_dir_table.lock, "ext4_dir_table");
    initlock(&ext4_file_table.lock, "ext4_file_table");
    memset(ftable.file, 0, sizeof(ftable.file));
}

struct ext4_dir *alloc_ext4_dir(void) {
    int i;
    acquire(&ext4_dir_table.lock);
    for (i = 0; i < NFILE; i++) {
        if (ext4_dir_table.valid[i] == 0) {
            ext4_dir_table.valid[i] = 1;
            break;
        }
    }
    release(&ext4_dir_table.lock);
    if (i == NFILE) {
        return NULL;
    }
    return &ext4_dir_table.dir[i];
}

struct ext4_file *alloc_ext4_file(void) {
    int i;
    acquire(&ext4_file_table.lock);
    for (i = 0; i < NFILE; i++) {
        if (ext4_file_table.valid[i] == 0) {
            ext4_file_table.valid[i] = 1;
            break;
        }
    }
    release(&ext4_file_table.lock);
    if (i == NFILE) {
        return NULL;
    }
    return &ext4_file_table.file[i];
}

void free_ext4_dir(struct ext4_dir *dir) {
    int i;
    acquire(&ext4_dir_table.lock);
    for (i = 0; i < NFILE; i++) {
        if (dir == &ext4_dir_table.dir[i]) {
            ext4_dir_table.valid[i] = 0;
            release(&ext4_dir_table.lock);
            return;
        }
    }
}

void free_ext4_file(struct ext4_file *file) {
    int i;
    acquire(&ext4_file_table.lock);
    for (i = 0; i < NFILE; i++) {
        if (file == &ext4_file_table.file[i]) {
            ext4_file_table.valid[i] = 0;
            release(&ext4_file_table.lock);
            return;
        }
    }
}
