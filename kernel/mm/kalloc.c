// Physical memory allocator, now using buddy allocator
// Provides compatibility interface for existing kernel code

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include <lock/lock.h>
#include "platform.h"
#include "defs.h"
#include "buddy.h"

// Initialize memory allocator using buddy system
void kinit() { buddy_init(); }

// Allocate one page of physical memory using buddy allocator
void *kalloc(void) { return buddy_kalloc(PGSIZE); }

// Free memory using buddy allocator
void kfree(void *pa) {
    if (!pa) return;

    // Fill with junk to catch dangling refs, just like original
    memset(pa, 1, PGSIZE);
    buddy_kfree(pa);
}

// Allocate variable size memory
void *kmalloc(uint64 size) { return buddy_kalloc(size); }

// Allocate cleared memory
void *kcalloc(uint n, uint64 size) {
    if (n == 0 || size == 0) return 0;

    // Check for overflow
    if (size != 0 && n > ((uint64)-1) / size) {
        return 0;
    }

    uint64 total = n * size;
    void *ptr = buddy_kalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}