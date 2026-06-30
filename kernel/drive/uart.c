//
// low-level driver routines for 16550a UART.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "platform.h"
#include <lock/lock.h>
#include <proc/proc.h>
#include "defs.h"

// the UART control registers are memory-mapped at UART0.
// 16550 兼容 UART 的寄存器步长设置
// VF2 为 4 字节步长；2K1000LA 参考工程按字节地址编程，这里采用 1 字节步长
#if defined(VF2)
#define UART_REG_STRIDE 4
#elif defined(LOONGARCH) && defined(LA2K1000)
#define UART_REG_STRIDE 1
#else
#define UART_REG_STRIDE 1
#endif

#define Reg(reg) ((volatile unsigned char *)(UART0 + UART_REG_STRIDE * (reg)))

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0  // receive holding register (for input bytes)
#define THR 0  // transmit holding register (for output bytes)
#define IER 1  // interrupt enable register
#define IER_RX_ENABLE (1 << 0)
#define IER_TX_ENABLE (1 << 1)
#define FCR 2  // FIFO control register
#define FCR_FIFO_ENABLE (1 << 0)
#define FCR_FIFO_CLEAR (3 << 1)  // clear the content of the two FIFOs
#define ISR 2                    // interrupt status register
#define LCR 3                    // line control register
#define LCR_EIGHT_BITS (3 << 0)
#define LCR_BAUD_LATCH (1 << 7)  // special mode to set baud rate
#define MCR 4                    // modem control register
#define MCR_DTR (1 << 0)
#define MCR_RTS (1 << 1)
#define MCR_OUT2 (1 << 3)      // required on some 16550 to enable IRQ output
#define LSR 5                  // line status register
#define LSR_RX_READY (1 << 0)  // input is waiting to be read from RHR
#define LSR_TX_IDLE (1 << 5)   // THR can accept another character to send
#ifdef VF2
// VF2: extra 16550 registers/values used for robust IRQ draining
#define MSR 6  // modem status register
// IIR (a.k.a. ISR) decoding
#define IIR_INT_PENDING 0x01  // bit0=1 means no pending
#define IIR_ID_MASK 0x0E
#define IIR_ID_MODEM 0x00       // 000: modem status
#define IIR_ID_THRE 0x02        // 010: THR empty
#define IIR_ID_RDA 0x04         // 100: received data available
#define IIR_ID_LS 0x06          // 110: line status
#define IIR_ID_RX_TIMEOUT 0x0C  // 1100: character timeout
#endif

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

// the transmit output buffer.
struct spinlock uart_tx_lock;
#define UART_TX_BUF_SIZE 32
char uart_tx_buf[UART_TX_BUF_SIZE];
uint64 uart_tx_w;  // write next to uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE]
uint64 uart_tx_r;  // read next from uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE]

extern volatile int panicked;  // from printf.c

void uartstart();

void uartinit(void) {
    // 对 VF2 / LA2K1000：不修改波特率相关设置，沿用 U-Boot 配置；清 FIFO，打开 RX 中断
#if defined(VF2) || (defined(LOONGARCH) && defined(LA2K1000))
    WriteReg(IER, 0x00);
    WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
    // Ensure IRQ output is enabled via OUT2, and assert RTS/DTR
    WriteReg(MCR, MCR_OUT2 | MCR_RTS | MCR_DTR);
    // Only enable RX interrupt to avoid THRE interrupt storms (output uses SBI)
    WriteReg(IER, IER_RX_ENABLE);
#else
    // Generic initialization for platforms with byte-spaced 16550.
    // disable interrupts.
    WriteReg(IER, 0x00);

    // special mode to set baud rate.
    WriteReg(LCR, LCR_BAUD_LATCH);

    // LSB for baud rate of 38.4K.
    WriteReg(0, 0x03);

    // MSB for baud rate of 38.4K.
    WriteReg(1, 0x00);

    // leave set-baud mode, and set word length to 8 bits, no parity.
    WriteReg(LCR, LCR_EIGHT_BITS);

    // reset and enable FIFOs.
    WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

    // enable transmit and receive interrupts.
    WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);
