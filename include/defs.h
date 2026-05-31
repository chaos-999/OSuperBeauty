#ifndef _DEFS_H
#define _DEFS_H

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;
struct vma;

// bio.c
void binit(void);
struct buf *bread(uint, uint);
void brelse(struct buf *);
void bwrite(struct buf *);
void bpin(struct buf *);
void bunpin(struct buf *);

// console.c
void consoleinit(void);
void consoleintr(int);
void consputc(int);

// exec.c
int exec(char *, char **);

// file.c
struct file *filealloc(void);
void fileclose(struct file *);
struct file *filedup(struct file *);
void fileinit(void);
int fileread(struct file *, uint64, int n);
int filestat(struct file *, uint64 addr);
int filewrite(struct file *, uint64, int n);

// fs.c
void fsinit(int);
void futex_init(void);
int dirlink(struct inode *, char *, uint);
struct inode *dirlookup(struct inode *, char *, uint *);
struct inode *ialloc(uint, short);
struct inode *idup(struct inode *);
void iinit();
void ilock(struct inode *);
void iput(struct inode *);
void iunlock(struct inode *);
void iunlockput(struct inode *);
void iupdate(struct inode *);
int namecmp(const char *, const char *);
struct inode *namei(char *);
struct inode *nameiparent(char *, char *);
int readi(struct inode *, int, uint64, uint, uint);
void stati(struct inode *, struct stat *);
int writei(struct inode *, int, uint64, uint, uint);
void itrunc(struct inode *);

// kalloc.c
void *kalloc(void);
void kfree(void *);
void kinit(void);

// pipe.c
int pipealloc(struct file **, struct file **);
void pipeclose(struct pipe *, int);
int piperead(struct pipe *, uint64, int);
int pipewrite(struct pipe *, uint64, int);

// printf.c
int printf(char *, ...) __attribute__((format(printf, 1, 2)));
void panic(char *) __attribute__((noreturn));
void printfinit(void);
void set_text_color(const char *color);
void reset_text_color(void);
int printf_color(const char *color, char *fmt, ...);
int printf_bold(char *fmt, ...);
int printf_highlight(const char *color, char *fmt, ...);
void backtrace(void);

// proc.c
int cpuid(void);
void exit(int);
int fork(void);
int growproc(int);
void proc_mapstacks(pagetable_t);
pagetable_t proc_pagetable(struct proc *);
void proc_freepagetable(pagetable_t, uint64);
int kill(int);
int killed(struct proc *);
void setkilled(struct proc *);
struct cpu *mycpu(void);
struct cpu *getmycpu(void);
struct proc *myproc();
void procinit(void);
void scheduler(void) __attribute__((noreturn));
void sched(void);
void sleep(void *, struct spinlock *);
void userinit(void);
int wait(uint64);
void wakeup(void *);
void yield(void);
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void procdump(void);
uint64 countprocs(void);

// swtch.S
void swtch(struct context *, struct context *);

// spinlock.c
void acquire(struct spinlock *);
int holding(struct spinlock *);
void initlock(struct spinlock *, char *);
void release(struct spinlock *);
void push_off(void);
void pop_off(void);

// sleeplock.c
void acquiresleep(struct sleeplock *);
void releasesleep(struct sleeplock *);
int holdingsleep(struct sleeplock *);
void initsleeplock(struct sleeplock *, char *);

// string.c
int memcmp(const void *, const void *, uint);
void *memmove(void *, const void *, uint);
void *memset(void *, int, uint);
int memcpy(void *, const void *, uint);
char *strcpy(char *, const char *);
int strlen(const char *);
size_t strnlen(const char *s, size_t count);
int strncmp(const char *, const char *, uint);
int strcmp(const char *, const char *);
char *strncpy(char *, const char *, int);
char *strchr(const char *, int);
int str_split(const char *, char, char *, char *);
char *strcat(char *dest, const char *src);
char *safestrcpy(char *dest, const char *src, int n);
int snprintf(char *str, size_t size, const char *fmt, ...);

