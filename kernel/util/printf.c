#include <stdarg.h>
#include "types.h"
#include "param.h"
#include <lock/lock.h>
#include <fs/vfs/fs.h>
#include <fs/vfs/file.h>
#include "memlayout.h"
#include "platform.h"
#include "defs.h"
#include <proc/proc.h>

// ANSI Color codes
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_BLACK "\033[30m"
#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_BLUE "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN "\033[36m"
#define ANSI_COLOR_WHITE "\033[37m"
#define ANSI_COLOR_LIGHT_PURPLE "\033[95m"

// ANSI Text effects
#define ANSI_BOLD "\033[1m"
#define ANSI_DIM "\033[2m"
#define ANSI_UNDERLINE "\033[4m"
#define ANSI_BLINK "\033[5m"
#define ANSI_REVERSE "\033[7m"
#define ANSI_STRIKETHROUGH "\033[9m"

// Combined effects
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_BOLD_BLUE "\033[1;34m"
#define ANSI_BOLD_MAGENTA "\033[1;35m"
#define ANSI_BOLD_CYAN "\033[1;36m"
#define ANSI_BOLD_WHITE "\033[1;37m"
#define ANSI_BOLD_LIGHT_PURPLE "\033[1;95m"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
    struct spinlock lock;
    int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void printint(long long xx, int base, int sign) {
    char buf[16];
    int i;
    unsigned long long x;

    if (sign && (sign = (xx < 0)))
        x = -xx;
    else
        x = xx;

    i = 0;
    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign) buf[i++] = '-';

    while (--i >= 0) consputc(buf[i]);
}

static void printptr(uint64 x) {
    int i;
    consputc('0');
    consputc('x');
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console.
int printf(char *fmt, ...) {
    va_list ap;
    int i, cx, c0, c1, c2, locking;
    char *s;

    locking = pr.locking;
    if (locking) acquire(&pr.lock);

    va_start(ap, fmt);
    for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
        if (cx != '%') {
            consputc(cx);
            continue;
        }
        i++;
        c0 = fmt[i + 0] & 0xff;
        c1 = c2 = 0;
        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;
        if (c0 == 'd') {
            printint(va_arg(ap, int), 10, 1);
        } else if (c0 == 'l' && c1 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 2;
        } else if (c0 == 'u') {
            printint(va_arg(ap, int), 10, 0);
        } else if (c0 == 'l' && c1 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 2;
        } else if (c0 == 'x') {
            printint(va_arg(ap, int), 16, 0);
        } else if (c0 == 'l' && c1 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 2;
        } else if (c0 == 'p') {
            printptr(va_arg(ap, uint64));
        } else if (c0 == 's') {
            if ((s = va_arg(ap, char *)) == 0) s = "(null)";
            for (; *s; s++) consputc(*s);
        } else if (c0 == '%') {
            consputc('%');
        } else if (c0 == 0) {
            break;
        } else {
            // Print unknown % sequence to draw attention.
            consputc('%');
            consputc(c0);
        }

#if 0
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
#endif
    }
    va_end(ap);

    if (locking) release(&pr.lock);

    return 0;
}

void panic(char *s) {
    pr.locking = 0;
    printf("panic: ");
    printf("%s\n", s);
#ifdef RISCV
    backtrace();
#endif
#ifdef LA2K1000
    backtrace();
#endif
    panicked = 1;  // freeze uart output from other CPUs
    for (;;);
}

void printfinit(void) {
    initlock(&pr.lock, "pr");
    pr.locking = 1;
}

// Set text color using ANSI escape sequences
void set_text_color(const char *color) {
    const char *s = color;
    for (; *s; s++) consputc(*s);
}

// Reset text color to default
void reset_text_color(void) { set_text_color(ANSI_COLOR_RESET); }

