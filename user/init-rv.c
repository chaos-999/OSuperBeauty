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



void test_pre() {
    int pid;
    char *bb_testcode_local[] = {"busybox", "sh", "busybox_testcode.sh", NULL};

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
    printf("#### OS COMP TEST GROUP START libctest ####\n");

    char *libc_tests[] = {
        "argv", "basename", "clocale_mbfuncs", "clock_gettime",
        "dirname", "env", "fdopen", "fnmatch", "fscanf", "fwscanf",
        "iconv_open", "inet_pton", "mbc", "memstream",
        "qsort", "random", "search_hsearch",
        "search_insque", "search_lsearch", "search_tsearch",
        "setjmp", "snprintf", "socket", "sscanf", "sscanf_long",
        "stat", "strftime", "string", "string_memcpy", "string_memmem",
        "string_memset", "string_strchr", "string_strcspn", "string_strstr",
        "strptime", "strtod", "strtod_simple", "strtof", "strtol",
        "strtold", "swprintf", "tgmath", "time", "tls_align",
        "udiv", "ungetc", "utime", "wcsstr", "wcstol",
        "daemon_failure", "dn_expand_empty", "dn_expand_ptr_0",
        "fflush_exit", "fgets_eof", "fgetwc_buffering",
        "fpclassify_invalid_ld80", "ftello_unflushed_append",
        "getpwnam_r_crash", "getpwnam_r_errno", "iconv_roundtrips",
        "inet_ntop_v4mapped", "inet_pton_empty_last_field",
        "iswspace_null", "lrand48_signextend", "lseek_large",
        "malloc_0", "mbsrtowcs_overflow", "memmem_oob_read",
        "memmem_oob", "mkdtemp_failure", "mkstemp_failure",
        "printf_1e9_oob", "printf_fmt_g_round", "printf_fmt_g_zeros",
        "printf_fmt_n",
        "putenv_doublefree", "regex_backref_0", "regex_bracket_icase",
        "regex_ere_backref", "regex_escaped_high_byte",
        "regex_negated_range", "regexec_nosub",
        "rewind_clear_error", "rlimit_open_files",
        "scanf_bytes_consumed", "scanf_match_literal_eof",
        "scanf_nullbyte_char", "setvbuf_unget",
        "sigprocmask_internal", "sscanf_eof", "statvfs",
        "strverscmp", "syscall_sign_extend", "uselocale_0",
        "wcsncpy_read_overflow", "wcsstr_false_negative",
        NULL
    };

    for (int i = 0; libc_tests[i] != NULL; i++) {
        pid = fork();
        if (pid < 0) {
            printf("libctest: fork failed\n");
            continue;
        }
        if (pid == 0) {
            char *av[] = {"entry-static.exe", libc_tests[i], NULL};
            execve("entry-static.exe", av, NULL);
            exit(99);
        }
        printf("========== START %s %s ==========\n", "entry-static.exe", libc_tests[i]);
        int st;
        wait(&st);
        int exit_code = st & 0xff;
        if (exit_code == 0) {
            printf("Pass!\n");
        } else {
            printf("FAIL %s [status %d]\n", libc_tests[i], exit_code);
        }
        printf("========== END %s %s ==========\n", "entry-static.exe", libc_tests[i]);
    }
    printf("#### OS COMP TEST GROUP END libctest ####\n");

    // ==========================================
    // 6. libctest (manual)
    // ==========================================
    chdir(bb_path_glibc);
    printf("#### OS COMP TEST GROUP START libctest ####\n");

    for (int i = 0; libc_tests[i] != NULL; i++) {
        pid = fork();
        if (pid < 0) {
            printf("libctest: fork failed\n");
            continue;
        }
        if (pid == 0) {
            char *av[] = {"entry-static.exe", libc_tests[i], NULL};
            execve("entry-static.exe", av, NULL);
            exit(99);
        }
        printf("========== START %s %s ==========\n", "entry-static.exe", libc_tests[i]);
        int st;
        wait(&st);
        int exit_code = st & 0xff;
        if (exit_code == 0) {
            printf("Pass!\n");
        } else {
            printf("FAIL %s [status %d]\n", libc_tests[i], exit_code);
        }
        printf("========== END %s %s ==========\n", "entry-static.exe", libc_tests[i]);
    }
    printf("#### OS COMP TEST GROUP END libctest ####\n");

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
    mkdirat(AT_FDCWD, "/proc", 0755);
    int fd = openat(AT_FDCWD, "/proc/uptime", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        write(fd, "0.00 0.00\n", 9);
        close(fd);
    }

    printf("\n===== OSuperbeauty Auto Test Runner =====\n\n");

    // // ★ 创建动态链接器路径（musl busybox 等动态链接程序需要）
    // mkdirat(AT_FDCWD, "/lib", 0755);
    // symlinkat("/musl/lib/libc.so", AT_FDCWD, "/lib/ld-musl-riscv64.so.1");
    // symlinkat("/musl/lib/libc.so", AT_FDCWD, "/lib/ld-musl-riscv64-sf.so.1");

    // // ★ 为 LTP 测试脚本创建必要的命令符号链接
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/basename");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/dirname");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/sh");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/cat");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/echo");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/ls");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/grep");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/wc");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/test");
    // symlinkat("/musl/busybox", AT_FDCWD, "/bin/[");

    // // 运行 LTP 测试
    // chdir("/musl");
    // int pid = fork();
    // if (pid == 0) {
    //     char *ltp_argv[] = {"busybox", "sh", "ltp_testcode.sh", NULL};
    //     char *ltp_env[] = {"PATH=/bin:/musl:/glibc", NULL};
    //     execve("/musl/busybox", ltp_argv, ltp_env);
    //     printf("execve ltp_testcode.sh FAILED\n");
    //     exit(1);
    // }
    // wait(0);

    // chdir(bb_path_glibc);
    // int pid = fork();
    // if (pid == 0) {
    //     char *argv[] = {"busybox", "sh", "busybox_testcode.sh", 0};
    //     execve("busybox", bb_testcode, NULL);
    //     printf("execve busybox cat FAILED\n");
    //     exit(1);
    // }
    // wait(0);

    test_pre();

    printf("\nAll tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
