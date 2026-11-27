#pragma once

#ifndef __TIME_H__
#define __TIME_H__

#include "types.h"

#define FREQUENCY 10000000L

/*
 * 定时器时基与调度 tick 的换算。
 *
 * RV  : timebase 10 MHz，trap.c 每 100000 cycles 触发一次 -> 100 Hz
 * LA  : QEMU stable timer 100 MHz，TCFG 周期 0x1000000 -> 约 5.96 Hz
 *
 * 不要在 syscall 里直接写死 "* 100" 或 "* 10"；统一用
 * timespec_to_ticks()，后期换真实硬件时只需要改这两个宏。
 */
#ifdef RISCV
#define TIMEBASE_FREQ 10000000ULL
#define TIMER_TICK_CYCLES 100000ULL
#elif defined(LOONGARCH)
#define TIMEBASE_FREQ 100000000ULL
#define TIMER_TICK_CYCLES 0x1000000ULL
#else
#error "unknown architecture for TIMEBASE_FREQ"
#endif

static inline uint64 timespec_to_ticks(int64 sec, int64 nsec) {
    uint64 cycles;

    if (sec < 0) sec = 0;
    if (nsec < 0) nsec = 0;
    if (nsec >= 1000000000LL) {
        sec += nsec / 1000000000LL;
        nsec %= 1000000000LL;
    }

    cycles = (uint64)sec * TIMEBASE_FREQ +
             (uint64)nsec * TIMEBASE_FREQ / 1000000000ULL;

    /* 向上取整：只要还有剩余时间，就至少等一个 tick */
    return (cycles + TIMER_TICK_CYCLES - 1) / TIMER_TICK_CYCLES;
}

#define TIME2SEC(time) (time / FREQUENCY)
#define TIME2MS(time) (time * 1000 / FREQUENCY)
#define TIME2US(time) (time * 1000 * 1000 / FREQUENCY)
#define TIME2NS(time) (time * 1000 * 1000 * 1000 / FREQUENCY)

#define TIMESEPC2NS(sepc) (sepc.tv_nsec + sepc.tv_sec * 1000 * 1000 * 1000)
#define TIMEVAL2NS(val) (val.tv_usec * 1000 + val.tv_sec * 1000000000)
#define TIMESEPC2SEC(sepc) (sepc.tv_sec + sepc.tv_nsec / (1000 * 1000 * 1000))
#define TIME2TIMESPEC(time) \
    (struct timespec) { .tv_sec = TIME2SEC(time), .tv_nsec = TIME2NS(time) % (1000 * 1000 * 1000) }

#define TIME2TIMEVAL(time) \
    (struct timeval) { .tv_sec = TIME2SEC(time), .tv_usec = TIME2US(time) % (1000 * 1000) }

#ifndef _SYS__TIMESPEC_H_  // <-- 使用从系统头文件中找到的宏
struct timespec {
    int64 tv_sec;
    int64 tv_nsec;
};
#endif

#ifndef _SYS__TIMESPEC_H_
struct timeval {
    int64 tv_sec;  /* Seconds */
    int64 tv_usec; /* Microseconds */
};

struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

struct itimerval {
    struct timeval it_interval; /* next value (unused, single-shot alarm) */
    struct timeval it_value;    /* current value */
};
#endif

#define ITIMER_REAL 0 /* setitimer: real-time timer */

extern uint ticks;

struct tms {
    long tms_utime;
    long tms_stime;
    long tms_cutime;
    long tms_cstime;
};

struct utsname {
    char sysname[65];  /* Operating system name */
    char nodename[65]; /* Name within network */
    char release[65];  /* OS release */
    char version[65];  /* OS version */
    char machine[65];  /* Hardware identifier */
};

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9
#define CLOCK_TAI 11

// Flags for clock_nanosleep
#define TIMER_ABSTIME 1

#endif
