# runtest.exe 全 FAIL / 卡死诊断：超时 tick 换算与 SIGKILL 唤醒

> 记录 libc-test static 脚本中 `runtest.exe` 全部 `[status 255]`、以及修好后又卡死在
> pthread 测试的问题。
>
> 结论：**`sigtimedwait` 把 100 Hz 的 tick 当成 10 Hz，5 秒超时实际只有 0.5 秒，
> 在 `entry-static.exe` 还没加载完时就把子进程杀了；另外 `SIGKILL` 只置了 pending，
> 没有唤醒睡眠进程，导致后面的 pthread 测试无法被超时杀死。**

---

## 1. 现象

libctest 的 static 测试链路：

```text
libctest_testcode.sh
  └── ./run-static.sh
        └── ./runtest.exe -w entry-static.exe argv
        └── ./runtest.exe -w entry-static.exe basename
        └── ...
```

实际运行结果：

```text
========== START entry-static.exe argv ==========
FAIL argv [status 255]
========== END entry-static.exe argv ==========
========== START entry-static.exe basename ==========
FAIL basename [status 255]
========== END entry-static.exe basename ==========
...
```

所有测试统一 `[status 255]`，没有 `Pass!`，也没有 `[internal]`。

而改成在 `init-rv.c` 里手工 fork + `execve("entry-static.exe", ...)` 时，测试能正常 PASS。
说明：

- `entry-static.exe` 本身没问题；
- 内核 fork/exec/wait 基本链路没彻底坏；
- 问题只出现在 `runtest.exe` 的启动/等待/超时流程里。

mydocs/27 中曾有一个初步结论：`sigtimedwait` 睡在 `sleep(&ticks)`，而 `exit()` 只
`wakeup(p->parent)`，所以超时后 `waitpid` 读到垃圾 status。这个结论**已经过时**：
当前内核的 `sigtimedwait` 会直接扫描僵尸子进程，`wait4` 也已经用局部变量打包 status。
继续沿用旧结论会直接走错方向。

---

## 2. 错误发现流程复盘

这一节完整记录这次排查中“以为是什么”和“实际是什么”的差别。

### 2.1 第一反应：沿用旧文档的 sigtimedwait/waitpid 结论

一开始看到所有测试 `[status 255]`，第一反应是：

1. `sigtimedwait` 没被唤醒；
2. 5 秒超时后走到 `kill + waitpid`；
3. `waitpid` 返回 EAGAIN，status 是栈垃圾；
4. 所以 `WEXITSTATUS(status)` 是 255。

这个推理听起来和 mydocs/27 完全一致，但有两个问题：

- 旧文档记录的是当时的代码，当前 `wait4()` 已经重写；
- “status 是栈垃圾”与“稳定输出 255”并不完全吻合，需要实测确认。

**教训：旧诊断文档只能当线索，不能当结论。**

### 2.2 先排除 argv/exec 参数错误

`entry-static.exe` 的 `main` 逻辑是：

```c
if (argc != 2)
    return -1;
```

如果 `runtest.exe` 传给子进程的 argv 不对，`entry.c` 会直接返回 `-1`，
`exit(-1)` 后 `WEXITSTATUS` 正好是 255。

所以在 `sys_execve()` 里临时加了打印：

```c
if (strcmp(path, "entry-static.exe") == 0) {
    printf("EXECDBG entry-static argc=%d", i);
    for (int j = 0; j < i; j++)
        printf(" [%s]", argv[j]);
    printf("\n");
}
```

实测：

```text
EXECDBG entry-static argc=2 [entry-static.exe] [argv]
```

参数完全正确。**排除 argv 传参错误。**

### 2.3 再排除“测试函数自己返回 -1”

在 `exit()` 和 `sys_exit_group()` 里临时加打印：

```c
printf("EXITDBG pid=%d name=%s status=%d\n", ...);
printf("EXITGDBG pid=%d name=%s status=%d\n", ...);
```

结果：

- `runtest.exe` 自己正常调用 `exit_group(1)`；
- 但 `entry-static.exe` **完全没有打印 EXITDBG/EXITGDBG**。

