#ifndef SBI_H
#define SBI_H
#include "types.h"

struct sbiret {
    uint64 error;
    uint64 value;
};

// 使用OpenSBI v0.2的传统扩展
#define SBI_LEGACY_ECALL(__num, __a0, __a1, __a2)                                \
    ({                                                                           \
        register unsigned long a0 asm("a0") = (unsigned long)(__a0);             \
        register unsigned long a1 asm("a1") = (unsigned long)(__a1);             \
        register unsigned long a2 asm("a2") = (unsigned long)(__a2);             \
        register unsigned long a7 asm("a7") = (unsigned long)(__num);            \
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory"); \
        a0;                                                                      \
    })

// Legacy Extensions (Pre-SBI v0.2)
#define SBI_LEGACY_PUTCHAR_NUM 0x01
#define SBI_LEGACY_GETCHAR_NUM 0x02
#define SBI_LEGACY_SET_TIMER_NUM 0x00

// SBI_PUTCHAR()是一个阻塞的函数，直到字符被成功输出到串口才会返回
#define SBI_PUTCHAR(ch) SBI_LEGACY_ECALL(SBI_LEGACY_PUTCHAR_NUM, ch, 0, 0)

// 成功返回读到的字符，失败返回-1
// SBI_GETCHAR()是非阻塞的，意味着如果没有读到字符，会直接返回-1
#define SBI_GETCHAR() SBI_LEGACY_ECALL(SBI_LEGACY_GETCHAR_NUM, 0, 0, 0)

// 设置时钟中断触发时间
#define SBI_LEGACY_SET_TIMER(stime_value) \
    SBI_LEGACY_ECALL(SBI_LEGACY_SET_TIMER_NUM, stime_value, 0, 0)

// 使用OpenSBI v0.2+的标准扩展
#define SBI_ECALL(__eid, __fid, __a0, __a1, __a2, __a3, __a4)              \
    ({                                                                     \
        unsigned long error = 0, value = 0;                                \
        register unsigned long a0 asm("a0") = (unsigned long)(__a0);       \
        register unsigned long a1 asm("a1") = (unsigned long)(__a1);       \
        register unsigned long a2 asm("a2") = (unsigned long)(__a2);       \
        register unsigned long a3 asm("a3") = (unsigned long)(__a3);       \
        register unsigned long a4 asm("a4") = (unsigned long)(__a4);       \
        register unsigned long a6 asm("a6") = (unsigned long)(__fid);      \
        register unsigned long a7 asm("a7") = (unsigned long)(__eid);      \
        asm volatile(                                                      \
            "ecall\n"                                                      \
            "mv %[error], a0\n"                                            \
            "mv %[value], a1\n"                                            \
            : "+r"(a0), "+r"(a1), [error] "=r"(error), [value] "=r"(value) \
            : "r"(a2), "r"(a3), "r"(a4), "r"(a6), "r"(a7)                  \
            : "memory");                                                   \
        (struct sbiret){error, value};                                     \
    })

// Standard SBI Extension IDs
#define SBI_TIMER_EID 0x54494D45
#define SBI_IPI_EID 0x735049
#define SBI_RFENCE_EID 0x52464E43
#define SBI_HSM_EID 0x48534D
#define SBI_SRST_EID 0x53525354

// Function IDs for each extension
#define SBI_SET_TIMER_FID 0
#define SBI_IPI_SEND_IPI_FID 0
#define SBI_RFENCE_FENCEI_FID 0
#define SBI_RFENCE_SFENCE_VMA_FID 1
#define SBI_RFENCE_SFENCE_VMA_ASID_FID 2
#define SBI_HSM_HART_START_FID 0
#define SBI_HSM_HART_STOP_FID 1
#define SBI_HSM_HART_GET_STATUS_FID 2
#define SBI_HSM_HART_SUSPEND_FID 3
#define SBI_SRST_SYSTEM_RESET_FID 0

// Hart状态定义
#define STARTED 0
#define STOPPED 1
#define START_PENDING 2
#define STOP_PENDING 3
#define SUSPENDED 4
#define SUSPEND_PENDING 5
#define RESUME_PENDING 6

// System Reset类型定义
#define SBI_SRST_RESET_TYPE_SHUTDOWN 0
#define SBI_SRST_RESET_TYPE_COLD_REBOOT 1
#define SBI_SRST_RESET_TYPE_WARM_REBOOT 2

// System Reset原因定义
#define SBI_SRST_RESET_REASON_NONE 0
#define SBI_SRST_RESET_REASON_SYSTEM_FAILURE 1

// 优先使用现代接口，如果失败则回退到传统接口
#define SBI_SET_TIMER(stime_value)                                                                \
    do {                                                                                          \
        struct sbiret ret = SBI_ECALL(SBI_TIMER_EID, SBI_SET_TIMER_FID, stime_value, 0, 0, 0, 0); \
        if (ret.error != 0) SBI_LEGACY_SET_TIMER(stime_value);                                    \
    } while (0)

#define SBI_SEND_IPI(hart_mask, hart_mask_base) \
    SBI_ECALL(SBI_IPI_EID, SBI_IPI_SEND_IPI_FID, hart_mask, hart_mask_base, 0, 0, 0)

#define SBI_HART_START(hartid, start_addr, opaque) \
    SBI_ECALL(SBI_HSM_EID, SBI_HSM_HART_START_FID, hartid, start_addr, opaque, 0, 0)

#define SBI_HART_STOP() SBI_ECALL(SBI_HSM_EID, SBI_HSM_HART_STOP_FID, 0, 0, 0, 0, 0)

#define SBI_HART_GET_STATUS(hartid) \
    SBI_ECALL(SBI_HSM_EID, SBI_HSM_HART_GET_STATUS_FID, hartid, 0, 0, 0, 0)

#define SBI_SYSTEM_RESET(reset_type, reset_reason) \
    SBI_ECALL(SBI_SRST_EID, SBI_SRST_SYSTEM_RESET_FID, reset_type, reset_reason, 0, 0, 0)

#define SBI_SHUTDOWN() SBI_SYSTEM_RESET(SBI_SRST_RESET_TYPE_SHUTDOWN, SBI_SRST_RESET_REASON_NONE)

#endif