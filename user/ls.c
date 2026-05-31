#define USER
#include "types.h"
#include "user/user.h"
#include "fs/stat.h"
#define STDOUT 1
#define STDOUT 1
#define BUF_SIZE 4096
int main(int argc, char *argv[]) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent64 *d;
    int bpos;

    const char *path = (argc > 1) ? argv[1] : ".";
    fd = openat(AT_FDCWD, argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY, 0600);

    if (fd < 0) {
        printf("ls: cannot open %s\n", path);
        exit(-1);
    }

    for (;;) {
        nread = getdents64(fd, (struct linux_dirent64 *)buf, BUF_SIZE);

        if (nread < 0) {
            printf("ls: getdents64 failed\n");
            break;
        }

        if (nread == 0) {
            break;  // 目录读取完毕
        }

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *)(buf + bpos);

            if (d->d_reclen == 0) {
                break;
            }

            // 忽略 . 和 .. 目录，让输出更干净
            if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
                // 【关键修复】使用最简单的 printf 格式！
                // 不再使用 "%-15s"，而是用 "%s" 加上一个制表符 "\n"。
                // 这样可以确保任何一个简单的 printf 实现都能处理。
                switch (d->d_type) {
                    case T_DIR:
                        // 你的 printf 可能也不支持 \033，为保证成功，暂时去掉颜色
                        printf("%s\n", d->d_name);
                        break;
                    case T_CHR:
                        printf("%s\n", d->d_name);
                        break;
                    default:
                        printf("%s\n", d->d_name);
                        break;
                }
            }

            bpos += d->d_reclen;
        }
    }

    close(fd);
    exit(0);
}