这说明子进程不是“测试失败后主动 exit”，而是被别的东西终止了。
如果测试函数真的返回 `-1`，至少应该看到一次 `EXITGDBG status=-1`。

**排除“libc-test 用例本身返回 -1”。**

### 2.4 查看 wait4 实际拿到的 status

在 `wait4()` 打包 status 的地方加打印：

```c
printf("WAITDBG pid=%d name=%s xstate=%d raw=0x%x\n",
       pp->pid, pp->name, pp->xstate, pp->xstate);
```

输出：

```text
WAITDBG pid=6 name=entry-static.ex xstate=-1 raw=0xffffffffffffffff
FAIL argv [status 255]
```

说明 `entry-static.exe` 的 `xstate` 是 **-1**，不是 0，也不是 255。
`(xstate & 0xff) << 8` 后正好得到 `0xff00`，用户态 `WEXITSTATUS` 就是 255。

而 `xstate = -1` 来自内核里的：

```c
if (killed(p))
    exit(-1);
```

所以子进程是**被信号杀死**的。

### 2.5 关键证据：timeout 发生在 exec 完成之前

继续在 `sys_rt_sigtimedwait()` 的超时分支和 `kill()` 系统调用里加打印：

```c
printf("SIGTIMED: timeout\n");
printf("KILLDBG pid=%d sig=%d\n", pid, sig);
```

又在 execve 完成处打印 tick：

```c
printf("EXECDBG ticks=%u entry-static argc=%d ...", ticks, i);
printf("EXECDONE ticks=%u pid=%d entry=0x%lx sp=0x%lx\n",
       ticks, p->pid, (uint64)elf.entry, (uint64)sp);
```

实测顺序：

```text
EXECDBG ticks=695 entry-static argc=2 [entry-static.exe] [argv]
SIGTIMED: timeout
KILLDBG pid=6 sig=9
EXECDONE ticks=823 pid=6 entry=0x25fe8 sp=0x3ffebffe30
FAIL argv [status 255]
```

这组日志非常关键：

- 695 tick 开始 exec；
- 还没执行完，`sigtimedwait` 已经 timeout；
- 然后 `kill(pid, SIGKILL)`；
- 823 tick 时 execve 才完成；
- 但进程已经被标记 killed，最终以 `exit(-1)` 结束。

也就是说：**不是 waitpid 拿错 status，而是 runtest 在子进程加载完之前就把它杀了。**

### 2.6 计算 tick 频率：这里才找到真正根因

差值是：

```text
823 - 695 = 128 ticks
```

而超时发生在 50 ticks 左右。继续查 tick 频率：

`kernel/trap/rv/trap.c`：

```c
SBI_SET_TIMER(r_time() + 100000);
```

RISC-V QEMU 的 `timebase-frequency` 是：

```text
0x00989680 = 10,000,000 Hz
```

所以：

```text
100000 / 10000000 = 10 ms
1 tick = 10 ms
频率 = 100 Hz
```

`entry-static.exe` 加载约：

```text
128 ticks * 10 ms = 1.28 s
```

而 `runtest.exe` 请求的 5 秒超时，在旧代码里是：

```c
timeout_ticks = ts.tv_sec * 10 + ts.tv_nsec / 100000000;
```

```text
5 * 10 = 50 ticks = 0.5 s
```

**5 秒被换算成了 0.5 秒，差 10 倍。**

这就是所有测试 `[status 255]` 的直接原因。

### 2.7 我在这里差点走错：以为是 exec 太慢，想去优化文件读取

看到“exec 需要 1.28 秒”，第一反应是：

- `execve` 每个 4KB 页面都调用一次 `vfs_ext_readi`；
- `vfs_ext_readi` 每次都 `ext4_fopen2 + fseek + fread + fclose`；
- 所以 exec 很慢，应该优化读路径。

这个方向不是完全错，但它**不是当前问题的修复点**：

- 1.28 秒本身可以接受；
- `runtest.exe` 的真实超时应该是 5 秒，而不是 0.5 秒；
- 如果只优化 exec，可能把 1.28 秒压到 0.8 秒，但一旦某个测试稍慢，仍然会被 0.5 秒超时误杀。

