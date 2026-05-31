// comprehensive_test_fixed.c - 修复格式化问题的测试程序
#include "types.h"
#include "fs/fcntl.h"
#include "user/user.h"

#define AT_FDCWD -100
#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM 0x0002

// 辅助函数
void print_hex(const char *data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", (unsigned char)data[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
        else if ((i + 1) % 8 == 0)
            printf("  ");
        else
            printf(" ");
    }
    if (len % 16 != 0) printf("\n");
}

long time_diff_ns(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000000000L + (end->tv_nsec - start->tv_nsec);
}

// 修复后的边界测试函数
void test_prlimit64_edge_cases() {
    printf("=== prlimit64 Edge Cases ===\n");

    struct rlimit64 limit;

    // 测试所有可能的资源类型
    printf("Testing all resource types:\n");
    const char *resource_names[] = {"CPU", "FSIZE", "DATA",   "STACK",   "CORE",
                                    "RSS", "NPROC", "NOFILE", "MEMLOCK", "AS"};

    for (int res = 0; res < 10; res++) {
        int ret = prlimit64(0, res, 0, &limit);
        if (ret == 0) {
            printf("  Resource %d (%s): cur=%lu, max=%lu\n", res, resource_names[res],
                   limit.rlim_cur, limit.rlim_max);
        } else {
            printf("  Resource %d (%s): FAIL (ret=%d)\n", res, resource_names[res], ret);
        }
    }

    // 测试边界PID值
    printf("Testing edge PID values:\n");
    int test_pids[] = {-1, 0, 1, getpid(), getppid()};
    const char *pid_names[] = {"invalid(-1)", "self(0)", "init(1)", "getpid()", "getppid()"};
    for (int i = 0; i < 5; i++) {
        int ret = prlimit64(test_pids[i], RLIMIT_STACK, 0, &limit);
        printf("  PID %s: %s", pid_names[i], ret == 0 ? "OK" : "FAIL");
        if (ret == 0) {
            printf(" (cur=%lu, max=%lu)", limit.rlim_cur, limit.rlim_max);
        }
        printf("\n");
    }

    printf("\n");
}

void test_clock_gettime_precision() {
    printf("=== clock_gettime Precision Test ===\n");

    struct timespec ts1, ts2;

    // 测试支持的时钟类型
    printf("Testing supported clocks:\n");
    int clocks[] = {CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID};
    const char *clock_names[] = {"REALTIME", "MONOTONIC", "PROCESS_CPUTIME_ID"};

    for (int i = 0; i < 3; i++) {
        if (clock_gettime(clocks[i], &ts1) == 0) {
            // 快速连续调用测试精度
            clock_gettime(clocks[i], &ts2);
            long diff = time_diff_ns(&ts1, &ts2);
            printf("  Clock %s: %ld.%09ld sec, precision: %ld ns\n", clock_names[i], ts1.tv_sec,
                   ts1.tv_nsec, diff);
        } else {
            printf("  Clock %s: NOT SUPPORTED\n", clock_names[i]);
        }
    }

    // 测试非法时钟ID
    printf("Testing invalid clock IDs:\n");
    int invalid_clocks[] = {-1, 999, 100};
    for (int i = 0; i < 3; i++) {
        int ret = clock_gettime(invalid_clocks[i], &ts1);
        printf("  Clock %d: %s\n", invalid_clocks[i],
               ret == 0 ? "UNEXPECTED SUCCESS" : "FAIL (expected)");
    }

    // 性能测试
    printf("Performance test (1000 calls):\n");
    if (clock_gettime(CLOCK_MONOTONIC, &ts1) == 0) {
        for (int i = 0; i < 1000; i++) {
            struct timespec dummy;
            clock_gettime(CLOCK_REALTIME, &dummy);
        }
        clock_gettime(CLOCK_MONOTONIC, &ts2);
        long total_time = time_diff_ns(&ts1, &ts2);
        printf("  1000 calls took %ld ns (avg: %ld ns per call)\n", total_time, total_time / 1000);
    }

    printf("\n");
}

