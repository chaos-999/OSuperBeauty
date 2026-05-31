#ifndef __FS_PIPE_H__
#define __FS_PIPE_H__

#include "types.h"
#include "lock/lock.h"

//
// In xv6, struct file is defined in fs/vfs/file.h.
// We forward-declare it here to avoid circular dependencies,
// as file.h might need to know about struct pipe.
//
struct file;

#define PIPESIZE 512

struct pipe {
    struct spinlock lock;
    char data[PIPESIZE];
    uint nread;     // number of bytes read
    uint nwrite;    // number of bytes written
    int readopen;   // read fd is still open
    int writeopen;  // write fd is still open
};

// Functions exported from pipe.c
int pipealloc(struct file **, struct file **);
void pipeclose(struct pipe *, int);
int pipewrite(struct pipe *, uint64, int);
int piperead(struct pipe *, uint64, int);
int pipewrite_kernel(struct pipe *, uint64, int);
int piperead_kernel(struct pipe *, uint64, int);

#endif  // __FS_PIPE_H__