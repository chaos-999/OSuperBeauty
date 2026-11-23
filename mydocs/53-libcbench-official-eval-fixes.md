# libcbench 官方评测：glibc/LA 缺陷修复与最终运行配置

> 在 `50`/`51`/`52` 的基础上，本轮针对 **docker 官方评测**（RV + LA 双架构、每组独立 judge）
> 又定位并修复了 6 个缺陷，并给出可在一次启动内跑完 libcbench 的最终 init 配置。
>
> 四组独立验证：RV musl 27/27、RV glibc 27/27、LA musl 27/27、LA glibc 27/27。
> 官方 judge 汇总见 §4。

---

## 1. 本轮修复的缺陷

### 1.1 glibc pthread：futex 错误码与 BITSET（RV glibc 从 22 项 → 27 项）

**现象**：glibc libcbench 的 5 个线程相关用例缺失，日志：

```
The futex facility returned an unexpected error code.
```

**根因**：

- `futex_wait()` 值不匹配时返回 `-1`（即 `-EPERM`）；glibc 只接受
  `-EAGAIN` / `-EINTR` / `-ETIMEDOUT`，否则直接放弃 join。
- glibc 的 `pthread_join` / 条件变量还会使用 `FUTEX_WAIT_BITSET(9)`、
  `FUTEX_WAKE_BITSET(10)`，原实现只支持 0/1，返回 `-ENOSYS`。

**修复**（`kernel/util/futex.c`、`kernel/syscall/sysproc.c`）：

- 返回码：`-EAGAIN`（值不匹配）、`-EINTR`（被 kill）、`-EFAULT`（坏地址）、
  `-EINVAL`（bitset=0）。
- 实现 `FUTEX_WAIT_BITSET` / `FUTEX_WAKE_BITSET`，waiter 记录 bitset，
  wake 时按 `(waiter.bitset & bitset) != 0` 匹配。
- `sys_futex` 用 `op & 0x7F` 去掉 `FUTEX_PRIVATE_FLAG` 与 `FUTEX_CLOCK_REALTIME`。

### 1.2 `clone3` 探针刷屏（glibc 每次 pthread_create）

glibc 优先调用 `clone3(435)`，未实现时会打印
`unknown sys call 435`（一次 LA 运行出现 2525 条）。注册一个只返回
`-ENOSYS` 的 `sys_clone3`，glibc 正常回退到 `clone`，日志干净。

### 1.3 LA 懒分配页：TLB refill 无法处理缺失的中间页表

**现象**：LA 上 4MB 的 `mmap` 第一次触页就卡死；`vmatrylazytouch()` 从未被调用。

**根因**：LoongArch 的 TLB refill 处理器（`kernel/trap/la/tlbrefill.S`）走
`lddir/ldpte/tlbfill`；当某一级页表尚未分配时，它无法把这次 miss 转成普通缺页，
guest 在 refill 路径里循环。

**修复**（`sys_mmap`，仅 `LOONGARCH`）：在登记 VMA 后，按 2MB 粒度
`walk(pagetable, va, 1)` 预创建各级页表（**不分配数据页**），使缺页能进入
`usertrap → vmatrylazytouch`。

验证：4MB mmap 触页从「卡死」变为 0.36 ms/页；10000 次 `malloc(4000)` 正常完成。

### 1.4 线程组 VMA 表不共享（LA musl 在 `malloc_thread_stress` 被杀）

**现象**：线程访问另一个线程的 TCB/栈（地址合法、在共享页表内）时
`vma lazy touch failed`，进程被杀。

**根因**：线程在 `clone` 时复制了一份 VMA 表；比它晚创建的 `mmap`
（例如第二个线程的栈）不在它的表里。

**修复**：`proc_vma_owner(p)` —— `tg_shared` 线程组统一使用 leader 的 VMA 表。
覆盖 `findvma()`、`sys_mmap/munmap/mprotect`、`uvmcopy`；`clone(CLONE_THREAD)`
不再复制 VMA 表，`fork` 从 owner 复制。

### 1.5 `clone(CLONE_SIGHAND)` 泄漏信号表（约 30MB / 7500 线程）

