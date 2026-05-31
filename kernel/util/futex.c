#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "lock/lock.h"
#include "proc/proc.h"
#include "defs.h"
#include "futex.h"
#include "lib/list.h"

// Global futex hash table
struct futex_bucket futex_table[FUTEX_HASHSIZE];

// Hash function for futex addresses
static uint32 futex_hash(uint64 uaddr) { return (uaddr >> 2) % FUTEX_HASHSIZE; }

// Initialize futex subsystem
void futex_init(void) {
    for (int i = 0; i < FUTEX_HASHSIZE; i++) {
        initlock(&futex_table[i].lock, "futex");
        list_init(&futex_table[i].waiters);
    }
}

// Wait on a futex
uint64 futex_wait(uint64 uaddr, uint32 val) {
    struct proc *p = myproc();
    uint32 bucket_id = futex_hash(uaddr);
    struct futex_bucket *bucket = &futex_table[bucket_id];
    struct futex_waiter waiter;
    uint32 current_val;

    // Copy current value from user space
    if (copyin(p->pagetable, (char *)&current_val, uaddr, sizeof(uint32)) < 0) {
        return -1;
    }

    // Check if value matches expected value
    if (current_val != val) {
        return -1;  // EAGAIN equivalent
    }

    // Initialize waiter
    waiter.uaddr = uaddr;
    waiter.p = p;
    waiter.val = val;
    waiter.woken = 0;
    list_init(&waiter.list);

    acquire(&bucket->lock);

    // Add to wait queue
    list_add(&waiter.list, &bucket->waiters);

    // Sleep until woken or interrupted
    while (!waiter.woken && p->killed == 0) {
        sleep(&waiter, &bucket->lock);
    }

    // Remove from wait queue
    list_del(&waiter.list);

    release(&bucket->lock);

    if (p->killed) {
        return -1;  // EINTR equivalent
    }

    return 0;
}

// Wake up processes waiting on a futex
uint64 futex_wake(uint64 uaddr, int nr_wake) {
    uint32 bucket_id = futex_hash(uaddr);
    struct futex_bucket *bucket = &futex_table[bucket_id];
    struct futex_waiter *waiter;
    struct list_head *pos, *next;
    int woken = 0;

    if (nr_wake <= 0) {
        return 0;
    }

    acquire(&bucket->lock);

    // Wake up to nr_wake processes waiting on this address
    for (pos = bucket->waiters.next; pos != &bucket->waiters && woken < nr_wake; pos = next) {
        next = pos->next;
        waiter = container_of(pos, struct futex_waiter, list);

        if (waiter->uaddr == uaddr) {
            waiter->woken = 1;
            wakeup(waiter);
            woken++;
        }
    }

    release(&bucket->lock);

    return woken;
}