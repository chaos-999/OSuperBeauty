#include "buddy.h"
#include "types.h"
#include "param.h"
#include "platform.h"
#include "memlayout.h"
#include "defs.h"
#include <lock/lock.h>

#define NODE_UNUSED 0
#define NODE_USED 1
#define NODE_SPLIT 2
#define NODE_FULL 3

struct buddy {
    int level;
    uint8_t tree[1];
};

// Support up to 512MB (131072 pages, level=17)
#define MAX_BUDDY_LEVEL 17
static char
    buddy_memory[sizeof(struct buddy) + sizeof(uint8_t) * (((1 << MAX_BUDDY_LEVEL) * 2) - 2)];
static struct buddy *global_buddy = 0;

struct buddy *buddy_new(int level) {
    int size = 1 << level;
    uint64 required_size = sizeof(struct buddy) + sizeof(uint8_t) * (size * 2 - 2);
    if (required_size > sizeof(buddy_memory)) {
        panic("buddy_new: insufficient memory for buddy allocator");
    }
    struct buddy *self = (struct buddy *)buddy_memory;
    self->level = level;
    memset(self->tree, NODE_UNUSED, size * 2 - 1);
    global_buddy = self;
    return self;
}

void buddy_delete(struct buddy *self) {
    // In kernel, we don't delete the global buddy allocator
    global_buddy = 0;
}

static inline int is_pow_of_2(uint32_t x) { return !(x & (x - 1)); }

