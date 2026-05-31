#include "types.h"
#include "platform.h"
#include "defs.h"

// 交换两个元素
static void swap(void *a, void *b, size_t size) {
    char tmp[size];
    memcpy(tmp, a, size);
    memcpy(a, b, size);
    memcpy(b, tmp, size);
}

// 快速排序的分区函数
static void *partition(void *base, size_t size, void *low, void *high,
                       int (*cmp)(const void *, const void *)) {
    // 选择一个基准元素
    void *pivot = high;
    void *i = low - size;
    for (void *j = low; j != high; j += size) {
        // 如果当前元素小于或等于基准
        if (cmp(j, pivot) <= 0) {
            i += size;
            swap(i, j, size);
        }
    }
    swap(i + size, high, size);
    return (i + size);
}

// 快速排序递归函数
static void quicksort(void *base, size_t size, void *low, void *high,
                      int (*cmp)(const void *, const void *)) {
    if (low < high) {
        // pi 为分区后的基准索引
        void *pi = partition(base, size, low, high, cmp);

        // 分别对左侧和右侧子数组进行递归排序
        quicksort(base, size, low, pi - size, cmp);
        quicksort(base, size, pi + size, high, cmp);
    }
}

// 通用的qsort函数
void qsort(void *base, size_t nitems, size_t size, int (*cmp)(const void *, const void *)) {
    quicksort(base, size, base, (char *)base + size * (nitems - 1), cmp);
}

// 示例比较函数
int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }