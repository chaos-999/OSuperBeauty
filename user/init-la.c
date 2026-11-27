// init: The initial user-level program

#include "types.h"
#include "fs/stat.h"
#include "lock/lock.h"
#include "user/user.h"
#include "fs/fcntl.h"

#define CONSOLE 1

char *argv[] = {"sh", 0};
char *argv2[] = {"", 0};

char basic_path_musl[] = "/musl/basic/";
char basic_path_glibc[] = "/glibc/basic/";
char bb_path_musl[] = "/musl/";
char bb_path_glibc[] = "/glibc/";
char *bb_testcode[10] = {"busybox", "ash", "busybox_testcode.sh", NULL};
char *interrupts_testcode[10] = {"busybox", "sh", "interrupts_testcode.sh", NULL};
char *git_testcode[10] = {"busybox", "sh", "git_testcode.sh", NULL};
char *lua_testcode[10] = {"busybox", "sh", "lua_testcode.sh", NULL};
char *libctest_testcode[10] = {"busybox", "ash", "libctest_testcode.sh", NULL};
// char *busybox_echo_testcode[10] = {
//     "busybox", "sh", "-c", "echo \"hello world\" > README.md", NULL
//   };

// char *basic_name[] = {"pipe"};

char *basic_name[] = {"brk",          "chdir",  "clone",  "close",  "dup",      "dup2",   "execve",
                      "exit",         "fork",   "fstat",  "getcwd", "getdents", "getpid", "getppid",
                      "gettimeofday", "mkdir_", "mount",  "open",   "openat",   "pipe",   "read",
                      "sleep",        "times",  "umount", "uname",  "unlink",   "wait",   "waitpid",
                      "write",        "yield",  "mmap",   "munmap"};

char *interrupt_name[] = {"interrupts-test-1", "interrupts-test-2"};
char *copy_file_range_name[] = {"copy-file-range-test-1", "copy-file-range-test-2",
                                "copy-file-range-test-3", "copy-file-range-test-4"};
char *splice_name[] = {"test_splice"};
char *test_args[] = {"1", "2", "3", "4", "5"};


/* ---------------------------------------------------------------- */
/* Optional high-value suites appended after the original groups:    */
/* libcbench (both libcs) and a small real LTP subset.               */
/* ---------------------------------------------------------------- */

static void run_libcbench_group(const char *dir, const char *script) {
    if (chdir(dir) < 0) return;
    char *av[] = {"busybox", "ash", (char *)script, 0};
    int pid = fork();
    if (pid == 0) {
        execve("busybox", av, NULL);
        exit(1);
    } else if (pid > 0) {
        wait(0);
    }
}

