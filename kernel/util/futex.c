#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "lock/lock.h"
#include "proc/proc.h"
#include "defs.h"
#include "futex.h"
#include "lib/list.h"
#include "errno.h"

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

/*
 * Common futex wait.  Linux ABI: return 0 on wake, -EAGAIN if *uaddr != val,
 * -EINTR when interrupted, -EFAULT on bad user address, -EINVAL for a zero
 * bitset.  glibc aborts with "The futex facility returned an unexpected
 * error code" if it sees anything else (e.g. the old -1/EPERM).
 */
static uint64 futex_wait_common(uint64 uaddr, uint32 val, uint32 bitset) {
    struct proc *p = myproc();
    uint32 bucket_id = futex_hash(uaddr);
    struct futex_bucket *bucket = &futex_table[bucket_id];
    struct futex_waiter waiter;
    uint32 current_val;

    if (bitset == 0) return -EINVAL;

    if (copyin(p->pagetable, (char *)&current_val, uaddr, sizeof(uint32)) < 0) {
        return -EFAULT;
    }
    if (current_val != val) {
        return -EAGAIN;
    }

    waiter.uaddr = uaddr;
    waiter.p = p;
    waiter.val = val;
    waiter.bitset = bitset;
    waiter.woken = 0;
    list_init(&waiter.list);

    acquire(&bucket->lock);
    list_add(&waiter.list, &bucket->waiters);

    while (!waiter.woken && p->killed == 0) {
        sleep(&waiter, &bucket->lock);
    }

    list_del(&waiter.list);
    release(&bucket->lock);

    if (p->killed) return -EINTR;
    return 0;
}

uint64 futex_wait(uint64 uaddr, uint32 val) {
    return futex_wait_common(uaddr, val, FUTEX_BITSET_MATCH_ANY);
}

uint64 futex_wait_bitset(uint64 uaddr, uint32 val, uint32 bitset) {
    return futex_wait_common(uaddr, val, bitset);
}

static uint64 futex_wake_common(uint64 uaddr, int nr_wake, uint32 bitset) {
    uint32 bucket_id = futex_hash(uaddr);
    struct futex_bucket *bucket = &futex_table[bucket_id];
    struct futex_waiter *waiter;
    struct list_head *pos, *next;
    int woken = 0;

    if (bitset == 0) return -EINVAL;
    if (nr_wake <= 0) return 0;

    acquire(&bucket->lock);

    for (pos = bucket->waiters.next; pos != &bucket->waiters && woken < nr_wake; pos = next) {
        next = pos->next;
        waiter = container_of(pos, struct futex_waiter, list);

        if (waiter->uaddr == uaddr && (waiter->bitset & bitset)) {
            waiter->woken = 1;
            wakeup(waiter);
            woken++;
        }
    }

    release(&bucket->lock);
    return woken;
}

uint64 futex_wake(uint64 uaddr, int nr_wake) {
    return futex_wake_common(uaddr, nr_wake, FUTEX_BITSET_MATCH_ANY);
}

uint64 futex_wake_bitset(uint64 uaddr, int nr_wake, uint32 bitset) {
    return futex_wake_common(uaddr, nr_wake, bitset);
}