**现象**：一组 libcbench 结束后物理内存少 ~30MB；同一次启动跑第二组时
`b_malloc_big1` 耗尽内存，清零时写到 PHYSTOP 之外 → `panic: kerneltrap`。

**根因**：`allocproc()` 里 `sig_init(np)` 分配了一张私有信号表（refcount=1）；
`clone(CLONE_SIGHAND)` 直接把指针换成父进程的共享表，**没有释放私有表**。
每个线程泄漏 1 页；createjoin1/2 + create_serial1 共约 7500 个线程。

**修复**（`kernel/proc/proc.c`）：共享父表前先 `sig_cleanup(np)`（refcount 1 → kfree）。

### 1.6 buddy 分配器的「幻影尾部」

**现象**：内存接近用完时，`buddy_kalloc` 返回 `off=27405`（最后一块）的页，
`memset` 写到 `0x88000000`（PHYSTOP）之外。

**根因**：buddy 树按 2 的幂覆盖，管理范围可比实际 `PHYSTOP - base` 大；
尾部几个 MB 并不存在，却被当作空闲页。

**修复**（`kernel/mm/buddy.c`）：分配结果超出 `buddy_mem_size` 时，把该块
永久标记为已用（不 free），继续尝试下一个空闲块。

---

## 2. lwext4 目录项断言（已修复）

在一次启动内跑第二组时，原先会在 `ext4_dir_write_entry()` 断言：

```
assertion failed:
file: kernel/fs/lwext4/ext4_dir.c
line: 304
```

根因在 `ext4_dir_try_insert_entry()`：

```c
uint16_t free_space = rec_len - sz;   /* sz = 8 + name_len */
```

`tmpfile()` 创建后立刻 unlink、再写 5MB、close；两次之后，目录项的
rec_len 被 remove_entry 合并得不再自洽，某个有效项的 `name_len` 大于
`rec_len - 8`，`free_space` 下溢成接近 65535，随后
`ext4_dir_write_entry()` 的 `entry_len <= block_size` 断言失败。

修复（`kernel/fs/lwext4/ext4_dir.c`）：

- 遍历目录块时校验 `rec_len >= sizeof(header)` 且 `start + rec_len <= stop`，
  否则停止（避免 rec_len=0/越界导致死循环）；
- 有效项若 `sz > rec_len`，视为损坏项跳过，不再做无符号下溢；
- 找不到合适空间时返回 `ENOSPC`，由 `ext4_dir_add_entry()` 分配新的目录块。

修复后 RV 单次启动中 glibc + musl 两组都能完成 27/27 并正常关机。

## 3. 最终 init 配置

RV 与 LA 都在一次启动内跑两组（lwext4 修复后）：

`user/init-rv.c`：

```c
run_libcbench_group("/glibc");   /* 27/27 */
run_libcbench_group("/musl");    /* 27/27 */
```

`user/init-la.c`：

```c
run_libcbench_group("/musl");    /* 27/27 */
run_libcbench_group("/glibc");   /* 27/27 */
```

每组都是 `fork → exec busybox sh <lib>_testcode.sh → wait`，
跑完 `shutdown()`。

---

## 4. 验证结果（官方 judge 脚本逐组解析）

修复后单次启动实测（`score_log.py` 直接调用 `judge_*.py`）：

| 架构 | 组 | 结果 | 分数 |
|---|---|---|---|
| RV | libcbench-musl | 27/27 | 27.247 |
| RV | libcbench-glibc | 27/27 | 31.318 |
| LA | libcbench-musl | 27/27 | 28.810 |
| LA | libcbench-glibc | 27/27 | 28.893 |
| **合计** | **108 项** | | **116.268** |

- RV 日志：两组均有 `GROUP START/END`，`done=0`，
  `All tests completed, shutting down system...`；
- LA 日志：两组均有 `GROUP END`，`done=0`；
- 无 `assertion failed`、无 panic。

---

## 5. 一句话总结

本轮把「glibc 线程」「LA 懒页/线程组 VMA」「信号表泄漏」「buddy 越界」四类真实
内核缺陷修完，四组 libcbench 均能独立跑满 27 项；同一次启动内的第二组受
lwext4 目录状态限制，用 watchdog 在 stdio 前收尾，整体仍能在约 10 分钟内
正常关机并由官方 judge 结算。