static int has_sub(const char *s, const char *t) {
    for (; *s; s++) { const char *a = s, *b = t; while (*b && *a == *b) { a++; b++; } if (!*b) return 1; }
    return 0;
}
static const char *ltp_tests[] = {
  "abort01",
  "access01",
  "alarm07",
  "atof01",
  "brk01",
  "chdir04",
  "chmod01",
  "chmod03",
  "chown03",
  "chown05",
  "chroot04",
  "clone01",
  "clone02",
  "clone03",
  "clone04",
  "clone06",
  "clone09",
  "clone301",
  "close01",
  "close02",
  "confstr01",
  "creat01",
  "creat07",
  "creat08",
  "creat09",
  "dup02",
  "dup04",
  "dup202",
  "dup206",
  "fchdir01",
  "fchdir02",
  "fchdir03",
  "fchmod01",
  "fchmod02",
  "fchmod04",
  "fchmodat01",
  "fchmodat02",
  "fchown01",
  "fcntl01",
  "fcntl02",
  "fcntl04",
  "fcntl05",
  "fcntl08",
  "fcntl10",
  "fcntl12",
  "fcntl13",
  "fcntl17",
  "fcntl19",
  "fcntl21",
  "fcntl23",
  "fcntl29",
  "fcntl32",
  "fcntl34",
  "fcntl35",
  "fork03",
  "fork04",
  "fork05",
  "fork09",
  "fork10",
  "fptest01",
  "fptest02",
  "fsconfig01",
  "fsconfig02",
  "fspick01",
  "fstat02",
  "fstat03",
  "fstatfs02",
  "ftest06",
  "ftruncate01",
  "futimesat01",
  "genlog10",
  "getcontext01",
  "getcwd01",
  "getdents01",
  "getgroups03",
  "getpid01",
  "getppid01",
  "getrandom02",
  "getresgid01",
  "getresuid03",
  "getrlimit02",
  "getsockname01",
  "getsockopt02",
  "gettid02",
  "gettimeofday02",
  "getuid01",
  "hugefork01",
  "hugeshmat01",
  "hugeshmat02",
  "hugeshmctl02",
  "hugeshmdt01",
  "input06",
  "kill07",
  "kill11",
  "kill12",
  "kill13",
  "ksm04",
  "leapsec01",
  "link02",
  "link05",
  "llseek01",
  "lseek01",
  "lstat01",
  "mallopt01",
  "mem02",
  "mincore02",
  "mkdir09",
  "mknod04",
  "mknod05",
  "mknod08",
  "mknod09",
  "mmap03",
  "mmap04",
  "mmap08",
  "mmap09",
  "mmap13",
  "mmap15",
  "mmap16",
  "mmap17",
  "mmapstress01",
  "mmapstress03",
  "mountns02",
  "mprotect01",
  "mprotect02",
  "msgctl02",
  "msgctl05",
  "msgctl06",
  "msgget02",
  "msgget03",
  "munmap02",
  "nanosleep02",
  "newuname01",
  "nice02",
  "open02",
  "open03",
  "open11",
  "open12",
  "open14",
  "page02",
  "pause03",
  "pipe04",
  "pipe07",
  "pipe09",
  "pipe11",
  "pipe13",
  "prctl01",
  "prctl02",
  "prctl03",
  "proc01",
  "pwrite01",
  "pwrite03",
  "pwrite04",
  "read03",
  "readahead02",
  "readdir01",
  "readlink01",
  "recvfrom01",
  "recvmsg02",
  "recvmsg03",
  "rename01",
  "rename03",
  "rename08",
  "rename11",
  "rename13",
  "sbrk03",
  "semctl04",
  "semctl07",
  "semget01",
  "semget02",
  "semget05",
  "semop03",
  "sendmsg01",
  "sendmsg03",
  "sendto02",
  "symlink01",
  "setgroups01",
  "setitimer01",
  "setitimer02",
  "setpgid03",
  "setregid01",
  "setregid02",
  "setregid03",
  "setresgid01",
  "setresgid03",
  "setresuid03",
  "setreuid02",
  "setreuid04",
  "setreuid07",
  "setrlimit02",
  "setrlimit05",
  "setsockopt03"
};
#define LTP_N (int)(sizeof(ltp_tests)/sizeof(ltp_tests[0]))
#define LTP_TIMEOUT 30
static void run_ltp_dir(const char *dir, const char *group) {
    printf("#### OS COMP TEST GROUP START %s ####\n", group);
    if (chdir(dir) < 0) { printf("#### OS COMP TEST GROUP END %s ####\n", group); return; }
    for (int i = 0; i < LTP_N; i++) {
        printf("RUN LTP CASE %s\n", ltp_tests[i]);
        unlinkat(AT_FDCWD, "/tmp/ltp.out", 0);
        int pid = fork();
        if (pid == 0) {
            close(1); close(2);
            int fd = open("/tmp/ltp.out", O_CREAT|O_TRUNC|O_RDWR, 0644);
            if (fd >= 0) { int f2 = dup(1); (void)f2; }
            char *av[] = {(char*)ltp_tests[i], 0};
            char *ev[] = {"LD_LIBRARY_PATH=/musl/lib", 0};
            execve((char*)ltp_tests[i], av, ev);
            exit(127);
        }
        int killer = -1;
        if (pid > 0) {
            killer = fork();
            if (killer == 0) { struct timespec wts = {LTP_TIMEOUT, 0}; nanosleep(&wts, 0); kill(pid); exit(0); }
        }
        int st = 0;
        if (pid > 0) waitpid(pid, &st, 0);
        if (killer > 0) { kill(killer); int kst; waitpid(killer, &kst, 0); }
        int pass=0, fail=0, brk=0, skip=0;
        int fd = open("/tmp/ltp.out", O_RDONLY, 0);
        if (fd >= 0) {
            static char buf[4096]; static char line[1024]; int ll=0, r;
            while ((r=read(fd, buf, sizeof(buf))) > 0) {
                for (int k=0;k<r;k++){ char c=buf[k];
                    if (c=='\n'){ line[ll]=0;
                        if (has_sub(line,"TPASS")){pass++;printf("%s\n",line);}
                        else if (has_sub(line,"TFAIL")){fail++;printf("%s\n",line);}
                        else if (has_sub(line,"TBROK")){brk++;printf("%s\n",line);}
                        else if (has_sub(line,"TCONF")||has_sub(line,"TSKIP")){skip++;printf("%s\n",line);}
                        ll=0;
                    } else if (ll < (int)sizeof(line)-1) line[ll++]=c;
                }
            }
            if (ll>0){ line[ll]=0; if (has_sub(line,"TPASS")){pass++;printf("%s\n",line);} }
            close(fd);
        }
        printf("Summary:\npassed   %d\nfailed   %d\nbroken   %d\nskipped  %d\nwarnings 0\n\n", pass, fail, brk, skip);
        printf("FAIL LTP CASE %s : %d\n", ltp_tests[i], st & 0xff);
    }
    chdir("/");
    printf("#### OS COMP TEST GROUP END %s ####\n", group);
}
static const char *lua_scripts[] = {"date.lua", "file_io.lua", "max_min.lua", "random.lua",
                                    "remove.lua", "round_num.lua", "sin30.lua", "sort.lua",
                                    "strings.lua"};

