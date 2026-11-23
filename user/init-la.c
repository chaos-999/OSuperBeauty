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


static void run_eval_script(const char *script) {
    int pid = fork();
    if (pid < 0) return;
    if (pid == 0) {
        char *av[] = {"busybox", "sh", (char *)script, 0};
        execve("busybox", av, 0);
        exit(127);
    }
    wait(0);
}

void test_pre() {
    static const char *scripts[] = {
        "libcbench_testcode.sh",
        "basic_testcode.sh",
        "busybox_testcode.sh",
        "libctest_testcode.sh",
        "lmbench_testcode.sh",
        "iozone_testcode.sh",
        "unixbench_testcode.sh",
        "lua_testcode.sh",
        "cyclictest_testcode.sh",
        "netperf_testcode.sh",
        "iperf_testcode.sh",
    };
    const char *dirs[] = {"/glibc", "/musl"};
    for (int d = 0; d < 2; d++) {
        if (chdir(dirs[d]) < 0) continue;
        for (unsigned i = 0; i < sizeof(scripts) / sizeof(scripts[0]); i++)
            run_eval_script(scripts[i]);
    }
    chdir("/");
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

static void __attribute__((unused)) run_libcbench_group(const char *dir) {
    chdir(dir);
    int pid = fork();
    if (pid == 0) {
        char *av[] = {"busybox", "sh", "libcbench_testcode.sh", 0};
        execve("busybox", av, 0);
        exit(1);
    }
    if (pid > 0) wait(0);
}

int main() {
    if (openat(AT_FDCWD, "console", O_RDWR, 0600) < 0) {
        mknod("console", CONSOLE, 0);
        openat(AT_FDCWD, "console", O_RDWR, 0600);
    }
    dup(0); dup(0);
    mkdirat(AT_FDCWD, "/proc", 0666);
    mkdirat(AT_FDCWD, "/tmp", 0777);

    test_pre();

    printf("\nAll tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
