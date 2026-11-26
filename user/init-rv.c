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
// bb_testcode moved to local in test_pre() to avoid global corruption
// char *bb_testcode[10] = {"busybox", "sh", "busybox_testcode.sh", NULL};
char *interrupts_testcode[10] = {"busybox", "sh", "interrupts_testcode.sh", NULL};
char *copy_file_range_testcode[10] = {"busybox", "sh", "copy-file-range_testcode.sh", NULL};
char *splice_testcode[10] = {"busybox", "sh", "splice_testcode.sh", NULL};
char *env_testcode[10] = {"busybox", "env", NULL};
char *printenv_testcode[10] = {"busybox", "printenv", NULL};
char *git_testcode[10] = {"busybox", "sh", "git_testcode.sh", NULL};
char *lua_testcode[10] = {"busybox", "sh", "lua_testcode.sh", NULL};
char *libctest_testcode[10] = {"busybox", "sh", "libctest_testcode.sh", NULL};

// char *basic_name[] = {"pipe"};

char *basic_name[] = {"brk",          "chdir",  "clone",  "close",  "dup",      "dup2",   "execve",
                      "exit",         "fork",   "fstat",  "getcwd", "getdents", "getpid", "getppid",
                      "gettimeofday", "mkdir_", "mount",  "open",   "openat",   "pipe",   "read",
                      "sleep",        "times",  "umount", "uname",  "unlink",   "wait",   "waitpid",
                      "write",        "yield",  "mmap",   "munmap"};

// char *basic_name[] = {"fork", "pipe", "wait", "waitpid", "yield"};
char *interrupt_name[] = {"interrupts-test-1", "interrupts-test-2"};
char *copy_file_range_name[] = {"copy-file-range-test-1", "copy-file-range-test-2",
                                "copy-file-range-test-3", "copy-file-range-test-4"};




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

static int has_sub(const char *s, const char *t) {
    for (; *s; s++) {
        const char *a = s, *b = t;
        while (*b && *a == *b) { a++; b++; }
        if (!*b) return 1;
    }
    return 0;
}

static const char *ltp_tests[] = {"brk01", "pipe01", "read01",
                                  "write01", "open01", "lseek01"};

static void run_ltp_dir(const char *dir, const char *group) {
    printf("#### OS COMP TEST GROUP START %s ####\n", group);
    if (chdir(dir) < 0) {
        printf("#### OS COMP TEST GROUP END %s ####\n", group);
        return;
    }
    int n = (int)(sizeof(ltp_tests) / sizeof(ltp_tests[0]));
    for (int i = 0; i < n; i++) {
        printf("RUN LTP CASE %s\n", ltp_tests[i]);
        unlinkat(AT_FDCWD, "/tmp/ltp.out", 0);
        int pid = fork();
        if (pid == 0) {
            close(1);
            close(2);
            int fd = open("/tmp/ltp.out", O_CREAT | O_TRUNC | O_RDWR, 0644);
            if (fd >= 0) {
                int f2 = dup(1);   /* dup the file to stderr as well */
                (void)f2;
            }
            char *av[] = {(char *)ltp_tests[i], 0};
            char *ev[] = {"LD_LIBRARY_PATH=/musl/lib", 0};
            execve((char *)ltp_tests[i], av, ev);
            exit(127);
        }
        int st = 0;
        if (pid > 0) wait(&st);

        int pass = 0, fail = 0, brk = 0, skip = 0;
        int fd = open("/tmp/ltp.out", O_RDONLY, 0);
        if (fd >= 0) {
            static char buf[4096];
            static char line[1024];
            int ll = 0, r;
            while ((r = read(fd, buf, sizeof(buf))) > 0) {
                for (int k = 0; k < r; k++) {
                    char c = buf[k];
                    if (c == '\n') {
                        line[ll] = 0;
                        if (has_sub(line, "TPASS")) { pass++; printf("%s\n", line); }
                        else if (has_sub(line, "TFAIL")) { fail++; printf("%s\n", line); }
                        else if (has_sub(line, "TBROK")) { brk++; printf("%s\n", line); }
                        else if (has_sub(line, "TCONF") || has_sub(line, "TSKIP")) { skip++; printf("%s\n", line); }
                        ll = 0;
                    } else if (ll < (int)sizeof(line) - 1) {
                        line[ll++] = c;
                    }
                }
            }
            if (ll > 0) {
                line[ll] = 0;
                if (has_sub(line, "TPASS")) { pass++; printf("%s\n", line); }
                else if (has_sub(line, "TFAIL")) { fail++; printf("%s\n", line); }
                else if (has_sub(line, "TBROK")) { brk++; printf("%s\n", line); }
            }
            close(fd);
        }
        printf("Summary:\npassed   %d\nfailed   %d\nbroken   %d\nskipped  %d\nwarnings 0\n\n",
               pass, fail, brk, skip);
        printf("FAIL LTP CASE %s : %d\n", ltp_tests[i], st & 0xff);
    }
    chdir("/");
    printf("#### OS COMP TEST GROUP END %s ####\n", group);
}


