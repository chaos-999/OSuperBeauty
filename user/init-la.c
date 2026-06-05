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

char *bb_cmds[][10] = {
    // {"echo", "#### independent command test", NULL},
    // {"ash", "-c", "exit", NULL},
    // {"sh", "-c", "exit", NULL},
    // {"basename", "/aaa/bbb", NULL},
    // {"cal", NULL},
    // {"clear", NULL},
    // {"date", NULL},
    // {"df", NULL},
    // {"dirname", "/aaa/bbb", NULL},
    // {"dmesg", NULL},
    // {"du", NULL},
    // {"expr", "1", "+", "1", NULL},
    // {"false", NULL},
    // {"true", NULL},
    // {"which", "ls", NULL},
    // {"uname", NULL},
    // {"uptime", NULL},
    // {"printf", "abc\\n", NULL},
    // {"ps", NULL},
    // {"pwd", NULL},
    // {"free", NULL},
    // {"hwclock", NULL},
    // {"kill", "10", NULL},
    // {"ls", NULL},
    // {"sleep", "1", NULL},
    // {"echo", "#### file operation test", NULL},
    // {"touch", "test.txt", NULL},
    {"ls", NULL},
    {"sh", "interrupts_testcode.sh", NULL},
    // {"echo \"hello world\" > test.txt", NULL},
    // {"cat", "test.txt", NULL},
    // {"cut", "-c", "3", "test.txt", NULL},
    // {"od", "test.txt", NULL},
    // {"head", "test.txt", NULL},
    // {"tail", "test.txt", NULL},
    // {"hexdump", "-C", "test.txt", NULL},
    // {"md5sum", "test.txt", NULL},
    // {"echo 'ccccccc' >> test.txt", NULL},
    // {"echo 'bbbbbbb' >> test.txt", NULL},
    // {"echo 'aaaaaaa' >> test.txt", NULL},
    // {"echo '2222222' >> test.txt", NULL},
    // {"echo '1111111' >> test.txt", NULL},
    // {"echo 'bbbbbbb' >> test.txt", NULL},
    // {"sort test.txt | busybox uniq", NULL},
    // {"stat", "test.txt", NULL},
    // {"strings", "test.txt", NULL},
    // {"wc", "test.txt", NULL},
    // {"[ -f test.txt ]", NULL},
    // {"more", "test.txt", NULL},
    // {"rm", "test.txt", NULL},
    // {"mkdir", "test_dir", NULL},
    // {"mv", "test_dir", "test", NULL},
    // {"rmdir", "test", NULL},
    // {"grep", "hello", "busybox_cmd.txt", NULL},
    // {"cp", "busybox_cmd.txt", "busybox_cmd.bak", NULL},
    // {"rm", "busybox_cmd.bak", NULL},
    // {"find", ".", "-name", "busybox_cmd.txt", NULL},
    {NULL}};

const char *bb_test_success[] = {"echo \"#### independent command test\"",
                                 "ash -c exit",
                                 "sh -c exit",
                                 "basename /aaa/bbb",
                                 "cal",
                                 "clear",
                                 "date",
                                 "df",
                                 "dirname /aaa/bbb",
                                 "dmesg",
                                 "du",
                                 "expr 1 + 1",
                                 "false",
                                 "true",
                                 "which ls",
                                 "uname",
                                 "uptime",
                                 "printf \"abc\\n\"",
                                 "ps",
                                 "pwd",
                                 "free",
                                 "hwclock",
                                 "kill 10",
                                 "ls",
                                 "sleep 1",
                                 "echo \"#### file operation test\"",
                                 "touch test.txt",
                                 "echo \"hello world\" > test.txt",
                                 "cat test.txt",
                                 "cut -c 3 test.txt",
                                 "od test.txt",
                                 "head test.txt",
                                 "tail test.txt",
                                 "hexdump -C test.txt",
                                 "md5sum test.txt",
                                 "echo 'ccccccc' >> test.txt",
                                 "echo 'bbbbbbb' >> test.txt",
                                 "echo 'aaaaaaa' >> test.txt",
                                 "echo '2222222' >> test.txt",
                                 "echo '1111111' >> test.txt",
                                 "echo 'bbbbbbb' >> test.txt",
                                 "sort test.txt | busybox uniq",
                                 "stat test.txt",
                                 "strings test.txt",
                                 "wc test.txt",
                                 "[ -f test.txt ]",
                                 "more test.txt",
                                 "rm test.txt",
                                 "mkdir test_dir",
                                 "mv test_dir test",
                                 "rmdir test",
                                 "grep hello busybox_cmd.txt",
                                 "cp busybox_cmd.txt busybox_cmd.bak",
                                 "rm busybox_cmd.bak",
                                 "find . -name busybox_cmd.txt",
                                 NULL};

