#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include "lock/lock.h"
#include "proc/proc.h"
#include "syscall/syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64 *ip) {
    struct proc *p = myproc();
    // if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    //   return -1;
    if (copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0) return -1;
    return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char *buf, int max) {
    struct proc *p = myproc();
    if (copyinstr(p->pagetable, buf, addr, max) < 0) {
        return -1;
    }
    return strlen(buf);
}

static uint64 argraw(int n) {
    struct proc *p = myproc();
    switch (n) {
        case 0:
            return p->trapframe->a0;
        case 1:
            return p->trapframe->a1;
        case 2:
            return p->trapframe->a2;
        case 3:
            return p->trapframe->a3;
        case 4:
            return p->trapframe->a4;
        case 5:
            return p->trapframe->a5;
    }
    panic("argraw");
    return -1;
}

// Fetch the nth 32-bit system call argument.
void argint(int n, int *ip) { *ip = argraw(n); }

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void argaddr(int n, uint64 *ip) { *ip = argraw(n); }

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int argstr(int n, char *buf, int max) {
    uint64 addr;
    argaddr(n, &addr);
    int r = fetchstr(addr, buf, max);
    return r;
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_read(void);
extern uint64 sys_pread64(void);
extern uint64 sys_kill(void);
extern uint64 sys_execve(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_fchmodat(void);
extern uint64 sys_fchown(void);
extern uint64 sys_fchownat(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_getppid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_openat(void);
extern uint64 sys_write(void);
extern uint64 sys_unlinkat(void);
extern uint64 sys_linkat(void);
extern uint64 sys_mkdirat(void);
extern uint64 sys_close(void);
extern uint64 sys_mknod(void);
extern uint64 sys_exec(void);
extern uint64 sys_pipe2(void);
extern uint64 sys_getcwd(void);
extern uint64 sys_dup3(void);
extern uint64 sys_getdents64(void);
extern uint64 sys_mount(void);
extern uint64 sys_umount2(void);
extern uint64 sys_shutdown(void);
extern uint64 sys_brk(void);
extern uint64 sys_clone(void);
extern uint64 sys_gettimeofday(void);
extern uint64 sys_nanosleep(void);
extern uint64 sys_wait4(void);
extern uint64 sys_waitpid(void);
extern uint64 sys_sched_yield(void);
extern uint64 sys_times(void);
extern uint64 sys_uname(void);
extern uint64 sys_mmap(void);
extern uint64 sys_munmap(void);
extern uint64 sys_statx(void);
extern uint64 sys_set_tid_address(void);
extern uint64 sys_getuid(void);
extern uint64 sys_setuid(void);
extern uint64 sys_getgid(void);
extern uint64 sys_setgid(void);
extern uint64 sys_geteuid(void);
extern uint64 sys_getegid(void);
extern uint64 sys_setreuid(void);
extern uint64 sys_setregid(void);
extern uint64 sys_fcntl(void);
extern uint64 sys_exit_group(void);
extern uint64 sys_kills(void);
extern uint64 sys_tkill(void);
extern uint64 sys_tgkill(void);
extern uint64 sys_rt_sigaction(void);
extern uint64 sys_rt_sigprocmask(void);
extern uint64 sys_rt_sigtimedwait(void);
extern uint64 sys_rt_sigreturn(void);
extern uint64 sys_sendfile(void);
extern uint64 sys_ppoll(void);
extern uint64 sys_gettid(void);
extern uint64 sys_writev(void);
extern uint64 sys_set_robust_list(void);
extern uint64 sys_readlinkat(void);
extern uint64 sys_clock_gettime(void);
extern uint64 sys_prlimit64(void);
extern uint64 sys_getrandom(void);
extern uint64 sys_fstatat(void);
extern uint64 sys_ioctl(void);
extern uint64 sys_syslog(void);
extern uint64 sys_faccessat(void);
extern uint64 sys_sysinfo(void);
extern uint64 sys_clock_nanosleep(void);
extern uint64 sys_utimensat(void);
extern uint64 sys_readv(void);
extern uint64 sys_lseek(void);
extern uint64 sys_renameat2(void);
extern uint64 sys_madvise(void);
extern uint64 sys_mremap(void);
extern uint64 sys_mprotect(void);
extern uint64 sys_futex(void);
extern uint64 sys_getpgid(void);
extern uint64 sys_copy_file_range(void);
extern uint64 sys_ftruncate(void);
extern uint64 sys_splice(void);
extern uint64 sys_symlinkat(void);
extern uint64 sys_fsync(void);
extern uint64 sys_fdatasync(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
    [SYS_exit] sys_exit,
    [SYS_wait] sys_wait,
    [SYS_pipe2] sys_pipe2,
    [SYS_read] sys_read,
    [SYS_pread64] sys_pread64,
    [SYS_kill] sys_kill,
    [SYS_execve] sys_execve,
    [SYS_fstat] sys_fstat,
    [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,
    [SYS_getpid] sys_getpid,
    [SYS_getppid] sys_getppid,
    [SYS_sbrk] sys_sbrk,
    [SYS_sleep] sys_sleep,
    [SYS_uptime] sys_uptime,
    [SYS_openat] sys_openat,
    [SYS_write] sys_write,
    [SYS_unlinkat] sys_unlinkat,
    [SYS_linkat] sys_linkat,
    [SYS_mkdirat] sys_mkdirat,
    [SYS_close] sys_close,
    [SYS_mknod] sys_mknod,
    [SYS_exec] sys_exec,
    [SYS_getcwd] sys_getcwd,
    [SYS_dup3] sys_dup3,
    [SYS_getdents64] sys_getdents64,
    [SYS_umount2] sys_umount2,
    [SYS_mount] sys_mount,
    [SYS_shutdown] sys_shutdown,
    [SYS_brk] sys_brk,
    [SYS_clone] sys_clone,
    [SYS_gettimeofday] sys_gettimeofday,
    [SYS_nanosleep] sys_nanosleep,
    [SYS_wait4] sys_wait4,
    [SYS_waitpid] sys_waitpid,
    [SYS_sched_yield] sys_sched_yield,
    [SYS_times] sys_times,
    [SYS_uname] sys_uname,
    [SYS_mmap] sys_mmap,
    [SYS_munmap] sys_munmap,
    [SYS_statx] sys_statx,
    [SYS_set_tid_address] sys_set_tid_address,
    [SYS_getuid] sys_getuid,
    [SYS_setuid] sys_setuid,
    [SYS_getgid] sys_getgid,
    [SYS_setgid] sys_setgid,
    [SYS_geteuid] sys_geteuid,
    [SYS_getegid] sys_getegid,
    [SYS_setreuid] sys_setreuid,
    [SYS_setregid] sys_setregid,
    [SYS_fcntl] sys_fcntl,
    [SYS_exit_group] sys_exit_group,
    [SYS_kills] sys_kills,
    [SYS_tkill] sys_tkill,
    [SYS_tgkill] sys_tgkill,
    [SYS_rt_sigaction] sys_rt_sigaction,
    [SYS_rt_sigprocmask] sys_rt_sigprocmask,
    [SYS_rt_sigtimedwait] sys_rt_sigtimedwait,
    [SYS_rt_sigreturn] sys_rt_sigreturn,
    [SYS_sendfile] sys_sendfile,
    [SYS_ppoll] sys_ppoll,
    [SYS_gettid] sys_gettid,
    [SYS_writev] sys_writev,
    [SYS_set_robust_list] sys_set_robust_list,
    [SYS_readlinkat] sys_readlinkat,
    [SYS_clock_gettime] sys_clock_gettime,
    [SYS_prlimit64] sys_prlimit64,
    [SYS_getrandom] sys_getrandom,
    [SYS_fstatat] sys_fstatat,
    [SYS_ioctl] sys_ioctl,
    [SYS_syslog] sys_syslog,
    [SYS_faccessat] sys_faccessat,
    [SYS_sysinfo] sys_sysinfo,
    [SYS_clock_nanosleep] sys_clock_nanosleep,
    [SYS_lseek] sys_lseek,
    [SYS_readv] sys_readv,
    [SYS_utimensat] sys_utimensat,
    [SYS_mremap] sys_mremap,
    [SYS_mprotect] sys_mprotect,
    [SYS_madvise] sys_madvise,
    [SYS_renameat2] sys_renameat2,
    [SYS_futex] sys_futex,
    [SYS_getpgid] sys_getpgid,
    [SYS_copy_file_range] sys_copy_file_range,
    [SYS_ftruncate] sys_ftruncate,
    [SYS_splice] sys_splice,
    [SYS_fchmodat] sys_fchmodat,
    [SYS_fchown] sys_fchown,
    [SYS_fchownat] sys_fchownat,
    [SYS_symlinkat] sys_symlinkat,
    [SYS_fsync] sys_fsync,
    [SYS_fdatasync] sys_fdatasync,
};

void syscall(void) {
    int num;
    struct proc *p = myproc();

    num = p->trapframe->a7;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        // Use num to lookup the system call function for num, call it,
        // and store its return value in p->trapframe->a0
        // if(num == 73)
        //     printf("%d %s: syscall %d\n", p->pid, p->name, num);
        uint64 ret = syscalls[num]();
        // printf("ret = %lx\n", ret);
        p->trapframe->a0 = ret;
    } else {
        printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
        p->trapframe->a0 = -1;
    }
}