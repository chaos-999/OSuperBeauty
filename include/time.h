#pragma once

#ifndef __TIME_H__
#define __TIME_H__

#include "types.h"

#define FREQUENCY 10000000L

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
#endif

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