// syscall.c
void argint(int, int *);
int argstr(int, char *, int);
void argaddr(int, uint64 *);
int fetchstr(uint64, char *, int);
int fetchaddr(uint64, uint64 *);
void syscall();

// sysfile.c
int argfd(int, int *, struct file **);
int fdalloc(struct file *);
int fdalloc2(struct file *, int);
int copyfd(struct file *, struct file *);

// trap.c
extern uint ticks;
void trapinit(void);
void trapinithart(void);
extern struct spinlock tickslock;
extern struct spinlock interrupt;
void usertrapret(void);
uint64 get_clock(void);
uint64 get_virtio(void);
uint64 get_uart(void);

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);
void uartputc_sync(int);
int uartgetc(void);

// vm.c
void kvminit(void);
void kvminithart(void);
void kvmmap(pagetable_t, uint64, uint64, uint64, uint64);
int mappages(pagetable_t, uint64, uint64, uint64, uint64);
pagetable_t uvmcreate(void);
void uvmfirst(pagetable_t, uchar *, uint);
uint64 uvmalloc(pagetable_t, uint64, uint64, int);
uint64 uvmdealloc(pagetable_t, uint64, uint64);
int uvmcopy(pagetable_t, pagetable_t, uint64);
void uvmfree(pagetable_t, uint64);
void uvmunmap(pagetable_t, uint64, uint64, int);
void uvmclear(pagetable_t, uint64);
pte_t *walk(pagetable_t, uint64, int);
uint64 walkaddr(pagetable_t, uint64);
int copyout(pagetable_t, uint64, char *, uint64);
int copyin(pagetable_t, char *, uint64, uint64);
int copyinstr(pagetable_t, char *, uint64, uint64);
uint64 kwalkaddr(uint64 va);
void pci_map(int bus, int dev, int func, void *pages);
void vmaunmap(pagetable_t, uint64, uint64, struct vma *);

// plic.c
void plicinit(void);
void plicinithart(void);
int plic_claim(void);
void plic_complete(int);

// virtio_disk.c
void virtio_disk_init(void);
void virtio_disk_rw(struct buf *, int);
void virtio_disk_intr(void);

// start.c
void start(unsigned long hartid);

// qsort.c
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

// buddy.c
void buddy_init(void);
void *buddy_kalloc(uint64 size);
void buddy_kfree(void *ptr);
uint64 buddy_get_size(void *ptr);

// sysext4.c
uint64 sys_mount_ext4fs(void);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

// trap.c
struct vma *findvma(struct proc *p, uint64 va);
int vmatrylazytouch(uint64 va);

// add
int waitpid(int pid, uint64 wstatus_addr, int options);
int wait4(int pid, uint64 wstatus_addr, int options);
int clone(unsigned long flags, void *stack, uint64 ptid, unsigned long tls, uint64 ctid);

#define CLONE_VM 0x00000100
#define CLONE_FS 0x00000200
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_PIDFD 0x00001000
#define CLONE_PTRACE 0x00002000
#define CLONE_VFORK 0x00004000
#define CLONE_PARENT 0x00008000
#define CLONE_THREAD 0x00010000
#define CLONE_NEWNS 0x00020000
#define CLONE_SYSVSEM 0x00040000
#define CLONE_SETTLS 0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED 0x00400000
#define CLONE_UNTRACED 0x00800000
#define CLONE_CHILD_SETTID 0x01000000
#define CLONE_NEWCGROUP 0x02000000
#define CLONE_NEWUTS 0x04000000
#define CLONE_NEWIPC 0x08000000
#define CLONE_NEWUSER 0x10000000
#define CLONE_NEWPID 0x20000000
#define CLONE_NEWNET 0x40000000
#define CLONE_IO 0x80000000

// waitpid options
#define WNOHANG 1

uint64 getppid();

// LoongArch specific functions
#ifdef LOONGARCH
void virtio_probe(void);
void apic_init(void);
void extioi_init(void);
#endif

// VF2 / 2K1000 RAMDisk functions
#if defined(VF2) || defined(LA2K1000)
void ramdisk_init(void);
void ramdisk_rw(struct buf *b, int write);
#endif

#endif