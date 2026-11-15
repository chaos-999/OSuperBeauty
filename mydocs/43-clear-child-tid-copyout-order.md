# basic 全部通过但出现 copyout failed：clear_child_tid 清理顺序问题

> 记录 `os_serial_out_rv.txt` 中 basic 测试全部通过、但每个子进程退出时
> 都打印一次 `copyout failed: ... len = 4` 的现象。
>
> 结论：**不是 basic 测试失败，也不是用户数据写坏。根因是普通进程退出时
> `exit()` 先 `vmaunmap()` 清掉 TLS/TCB 所在 VMA，之后才 `copyout` 清零
> `clear_child_tid`，此时页表里已经没有该页，copyout 正确返回失败。**
>
> 真正线程退出的路径不会清 VMA，所以 `pthread_join` 语义没有受到影响。

---

## 1. 问题现象

最新 RV 日志中：

```text
#### OS COMP TEST GROUP START basic-glibc ####
...
#### OS COMP TEST GROUP END basic-glibc ####
#### OS COMP TEST GROUP START basic-musl ####
...
#### OS COMP TEST GROUP END basic-musl ####
```

basic 已经全部通过，但运行过程中反复出现：

```text
copyout failed: va0 = 3fffffb000, dstva = 3fffffb0f0, remaining len = 4, copied = 0
```

统计当前日志：

```text
copyout failed 总数：65

62 次  dstva = 3fffffb0f0
 2 次  dstva = 3fffff90f0
 1 次  dstva = 1c20d0
```

而 basic 测试数量正好是：

```text
basic-glibc: 32 个
basic-musl:  32 个
合计：64 个
```

也就是：

```text
每个 basic 子进程退出时 1 次
+
busybox-glibc 中某个子进程退出时 1 次
= 65 次
```

这不是测试点里的普通 `read` / `write` / `copyout` 失败，而是**进程退出路径上的清理动作**。

---

## 2. copyout 本身没有错

`kernel/mm/vm.c` 的 `copyout()`：

```c
while (len > 0) {
    va0 = PGROUNDDOWN(dstva);
    pa0 = walkaddr(pagetable, va0);
    if (pa0 == 0) {
        if (!vmatrylazytouch(va0)) {
            printf("copyout failed: va0 = %lx, dstva = %lx, remaining len = %ld, copied = %ld\n",
                   va0, dstva, len, original_len - len);
            return -1;
        }
        ...
    }
    ...
}
```

它做的事很正确：

```text
目标用户页在页表里吗？
  不在
    → 尝试按需分配
      → 没有对应 VMA，失败
        → 打印 copyout failed
```

所以问题不在这里，而在于**调用 copyout 的时机**。

---

## 3. 这个 4 字节 copyout 是谁发起的

日志中：

```text
remaining len = 4
```

正好对应 `exit()` 里的：

```c
int clear = 0;
copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
```

`clear_child_tid` 来自：

```c
set_tid_address(int *tidptr);
```

glibc/musl 启动时一般会调用：

```c
set_tid_address(&self->tid);
```

内核记录：

```c
p->clear_child_tid = tidptr;
```

这个指针通常位于：

- TLS / TCB 区域；
- mmap 出来的线程运行时区域；
- 堆或栈附近的私有地址。

日志里的地址：

```text
3fffffb0f0
3fffff90f0
```

位于 `TRAPFRAME` 下方的 mmap 区域：

```text
MAXVA      = 1 << 38 = 0x4000000000
TRAPFRAME  = MAXVA - 0x3000
MMAPEND    = TRAPFRAME
```

而：

```text
1c20d0
```

是 busybox 进程里的另一个 VMA/堆区域。

---

## 4. clear_child_tid 的作用

它不是退出状态，也不是文件描述符，而是**线程退出通知指针**。

### 4.1 pthread_join 的 futex 协议

`pthread_create` 创建线程时，通常带：

```c
clone(CLONE_VM | CLONE_THREAD | CLONE_CHILD_CLEARTID, ..., ctid = &td->tid)
```

内核记录：

```c
p->clear_child_tid = ctid;
```

线程退出时，内核做：

```c
*clear_child_tid = 0;
futex_wake(clear_child_tid, 1);
```

`pthread_join` 那边：

```text
读取 *tid
  → 非 0
      → futex_wait(&tid) 睡眠
  → 被唤醒后重新检查
      → 变成 0，说明线程已经退出
      → 继续回收线程资源
```

注意：

- `pthread_join` 等的是 **futex 地址上的值变化**；
- 不是等 `SIGCHLD`；
- `SIGCHLD` 是发给**父进程**的，不是给 join 用的。

### 4.2 它不是继承字段

`clear_child_tid` 是每个 task/thread 自己的字段：

```c
struct proc {
    ...
    uint64 clear_child_tid;
    ...
};
```

- `fork()` 时子进程的 `clear_child_tid` 会被清零，不继承父进程；
- `clone()` 只有在 `CLONE_CHILD_CLEARTID` 时才由父进程显式传入；
- exec 后由新程序的 libc 启动代码重新调用 `set_tid_address()`。

所以 basic 子进程的 `clear_child_tid` 不是从 init 或曾祖父进程继承来的，
而是它自己的 libc 启动时设置的。

---

## 5. 根因：exit() 清理顺序不对

`kernel/proc/proc.c` 当前 `exit()` 的关键顺序：