**正确做法是先修单位换算，再谈性能。**
后来实测修复后 16 个测试约 40 秒完成，说明 1.28 秒/次的 exec 不是阻塞性问题。

### 2.8 修好超时后暴露第二个问题：SIGKILL 唤不醒睡眠进程

超时改成 100 Hz 后，普通测试开始 PASS，但脚本卡在：

```text
========== START entry-static.exe pthread_cancel_points ==========
pthread_create(...) failed: Resource temporarily unavailable
```

之后没有输出，`runtest.exe` 不返回，`waitpid` 永久阻塞。

继续查 `kill(SIGKILL)`：

```c
int send_signal(int pid, int sig)
{
    ...
    p->sig_pending |= (1ULL << (sig - 1));
    release(&p->lock);
    return 0;
}
```

它只设置了 pending，没有：

- 设置 `p->killed`；
- 把 `SLEEPING` 进程改成 `RUNNABLE`。

pthread 测试在 `pthread_create` 失败后通常会阻塞在 `sem_wait` / `futex_wait`。
这种进程睡在 futex 队列上，不会回到用户态执行默认信号动作，所以 `SIGKILL` 形同虚设。
`runtest.exe` 的 `waitpid` 就一直等下去。

这就是第二个根因：**内核的 SIGKILL 语义不完整。**

---

## 3. 解决方案

### 3.1 `sigtimedwait` 按 100 Hz 换算超时

文件：`kernel/syscall/syssig.c`

```c
uint64 timeout_ticks = 0;
if (ts_ptr != 0) {
    /*
     * trap.c 每 100000 个 timebase cycle 触发一次时钟中断；
     * RISC-V timebase 为 10 MHz，所以 1 tick = 10 ms (100 Hz)。
     * 旧代码按 10 Hz 计算，5 秒超时实际只有 0.5 秒。
     */
    timeout_ticks = (uint64)ts.tv_sec * 100 +
                    ((uint64)ts.tv_nsec + 9999999) / 10000000;
}
```

要点：

- `tv_sec * 100`：1 秒 = 100 ticks；
- `tv_nsec` 向上取整，避免 1ns 被算成 0；
- `ts = {0,0}` 仍然立即返回 `EAGAIN`，不破坏零超时语义。

### 3.2 SIGKILL 必须终止并唤醒睡眠进程

文件：`kernel/trap/signal.c`

```c
if (p->state != UNUSED && p->pid == pid) {
    p->sig_pending |= (1ULL << (sig - 1));

    if (sig == SIGKILL) {
        p->killed = 1;
        if (p->state == SLEEPING)
            p->state = RUNNABLE;
    }

    release(&p->lock);
    return 0;
}
```

道理和 xv6 的 `kill()` 一致：

- `SIGKILL` 不能被捕获、不能被阻塞，必须立即生效；
- 睡眠进程被改成 `RUNNABLE` 后，`futex_wait` / `sleep` 等会从睡眠返回；
- 返回后检查 `p->killed`，随后 `usertrap()` 中 `exit(-1)`。

---

## 4. 验证

### 4.1 修复前

```text
========== START entry-static.exe argv ==========
FAIL argv [status 255]
========== START entry-static.exe basename ==========
FAIL basename [status 255]
...
```

脚本无法正常推进，pthread 测试还会永久卡死。

### 4.2 修复后

使用当前 `user/init-rv.c` 的脚本方式运行：

```text
libctest_testcode.sh
  └── run-static.sh
        └── runtest.exe -w entry-static.exe ...
```

结果：

- 普通测试正常 `Pass!`；
- 不再出现统一的 `[status 255]`；
- 不再出现 `waitpid failed` / `[internal]`；
- pthread 测试会正常失败并继续，不再拖死整个脚本；
- 脚本最终正常结束并 shutdown。

统计：

```text
START 条目：108
Pass!：87
FAIL：20
```

FAIL 主要是：