// Print with color
int printf_color(const char *color, char *fmt, ...) {
    va_list ap;
    int locking;

    locking = pr.locking;
    if (locking) acquire(&pr.lock);

    // Set color
    set_text_color(color);

    // Print formatted text
    va_start(ap, fmt);
    int i, cx, c0, c1, c2;
    char *s;

    for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
        if (cx != '%') {
            consputc(cx);
            continue;
        }
        i++;
        c0 = fmt[i + 0] & 0xff;
        c1 = c2 = 0;
        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;
        if (c0 == 'd') {
            printint(va_arg(ap, int), 10, 1);
        } else if (c0 == 'l' && c1 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 2;
        } else if (c0 == 'u') {
            printint(va_arg(ap, int), 10, 0);
        } else if (c0 == 'l' && c1 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 2;
        } else if (c0 == 'x') {
            printint(va_arg(ap, int), 16, 0);
        } else if (c0 == 'l' && c1 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 2;
        } else if (c0 == 'p') {
            printptr(va_arg(ap, uint64));
        } else if (c0 == 's') {
            if ((s = va_arg(ap, char *)) == 0) s = "(null)";
            for (; *s; s++) consputc(*s);
        } else if (c0 == '%') {
            consputc('%');
        } else if (c0 == 0) {
            break;
        } else {
            // Print unknown % sequence to draw attention.
            consputc('%');
            consputc(c0);
        }
    }
    va_end(ap);

    // Reset color
    reset_text_color();

    if (locking) release(&pr.lock);

    return 0;
}

// Print with bold effect
int printf_bold(char *fmt, ...) {
    va_list ap;
    int locking;

    locking = pr.locking;
    if (locking) acquire(&pr.lock);

    // Set bold effect
    set_text_color(ANSI_BOLD);

    // Print formatted text
    va_start(ap, fmt);
    int i, cx, c0, c1, c2;
    char *s;

    for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
        if (cx != '%') {
            consputc(cx);
            continue;
        }
        i++;
        c0 = fmt[i + 0] & 0xff;
        c1 = c2 = 0;
        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;
        if (c0 == 'd') {
            printint(va_arg(ap, int), 10, 1);
        } else if (c0 == 'l' && c1 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 2;
        } else if (c0 == 'u') {
            printint(va_arg(ap, int), 10, 0);
        } else if (c0 == 'l' && c1 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 2;
        } else if (c0 == 'x') {
            printint(va_arg(ap, int), 16, 0);
        } else if (c0 == 'l' && c1 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 2;
        } else if (c0 == 'p') {
            printptr(va_arg(ap, uint64));
        } else if (c0 == 's') {
            if ((s = va_arg(ap, char *)) == 0) s = "(null)";
            for (; *s; s++) consputc(*s);
        } else if (c0 == '%') {
            consputc('%');
        } else if (c0 == 0) {
            break;
        } else {
            // Print unknown % sequence to draw attention.
            consputc('%');
            consputc(c0);
        }
    }
    va_end(ap);

    // Reset effects
    reset_text_color();

    if (locking) release(&pr.lock);

    return 0;
}

// Print with bold color (highlight effect)
int printf_highlight(const char *color, char *fmt, ...) {
    va_list ap;
    int locking;

    locking = pr.locking;
    if (locking) acquire(&pr.lock);

    // Set bold effect first
    set_text_color(ANSI_BOLD);
    // Then set color
    set_text_color(color);

    // Print formatted text
    va_start(ap, fmt);
    int i, cx, c0, c1, c2;
    char *s;

    for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
        if (cx != '%') {
            consputc(cx);
            continue;
        }
        i++;
        c0 = fmt[i + 0] & 0xff;
        c1 = c2 = 0;
        if (c0) c1 = fmt[i + 1] & 0xff;
        if (c1) c2 = fmt[i + 2] & 0xff;
        if (c0 == 'd') {
            printint(va_arg(ap, int), 10, 1);
        } else if (c0 == 'l' && c1 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
            printint(va_arg(ap, uint64), 10, 1);
            i += 2;
        } else if (c0 == 'u') {
            printint(va_arg(ap, int), 10, 0);
        } else if (c0 == 'l' && c1 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
            printint(va_arg(ap, uint64), 10, 0);
            i += 2;
        } else if (c0 == 'x') {
            printint(va_arg(ap, int), 16, 0);
        } else if (c0 == 'l' && c1 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 1;
        } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
            printint(va_arg(ap, uint64), 16, 0);
            i += 2;
        } else if (c0 == 'p') {
            printptr(va_arg(ap, uint64));
        } else if (c0 == 's') {
            if ((s = va_arg(ap, char *)) == 0) s = "(null)";
            for (; *s; s++) consputc(*s);
        } else if (c0 == '%') {
            consputc('%');
        } else if (c0 == 0) {
            break;
        } else {
            // Print unknown % sequence to draw attention.
            consputc('%');
            consputc(c0);
        }
    }
    va_end(ap);

    // Reset effects
    reset_text_color();

    if (locking) release(&pr.lock);

    return 0;
}
