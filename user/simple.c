// simple_test.c - 适合xv6环境的简化测试程序
#include <types.h>
#include <fs/stat.h>
#include "user/user.h"

void test_writev() {
    printf("=== 测试 writev 系统调用 ===\n");

    // 测试1: 基本功能测试
    printf("测试1: 基本功能\n");

    char buf1[] = "Hello, ";
    char buf2[] = "World!\n";

    struct iovec iov[2];
    iov[0].iov_base = buf1;
    iov[0].iov_len = strlen(buf1);
    iov[1].iov_base = buf2;
    iov[1].iov_len = strlen(buf2);

    // 写入到标准输出
    int result = writev(1, iov, 2);
    printf("writev 返回值: %d (期望: %d)\n", result, strlen(buf1) + strlen(buf2));

    // 测试2: 写入文件
    printf("\n测试2: 写入文件\n");
    int fd = open("test.txt", O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf("无法创建测试文件\n");
        return;
    }

    char data1[] = "Line 1\n";
    char data2[] = "Line 2\n";
    char data3[] = "Line 3\n";

    struct iovec file_iov[3];
    file_iov[0].iov_base = data1;
    file_iov[0].iov_len = strlen(data1);
    file_iov[1].iov_base = data2;
    file_iov[1].iov_len = strlen(data2);
    file_iov[2].iov_base = data3;
    file_iov[2].iov_len = strlen(data3);

    int file_result = writev(fd, file_iov, 3);
    close(fd);

    printf("文件写入返回值: %d\n", file_result);

    // 验证文件内容
    fd = open("test.txt", O_RDONLY);
    if (fd >= 0) {
        char read_buf[100];
        int bytes_read = read(fd, read_buf, sizeof(read_buf));
        close(fd);
        printf("读取字节数: %d\n", bytes_read);
        printf("文件内容:\n");
        write(1, read_buf, bytes_read);
    }

    // 测试3: 错误情况
    printf("\n测试3: 错误情况\n");
    int error_result = writev(-1, iov, 2);  // 无效文件描述符
    printf("无效fd测试返回值: %d (期望: -1)\n", error_result);

    error_result = writev(1, iov, 0);  // iovcnt为0
    printf("iovcnt=0测试返回值: %d (期望: -1)\n", error_result);

    unlinkat(AT_FDCWD, "test.txt", 0);  // 清理测试文件
}

void test_set_robust_list() {
    printf("\n=== 测试 set_robust_list 系统调用 ===\n");

    // 测试1: 基本调用
    printf("测试1: 基本调用\n");
    void *head = (void *)0x12345678;
    int result = set_robust_list(head, sizeof(void *));
    printf("基本调用返回值: %d (期望: 0)\n", result);

    // 测试2: NULL头指针
    printf("\n测试2: NULL头指针\n");
    result = set_robust_list(0, sizeof(void *));
    printf("NULL头指针返回值: %d (期望: 0)\n", result);

    // 测试3: 长度为0
    printf("\n测试3: 长度为0\n");
    result = set_robust_list(head, 0);
    printf("长度为0返回值: %d (期望: -1)\n", result);

    // 测试4: 无效长度
    printf("\n测试4: 无效长度\n");
    result = set_robust_list(head, 123);
    printf("无效长度返回值: %d (期望: -1)\n", result);
}

int main() {
    printf("开始系统调用测试...\n\n");

    test_writev();
    test_set_robust_list();

    printf("\n测试完成!\n");
    printf("注意: 请手动检查输出结果是否符合期望值\n");

    return 0;
}