```c
void exit(int status) {
    struct proc *p = myproc();
    int is_thread = (p->tid != p->tgid);

    if (!is_thread) {
        // 1. 关闭文件
        for (...) { ... }

        // 2. 清理 VMA，并 unmap 页表
        if (!p->pt_shared) {
            for (int i = 0; i < NVMA; i++) {
                struct vma *v = &p->vmas[i];
                if (v->valid) {
                    vmaunmap(p->pagetable, v->addr, v->length, v);
                    ...
                    v->valid = 0;
                }
            }
        }
    }

    ...

    // 3. 最后才处理 clear_child_tid
    if (p->clear_child_tid) {
        int clear = 0;
        copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
        futex_wake(p->clear_child_tid, 1);
        p->clear_child_tid = 0;
    }
}
```

执行顺序变成：

```text
exit()
  → vmaunmap() 把 TLS/TCB 所在 VMA 清掉
  → copyout(clear_child_tid)
      → clear_child_tid 所在页已经不在页表
      → vmatrylazytouch 找不到有效 VMA
      → copyout 失败，打印 warning
```

`remaining len = 4` 正好是 `int clear` 的大小。

---

## 6. 为什么 basic 仍然全部通过

这个 copyout 的目的只是：

```c
*clear_child_tid = 0;
```

对于普通进程退出：

- 进程马上变成 ZOMBIE；
- 页表随后整体释放；
- 没有别的线程还需要这个地址；
- copyout 失败不影响退出码；
- 父进程 `wait()` 仍然拿到正确状态。

所以：

```text
copyout 失败是日志噪声，不是测试数据错误；
basic 全部通过是合理的。
```

---

## 7. 为什么真正线程退出不受影响

`exit()` 里有一个关键判断：

```c
int is_thread = (p->tid != p->tgid);

if (!is_thread) {
    // 只有普通进程退出才 vmaunmap()
}
```

所以：

- **线程退出（`is_thread == true`）**
  - 不执行 `vmaunmap()`；
  - 地址空间和 VMA 仍有效；
  - `clear_child_tid` 能正常清零；
  - `futex_wake` 正常唤醒 `pthread_join`。

- **普通进程退出（`is_thread == false`）**
  - 先 `vmaunmap()`；
  - 再 copyout `clear_child_tid`；
  - 此时页已经没了，于是失败。

因此目前 `pthread_join` 的语义没有被破坏，问题集中在普通进程退出路径。

---

## 8. 解决方案

### 方案 A：把 clear_child_tid 处理提前到 VMA unmap 之前

这是语义最完整的修复。

```c
void exit(int status) {
    struct proc *p = myproc();

    if (p == initproc) panic("init exiting");

    int is_thread = (p->tid != p->tgid);

    /*
     * 先清零 clear_child_tid 并唤醒 futex 等待者。
     * 此时 TLS/TCB 所在页还在，copyout 不会失败。
     */
    if (p->clear_child_tid) {
        int clear = 0;
        copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));

        extern uint64 futex_wake(uint64 uaddr, int nr_wake);
        futex_wake(p->clear_child_tid, 1);
        p->clear_child_tid = 0;
    }

    if (!is_thread) {
        // 关闭文件
        // vmaunmap() 清理 VMA
    }

    ...
}
```

顺序变成：

```text
clear_child_tid 清零 + futex_wake
  → 关闭文件
  → vmaunmap
  → 进入 ZOMBIE
```

保证通知发生在页表还有效的时候。

### 方案 B：普通进程退出时跳过 clear_child_tid

普通进程没有 `pthread_join` 在等它，因此可以只在真正线程退出时处理：

```c
if (p->clear_child_tid && is_thread) {
    int clear = 0;
    copyout(p->pagetable, p->clear_child_tid, (char *)&clear, sizeof(clear));
    futex_wake(p->clear_child_tid, 1);
    p->clear_child_tid = 0;
}
```

这样普通进程就不会再打印 copyout warning。

### 推荐

推荐 **方案 A**：

- 保留 `clear_child_tid` 的完整语义；
- 不依赖“普通进程一定没有 joiner”这个隐含假设；
- 从顺序上消除 page table 已经销毁后再写用户地址的问题。

---

## 9. Debug 过程复盘

这次问题的发现过程：

1. 先确认 basic 测试全部通过；
2. grep 日志，发现 `copyout failed` 集中在 basic 子进程退出时；
3. 统计地址和次数：
   - 64 次对应 32 + 32 个 basic 子进程；
   - 1 次对应 busybox 子进程；
4. 看到 `remaining len = 4`，定位到 `int clear` 清零操作；
5. 检查 `exit()`：
   - 先 `vmaunmap()`；
   - 后 `copyout(clear_child_tid)`；
6. 检查 `is_thread`：
   - 真正线程退出不清 VMA；
   - 普通进程退出才走问题路径；
7. 结论：不是 copyout 实现错误，而是退出清理顺序问题；
8. 确认 copyout 失败不影响 basic 退出状态，所以测试仍全部通过。

---

## 10. 经验总结

1. **看到 copyout failed 不要先怀疑 copyout。** 先看调用者传进来的用户地址是否还在 VMA 里。
2. **退出路径的清理顺序很重要。** 先销毁资源，再访问资源，必然失败。
3. **区分“线程退出”和“普通进程退出”。** 两者资源清理策略不同。
4. **clear_child_tid 不是信号，也不是退出状态。** 它是线程退出时给 futex joiner 的通知地址。
5. **日志噪声和功能错误要分开。** 这次 copyout 失败不影响测试结果，但属于应该修掉的清理顺序问题。

---

## 11. 一句话总结

```text
copyout failed 的 4 字节不是 basic 测试写坏了数据，
而是普通进程 exit() 先 vmaunmap 掉 TLS/TCB 所在 VMA，
再 copyout 清零 clear_child_tid，访问了已经不存在的用户页。

真正线程退出不清 VMA，所以 pthread_join 正常；
修复方式是把 clear_child_tid 清零和 futex_wake 提前到 VMA unmap 之前。
```
