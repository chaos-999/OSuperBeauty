#include <types.h>
#include <fs/stat.h>
#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

// Print to the given fd. Only understands %d, %x, %p, %s.
void vprintf(int fd, const char *fmt, va_list ap) {
    char *s;
    int c0, c1, c2, i, state;
    char buf[1024];   // 添加缓冲区
    int buf_pos = 0;  // 缓冲区位置

// 使用宏来替代嵌套函数
#define putc_buf(c)                      \
    do {                                 \
        if (buf_pos < sizeof(buf) - 1) { \
            buf[buf_pos++] = (c);        \
        }                                \
    } while (0)

    state = 0;
    for (i = 0; fmt[i]; i++) {
        c0 = fmt[i] & 0xff;
        if (state == 0) {
            if (c0 == '%') {
                state = '%';
            } else {
                putc_buf(c0);
            }
        } else if (state == '%') {
            c1 = c2 = 0;
            if (c0) c1 = fmt[i + 1] & 0xff;
            if (c1) c2 = fmt[i + 2] & 0xff;
            if (c0 == 'd') {
                // 暂时将printint输出收集到局部字符串
                char int_buf[16];
                int int_pos = 0;
                int xx = va_arg(ap, int);
                int neg = 0;
                uint x;

                if (xx < 0) {
                    neg = 1;
                    x = -xx;
                } else {
                    x = xx;
                }

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);
                if (neg) int_buf[int_pos++] = '-';

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
            } else if (c0 == 'l' && c1 == 'd') {
                // 类似处理long long
                char int_buf[24];
                int int_pos = 0;
                uint64 xx = va_arg(ap, uint64);
                int neg = 0;
                uint64 x;

                if ((int64)xx < 0) {
                    neg = 1;
                    x = -xx;
                } else {
                    x = xx;
                }

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);
                if (neg) int_buf[int_pos++] = '-';

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 1;
            } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
                // 类似处理long long
                char int_buf[24];
                int int_pos = 0;
                uint64 xx = va_arg(ap, uint64);
                int neg = 0;
                uint64 x;

                if ((int64)xx < 0) {
                    neg = 1;
                    x = -xx;
                } else {
                    x = xx;
                }

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);
                if (neg) int_buf[int_pos++] = '-';

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 2;
            } else if (c0 == 'u') {
                char int_buf[16];
                int int_pos = 0;
                uint x = va_arg(ap, int);

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
            } else if (c0 == 'l' && c1 == 'u') {
                char int_buf[24];
                int int_pos = 0;
                uint64 x = va_arg(ap, uint64);

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 1;
            } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
                char int_buf[24];
                int int_pos = 0;
                uint64 x = va_arg(ap, uint64);

                do {
                    int_buf[int_pos++] = digits[x % 10];
                } while ((x /= 10) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 2;
            } else if (c0 == 'x') {
                char int_buf[16];
                int int_pos = 0;
                uint x = va_arg(ap, int);

                do {
                    int_buf[int_pos++] = digits[x % 16];
                } while ((x /= 16) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
            } else if (c0 == 'l' && c1 == 'x') {
                char int_buf[24];
                int int_pos = 0;
                uint64 x = va_arg(ap, uint64);

                do {
                    int_buf[int_pos++] = digits[x % 16];
                } while ((x /= 16) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 1;
            } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
                char int_buf[24];
                int int_pos = 0;
                uint64 x = va_arg(ap, uint64);

                do {
                    int_buf[int_pos++] = digits[x % 16];
                } while ((x /= 16) != 0);

                while (--int_pos >= 0) putc_buf(int_buf[int_pos]);
                i += 2;
            } else if (c0 == 'p') {
                uint64 x = va_arg(ap, uint64);
                putc_buf('0');
                putc_buf('x');
                for (int j = 0; j < (sizeof(uint64) * 2); j++, x <<= 4)
                    putc_buf(digits[x >> (sizeof(uint64) * 8 - 4)]);
            } else if (c0 == 's') {
                if ((s = va_arg(ap, char *)) == 0) s = "(null)";
                for (; *s; s++) putc_buf(*s);
            } else if (c0 == '%') {
                putc_buf('%');
            } else {
                // Unknown % sequence.  Print it to draw attention.
                putc_buf('%');
                putc_buf(c0);
            }
            state = 0;
        }
    }

    // 一次性写入整个缓冲区
    if (buf_pos > 0) {
        write(fd, buf, buf_pos);
    }
}

void fprintf(int fd, const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(fd, fmt, ap);
}

void printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(1, fmt, ap);
}