void test_getrandom_quality() {
    printf("=== getrandom Quality Test ===\n");

    // 基本功能测试
    printf("Testing basic functionality:\n");
    char small_buf[16];
    int ret = getrandom(small_buf, sizeof(small_buf), 0);
    if (ret == sizeof(small_buf)) {
        printf("  Basic test: OK, got %d bytes\n", ret);
        printf("  Sample data: ");
        print_hex(small_buf, 8);
    } else {
        printf("  Basic test: FAIL, ret=%d\n", ret);
        return;
    }

    // 统计分析测试
    printf("Statistical analysis of random data:\n");
    const int sample_size = 1000;
    char *data = malloc(sample_size);
    if (!data) {
        printf("  Memory allocation failed\n");
        return;
    }

    ret = getrandom(data, sample_size, 0);
    if (ret == sample_size) {
        // 计算字节分布
        int byte_counts[256] = {0};
        for (int i = 0; i < sample_size; i++) {
            byte_counts[(unsigned char)data[i]]++;
        }

        // 检查分布均匀性
        int min_count = sample_size, max_count = 0;
        int zero_count = 0;
        for (int i = 0; i < 256; i++) {
            if (byte_counts[i] < min_count) min_count = byte_counts[i];
            if (byte_counts[i] > max_count) max_count = byte_counts[i];
            if (byte_counts[i] == 0) zero_count++;
        }

        float ratio = (min_count > 0) ? (float)max_count / min_count : (float)max_count;
        printf("  Byte distribution: min=%d, max=%d, ratio=%.2f, unused_bytes=%d\n", min_count,
               max_count, ratio, zero_count);

        // 简单的自相关测试
        int matches = 0;
        for (int i = 0; i < sample_size - 1; i++) {
            if (data[i] == data[i + 1]) matches++;
        }
        float autocorr = (float)matches / (sample_size - 1);
        printf("  Adjacent byte matches: %.2f%% (good if ~0.4%%)\n", autocorr * 100);

        // 检查常见模式
        int zero_bytes = byte_counts[0];
        int ff_bytes = byte_counts[255];
        printf("  Zero bytes: %d (%.1f%%), 0xFF bytes: %d (%.1f%%)\n", zero_bytes,
               zero_bytes * 100.0 / sample_size, ff_bytes, ff_bytes * 100.0 / sample_size);
    } else {
        printf("  Failed to get %d bytes, got %d\n", sample_size, ret);
    }

    // 测试不同大小的请求
    printf("Testing different buffer sizes:\n");
    int sizes[] = {0, 1, 4, 16, 64, 256, 1024};
    for (int i = 0; i < 7; i++) {
        if (sizes[i] == 0) {
            ret = getrandom(small_buf, 0, 0);
            printf("  Size %d: ret=%d %s\n", 0, ret, ret == 0 ? "(OK)" : "(may vary)");
        } else {
            char *buf = malloc(sizes[i]);
            if (buf) {
                ret = getrandom(buf, sizes[i], 0);
                printf("  Size %d: %s (%d bytes)\n", sizes[i], ret == sizes[i] ? "OK" : "FAIL",
                       ret);
                free(buf);
            }
        }
    }

    // 测试标志位
    printf("Testing flags:\n");
    ret = getrandom(small_buf, sizeof(small_buf), GRND_NONBLOCK);
    printf("  GRND_NONBLOCK: %s\n", ret > 0 ? "OK" : "FAIL or unavailable");

    ret = getrandom(small_buf, sizeof(small_buf), 0xFFFF);
    printf("  Invalid flags (0xFFFF): %s\n", ret < 0 ? "FAIL (expected)" : "UNEXPECTED SUCCESS");

    free(data);
    printf("\n");
}

