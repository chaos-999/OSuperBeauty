#pragma once
#include "types.h"
#include "lock/lock.h"
#include "param.h"
#include "platform.h"
#include "proc/proc.h"

typedef struct semaphore {
    volatile int value;
    volatile int wakeup;
    struct spinlock lock;
    struct proc *wait_list[NPROC];
    int top;
} sem;

void sem_init(sem *s, int value, char *name);

void sem_p(sem *s);

void sem_v(sem *s);
