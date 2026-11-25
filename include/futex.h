#pragma once

#include "types.h"
#include "lock/lock.h"
#include "lib/list.h"

// Futex operation flags
#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

// Maximum number of futex hash buckets
#define FUTEX_HASHSIZE 32

// Futex waiter structure
struct futex_waiter {
    struct list_head list;  // List node for hash bucket
    uint64 uaddr;           // User space address
    struct proc *p;         // Waiting process
    uint32 val;             // Expected value
    int woken;              // Whether process has been woken
};

// Futex hash bucket
struct futex_bucket {
    struct spinlock lock;
    struct list_head waiters;
};

// Global futex hash table
extern struct futex_bucket futex_table[FUTEX_HASHSIZE];

// Function prototypes
void futex_init(void);
uint64 futex_wait(uint64 uaddr, uint32 val);
uint64 futex_wake(uint64 uaddr, int nr_wake);