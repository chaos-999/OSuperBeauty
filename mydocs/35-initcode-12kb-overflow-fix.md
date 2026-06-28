# Initcode 12KB 溢出修复

> 修复硬编码限制导致 LA 测评 basic test 全部崩溃 (badi=29c06061) 的内核 bug。

## 现象

LA basic test 所有 fork/exec 子进程崩溃，相同错误：
```
vma lazy touch failed
usertrap(): unexpected trapcause 10004
era=496 badi=29c06061
```

修改 `init-la.c`（添加代码、取消注释、增删 printf）会触发或修复该崩溃，但真正的根因不在 init-la.c。

## 根因

initcode 二进制嵌入内核，内核中两处硬编码假设 initcode ≤ 12KB（3 页）：

### 1. `kernel/mm/vm.c` — `uvmfirst()` 只分配 3 页

```c
void uvmfirst(pagetable_t pagetable, uchar *src, uint sz) {
    // 只分配了 page 0, 1, 2 — 共 12KB
    mem = kalloc(); mappages(pagetable, 0, PGSIZE, ...);     // page 0
    mem = kalloc(); mappages(pagetable, PGSIZE, PGSIZE, ...); // page 1
    mem = kalloc(); mappages(pagetable, 2*PGSIZE, PGSIZE, ...); // page 2
    // 没有 page 3！sz > 12KB 的数据被丢弃
}
```

当前 initcode = 13040 字节 → 需要 4 页（16KB），超出部分被丢弃。

### 2. `kernel/proc/proc.c` — `userinit()` 的 `p->sz` 硬编码

```c
p->sz = 3 * PGSIZE; // 始终 12KB，与实际 initcode 大小无关
```

`fork()` 时 `uvmcopy()` 以此值确定复制范围。子进程缺失超出 12KB 的页面，exec 时读取垃圾数据 → `badi=29c06061`。

## 修复

### vm.c — uvmfirst 加第 4 页

RV 和 LA 各添加 page 3 的分配：

```c
mem = kalloc();
memset(mem, 0, PGSIZE);
mappages(pagetable, 3 * PGSIZE, PGSIZE, (uint64)mem, ...);
if (sz > 3 * PGSIZE) {
    memmove(mem, src + 3 * PGSIZE, MIN(sz - 3 * PGSIZE, PGSIZE));
}
```

### proc.c — userinit 动态计算

```c
// 旧
p->sz = 3 * PGSIZE;
p->trapframe->sp = 3 * PGSIZE;

// 新
p->sz = PGROUNDUP((uint64)(initcode_end - initcode_start));
p->trapframe->sp = PGROUNDUP((uint64)(initcode_end - initcode_start));
```

## 影响

- RV 和 LA 均受益，initcode 最大支持 16KB（4 页）
- init-la.c 不再受代码量限制，可正常添加 libctest 等测试逻辑
