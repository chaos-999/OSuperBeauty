#include "types.h"
#include "platform.h"
#include "defs.h"
#include "param.h"
#include "lock/lock.h"
#include "proc/proc.h"
#include "fs/fcntl.h"
#include "fs/vfs/file.h"

#define PIPESIZE 512

struct pipe {
    struct spinlock lock;
    char data[PIPESIZE];
    uint nread;     // number of bytes read
    uint nwrite;    // number of bytes written
    int readopen;   // read fd is still open
    int writeopen;  // write fd is still open
};

int pipealloc(struct file **f0, struct file **f1) {
    struct pipe *pi;

    pi = 0;
    *f0 = *f1 = 0;
    if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0) goto bad;
    if ((pi = (struct pipe *)kalloc()) == 0) goto bad;
    pi->readopen = 1;
    pi->writeopen = 1;
    pi->nwrite = 0;
    pi->nread = 0;
    initlock(&pi->lock, "pipe");
    (*f0)->f_type = FD_PIPE;
    (*f0)->f_flags = O_RDONLY;
    (*f0)->f_pipe = pi;
    (*f1)->f_type = FD_PIPE;
    (*f1)->f_flags = O_WRONLY;
    (*f1)->f_pipe = pi;
    return 0;

bad:
    if (pi) kfree((char *)pi);
    if (*f0) get_fops()->close(*f0);
    if (*f1) get_fops()->close(*f1);
    return -1;
}

void pipeclose(struct pipe *pi, int writable) {
    acquire(&pi->lock);
    if (writable) {
        pi->writeopen = 0;
        wakeup(&pi->nread);
    } else {
        pi->readopen = 0;
        wakeup(&pi->nwrite);
    }
    if (pi->readopen == 0 && pi->writeopen == 0) {
        release(&pi->lock);
        kfree((char *)pi);
    } else
        release(&pi->lock);
}

int pipewrite(struct pipe *pi, uint64 addr, int n) {
    int i = 0;
    struct proc *pr = myproc();

    acquire(&pi->lock);
    while (i < n) {
        if (pi->readopen == 0 || killed(pr)) {
            release(&pi->lock);
            return -1;
        }
        if (pi->nwrite == pi->nread + PIPESIZE) {
            wakeup(&pi->nread);
            sleep(&pi->nwrite, &pi->lock);
        } else {
            char ch;
            if (copyin(pr->pagetable, &ch, addr + i, 1) == -1) break;
            pi->data[pi->nwrite++ % PIPESIZE] = ch;
            i++;
        }
    }
    wakeup(&pi->nread);
    release(&pi->lock);

    return i;
}

int piperead(struct pipe *pi, uint64 addr, int n) {
    int i;
    struct proc *pr = myproc();
    char ch;

    acquire(&pi->lock);
    while (pi->nread == pi->nwrite && pi->writeopen) {
        if (killed(pr)) {
            release(&pi->lock);
            return -1;
        }
        sleep(&pi->nread, &pi->lock);
    }
    for (i = 0; i < n; i++) {
        if (pi->nread == pi->nwrite) break;
        ch = pi->data[pi->nread++ % PIPESIZE];
        if (copyout(pr->pagetable, addr + i, &ch, 1) == -1) break;
    }
    wakeup(&pi->nwrite);
    release(&pi->lock);
    return i;
}

int pipewrite_kernel(struct pipe *pi, uint64 addr, int n) {
    int bytes_written = 0;
    struct proc *current_proc = myproc();

    acquire(&pi->lock);

    for (bytes_written = 0; bytes_written < n;) {
        // Check if pipe is still readable or process was killed
        if (pi->readopen == 0 || killed(current_proc)) {
            release(&pi->lock);
            return -1;
        }

        // Wait if pipe buffer is full
        if (pi->nwrite == pi->nread + PIPESIZE) {
            wakeup(&pi->nread);
            sleep(&pi->nwrite, &pi->lock);
            continue;
        }

        // Write single byte to pipe buffer
        char byte_to_write = ((char *)addr)[bytes_written];
        pi->data[pi->nwrite++ % PIPESIZE] = byte_to_write;
        bytes_written++;
    }

    // Wake up any waiting readers
    wakeup(&pi->nread);
    release(&pi->lock);

    return bytes_written;
}

int piperead_kernel(struct pipe *pi, uint64 addr, int n) {
    int i = 0;
    struct proc *p = myproc();

    acquire(&pi->lock);
    while (pi->nread == pi->nwrite && pi->writeopen) {
        if (killed(p)) {
            release(&pi->lock);
            return -1;
        }
        sleep(&pi->nread, &pi->lock);
    }
    for (i = 0; i < n; i++) {
        if (pi->nread == pi->nwrite) break;
        ((char *)addr)[i] = pi->data[pi->nread++ % PIPESIZE];
    }
    wakeup(&pi->nwrite);
    release(&pi->lock);
    return i;
}