static void run_lua_group(const char *dir, const char *group) {
    printf("#### OS COMP TEST GROUP START %s ####\n", group);
    if (chdir(dir) < 0) {
        printf("#### OS COMP TEST GROUP END %s ####\n", group);
        return;
    }
    int n = (int)(sizeof(lua_scripts) / sizeof(lua_scripts[0]));
    for (int i = 0; i < n; i++) {
        int pid = fork();
        if (pid == 0) {
            char *av[] = {"lua", (char *)lua_scripts[i], 0};
            execve("lua", av, 0);
            exit(1);
        }
        int st = 0;
        if (pid > 0) wait(&st);
        printf("testcase lua %s %s\n", lua_scripts[i],
               (pid > 0 && (st & 0xff) == 0) ? "success" : "fail");
    }
    chdir("/");
    printf("#### OS COMP TEST GROUP END %s ####\n", group);
}

void test_pre() {
    int pid;

    int basic_testcases = 32;

    printf("before chdir basic-musl\n");
    chdir(basic_path_musl);
    printf("after chdir basic-musl\n");
    printf("#### OS COMP TEST GROUP START basic-musl ####\n");
    for (int i = 0; i < basic_testcases; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(basic_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END basic-musl ####\n");

    printf("before chdir basic-glibc\n");
    chdir(basic_path_glibc);
    printf("after chdir basic-glibc\n");
    printf("#### OS COMP TEST GROUP START basic-glibc ####\n");
    for (int i = 0; i < basic_testcases; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(basic_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END basic-glibc ####\n");

    // ==========================================
    // 3. busybox-musl
    // ==========================================
    chdir(bb_path_musl);
    // printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
    } else if (pid == 0) {
        execve("busybox", bb_testcode, NULL);
        printf("init: exec busybox_testcode failed\n");
        exit(1);
    } else {
        wait(0);
    }
    // printf("#### OS COMP TEST GROUP END busybox-musl ####\n");

    // ==========================================
    // 4. busybox-glibc
    // ==========================================
    chdir(bb_path_glibc);
    // printf("#### OS COMP TEST GROUP START busybox-glibc ####\n");
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
    } else if (pid == 0) {
        execve("busybox", bb_testcode, NULL);
        printf("init: exec busybox_testcode failed\n");
        exit(1);
    } else {
        wait(0);
    }
    // printf("#### OS COMP TEST GROUP END busybox-glibc ####\n");

    // ==========================================
    // 5. libctest (manual — runtest.exe not available, use entry-static.exe directly)
    // ==========================================
    chdir(bb_path_musl);

    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
    } else if (pid == 0) {
        execve("busybox", libctest_testcode, NULL);
        printf("init: exec libctest_testcode failed\n");
        exit(1);
    } else {
        wait(0);
    }

    // char *libc_tests[] = {
    //     "argv", "basename", "clocale_mbfuncs", "clock_gettime",
    //     "dirname", "env", "fdopen", "fnmatch", "fscanf", "fwscanf",
    //     "iconv_open", "inet_pton", "mbc", "memstream",
    //     "qsort", "random", "search_hsearch",
    //     "search_insque", "search_lsearch", "search_tsearch",
    //     "setjmp", "snprintf", "socket", "sscanf", "sscanf_long",
    //     "stat", "strftime", "string", "string_memcpy", "string_memmem",
    //     "string_memset", "string_strchr", "string_strcspn", "string_strstr",
    //     "strptime", "strtod", "strtod_simple", "strtof", "strtol",
    //     "strtold", "swprintf", "tgmath", "time", "tls_align",
    //     "udiv", "ungetc", "utime", "wcsstr", "wcstol",
    //     "daemon_failure", "dn_expand_empty", "dn_expand_ptr_0",
    //     "fflush_exit", "fgets_eof", "fgetwc_buffering",
    //     "fpclassify_invalid_ld80", "ftello_unflushed_append",
    //     "getpwnam_r_crash", "getpwnam_r_errno", "iconv_roundtrips",
    //     "inet_ntop_v4mapped", "inet_pton_empty_last_field",
    //     "iswspace_null", "lrand48_signextend", "lseek_large",
    //     "malloc_0", "mbsrtowcs_overflow", "memmem_oob_read",
    //     "memmem_oob", "mkdtemp_failure", "mkstemp_failure",
    //     "printf_1e9_oob", "printf_fmt_g_round", "printf_fmt_g_zeros",
    //     "printf_fmt_n",
    //     "putenv_doublefree", "regex_backref_0", "regex_bracket_icase",
    //     "regex_ere_backref", "regex_escaped_high_byte",
    //     "regex_negated_range", "regexec_nosub",
    //     "rewind_clear_error", "rlimit_open_files",
    //     "scanf_bytes_consumed", "scanf_match_literal_eof",
    //     "scanf_nullbyte_char", "setvbuf_unget",
    //     "sigprocmask_internal", "sscanf_eof", "statvfs",
    //     "strverscmp", "syscall_sign_extend", "uselocale_0",
    //     "wcsncpy_read_overflow", "wcsstr_false_negative",
    //     NULL
    // };

    // chdir(bb_path_musl);
    // printf("#### OS COMP TEST GROUP START libctest-musl ####\n");
    // for (int i = 0; libc_tests[i] != NULL; i++) {
    //     pid = fork();
    //     if (pid < 0) { printf("libctest: fork failed\n"); continue; }
    //     if (pid == 0) {
    //         char *av[] = {"entry-static.exe", libc_tests[i], NULL};
    //         execve("entry-static.exe", av, NULL);
    //         exit(99);
    //     }
    //     printf("========== START %s %s ==========\n", "entry-static.exe", libc_tests[i]);
    //     int st; wait(&st);
    //     int exit_code = st & 0xff;
    //     if (exit_code == 0) {
    //         printf("Pass!\n");
    //     } else {
    //         printf("FAIL %s [status %d]\n", libc_tests[i], exit_code);
    //     }
    //     printf("========== END %s %s ==========\n", "entry-static.exe", libc_tests[i]);
    // }
    // printf("#### OS COMP TEST GROUP END libctest-musl ####\n");

    chdir("/");
    run_libcbench_group("/glibc", "libcbench_testcode.sh");
    run_libcbench_group("/musl", "libcbench_testcode.sh");
    run_lua_group("/musl", "lua-musl");
    run_lua_group("/glibc", "lua-glibc");
    run_ltp_dir("/musl/ltp/testcases/bin", "ltp-musl");

    return;
}

void test_final() {
    int pid;
    chdir(bb_path_musl);
    printf("#### OS COMP TEST GROUP START interrupts-musl ####\n");
    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(interrupt_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END interrupts-musl ####\n");

    chdir(bb_path_glibc);
    printf("#### OS COMP TEST GROUP START interrupts-glibc ####\n");
    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(interrupt_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END interrupts-glibc ####\n");

    chdir(bb_path_musl);
    printf("#### OS COMP TEST GROUP START copyfilerange-musl ####\n");
    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(copy_file_range_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END copyfilerange-musl ####\n");

    chdir(bb_path_glibc);
    printf("#### OS COMP TEST GROUP START copyfilerange-glibc ####\n");
    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec(copy_file_range_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END copyfilerange-glibc ####\n");

    chdir(bb_path_musl);
    printf("#### OS COMP TEST GROUP START splice-musl ####\n");
    for (int i = 0; i < 5; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            char *test_argv[] = {splice_name[0], test_args[i], NULL};
            execve(splice_name[0], test_argv, NULL);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END splice-musl ####\n");

    chdir(bb_path_glibc);
    printf("#### OS COMP TEST GROUP START splice-glibc ####\n");
    for (int i = 0; i < 5; i++) {
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            char *test_argv[] = {splice_name[0], test_args[i], NULL};
            execve(splice_name[0], test_argv, NULL);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END splice-glibc ####\n");

    return;
}

// 【暂时禁用】git 测试依赖 busybox sh + git_testcode.sh，当前不可用，待修复后启用
#if 0
void test_on_site() {
    int pid;

    mkdirat(AT_FDCWD, "/etc", 0755);

    // Create git configuration files to avoid warnings
    int fd;

    // Create /etc/gitconfig
    fd = openat(AT_FDCWD, "/etc/gitconfig", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        // const char *syscfg = "[core]\n\tautocrlf = false\n";
        const char *syscfg =
            "[core]\n"
            "\tautocrlf = false\n"
            "\tcompression = 0\n"
            "\tlooseCompression = 0\n"
            "[pack]\n"
            "\tcompression = 0\n";
        write(fd, syscfg, strlen(syscfg));
        close(fd);
    }

    // Create /tmp/.config directory and git config
    mkdirat(AT_FDCWD, "/tmp/.config", 0755);
    mkdirat(AT_FDCWD, "/tmp/.config/git", 0755);
    fd = openat(AT_FDCWD, "/tmp/.config/git/config", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        const char *usrcfg = "[user]\n\tname = Test User\n\temail = test@example.com\n";
        write(fd, usrcfg, strlen(usrcfg));
        close(fd);
    }

    // Create /tmp/.gitconfig
    fd = openat(AT_FDCWD, "/tmp/.gitconfig", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        const char *homecfg = "[user]\n\tname = Test User\n\temail = test@example.com\n";
        write(fd, homecfg, strlen(homecfg));
        close(fd);
        // Set proper permissions for git to modify the file
        fchmodat(AT_FDCWD, "/tmp/.gitconfig", 0666, 0);
    }

    chdir(bb_path_glibc);

    char *env[] = {"HOME=/tmp", "PATH=/bin", NULL};

    printf("-------------------------------- git test --------------------------------\n");

    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", git_testcode, env);
        printf("init: exec git_testcode failed\n");
        exit(1);
    }
    wait(0);

    chdir(bb_path_musl);

    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", git_testcode, env);
        printf("init: exec git_testcode failed\n");
        exit(1);
    }
    wait(0);

    return;
}
#endif

int main() {
    if (openat(AT_FDCWD, "console", O_RDWR, 0600) < 0) {
        mknod("console", CONSOLE, 0);
        openat(AT_FDCWD, "console", O_RDWR, 0600);
    }
    dup(0);  // stdout
    dup(0);  // stderr

    mkdirat(AT_FDCWD, "/proc", 0666);
    mkdirat(AT_FDCWD, "/tmp", 0777);
    mkdirat(AT_FDCWD, "/lib64", 0755);
    linkat(AT_FDCWD, "/musl/lib/libc.so", AT_FDCWD,
           "/lib64/ld-musl-loongarch-lp64d.so.1", 0);

    // basic测试：直接执行 basic 测试二进制
    test_pre();

    // int pid;
    // chdir(bb_path_musl);
    // // printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
    // pid = fork();
    // if (pid < 0) {
    //     printf("init: fork failed\n");
    // } else if (pid == 0) {
    //     execve("busybox", libctest_testcode, NULL);
    //     printf("init: exec libctest_testcode failed\n");
    //     exit(1);
    // } else {
    //     wait(0);
    // }

    // interrupt / copy-file-range / splice 测试
    //test_final();

    // test_on_site();  // busybox/git 依赖未就绪，详见上方 #if 0

    printf("All tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
