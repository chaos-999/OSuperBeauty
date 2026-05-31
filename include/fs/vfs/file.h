#pragma once

#include "param.h"
#include "types.h"
#include "lock/lock.h"
#include "fs/ext4/ext4.h"

struct file;

struct file_operations {
    struct file *(*dup)(struct file *self);
    int (*read)(struct file *self, uint64 addr, int n);
    int (*readat)(struct file *self, uint64 addr, int n, uint64 offset);
    int (*write)(struct file *self, uint64 addr, int n);
    int (*writeat)(struct file *self, uint64 addr, int n, uint64 offset);
    void (*close)(struct file *self);
    char (*writable)(struct file *self);
    char (*readable)(struct file *self);
    int (*fstat)(struct file *self, uint64 addr);
    int (*statx)(struct file *self, uint64 addr);
};

extern struct file_operations file_ops;

struct file_vnode {
    char path[MAXPATH];    /* full path */
    struct filesystem *fs; /* filesystem */
    void *data;            /* file-specific data */
};

#define FD_INTERRUPT 18

struct file {
    enum { FD_NONE, FD_PIPE, FD_REG, FD_DEVICE } f_type;
    uint8 f_mode; /* 访问模式 */
    int f_flags;  /* 进程打开的时候的标志 (扩展为int以支持更多标志位) */
    uint64 f_pos; /* 偏移量 */
    uint16 f_count;
    short f_major;

    void *private_data;   /* 隐私数据 */
    int f_owner;          /* 拥有file的进程 */
    struct inode *f_ip;   /* FD_REG and FD_DEVICE */
    struct pipe *f_pipe;  /* FD_PIPE */
    void *f_extfile;      /* For EXT4 ext_dir || ext_file */
    char f_path[MAXPATH]; /* 文件路径 */

    uint32 removed; /* when calling sys_unlinkat, mark as removed; when file ref is 0, REMOVE it in
                       generic_fileclose */
};

struct devsw {
    int (*read)(int, uint64, int);
    int (*write)(int, uint64, int);
};

extern struct devsw devsw[];

struct iovec {
    void *iov_base;  // 缓冲区起始地址
    int iov_len;     // 缓冲区长度
};

#define IOV_MAX 1024

void fileinit(void);
struct ext4_dir *alloc_ext4_dir(void);
struct ext4_file *alloc_ext4_file(void);
void free_ext4_dir(struct ext4_dir *dir);
void free_ext4_file(struct ext4_file *file);
struct file_operations *get_fops();
struct file *filealloc(void);