void test_readlinkat_comprehensive() {
    printf("=== readlinkat Comprehensive Test ===\n");

    char buffer[1024];
    int ret;

    // 测试不同的缓冲区大小
    printf("Testing buffer sizes with /proc/self/exe:\n");
    int sizes[] = {0, 1, 4, 16, 64, 256, 1024};
    for (int i = 0; i < 7; i++) {
        ret = readlinkat(AT_FDCWD, "/proc/self/exe", buffer, sizes[i]);
        if (ret > 0) {
            printf("  Size %d: ret=%d (truncated: %s)\n", sizes[i], ret,
                   ret >= sizes[i] ? "yes" : "no");
        } else {
            printf("  Size %d: ret=%d\n", sizes[i], ret);
        }
    }

    // 测试各种路径
    printf("Testing various paths:\n");
    const char *paths[] = {"/proc/self/exe",
                           "/proc/self/cwd",
                           "/proc/self/root",
                           "/",
                           "/proc",
                           "/dev/null",
                           "/tmp",
                           ".",
                           "..",
                           "nonexistent_file"};

    for (int i = 0; i < 10; i++) {
        ret = readlinkat(AT_FDCWD, paths[i], buffer, sizeof(buffer) - 1);
        if (ret > 0) {
            buffer[ret] = '\0';
            printf("  %-20s -> %s\n", paths[i], buffer);
        } else if (ret == 0) {
            printf("  %-20s -> (empty)\n", paths[i]);
        } else {
            printf("  %-20s -> FAIL (%d)\n", paths[i], ret);
        }
    }

    printf("\n");
}

void test_openat_functionality() {
    printf("=== openat Functionality Test ===\n");

    // 测试基本功能
    printf("Testing basic openat functionality:\n");

    // 创建测试目录
    int ret = mkdirat(AT_FDCWD, "test_openat_dir", 0755);
    printf("  mkdir: %s\n", ret == 0 ? "OK" : "FAIL");

    if (ret == 0) {
        // 使用AT_FDCWD打开文件
        int fd1 = openat(AT_FDCWD, "test_openat_dir/file1.txt", O_CREATE | O_WRONLY, 0644);
        printf("  openat(AT_FDCWD, create): %s\n", fd1 >= 0 ? "OK" : "FAIL");
        if (fd1 >= 0) {
            write(fd1, "Hello, World!", 13);
            close(fd1);
        }

        // 打开目录获取文件描述符
        int dirfd = openat(AT_FDCWD, "test_openat_dir", O_RDONLY, 0);
        printf("  openat(dir): %s\n", dirfd >= 0 ? "OK" : "FAIL");

        if (dirfd >= 0) {
            // 使用目录文件描述符打开文件
            int fd2 = openat(dirfd, "file2.txt", O_CREATE | O_WRONLY, 0644);
            printf("  openat(dirfd, create): %s\n", fd2 >= 0 ? "OK" : "FAIL");
            if (fd2 >= 0) {
                write(fd2, "Test data", 9);
                close(fd2);
            }

            // 读取已存在的文件
            int fd3 = openat(dirfd, "file1.txt", O_RDONLY, 0);
            printf("  openat(dirfd, read existing): %s\n", fd3 >= 0 ? "OK" : "FAIL");
            if (fd3 >= 0) {
                char buf[32];
                int n = read(fd3, buf, sizeof(buf) - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    printf("    Content: %s\n", buf);
                }
                close(fd3);
            }

            close(dirfd);
        }

        // 测试不同的标志
        printf("Testing different flags:\n");
        struct {
            int flags;
            const char *name;
        } flag_tests[] = {{O_RDONLY, "O_RDONLY"},
                          {O_WRONLY, "O_WRONLY"},
                          {O_RDWR, "O_RDWR"},
                          {O_CREATE, "O_CREATE"},
                          {O_TRUNC, "O_TRUNC"},
                          {O_CREATE | O_WRONLY, "O_CREATE|O_WRONLY"},
                          {O_CREATE | O_RDWR, "O_CREATE|O_RDWR"},
                          {O_RDWR | O_TRUNC, "O_RDWR|O_TRUNC"}};

        for (int i = 0; i < 8; i++) {
            int fd = openat(AT_FDCWD, "test_openat_dir/flag_test.txt", flag_tests[i].flags, 0644);
            printf("  %-20s: %s\n", flag_tests[i].name, fd >= 0 ? "OK" : "FAIL");
            if (fd >= 0) close(fd);
        }

        // 清理
        unlinkat(AT_FDCWD, "test_openat_dir/file1.txt", 0);
        unlinkat(AT_FDCWD, "test_openat_dir/file2.txt", 0);
        unlinkat(AT_FDCWD, "test_openat_dir/flag_test.txt", 0);
        unlinkat(AT_FDCWD, "test_openat_dir", AT_REMOVEDIR);
    }

    printf("\n");
}

