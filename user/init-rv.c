#include "types.h"
#include "fs/stat.h"
#include "lock/lock.h"
#include "user/user.h"
#include "fs/fcntl.h"
#define CONSOLE 1
static const char *ltp_tests[] = {
    "abort01", "access01", "alarm07", "atof01", "brk01", "chdir04",
    "chmod01", "chmod03", "chown03", "chown05", "chroot04", "clone01",
    "clone02", "clone03", "clone04", "clone06", "clone09", "clone301",
    "close01", "close02", "confstr01", "creat01", "creat07", "creat08",
    "creat09", "dup02", "dup04", "dup202", "dup206", "fchdir01",
    "fchdir02", "fchdir03", "fchmod01", "fchmod02", "fchmod04", "fchmodat01",
    "fchmodat02", "fchown01", "fcntl01", "fcntl02", "fcntl04", "fcntl05",
    "fcntl08", "fcntl10", "fcntl12", "fcntl13", "fcntl17", "fcntl19",
    "fcntl21", "fcntl23", "fcntl29", "fcntl32", "fcntl34", "fcntl35",
    "fork03", "fork04", "fork05", "fork09", "fork10", "fptest01",
    "fptest02", "fsconfig01", "fsconfig02", "fspick01", "fstat02", "fstat03",
    "fstatfs02", "ftest06", "ftruncate01", "futimesat01", "genlog10", "getcontext01",
    "getcwd01", "getdents01", "getgroups03", "getpid01", "getppid01", "getrandom02",
    "getresgid01", "getresuid03", "getrlimit02", "getsockname01", "getsockopt02", "gettid02",
    "gettimeofday02", "getuid01", "hugefork01", "hugeshmat01", "hugeshmat02", "hugeshmctl02",
    "hugeshmdt01", "input06", "kill07", "kill11", "kill12", "kill13",
    "ksm04", "leapsec01", "link02", "link05", "llseek01", "lseek01",
    "lstat01", "mallopt01", "mem02", "mincore02", "mkdir09", "mknod04",
    "mknod05", "mknod08", "mknod09", "mmap03", "mmap04", "mmap08",
    "mmap09", "mmap13", "mmap15", "mmap16", "mmap17", "mmapstress01",
    "mmapstress03", "mountns02", "mprotect01", "mprotect02", "msgctl02", "msgctl05",
    "msgctl06", "msgget02", "msgget03", "munmap02", "nanosleep02", "newuname01",
    "nice02", "open02", "open03", "open11", "open12", "open14",
    "page02", "pause03", "pipe04", "pipe07", "pipe09", "pipe11",
    "pipe13", "prctl01", "prctl02", "prctl03", "proc01", "pwrite01",
    "pwrite03", "pwrite04", "read03", "readahead02", "readdir01", "readlink01",
    "recvfrom01", "recvmsg02", "recvmsg03", "rename01", "rename03", "rename08",
    "rename11", "rename13", "sbrk03", "semctl04", "semctl07", "semget01",
    "semget02", "semget05", "semop03", "sendmsg01", "sendmsg03", "sendto02",
    "symlink01", "setgroups01", "setitimer01", "setitimer02", "setpgid03", "setregid01",
    "setregid02", "setregid03", "setresgid01", "setresgid03", "setresuid03", "setreuid02",
    "setreuid04", "setreuid07", "setrlimit02", "setrlimit05", "setsockopt03"
};
static int has_sub(const char *s, const char *t) { for(;*s;s++){const char*a=s,*b=t;while(*b&&*a==*b){a++;b++;}if(!*b)return 1;} return 0; }
static void run_ltp(const char *dir) {
    printf("#### OS COMP TEST GROUP START ltp-musl ####\n");
    if (chdir(dir) < 0) { printf("#### OS COMP TEST GROUP END ltp-musl ####\n"); return; }
    int n=(int)(sizeof(ltp_tests)/sizeof(ltp_tests[0]));
    for (int i=0;i<n;i++) {
        printf("RUN LTP CASE %s\n", ltp_tests[i]);
        unlinkat(AT_FDCWD,"/tmp/ltp.out",0);
        int pid=fork();
        if (pid==0) { close(1); close(2);
            int fd=open("/tmp/ltp.out",O_CREAT|O_TRUNC|O_RDWR,0644);
            if(fd>=0){int f2=dup(1);(void)f2;}
            char *av[]={(char*)ltp_tests[i],0}; char *ev[]={"LD_LIBRARY_PATH=/musl/lib",0};
            execve((char*)ltp_tests[i],av,ev); exit(127);
        }
        int st=0; if(pid>0) wait(&st);
        int pass=0,fail=0,brk=0,skip=0;
        int fd=open("/tmp/ltp.out",O_RDONLY,0);
        if(fd>=0){ static char buf[4096]; static char line[1024]; int ll=0,r;
            while((r=read(fd,buf,sizeof(buf)))>0){ for(int k=0;k<r;k++){ char c=buf[k];
                if(c=='\n'){ line[ll]=0;
                    if(has_sub(line,"TPASS")){pass++;printf("%s\n",line);} else if(has_sub(line,"TFAIL")){fail++;} else if(has_sub(line,"TBROK")){brk++;} else if(has_sub(line,"TCONF")||has_sub(line,"TSKIP")){skip++;}
                    ll=0; } else if(ll<(int)sizeof(line)-1) line[ll++]=c; } }
            close(fd); }
        printf("Summary: passed %d failed %d broken %d skipped %d\n",pass,fail,brk,skip);
        printf("FAIL LTP CASE %s : %d\n", ltp_tests[i], st & 0xff);
    }
    chdir("/");
    printf("#### OS COMP TEST GROUP END ltp-musl ####\n");
}
int main() {
    if (openat(AT_FDCWD,"console",O_RDWR,0600)<0){mknod("console",CONSOLE,0);openat(AT_FDCWD,"console",O_RDWR,0600);}
    dup(0);dup(0);
    mkdirat(AT_FDCWD,"/tmp",0777); mkdirat(AT_FDCWD,"/lib",0755);
    linkat(AT_FDCWD,"/musl/lib/libc.so",AT_FDCWD,"/lib/ld-musl-riscv64.so.1",0);
    mkdirat(AT_FDCWD,"/proc",0755);
    printf("FULL191_START\n");
    run_ltp("/musl/ltp/testcases/bin");
    printf("FULL191_END\n");
    shutdown();
    return 0;
}