static inline uint32_t next_pow_of_2(uint32_t x) {
    if (is_pow_of_2(x)) return x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

static inline int _index_offset(int index, int level, int max_level) {
    return ((index + 1) - (1 << level)) << (max_level - level);
}

static void _mark_parent(struct buddy *self, int index) {
    for (;;) {
        int buddy = index - 1 + (index & 1) * 2;
        if (buddy > 0 && (self->tree[buddy] == NODE_USED || self->tree[buddy] == NODE_FULL)) {
            index = (index + 1) / 2 - 1;
            self->tree[index] = NODE_FULL;
        } else {
            return;
        }
    }
}

int buddy_alloc(struct buddy *self, int s) {
    int size;
    if (s == 0) {
        size = 1;
    } else {
        size = (int)next_pow_of_2(s);
    }
    int length = 1 << self->level;

    if (size > length) return -1;

    int index = 0;
    int level = 0;

    while (index >= 0) {
        if (size == length) {
            if (self->tree[index] == NODE_UNUSED) {
                self->tree[index] = NODE_USED;
                _mark_parent(self, index);
                return _index_offset(index, level, self->level);
            }
        } else {
            // size < length
            switch (self->tree[index]) {
                case NODE_USED:
                case NODE_FULL:
                    break;
                case NODE_UNUSED:
                    // split first
                    self->tree[index] = NODE_SPLIT;
                    self->tree[index * 2 + 1] = NODE_UNUSED;
                    self->tree[index * 2 + 2] = NODE_UNUSED;
                default:
                    index = index * 2 + 1;
                    length /= 2;
                    level++;
                    continue;
            }
        }
        if (index & 1) {
            ++index;
            continue;
        }
        for (;;) {
            level--;
            length *= 2;
            index = (index + 1) / 2 - 1;
            if (index < 0) return -1;
            if (index & 1) {
                ++index;
                break;
            }
        }
    }

    return -1;
}

static void _combine(struct buddy *self, int index) {
    for (;;) {
        int buddy = index - 1 + (index & 1) * 2;
        if (buddy < 0 || self->tree[buddy] != NODE_UNUSED) {
            self->tree[index] = NODE_UNUSED;
            while (((index = (index + 1) / 2 - 1) >= 0) && self->tree[index] == NODE_FULL) {
                self->tree[index] = NODE_SPLIT;
            }
            return;
        }
        index = (index + 1) / 2 - 1;
    }
}

void buddy_free(struct buddy *self, int offset) {
    if (offset >= (1 << self->level)) {
        printf("buddy_free: WARNING invalid offset %d (max %d)\n", offset, 1 << self->level);
        return;
    }
    int left = 0;
    int length = 1 << self->level;
    int index = 0;

    for (;;) {
        switch (self->tree[index]) {
            case NODE_USED:
                if (offset != left) {
                    printf("buddy_free: WARNING offset mismatch (expected %d, got %d)\n", left, offset);
                    return;
                }
                _combine(self, index);
                return;
            case NODE_UNUSED:
                printf("buddy_free: WARNING freeing unused block at offset %d\n", offset);
                return;
            default:
                length /= 2;
                if (offset < left + length) {
                    index = index * 2 + 1;
                } else {
                    left += length;
                    index = index * 2 + 2;
                }
                break;
        }
    }
}

int buddy_size(struct buddy *self, int offset) {
    if (offset >= (1 << self->level)) panic("buddy_size: invalid offset");
    int left = 0;
    int length = 1 << self->level;
    int index = 0;

    for (;;) {
        switch (self->tree[index]) {
            case NODE_USED:
                if (offset != left) panic("buddy_size: offset mismatch");
                return length;
            case NODE_UNUSED:
                panic("buddy_size: checking unused block");
                return length;
            default:
                length /= 2;
                if (offset < left + length) {
                    index = index * 2 + 1;
                } else {
                    left += length;
                    index = index * 2 + 2;
                }
                break;
        }
    }
}

static void _dump(struct buddy *self, int index, int level) {
    switch (self->tree[index]) {
        case NODE_UNUSED:
            printf("(%d:%d)", _index_offset(index, level, self->level), 1 << (self->level - level));
            break;
        case NODE_USED:
            printf("[%d:%d]", _index_offset(index, level, self->level), 1 << (self->level - level));
            break;
        case NODE_FULL:
            printf("{");
            _dump(self, index * 2 + 1, level + 1);
            _dump(self, index * 2 + 2, level + 1);
            printf("}");
            break;
        default:
            printf("(");
            _dump(self, index * 2 + 1, level + 1);
            _dump(self, index * 2 + 2, level + 1);
            printf(")");
            break;
    }
}

void buddy_dump(struct buddy *self) {
    _dump(self, 0, 0);
    printf("\n");
}

// Global physical memory base for buddy allocator
static char *buddy_mem_base = 0;
static uint64 buddy_mem_size = 0;

// Initialize buddy allocator with physical memory
void buddy_init(void) {
#ifdef RISCV
    extern char end[];
    buddy_mem_base = (char *)PGROUNDUP((uint64)end);
    buddy_mem_size = PHYSTOP - (uint64)buddy_mem_base;
#elif defined(LOONGARCH)
    buddy_mem_base = (char *)PHYSBASE;
    buddy_mem_size = PHYSTOP - PHYSBASE;
#endif

    // Calculate buddy level - we need power of 2 that fits our memory
    // Also ensure it fits within our static buffer size
    int level = 0;
    uint64 size = PGSIZE;             // Start with page size
    int max_level = MAX_BUDDY_LEVEL;  // Allow managing up to 512MB

    while (size < buddy_mem_size && level < max_level) {
        level++;
        size <<= 1;
    }

    // Ensure we don't exceed our static buffer capacity
    if (level > max_level) {
        level = max_level;
        size = PGSIZE << level;
    }

    printf("buddy_init: memory range [%p, %p), size=%lu MB, level=%d, managing=%lu MB\n",
           buddy_mem_base, buddy_mem_base + buddy_mem_size, buddy_mem_size / (1024 * 1024), level,
           size / (1024 * 1024));

    buddy_new(level);
}

// Allocate memory using buddy allocator
void *buddy_kalloc(uint64 size) {
    if (!global_buddy) {
        panic("buddy_kalloc: buddy allocator not initialized");
    }

    if (size == 0) return 0;

    // Convert size to pages
    uint64 pages = (size + PGSIZE - 1) / PGSIZE;
    int offset = buddy_alloc(global_buddy, pages);

    if (offset < 0) {
        return 0;  // Allocation failed
    }

    char *addr = buddy_mem_base + (offset * PGSIZE);
    memset(addr, 0, pages * PGSIZE);  // Fill with junk like original kalloc
    return addr;
}

// Free memory using buddy allocator
void buddy_kfree(void *ptr) {
    if (!ptr || !global_buddy) return;

    char *addr = (char *)ptr;
    if (addr < buddy_mem_base || addr >= buddy_mem_base + buddy_mem_size) {
        printf("buddy_kfree: WARNING address %p out of buddy range [%p, %p)\n",
               addr, buddy_mem_base, buddy_mem_base + buddy_mem_size);
        return;
    }

    int offset = (addr - buddy_mem_base) / PGSIZE;
    buddy_free(global_buddy, offset);
}

// Get size of allocated block
uint64 buddy_get_size(void *ptr) {
    if (!ptr || !global_buddy) return 0;

    char *addr = (char *)ptr;
    if (addr < buddy_mem_base || addr >= buddy_mem_base + buddy_mem_size) {
        return 0;
    }

    int offset = (addr - buddy_mem_base) / PGSIZE;
    int pages = buddy_size(global_buddy, offset);
    return pages * PGSIZE;
}
