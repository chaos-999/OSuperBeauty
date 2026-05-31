#include "types.h"
#include "memlayout.h"

// 极早期 UART 轮询输出，不依赖控制台/中断/锁
// 16550 寄存器布局（2K1000 APB: 多为 4 字节步长。若将来发现为 1 字节，可再切回）
#define THR 0  // transmit holding register
#define LSR 5  // line status register
#define UART_REG_STRIDE 1
#define LSR_TX_IDLE (1 << 5)

static inline void mmio_write8(uint64 addr, uint8 val) {
    volatile uint8 *p = (volatile uint8 *)addr;
    *p = val;
}

static inline uint8 mmio_read8(uint64 addr) {
    volatile uint8 *p = (volatile uint8 *)addr;
    return *p;
}

static inline void early_uart_putc(int c) {
    // 尝试等待一小段时间，若一直未就绪也直接写，避免死等
    unsigned int tries = 100000;
    while (tries-- && (mmio_read8(UART0 + UART_REG_STRIDE * LSR) & LSR_TX_IDLE) == 0) {
        // 小延迟
        __asm__ __volatile__("nop");
    }
    mmio_write8(UART0 + UART_REG_STRIDE * THR, (uint8)c);
}

void early_uart_puts(const char *s) {
    if (!s) return;
    while (*s) {
        if (*s == '\n') {
            early_uart_putc('\r');
        }
        early_uart_putc(*s++);
    }
}

#ifdef LOONGARCH
#ifdef LA2K1000
// 供汇编快速取用的 UART 基址（DMW1 uncached），避免在汇编里写 li.d 64 位立即数
__attribute__((section(".data"))) unsigned long __early_uart_base =
    (unsigned long)(UART0 | DMWIN1_MASK);
#endif
#endif