void test_pre() {
    int pid;
    char *bb_testcode_local[] = {"busybox", "ash", "busybox_testcode.sh", NULL};

    int basic_testcases = 32;

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
            printf("Executing basic test: %s\n", basic_name[i]);
            exec(basic_name[i], argv2);
            exit(1);
        }
        wait(0);
    }
    printf("#### OS COMP TEST GROUP END basic-glibc ####\n");

    chdir(basic_path_musl);
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

    // ==========================================
    // 3. busybox-musl
    // ==========================================
    int tmp = chdir(bb_path_musl);
    printf("chdir to %s returned %d\n", bb_path_musl, tmp);
    //printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
    }
    if (pid == 0) {
        int tmp = execve("busybox", bb_testcode_local, NULL);
        printf("init: exec busybox_testcode failed exit code%d\n",tmp);
        exit(1);
    } 
    wait(0);
    //printf("#### OS COMP TEST GROUP END busybox-musl ####\n");

    // ==========================================
    // 4. busybox-glibc
    // ==========================================
    chdir(bb_path_glibc);
    //printf("#### OS COMP TEST GROUP START busybox-glibc ####\n");
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
    } else if (pid == 0) {
        execve("busybox", bb_testcode_local, NULL);
        printf("init: exec busybox_testcode failed\n");
        exit(1);
    } else {
        wait(0);
    }
    //printf("#### OS COMP TEST GROUP END busybox-glibc ####\n");

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
    // printf("#### OS COMP TEST GROUP START libctest-musl ####\n");

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

    // for (int i = 0; libc_tests[i] != NULL; i++) {
    //     pid = fork();
    //     if (pid < 0) {
    //         printf("libctest: fork failed\n");
    //         continue;
    //     }
    //     if (pid == 0) {
    //         char *av[] = {"entry-static.exe", libc_tests[i], NULL};
    //         execve("entry-static.exe", av, NULL);
    //         exit(99);
    //     }
    //     printf("========== START %s %s ==========\n", "entry-static.exe", libc_tests[i]);
    //     int st;
    //     wait(&st);
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



// ---------- 自动扫描：在目录里找到 basic_testcode.sh 并执行 ----------
// 对应比赛要求："主动扫描磁盘，并依次运行其中每一个测试点"
// 【暂时禁用】依赖 busybox sh applet，当前 busybox 不可用，待修复后启用
#if 0
static void run_dir_testscripts(const char *dirpath) {
    int fd;
    char buf[2048];        // getdents64 的缓冲区
    char *bpos;
    int nread;
    int found = 0;

    printf("init: scanning %s for *_testcode.sh ...\n", dirpath);

    // 1. 打开目录
    fd = openat(AT_FDCWD, dirpath, O_RDONLY, 0600);
    if (fd < 0) {
        printf("init: cannot open %s, skip\n", dirpath);
        return;
    }

    // 2. 用 getdents64 遍历目录项
    char *busybox_argv[] = {"busybox", "sh", NULL, NULL};

    while ((nread = getdents64(fd, (struct linux_dirent64 *)buf, sizeof(buf))) > 0) {
        bpos = buf;
        while (bpos < buf + nread) {
            struct linux_dirent64 *de = (struct linux_dirent64 *)bpos;

            // 跳过隐藏文件，只匹配 basic_testcode.sh（限定只跑 basic）
            if (de->d_name[0] != '.' && strcmp(de->d_name, "basic_testcode.sh") == 0) {
                printf("init: running [%s]\n", de->d_name);

                // 3. chdir 到脚本所在目录（脚本里用 ./busybox 相对路径）
                chdir(dirpath);
                busybox_argv[2] = de->d_name;

                int pid = fork();
                if (pid < 0) {
                    printf("init: fork failed for %s\n", de->d_name);
                } else if (pid == 0) {
                    // 子进程：exec busybox 执行测试脚本
                    execve("busybox", busybox_argv, NULL);
                    printf("init: exec busybox sh %s failed\n", de->d_name);
                    exit(1);
                } else {
                    // 父进程：等待脚本执行完（串行）
                    wait(0);
                }
                found++;
            }
            bpos += de->d_reclen;
        }
    }
    close(fd);

    // 4. 如果一个脚本都没找到，打印提示
    if (found == 0) {
        printf("init: no *_testcode.sh found in %s\n", dirpath);
    } else {
        printf("init: %d test scripts completed in %s\n", found, dirpath);
    }
}
#endif

int main() {

    // --- 初始化 console（stdin/stdout/stderr） ---
    if (openat(AT_FDCWD, "console", O_RDWR, 0600) < 0) {
        mknod("console", CONSOLE, 0);
        openat(AT_FDCWD, "console", O_RDWR, 0600);
    }
    dup(0);  // stdout
    dup(0);  // stderr

    // --- 创建 /tmp 目录 ---
    mkdirat(AT_FDCWD, "/tmp", 0777);
    mkdirat(AT_FDCWD, "/lib", 0755);
    linkat(AT_FDCWD, "/musl/lib/libc.so", AT_FDCWD,
           "/lib/ld-musl-riscv64.so.1", 0);

    mkdirat(AT_FDCWD, "/proc", 0755);
    int fd = openat(AT_FDCWD, "/proc/uptime", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        write(fd, "0.00 0.00\n", 9);
        close(fd);
    }

    printf("\n===== OSuperbeauty Auto Test Runner =====\n\n");


    test_pre();

    // chdir(bb_path_musl);
    // int pid;
    // pid = fork();
    // char * ls_argv[] = {"busybox", "sh", "libctest_testcode.sh", NULL};
    // if (pid < 0) {
    //     printf("init: fork failed\n");
    // } else if (pid == 0) {
    //     execve("busybox", ls_argv, NULL);
    //     printf("init: exec libctest_testcode failed\n");
    //     exit(1);
    // } else {
    //     wait(0);
    // }

    printf("\nAll tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
