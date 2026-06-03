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
char *copy_file_range_testcode[10] = {"busybox", "sh", "copy-file-range_testcode.sh", NULL};
char *splice_testcode[10] = {"busybox", "sh", "splice_testcode.sh", NULL};
char *env_testcode[10] = {"busybox", "env", NULL};
char *printenv_testcode[10] = {"busybox", "printenv", NULL};
char *git_testcode[10] = {"busybox", "sh", "git_testcode.sh", NULL};

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

const char *bb_test_success[] = {
    //   "echo \"#### independent command test\"",
    //   "ash -c exit",
    //   "sh -c exit",
    //   "basename /aaa/bbb",
    //   "cal",
    //   "clear",
    //   "date",
    //   "df",
    //   "dirname /aaa/bbb",
    //   "dmesg",
    //   "du",
    //   "expr 1 + 1",
    //   "false",
    //   "true",
    "which ls",
    //   "uname",
    //   "uptime",
    //   "printf \"abc\\n\"",
    //   "ps",
    //   "pwd",
    //   "free",
    //   "hwclock",
    //   "kill 10",
    //   "ls",
    "sleep 1", "echo \"#### file operation test\"", "touch test.txt", "ls",
    "echo \"hello world\" > test.txt", "cat test.txt", "cut -c 3 test.txt", "od test.txt",
    "head test.txt", "tail test.txt", "hexdump -C test.txt", "md5sum test.txt",
    "echo 'ccccccc' >> test.txt", "echo 'bbbbbbb' >> test.txt", "echo 'aaaaaaa' >> test.txt",
    "echo '2222222' >> test.txt", "echo '1111111' >> test.txt", "echo 'bbbbbbb' >> test.txt",
    "sort test.txt | busybox uniq", "stat test.txt", "strings test.txt", "wc test.txt",
    "[ -f test.txt ]", "more test.txt", "rm test.txt", "mkdir test_dir", "mv test_dir test",
    "rmdir test", "grep hello busybox_cmd.txt", "cp busybox_cmd.txt busybox_cmd.bak",
    "rm busybox_cmd.bak", "find . -name busybox_cmd.txt", NULL};

void test_pre() {
    int pid;

    int basic_testcases = 32;

    // printf("before chdir basic-glibc\n");
    // chdir(basic_path_glibc);
    // printf("after chdir basic-glibc\n");
    // printf("#### OS COMP TEST GROUP START basic-glibc ####\n");
    // for (int i = 0; i < basic_testcases; i++) {
    //     pid = fork();
    //     if (pid < 0) {
    //         printf("init: fork failed\n");
    //         exit(1);
    //     }
    //     if (pid == 0) {
    //         exec(basic_name[i], argv2);
    //         exit(1);
    //     }
    //     wait(0);
    // }
    // printf("#### OS COMP TEST GROUP END basic-glibc ####\n");

    // chdir(basic_path_musl);
    // printf("#### OS COMP TEST GROUP START basic-musl ####\n");
    // for (int i = 0; i < basic_testcases; i++) {
    //     pid = fork();
    //     if (pid < 0) {
    //         printf("init: fork failed\n");
    //         exit(1);
    //     }
    //     if (pid == 0) {
    //         exec(basic_name[i], argv2);
    //         exit(1);
    //     }
    //     wait(0);
    // }
    // printf("#### OS COMP TEST GROUP END basic-musl ####\n");

    // 实际应该使用的正确busybox测试命令，但是目前存在内存问题

    printf("before chdir busybox\n");
    chdir(bb_path_musl);  // 切换到musl测试
    printf("after chdir busybox\n");
    pid = fork();
    if (pid < 0) {
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0) {
        execve("busybox", bb_testcode, NULL);
        printf("init: exec busybox_testcode failed\n");
        exit(1);
    }
    wait(0);

    printf("before chdir busybox\n");
    chdir(bb_path_glibc);  // 切换到glibc测试
    printf("after chdir busybox\n");
    pid = fork();
    if (pid < 0) {
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0) {
        execve("busybox", bb_testcode, NULL);
        printf("init: exec busybox_testcode failed\n");
        exit(1);
    }
    wait(0);

    return;
}

// 【暂时禁用】依赖 busybox sh，当前不可用，待 busybox 修复后启用
#if 0
void test_final() {
    int pid;

    chdir(bb_path_musl);
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", interrupts_testcode, NULL);
        printf("init: exec interrupts_testcode failed\n");
        exit(1);
    }
    wait(0);

    chdir(bb_path_glibc);
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", interrupts_testcode, NULL);
        printf("init: exec interrupts_testcode failed\n");
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
        execve("busybox", copy_file_range_testcode, NULL);
        printf("init: exec copy_file_range_testcode failed\n");
        exit(1);
    }
    wait(0);

    chdir(bb_path_glibc);
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", copy_file_range_testcode, NULL);
        printf("init: exec copy_file_range_testcode failed\n");
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
        execve("busybox", splice_testcode, NULL);
        printf("init: exec splice_testcode failed\n");
        exit(1);
    }
    wait(0);

    chdir(bb_path_glibc);
    pid = fork();
    if (pid < 0) {
        printf("init: fork failed\n");
        exit(1);
    }
    if (pid == 0) {
        execve("busybox", splice_testcode, NULL);
        printf("init: exec splice_testcode failed\n");
        exit(1);
    }
    wait(0);

    return;
}

// 【暂时禁用】git 测试依赖 busybox sh + git_testcode.sh，当前不可用
void test_on_site() {
    int pid;

    mkdirat(AT_FDCWD, "/etc", 0755);

    // Create git configuration files to avoid warnings
    int fd;

    // Create /etc/gitconfig
    fd = openat(AT_FDCWD, "/etc/gitconfig", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        const char *syscfg = "[core]\n\tautocrlf = false\n";
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

    printf("\n===== SpringOS Auto Test Runner =====\n\n");

    // --- ★ basic 测试：直接 fork+exec 测试二进制（含完整评测标记格式）---
    // 无需 busybox，不依赖 basic_testcode.sh 脚本
    // 在 test_pre() 之前插入，试一下 busybox 到底什么反应
    chdir("/glibc/");
    int pid = fork();
    if (pid == 0) {
        char *argv[] = {"busybox", "cat", "busybox_cmd.txt", 0};
        execve("busybox", argv, NULL);
        printf("execve busybox cat FAILED\n");
        exit(1);
    }
    wait(0);
    test_pre();

    // --- ★ 自动扫描并执行测试脚本（需要 busybox sh applet，当前未完善）---
    // 比赛镜像路径结构：/glibc/*_testcode.sh  和  /musl/*_testcode.sh
    // 待 busybox 完善后取消注释
    // run_dir_testscripts("/glibc");
    // run_dir_testscripts("/musl");

    printf("\nAll tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
