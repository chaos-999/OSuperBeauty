#ifndef _LOCK_H
#define _LOCK_H

// Mutual exclusion lock.
struct spinlock {
    uint locked;  // Is the lock held?

    // For debugging:
    char *name;       // Name of lock.
    struct cpu *cpu;  // The cpu holding the lock.
};

// Long-term locks for processes
struct sleeplock {
    uint locked;         // Is the lock held?
    struct spinlock lk;  // spinlock protecting this sleep lock

    // For debugging:
    char *name;  // Name of lock.
    int pid;     // Process holding lock
};

#endif