#endif

    initlock(&uart_tx_lock, "uart");
}

// add a character to the output buffer and tell the
// UART to start sending if it isn't already.
// blocks if the output buffer is full.
// because it may block, it can't be called
// from interrupts; it's only suitable for use
// by write().
void uartputc(int c) {
    acquire(&uart_tx_lock);

    if (panicked) {
        for (;;);
    }
    while (uart_tx_w == uart_tx_r + UART_TX_BUF_SIZE) {
        // buffer is full. Busy-wait until TX ready, then push a byte
        while ((ReadReg(LSR) & LSR_TX_IDLE) == 0) ;
        WriteReg(THR, uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE]);
        uart_tx_r += 1;
    }
    uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE] = c;
    uart_tx_w += 1;
    uartstart();
    release(&uart_tx_lock);
}

// alternate version of uartputc() that doesn't
// use interrupts, for use by kernel printf() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void uartputc_sync(int c) {
    push_off();

    if (panicked) {
        for (;;);
    }

    // For LA2K1000, many terminal setups expect CRLF for a new line.
    // Insert a carriage return before line feed to ensure the cursor returns to column 0.
#if defined(LOONGARCH) && defined(LA2K1000)
    if (c == '\n') {
        while ((ReadReg(LSR) & LSR_TX_IDLE) == 0);
        WriteReg(THR, '\r');
    }
#endif

    // wait for Transmit Holding Empty to be set in LSR and send the character.
    while ((ReadReg(LSR) & LSR_TX_IDLE) == 0);
    WriteReg(THR, c);

    pop_off();
}

// if the UART is idle, and a character is waiting
// in the transmit buffer, send it.
// caller must hold uart_tx_lock.
// called from both the top- and bottom-half.
void uartstart() {
    while (1) {
        if (uart_tx_w == uart_tx_r) {
            // transmit buffer is empty.
            ReadReg(ISR);
            return;
        }

        if ((ReadReg(LSR) & LSR_TX_IDLE) == 0) {
            // the UART transmit holding register is full,
            // so we cannot give it another byte.
            // it will interrupt when it's ready for a new byte.
            return;
        }

        int c = uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE];
        uart_tx_r += 1;

        // maybe uartputc() is waiting for space in the buffer.
        wakeup(&uart_tx_r);

        WriteReg(THR, c);
    }
}

// read one input character from the UART.
// return -1 if none is waiting.
int uartgetc(void) {
    if (ReadReg(LSR) & 0x01) {
        // input data is ready.
        return ReadReg(RHR);
    } else {
        return -1;
    }
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from devintr().
void uartintr(void) {
#ifdef VF2
    // VF2: drain IIR sources to avoid THRE/LS/MDM storms
    while (1) {
        uint8 iir = ReadReg(ISR);
        if (iir & IIR_INT_PENDING) break;  // no pending
        uint8 iid = iir & IIR_ID_MASK;
        switch (iid) {
            case IIR_ID_RDA:
            case IIR_ID_RX_TIMEOUT:
                // read all available data
                while (ReadReg(LSR) & LSR_RX_READY) {
                    int ch = ReadReg(RHR);
                    consoleintr(ch);
                }
                break;
            case IIR_ID_THRE:
                // disable TX interrupt; we use polling-based uartstart
                WriteReg(IER, IER_RX_ENABLE);
                break;
            case IIR_ID_LS:
                (void)ReadReg(LSR);
                break;
            case IIR_ID_MODEM:
                (void)(*(Reg(MSR)));
                break;
            default:
                // defensive clear
                (void)ReadReg(LSR);
                while (ReadReg(LSR) & LSR_RX_READY) (void)ReadReg(RHR);
                break;
        }
    }
    acquire(&uart_tx_lock);
    uartstart();
    release(&uart_tx_lock);
#else
    // Generic path: read any pending rx and kick tx
    while (1) {
        int c = uartgetc();
        if (c == -1) break;
        consoleintr(c);
    }

    acquire(&uart_tx_lock);
    uartstart();
    release(&uart_tx_lock);
#endif
}
