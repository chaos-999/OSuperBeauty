#ifndef BUDDY_MEMORY_ALLOCATION_H
#define BUDDY_MEMORY_ALLOCATION_H

#include "types.h"

struct buddy;

struct buddy *buddy_new(int level);
void buddy_delete(struct buddy *);
int buddy_alloc(struct buddy *, int size);
void buddy_free(struct buddy *, int offset);
int buddy_size(struct buddy *, int offset);
void buddy_dump(struct buddy *);

// Kernel-specific buddy allocator functions
void buddy_init(void);
void *buddy_kalloc(uint64 size);
void buddy_kfree(void *ptr);
uint64 buddy_get_size(void *ptr);

#endif