- `pthread_cancel_points` / `pthread_cancel` / `pthread_cond` / `pthread_tsd`
- `pthread_robust_detach` / `pthread_cancel_sem_wait` / `pthread_cond_smasher`
- `pthread_condattr_setclock` / `pthread_exit_cancel` / `pthread_once_deadlock`
- `pthread_rwlock_ebusy`

这些是内核 pthread/clone 语义仍未完整支持导致的真实 FAIL，属于预期。
另外还有少量 libc-test 自身功能 FAIL（socket/stat/utime/statvfs 等），
它们的状态是 `[status 1]`，不是 `[status 255]`，说明测试已经真正跑起来了。

> 本地 SD 卡镜像中 `libctest_testcode.sh` 还会执行 `./run-dynamic.sh`，
> 当前日志里有 `./run-dynamic.sh: not found`。这是动态测试/镜像内容的问题，
> 不属于本次 static `runtest.exe` 修复范围。

---

## 5. 复盘：这次为什么一开始会判断错

### 5.1 直接相信了旧文档

mydocs/27 的 sigtimedwait/waitpid 分析在当时的代码状态下可能成立，但代码已经继续演进。
旧文档里的“waitpid 返回 EAGAIN”与当前日志的 `[status 255]` 并不是同一个现象。

**教训：先看当前代码，再看旧结论。**

### 5.2 把“所有测试都 255”误认为“测试逻辑失败”

255 很容易被当成“测试返回 -1”。实际上：

- `xstate = -1` 来自内核 `exit(-1)`；
- 只有被信号杀死、或 `usertrap` 判定 killed 时才会这样；
- 真正测试失败通常是 `exit(1)`，显示 `[status 1]`。

**教训：先区分“用户主动 exit 的 255”和“内核 kill 后的 255”。**

### 5.3 没有一开始就给事件加时间戳

最初只看到“timeout、kill、wait”这些事件，没有看它们与 exec 的先后关系。
加上 `ticks` 之后，才一眼看出：

```text
timeout 发生在 exec 完成之前
```

**教训：并发/超时问题必须记录时间，不只看事件顺序。**

### 5.4 差点把性能问题当成根因

exec 1.28 秒确实比理想情况慢，但它没有超过真正的 5 秒。
如果当时直接去改 `vfs_ext_readi` 的 open/close，可能花很大力气，
而真正的问题只是 `* 10` 写成了 `* 100`。

**教训：先测量，再优化；先确认超时是否真的是 5 秒。**

### 5.5 修了第一个 bug 就以为结束了

修好 tick 换算后，普通测试 PASS，但 pthread 测试卡死。
如果只验证前几个测试就收工，就会漏掉 SIGKILL 不唤醒的问题。

**教训：必须跑到脚本真正结束，而不是看到第一个 Pass 就结束。**

---

## 6. 可复核证据

```bash
# 1. 时钟中断周期
grep -n 'SBI_SET_TIMER' kernel/trap/rv/trap.c
# 26:  SBI_SET_TIMER(r_time() + 1000000);
# 245: SBI_SET_TIMER(r_time() + 100000);

# 2. timebase frequency
qemu-system-riscv64 -machine virt,dumpdtb=/tmp/virt.dtb -bios default -m 1G
# 解析 DTB 得到 timebase-frequency = 0x00989680 = 10000000

# 3. 旧超时换算（10 Hz）
grep -n 'timeout_ticks' kernel/syscall/syssig.c

# 4. SIGKILL 原实现
grep -n 'int send_signal' -A20 kernel/trap/signal.c

# 5. 修复后关键代码
grep -n '100 +' kernel/syscall/syssig.c
grep -n 'sig == SIGKILL' -A5 kernel/trap/signal.c
```

---

## 7. 一句话总结

`runtest.exe` 不是测试运行器本身坏了，而是：

1. **内核把 100 Hz 的 tick 按 10 Hz 换算，5 秒超时变成 0.5 秒，子进程在 exec 阶段就被杀掉；**
2. **`SIGKILL` 只置 pending 不唤醒睡眠进程，导致超时后无法真正杀死卡住的 pthread 测试。**

修复这两点后，libc-test static 的 `runtest.exe` 链路可以正常跑完，
`[status 255]` 全灭和 pthread 永久卡死都消失。