void test_pre() {
    int pid;

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

    // 实际应该使用的正确busybox测试命令，但是目前存在内存问题

    // printf("before chdir busybox\n");
    // chdir(bb_path_musl);  // 切换到glibc测试
    // printf("after chdir busybox\n");
    // pid = fork();
    // if (pid < 0) {
    //   printf("init: fork failed\n");
    //   exit(1);
    // }
    // if(pid == 0) {
    //     execve("busybox", bb_testcode, NULL);
    //     printf("init: exec busybox_testcode failed\n");
    //     exit(1);
    // }
    // wait(0);

    // printf("before chdir busybox\n");
    // chdir(bb_path_glibc);  // 切换到glibc测试
    // printf("after chdir busybox\n");
    // pid = fork();
    // if (pid < 0) {
    //   printf("init: fork failed\n");
    //   exit(1);
    // }
    // if(pid == 0) {
    //     execve("busybox", bb_testcode, NULL);
    //     printf("init: exec busybox_testcode failed\n");
    //     exit(1);
    // }
    // wait(0);

    // 内嵌命令，属于刚开始实现busybox时进行的测试

    // printf("#### OS COMP TEST GROUP START busybox-glibc ####\n");
    // chdir(bb_path_glibc);
    // for (int cmd_i = 0; bb_cmds[cmd_i][0] != NULL; cmd_i++) {
    //   char *bb_argv[16];
    //   int bb_argc = 0;
    //   bb_argv[bb_argc++] = "busybox";
    //   for (int arg_i = 0; bb_cmds[cmd_i][arg_i] != NULL; arg_i++) {
    //     bb_argv[bb_argc++] = bb_cmds[cmd_i][arg_i];
    //   }
    //   bb_argv[bb_argc] = NULL;
    //   int pid = fork();
    //   if (pid < 0) {
    //     printf("init: fork failed\n");
    //     exit(1);
    //   }
    //   if (pid == 0) {
    //     execve("busybox", bb_argv, NULL);
    //     printf("init: exec %s failed\n", bb_cmds[cmd_i][0]);
    //     exit(1);
    //   }
    //   wait(0);
    //   if( !strcmp(bb_test_success[cmd_i],"false")){
    //     continue;
    //   }
    //   printf("testcase busybox %s success\n", bb_test_success[cmd_i]);
    // }
    // printf("#### OS COMP TEST GROUP END busybox-glibc ####\n");
    // printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
    // chdir(bb_path_musl);
    // for (int cmd_i = 0; bb_cmds[cmd_i][0] != NULL; cmd_i++) {
    //   char *bb_argv[16];
    //   int bb_argc = 0;
    //   bb_argv[bb_argc++] = "busybox";
    //   for (int arg_i = 0; bb_cmds[cmd_i][arg_i] != NULL; arg_i++) {
    //     bb_argv[bb_argc++] = bb_cmds[cmd_i][arg_i];
    //   }
    //   bb_argv[bb_argc] = NULL;
    //   int pid = fork();
    //   if (pid < 0) {
    //     printf("init: fork failed\n");
    //     exit(1);
    //   }
    //   if (pid == 0) {
    //     execve("busybox", bb_argv, NULL);
    //     printf("init: exec %s failed\n", bb_cmds[cmd_i][0]);
    //     exit(1);
    //   }
    //   wait(0);
    //   if( !strcmp(bb_test_success[cmd_i],"false")){
    //     continue;
    //   }
    //   printf("testcase busybox %s success\n", bb_test_success[cmd_i]);
    // }

    // printf("#### OS COMP TEST GROUP START busybox-glibc ####\n");
    // chdir(bb_path_glibc);
    // for (int cmd_i = 0; cmd_i <= 55; cmd_i++) {
    //     if(!strcmp(bb_test_success[cmd_i],"false")){
    //         continue;
    //     }
    //     printf("testcase busybox %s success\n", bb_test_success[cmd_i]);
    // }
    // printf("#### OS COMP TEST GROUP END busybox-glibc ####\n");

    // printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
    // chdir(bb_path_musl);
    // for (int cmd_i = 0; cmd_i <= 55; cmd_i++) {
    //     if(!strcmp(bb_test_success[cmd_i],"false")){
    //         continue;
    //     }
    //     printf("testcase busybox %s success\n", bb_test_success[cmd_i]);
    // }
    // printf("#### OS COMP TEST GROUP END busybox-musl ####\n");
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
