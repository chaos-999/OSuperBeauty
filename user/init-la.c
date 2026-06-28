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
char *bb_testcode[10] = {"busybox", "sh", "busybox_testcode.sh", NULL};
char *interrupts_testcode[10] = {"busybox", "sh", "interrupts_testcode.sh", NULL};
char *git_testcode[10] = {"busybox", "sh", "git_testcode.sh", NULL};
char *lua_testcode[10] = {"busybox", "sh", "lua_testcode.sh", NULL};
char *libctest_testcode[10] = {"busybox", "sh", "libctest_testcode.sh", NULL};
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
    // 5. libctest-musl
    // ==========================================

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

    chdir(bb_path_musl);
    printf("#### OS COMP TEST GROUP START libctest-musl ####\n");
    for (int i = 0; libc_tests[i] != NULL; i++) {
        pid = fork();
        if (pid < 0) { printf("libctest: fork failed\n"); continue; }
        if (pid == 0) {
            char *av[] = {"entry-static.exe", libc_tests[i], NULL};
            execve("entry-static.exe", av, NULL);
            exit(99);
        }
        int st; wait(&st);
        int exit_code = st & 0xff;
        if (exit_code == 0) {
            printf("%s PASS\n", libc_tests[i]);
        } else {
            printf("%s FAIL [exit %d]\n", libc_tests[i], exit_code);
        }
    }
    printf("#### OS COMP TEST GROUP END libctest-musl ####\n");

    // ==========================================
//     // 6. libctest-glibc
//     // ==========================================
//     chdir(bb_path_glibc);
//     printf("#### OS COMP TEST GROUP START libctest-glibc ####\n");
//     for (int i = 0; libc_tests[i] != NULL; i++) {
//         pid = fork();
//         if (pid < 0) { printf("libctest: fork failed\n"); continue; }
//         if (pid == 0) {
//             char *av[] = {"entry-static.exe", libc_tests[i], NULL};
//             execve("entry-static.exe", av, NULL);
//             exit(99);
//         }
//         int st; wait(&st);
//         int exit_code = st & 0xff;
//         if (exit_code == 0) {
//             printf("%s PASS\n", libc_tests[i]);
//         } else {
//             printf("%s FAIL [exit %d]\n", libc_tests[i], exit_code);
//         }
//     }
//     printf("#### OS COMP TEST GROUP END libctest-glibc ####\n");

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

    // basic测试：直接执行 basic 测试二进制
    test_pre();

    // interrupt / copy-file-range / splice 测试
    //test_final();

    // test_on_site();  // busybox/git 依赖未就绪，详见上方 #if 0

    printf("All tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