void test_error_conditions() {
    printf("=== Error Condition Tests ===\n");

    printf("Testing invalid parameters:\n");

    // prlimit64 错误测试
    struct rlimit64 limit;
    printf("  prlimit64 tests:\n");
    printf("    Invalid PID (-999): ret=%d\n", prlimit64(-999, RLIMIT_STACK, 0, &limit));
    printf("    Invalid resource (999): ret=%d\n", prlimit64(0, 999, 0, &limit));
    printf("    NULL output pointer: ret=%d\n", prlimit64(0, RLIMIT_STACK, 0, 0));

    // clock_gettime 错误测试
    struct timespec ts;
    printf("  clock_gettime tests:\n");
    printf("    Invalid clock (999): ret=%d\n", clock_gettime(999, &ts));
    printf("    Negative clock (-1): ret=%d\n", clock_gettime(-1, &ts));
    printf("    NULL timespec: ret=%d\n", clock_gettime(CLOCK_REALTIME, 0));

    // getrandom 错误测试
    char buf[16];
    printf("  getrandom tests:\n");
    printf("    NULL buffer: ret=%d\n", getrandom(0, 16, 0));
    printf("    Large negative size: ret=%d\n", getrandom(buf, -1, 0));
    printf("    Invalid flags: ret=%d\n", getrandom(buf, 16, 0xFFFF));

    // readlinkat 错误测试
    printf("  readlinkat tests:\n");
    printf("    Invalid dirfd (-999): ret=%d\n", readlinkat(-999, "test", buf, 16));
    printf("    NULL buffer: ret=%d\n", readlinkat(AT_FDCWD, "/proc/self/exe", 0, 16));
    printf("    Zero buffer size: ret=%d\n", readlinkat(AT_FDCWD, "/proc/self/exe", buf, 0));

    printf("\n");
}

void stress_test_all_syscalls() {
    printf("=== Stress Test All Syscalls ===\n");

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        printf("clock_gettime not available for timing\n");
        return;
    }

    printf("Running stress test for 5 seconds...\n");

    int iterations = 0;
    struct timespec current;

    do {
        // prlimit64
        struct rlimit64 limit;
        prlimit64(0, RLIMIT_STACK, 0, &limit);

        // clock_gettime
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        // getrandom
        char rand_buf[16];
        getrandom(rand_buf, sizeof(rand_buf), 0);

        // readlinkat
        char link_buf[64];
        readlinkat(AT_FDCWD, "/proc/self/exe", link_buf, sizeof(link_buf));

        iterations++;

        if (iterations % 1000 == 0) {
            if (clock_gettime(CLOCK_MONOTONIC, &current) == 0) {
                if (time_diff_ns(&start, &current) >= 5000000000L) break;
            }
        }
    } while (iterations < 100000);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == 0) {
        long total_time = time_diff_ns(&start, &end);
        float rate = iterations * 1000.0 / (total_time / 1000000.0);
        float syscall_rate = rate * 4.0;

        printf("Completed %d iterations in %ld ms\n", iterations, total_time / 1000000);
        printf("Average rate: %.1f syscall-sets/second\n", rate);
        printf("Individual syscall rate: %.1f calls/second\n", syscall_rate);
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    printf("Comprehensive System Call Test Suite (Fixed Version)\n");
    printf("===================================================\n");
    printf("Testing system calls: prlimit64, clock_gettime, getrandom, readlinkat, openat\n\n");

    if (argc > 1) {
        if (strcmp(argv[1], "edge") == 0) {
            test_prlimit64_edge_cases();
            test_clock_gettime_precision();
            test_getrandom_quality();
            test_readlinkat_comprehensive();
            test_openat_functionality();
        } else if (strcmp(argv[1], "stress") == 0) {
            stress_test_all_syscalls();
        } else if (strcmp(argv[1], "error") == 0) {
            test_error_conditions();
        } else {
            printf("Usage: %s [edge|stress|error]\n", argv[0]);
            return 1;
        }
    } else {
        test_prlimit64_edge_cases();
        test_clock_gettime_precision();
        test_getrandom_quality();
        test_readlinkat_comprehensive();
        test_openat_functionality();
        test_error_conditions();
        stress_test_all_syscalls();
    }

    printf("All comprehensive tests completed!\n");
    return 0;
}