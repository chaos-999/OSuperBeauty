#include "ctype.h"
#include "types.h"
#include "platform.h"
#include "defs.h"
#include <stdarg.h>

void *memset(void *dst, int c, uint n) {
    char *cdst = (char *)dst;
    int i;
    for (i = 0; i < n; i++) {
        cdst[i] = c;
    }
    return dst;
}

int memcmp(const void *v1, const void *v2, uint n) {
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0) {
        if (*s1 != *s2) return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void *memmove(void *dst, const void *src, uint n) {
    const char *s;
    char *d;

    if (n == 0) return dst;

    s = src;
    d = dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0) {
            *--d = *--s;
        }

    } else
        while (n-- > 0) {
            *d++ = *s++;
        }

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
int memcpy(void *dst, const void *src, uint n) {
    memmove(dst, src, n);
    return 0;
}

int strncmp(const char *p, const char *q, uint n) {
    while (n > 0 && *p && *p == *q) n--, p++, q++;
    if (n == 0) return 0;
    return (uchar)*p - (uchar)*q;
}

int strcmp(const char *p, const char *q) {
    while (*p && *p == *q) p++, q++;
    return (uchar)*p - (uchar)*q;
}

char *strcpy(char *s, const char *t) {
    char *os = s;
    while ((*s++ = *t++) != 0);
    return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char *strncpy(char *s, const char *t, int n) {
    char *os = s;
    if (n <= 0) return os;
    while (n-- > 0 && (*s++ = *t++) != 0);
    *s = 0;
    return os;
}

int strlen(const char *s) {
    int n;

    for (n = 0; s[n]; n++);
    return n;
}

size_t strnlen(const char *s, size_t count) {
    const char *sc;

    for (sc = s; *sc != '\0' && count--; ++sc) /* nothing */
        ;
    return sc - s;
}

char *strchr(const char *str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char *)str;
        }
        str++;
    }
    if (c == '\0') {
        return (char *)str;
    }
    return NULL;
}

int str_split(const char *str, const char ch, char *str1, char *str2) {
    char *p = strchr(str, ch);
    if (p == NULL) {
        return -1;
    }
    strncpy(str1, str, p - str);
    strncpy(str2, p + 1, strlen(str) - 1 - (p - str));

    return 1;
}

char *strcat(char *dest, const char *src) {
    char *p = dest;
    while (*p) {
        ++p;
    }
    while (*src) {
        *p++ = *src++;
    }
    *p = '\0';
    return dest;
}

char *safestrcpy(char *dest, const char *src, int n) {
    if (n <= 0) {
        return dest;
    }
    char *d = dest;
    const char *s = src;

    while (--n > 0 && (*d++ = *s++) != '\0');

    *d = '\0';  // ensure null-termination
    return dest;
}

// 数字字符数组，与 printf.c 中的保持一致
static char digits[] = "0123456789abcdef";

// 辅助函数：将整数转换为字符串并写入缓冲区
static int printint_to_buf(char *buf, int bufsize, int *pos, long long xx, int base, int sign) {
    char temp[32];
    int i = 0;
    unsigned long long x;
    int written = 0;

    if (sign && (sign = (xx < 0)))
        x = -xx;
    else
        x = xx;

    // 转换数字到临时缓冲区
    if (x == 0) {
        temp[i++] = '0';
    } else {
        do {
            temp[i++] = digits[x % base];
        } while ((x /= base) != 0);
    }

    if (sign) temp[i++] = '-';

    // 写入最终缓冲区（逆序）
    while (--i >= 0 && *pos < bufsize - 1) {
        buf[(*pos)++] = temp[i];
        written++;
    }

    return written;
}

// 写入单个字符到缓冲区
static int putchar_to_buf(char *buf, int bufsize, int *pos, char c) {
    if (*pos < bufsize - 1) {
        buf[(*pos)++] = c;
        return 1;
    }
    return 0;
}

// 写入字符串到缓冲区
static int puts_to_buf(char *buf, int bufsize, int *pos, const char *s) {
    int written = 0;
    if (!s) s = "(null)";

    while (*s && *pos < bufsize - 1) {
        buf[(*pos)++] = *s++;
        written++;
    }
    return written;
}

int snprintf(char *str, size_t size, const char *fmt, ...) {
    va_list ap;
    int pos = 0;     // 当前写入位置
    int needed = 0;  // 总共需要的字符数
    int i;

    if (!str || size == 0) return 0;

    va_start(ap, fmt);

    for (i = 0; fmt[i] != 0; i++) {
        if (fmt[i] != '%') {
            if (putchar_to_buf(str, size, &pos, fmt[i])) {
                needed++;
            } else {
                needed++;  // 即使写不下也要计数
            }
            continue;
        }

        i++;  // 跳过 '%'
        int c0 = fmt[i] & 0xff;
        int c1 = 0, c2 = 0;

        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;

        // 处理格式说明符（复用 printf.c 的逻辑）
        if (c0 == 'd') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, int), 10, 1);
            needed += written;
        } else if (c0 == 'l' && c1 == 'd') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 10, 1);
            needed += written;
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 10, 1);
            needed += written;
            i += 2;
        } else if (c0 == 'u') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, int), 10, 0);
            needed += written;
        } else if (c0 == 'l' && c1 == 'u') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 10, 0);
            needed += written;
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 10, 0);
            needed += written;
            i += 2;
        } else if (c0 == 'x') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, int), 16, 0);
            needed += written;
        } else if (c0 == 'l' && c1 == 'x') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 16, 0);
            needed += written;
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
            int written = printint_to_buf(str, size, &pos, va_arg(ap, uint64), 16, 0);
            needed += written;
            i += 2;
        } else if (c0 == 's') {
            char *s = va_arg(ap, char *);
            int written = puts_to_buf(str, size, &pos, s);
            needed += written;
        } else if (c0 == 'c') {
            char c = (char)va_arg(ap, int);
            if (putchar_to_buf(str, size, &pos, c)) {
                needed++;
            } else {
                needed++;
            }
        } else if (c0 == '%') {
            if (putchar_to_buf(str, size, &pos, '%')) {
                needed++;
            } else {
                needed++;
            }
        } else if (c0 == 0) {
            break;
        } else {
            // 未知格式，直接输出
            if (putchar_to_buf(str, size, &pos, '%')) needed++;
            if (putchar_to_buf(str, size, &pos, c0)) needed++;
        }
    }

    // 确保字符串以 null 结尾
    str[pos] = '\0';

    va_end(ap);
    return needed;
}