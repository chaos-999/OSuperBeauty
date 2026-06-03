# 🔧 Busybox 管道打通：sendfile + ppoll 修复实录

> 📅 生成时间：`2026-06-03 15:57 (GMT+8)`
>
> 目标：记录 busybox_testcode.sh 管道 `cat | while read` 从静默失败到正常产出 testcase 的完整修复过程。

---

## 一、现象回顾

busybox_testcode.sh 执行后：

```
#### OS COMP TEST GROUP START busybox-musl ####
（零条 testcase 输出）
#### OS COMP TEST GROUP END busybox-musl ####
```

首尾 `./busybox echo` 正常，但中间 `cat | while read` 管道零产出。glibc 版本甚至直接卡死（timeout）。

---

## 二、调试过程（三阶段递进）

### 阶段 1：syscall 日志 → 发现 sendfile(71) 而非 write(64)

在 `syscall.c` 加 printf 追踪，发现 cat 进程用了 `sendfile(71)` 而非 `write(64)`。

### 阶段 2：sendfile 插桩 → 发现 16MB count

```c
printf("SENDFILE: raw count from user=%d\n", count);
// 输出：16777216 (0x1000000) — 16MB！
```

busybox cat 传给 sendfile 的 count 是 16MB，导致 kmalloc(16MB)。

进一步追踪发现：文件确实被读了 868 字节并写入了管道，但 while-read 读不到。

### 阶段 3：ppoll 追踪 → 发现时序竞争

在 `ppoll` 的 pipe 检查中加 `nread/nwrite` 日志：

```
PPOLL-PIPE: nread=0 nwrite=0  ← 999 次检查，始终为空
...ppoll 放弃 1000 次循环...
SENDFILE: writing to pipe     ← cat 之后才写数据！
```

**根本原因：ppoll 在 cat 的 sendfile 执行之前就退出了。** 单核 QEMU (-smp 1) 下，while-read 的 ppoll 跑完 1000 次 yield 循环时，cat 进程还没走到 sendfile。

同时，ppoll 原本睡在 `&ticks` 上，**pipe 写入的 `wakeup(&pi->nread)` 无法唤醒 ppoll**——导致 ppoll 完全依赖时钟中断轮询。

---

## 三、两处内核修复

### 修复 1：sendfile — cap count 到一页

**文件：** `kernel/syscall/sysproc.c`，`sys_sendfile()` 函数

```c
argint(3, &count);

// busybox cat passes 16MB count — cap to one page to prevent huge kmalloc
count = MIN(count, PGSIZE);

void *buf = kmalloc(count);
```

**原因：** busybox cat 传 16MB 作为 sendfile 的 count。虽然 buddy allocator 能分配 16MB，但这是不必要的内存开销，且会让 ext4_fread 处理巨大请求。cap 到 PGSIZE(4096) 后行为不变（文件只有 868 字节），但安全得多。

### 修复 2：ppoll — 睡在 pipe 的 `&pi->nread` 频道上

**文件：** `kernel/syscall/sysproc.c`，`sys_ppoll()` 函数

```c
// 原逻辑（有问题）：
//   每次循环 yield() — 等 CPU 调度，pipe 写入不通知 ppoll
//   
// 新逻辑：
//   收集所有 fd 的 pipe 对象 → 如果无事件，sleep(&pi->nread, &pi->lock)
//   pipewrite_kernel 的 wakeup(&pi->nread) 直接唤醒 ppoll

for (int attempt = 0; attempt < 2000 && ready_count == 0; attempt++) {
    struct pipe *sleep_pipe = 0;

    for (nfds_t i = 0; i < nfds; i++) {
        // ... 检查每个 fd ...
        struct file *ff = p->ofile[pfd->fd];
        pfd->revents = ppoll_check(p, ff, pfd->events);
        if (pfd->revents) ready_count++;
        if (!sleep_pipe && ff->f_type == FD_PIPE)
            sleep_pipe = ff->f_pipe;   // 记住第一个 pipe
    }

    if (ready_count > 0) break;
    if (timeout_ticks > 0 && ticks >= timeout_ticks) break;
    if (killed(p)) { kfree(fds); return -1; }

    if (sleep_pipe) {
        acquire(&sleep_pipe->lock);
        // 双检：拿到锁后确认仍然空
        if (sleep_pipe->nread != sleep_pipe->nwrite
            || !sleep_pipe->writeopen) {
            release(&sleep_pipe->lock);
            continue;
        }
        // sleep 释放锁 → 休眠 → 被 pipe 写入或关闭唤醒 → 重新拿锁
        sleep(&sleep_pipe->nread, &sleep_pipe->lock);
        release(&sleep_pipe->lock);
    } else {
        yield();  // 非 pipe fd，fallback 到 yield
    }
}
```

**关键设计：**

| 元素 | 说明 |
|------|------|
| `sleep(&sleep_pipe->nread, &sleep_pipe->lock)` | ppoll 睡在 pipe 的 nread 地址上 |
| `acquire(&sleep_pipe->lock)` 先于 sleep | 锁在 sleep 时自动释放，醒来时自动重新获取 |
| 双检 `if (nread != nwrite \|\| !writeopen)` | 防止获取锁和 sleep 之间的 TOCTOU 竞争 |
| `2000` 次尝试上限 | 防止极端情况下无限循环 |

**为何这解决了时序问题：**

```
原来（yield）：
  ppoll → yield → yield → ...(1000次) → 超时退出
  cat  → fork → exec → sendfile → （太晚了，ppoll 已经结束）

现在（sleep on pipe）：
  ppoll → sleep(&pi->nread) → 阻塞等待
  cat  → fork → exec → sendfile → pipewrite_kernel → wakeup(&pi->nread)
  ppoll → 被唤醒 → 检查 pipe → nread=0, nwrite=868 → POLLIN!
  ppoll 返回 → while-read → read() → 获得数据 → 逐行 eval → testcase!
```

---

## 四、修复结果

```
===== SpringOS Auto Test Runner =====
#### OS COMP TEST GROUP START busybox-musl ####
testcase busybox echo "#### independent command test" success
testcase busybox ash -c exit success
testcase busybox sh -c exit success
testcase busybox basename /aaa/bbb success
testcase busybox cal success
testcase busybox clear success
testcase busybox date success
testcase busybox df success
testcase busybox dirname /aaa/bbb success
...
```

管道数据通路完全打通，while-read 循环正常产出 testcase 行。

---

## 五、变更清单

| 文件 | 变更 | 说明 |
|------|------|------|
| `kernel/syscall/sysproc.c` — `sys_sendfile()` | 加 `count = MIN(count, PGSIZE)` | 防 busybox cat 传 16MB count |
| `kernel/syscall/sysproc.c` — `sys_ppoll()` | sleep on `&pi->nread` 替代 yield | pipe 写入直接唤醒 ppoll，消除时序竞争 |

> **核心收获：** 管道调试不能靠猜。syscall 日志 → sendfile 追踪 → ppoll pipe 地址对比，三步递进才锁定了"同一个 pipe、不同时间"的时序 bug。最终修复的精髓是让 ppoll 睡在 pipe 的唤醒频道上，与 pipewrite_kernel 的 wakeup 对齐——这是 xv6 sleep/wakeup 机制的标准用法。
