#include <types.h>
#include <fs/stat.h>
#include "user/user.h"

void test_basic_sendfile() {
    printf("=== Test 1: Basic sendfile ===\n");

    // 创建输入文件
    int fd = openat(AT_FDCWD, "input.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);
    if (fd < 0) {
        printf("FAILED: Cannot create input.txt\n");
        return;
    }

    const char *data = "Hello sendfile! This is test data.\n";
    int data_len = strlen(data);
    write(fd, data, data_len);
    close(fd);
    printf("Created input.txt (%d bytes)\n", data_len);

    // 执行sendfile
    int in_fd = openat(AT_FDCWD, "input.txt", O_RDONLY, 0);
    int out_fd = openat(AT_FDCWD, "output.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);

    if (in_fd < 0 || out_fd < 0) {
        printf("FAILED: Cannot open files (in=%d, out=%d)\n", in_fd, out_fd);
        return;
    }

    ssize_t result = sendfile(out_fd, in_fd, NULL, 1024);
    // printf("sendfile result: %ld bytes\n", result);

    close(in_fd);
    close(out_fd);

    // 验证结果
    fd = openat(AT_FDCWD, "output.txt", O_RDONLY, 0);
    if (fd >= 0) {
        char buffer[128];
        int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Output: %s", buffer);
            if (bytes_read == data_len && strcmp(buffer, data) == 0) {
                printf("SUCCESS: Content matches!\n");
            } else {
                printf("WARNING: Content mismatch\n");
            }
        }
        close(fd);
    }
    printf("\n");
}

void test_sendfile_with_offset() {
    printf("=== Test 2: sendfile with offset ===\n");

    // 创建较大的输入文件
    int fd = openat(AT_FDCWD, "big_input.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);
    if (fd < 0) {
        printf("FAILED: Cannot create big_input.txt\n");
        return;
    }

    const char *line1 = "Line 1: ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
    const char *line2 = "Line 2: 0123456789\n";
    const char *line3 = "Line 3: This should be copied\n";

    write(fd, line1, strlen(line1));
    write(fd, line2, strlen(line2));
    write(fd, line3, strlen(line3));
    close(fd);
    printf("Created big_input.txt with 3 lines\n");

    // 从偏移位置开始发送
    int in_fd = openat(AT_FDCWD, "big_input.txt", O_RDONLY, 0);
    int out_fd = openat(AT_FDCWD, "offset_output.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);

    if (in_fd >= 0 && out_fd >= 0) {
        off_t offset = strlen(line1) + strlen(line2);  // 跳过前两行
        printf("Using offset: %ld\n", offset);

        ssize_t result = sendfile(out_fd, in_fd, &offset, 100);
        // printf("sendfile with offset result: %ld bytes, new offset: %ld\n", result, offset);

        close(in_fd);
        close(out_fd);

        // 验证结果
        fd = openat(AT_FDCWD, "offset_output.txt", O_RDONLY, 0);
        if (fd >= 0) {
            char buffer[128];
            int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Offset output: %s", buffer);
            }
            close(fd);
        }
    }
    printf("\n");
}

void test_sendfile_errors() {
    printf("=== Test 3: sendfile error cases ===\n");

    // 测试无效文件描述符
    ssize_t result = sendfile(-1, -1, NULL, 100);
    printf("Invalid fd result: %ld (should be -1)\n", result);

    // 测试零字节传输
    int fd = openat(AT_FDCWD, "test.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, "test", 4);
        close(fd);

        int in_fd = openat(AT_FDCWD, "test.txt", O_RDONLY, 0);
        int out_fd = openat(AT_FDCWD, "zero_out.txt", O_WRONLY | O_CREATE | O_TRUNC, 0644);

        if (in_fd >= 0 && out_fd >= 0) {
            result = sendfile(out_fd, in_fd, NULL, 0);
            printf("Zero byte transfer result: %ld\n", result);
            close(in_fd);
            close(out_fd);
        }
    }
    printf("\n");
}

int main() {
    printf("Testing sendfile implementation...\n\n");

    test_basic_sendfile();
    test_sendfile_with_offset();
    test_sendfile_errors();

    // printf("sendfile tests completed.\n");
    return 0;
}