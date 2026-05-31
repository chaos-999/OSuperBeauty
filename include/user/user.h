struct stat;
struct kstat;

// Time structures
struct timeval {
    int64 tv_sec;  /* seconds */
    int64 tv_usec; /* microseconds */
};

struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

struct timespec {
    int64 tv_sec;  /* seconds */
    int64 tv_nsec; /* nanoseconds */
};

struct tms {
    long tms_utime;
    long tms_stime;
    long tms_cutime;
    long tms_cstime;
};

struct utsname {
    char sysname[65];  /* Operating system name */
    char nodename[65]; /* Name within network */
    char release[65];  /* OS release */
    char version[65];  /* OS version */
    char machine[65];  /* Hardware identifier */
};

struct linux_dirent64 {
    uint64 d_ino;             // 索引结点号
    int64 d_off;              // 到下一个dirent的偏移
    unsigned short d_reclen;  // 当前dirent的长度
    unsigned char d_type;     // 文linux_dirent64件类型
    char d_name[];            // 文件名
};

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int *);
int pipe2(int *);
int write(int, const void *, int);
int read(int, void *, int);
int close(int);
int kill(int);
int execve(const char *, char **, char **);
int exec(const char *, char **);
int open(const char *, int, ...);
int openat(int, const char *, int, int);
int unlinkat(int, const char *, int);
int fstat(int fd, struct kstat *);
int linkat(int, const char *, int, const char *, int);
int mkdirat(int, const char *, int);
int mknod(const char *, short, short);
int chdir(const char *);
int dup(int);
int getpid(void);
int getppid(void);
char *sbrk(int);
int sleep(int);
int uptime(void);

// ulib.c
int stat(const char *, struct kstat *);
char *strcpy(char *, const char *);
void *memmove(void *, const void *, int);
char *strchr(const char *, char c);
int strcmp(const char *, const char *);
void fprintf(int, const char *, ...);
void printf(const char *, ...);
char *gets(char *, int max);
uint strlen(const char *);
void *memset(void *, int, uint);
void *malloc(uint);
void free(void *);
int atoi(const char *);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
int clone(void (*fn)(void *), void *arg, void *stack, int stack_size, int flags);
int getcwd(char *, int);
int dup3(int, int, int);
int getdents64(int, struct linux_dirent64 *, int);
int umount2(const char *, int);
int mount(const char *, const char *, const char *, unsigned long, const void *);
int shutdown(void);
void *brk(void *addr);
int gettimeofday(struct timeval *, struct timezone *);
int nanosleep(const struct timespec *, struct timespec *);
int waitpid(int pid, int *wstatus, int options);
int sched_yield(void);
long times(struct tms *);
int uname(struct utsname *);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int set_tid_address(int *tidptr);
int getuid(void);
int setuid(int uid);
int getgid(void);
int setgid(int gid);
int geteuid(void);
int getegid(void);
int setreuid(int ruid, int euid);
int setregid(int rgid, int egid);
int fcntl(int fd, int cmd, int arg);
int exit_group(int status);
int futex(uint64 uaddr, int op, uint32 val, void *timeout, uint64 uaddr2, uint32 val3);
int copy_file_range(int fd_in, off_t *off_in, int fd_out, off_t *off_out, size_t len,
                    unsigned int flags);
int ftruncate(int fd, off_t length);
int splice(int fd_in, off_t *off_in, int fd_out, off_t *off_out, size_t len, unsigned int flags);
int symlinkat(const char *oldpath, int newfd, const char *newpath);
int fsync(int fd);
int fdatasync(int fd);

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL SIGIO
#define SIGPWR 30
#define SIGSYS 31
#define SIGUNUSED 31

// sigprocmask操作
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

typedef void (*sig_handler_t)(int);

typedef struct {
    uint64 __val[1];  // 使用__val避免命名冲突
} sigset_t;

struct sigaction {
    sig_handler_t sa_handler;
    sigset_t sa_mask;
    int sa_flags;
    void *sa_restorer;  // 为了兼容性添加
};

typedef unsigned long nfds_t;

struct pollfd {
    int fd;        /* 文件描述符 */
    short events;  /* 请求的事件 */
    short revents; /* 返回的事件 */
};

/* poll事件类型 */
#define POLLIN 0x001   /* 可读 */
#define POLLPRI 0x002  /* 有紧急数据可读 */
#define POLLOUT 0x004  /* 可写 */
#define POLLERR 0x008  /* 错误条件 */
#define POLLHUP 0x010  /* 挂起 */
#define POLLNVAL 0x020 /* 无效请求 */

struct iovec {
    void *iov_base;
    uint iov_len;
};

struct rlimit64 {
    unsigned long rlim_cur;
    unsigned long rlim_max;
};

struct sysinfo {
    uint64 uptime;   // 系统运行时间
    uint64 freeram;  // 空闲内存
    uint64 procs;    // 进程数
};

int kills(int pid, int sig);
int tkill(int pid, int sig);
int tgkill(int pid, int tid, int sig);
int rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int rt_sigtimedwait(void);
int rt_sigreturn(void);
int sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask);
int gettid(void);
int writev(int fd, const struct iovec *iov, int iovcnt);
int set_robust_list(void *addr, size_t len);
int readlinkat(int fd, const char *pathname, char *buf, size_t bufsiz);
int clock_gettime(int clk_id, struct timespec *tp);
int prlimit64(int pid, int resource, const struct rlimit64 *new_limit, struct rlimit64 *old_limit);
int getrandom(void *buf, size_t bufsiz, unsigned int flags);
int fstatat(int dirfd, const char *pathname, struct kstat *statbuf, int flags);
int ioctl(int fd, int request, void *arg);
int syslog(int type, const char *buf, int len);
int faccessat(int dirfd, const char *pathname, int mode, int flags);
int fchmodat(int dirfd, const char *pathname, int mode, int flags);
int sysinfo(struct sysinfo *info);
int clock_nanosleep(int clockid, int flags, const struct timespec *req, struct timespec *rem);
int fchownat(int dirfd, const char *pathname, int owner, int group, int flags);
int fchown(int fd, int owner, int group);

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x40  // 修改为与测试程序一致
#define O_TRUNC 0x200
#define O_DIRECTORY 0x004
#define O_CLOEXEC 0x008

#define AT_FDCWD -100
#define DIRSIZ 260
