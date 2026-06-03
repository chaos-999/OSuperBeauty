# 🔍 Busybox 管道静默失败：从 syscall 日志定位 sendfile 根因

> 📅 生成时间：`2026-06-03 14:25 (GMT+8)`
>
> 目标：记录 busybox_testcode.sh 管道无输出的完整调试过程——从现象、假说、syscall 日志插桩，到最终定位 sendfile(71) 实现缺陷。

---

## 一、问题现象

修完 busybox 的 155 号系统调用（getpgid）后，init-rv.c 跑 busybox_testcode.sh 出现以下输出：

```
#### OS COMP TEST GROUP START busybox-musl ####   ← echo 首行成功
（空：零条 testcase 输出）                         ← 管道静默
#### OS COMP TEST GROUP END busybox-musl ####     ← echo 尾行成功
```

首尾两条 `./busybox echo` 成功，但中间的 `cat | while read` 管道**一条 testcase 都没产出**。glibc 版本行为相同。

---

## 二、排除假说

| 假说 | 证据 | 结论 |
|------|------|------|
| cat 没被执行 | execve(221) 成功，后续 syscall 序列完整 | ❌ 不成立 |
| pipe2 没建好 | pipe2(59) 成功调用，后续 fork+dup3+close 序列正确 | ❌ 不成立 |
| dup3 返回 -1 破坏重定向 | busybox ash 用 `dup2_or_raise` 规避了 oldfd==newfd | ❌ 不成立 |
| 管道读端没收到数据 | while-read 侧 ppoll(73) 在等，等不到就 exit | ⚠️ 表象，还需找上游 |

---

## 三、调试方法：syscall 日志插桩

在 `kernel/syscall/syscall.c` 的入口处加一行 printf：

```c
// kernel/syscall/syscall.c，约 265 行
void syscall(void) {
    ...
    num = p->trapframe->a7;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        printf("%d %s: syscall %d\n", p->pid, p->name, num);  // ← 加这行
        uint64 ret = syscalls[num]();
        ...
    }
}
```

编译 `make kernel-rv -j$(nproc)`，跑 QEMU，输出解读方式：

```
85 busybox: syscall 59    ← pid=85 的 busybox 进程调了 syscall 59 (pipe2)
85 busybox: syscall 220   ← syscall 220 (clone/fork)
87 busybox: syscall 221   ← pid=87 新进程，syscall 221 (execve) → 执行 cat
87 busybox: syscall 71    ← syscall 71 (sendfile) → cat 用 sendfile 传数据！
87 busybox: syscall 94    ← syscall 94 (exit_group) → cat 退出
```

每条日志 = 进程 + 名字 + 系统调用号，对照 `include/syscall/syscall.h` 查号即可。

---

## 四、关键证据：cat 用了 sendfile，没用 write

过滤出 cat 进程（pid=87）的完整 syscall 序列：

```
87 busybox: syscall 221   # execve("./busybox", ["busybox","cat","busybox_cmd.txt"])
87 busybox: syscall 96    # set_tid_address
87 busybox: syscall 174   # getuid
87 busybox: syscall 56    # openat  ← 打开 busybox_cmd.txt ✅
87 busybox: syscall 71    # sendfile ← ⚠️ cat 用 sendfile 写数据！
87 busybox: syscall 63    # read     ← 然后读文件
87 busybox: syscall 57    # close
87 busybox: syscall 94    # exit_group
                           ← 没有 syscall 64 (write)！
```

**cat 的整个生命周期里，没有一次普通 write(64) 调用。**

而 while-read 侧（pid=88）的系统调用：

```
88 busybox: syscall 73    # ppoll ← 等管道数据
88 busybox: syscall 94    # exit_group ← 管道里没数据，直接退出
```

### 行为解释

busybox cat（musl 版本编译）使用 Linux 的 `sendfile(out_fd, in_fd, offset, count)` 做零拷贝文件传输：

```
sendfile 典型场景：
  文件 → socket   → 内核态 DMA 零拷贝（高效）
  文件 → pipe     → 需要走 pipewrite 写管道缓冲区
```

当 `out_fd` 指向管道（FD_PIPE）时，SpringOS 的 `sys_sendfile` 实现**只处理了文件→文件或文件→socket 路径，没有处理文件→管道路径**。sendfile 返回了一个值让 cat 误以为成功，实际管道缓冲区为空。

### glibc vs musl 差异

从日志看两个版本的初始化 syscall 略有不同：

| 版本 | 初始化特征 |
|------|-----------|
| musl | syscall 135(sigprocmask) 使用较多，sendfile(71) |
| glibc | syscall 99(set_robust_list), 261(prlimit64), 278(getrandom) 等额外调用，也使用 sendfile(71) |

两个版本都用了 sendfile，所以都受影响，行为一致。

### 完整管道链路还原

```
shell (pid=85)
  │
  ├─ pipe2(59)              → fds[0]=r, fds[1]=w
  ├─ fork(220)              → pid=87 (cat 进程)
  │   ├─ close(r)
  │   ├─ dup2(w, 1)         → stdout = 管道写端
  │   ├─ close(w)
  │   └─ execve → busybox cat
  │       ├─ openat → busybox_cmd.txt  ✅
  │       ├─ sendfile(???)  → 写管道   ⚠️ 静默失败，数据未入管道
  │       └─ exit_group
  │
  ├─ fork(220)              → pid=88 (while-read 进程)
  │   ├─ close(w)
  │   ├─ dup2(r, 0)         → stdin = 管道读端
  │   ├─ close(r)
  │   └─ while read line
  │       ├─ ppoll(73)      → 等数据
  │       └─ exit_group     → EOF，零次循环体
  │
  └─ wait4(260) × N         → 回收子进程
```

---

## 五、结论与修复方向

**根因：** `sys_sendfile`（syscall 71）没有处理 `out_fd` 为管道（FD_PIPE）的情况。busybox cat 依赖 sendfile 做文件拷贝，sendfile 返回了误导性成功值，管道实际为空。

**不是：**
- ❌ cat 没执行
- ❌ pipe2 没建好
- ❌ dup2/dup3 重定向有问题
- ❌ busybox 代码有 bug

**修复入口：** `kernel/syscall/sysfile.c` 中的 `sys_sendfile` 函数。

需要增加对输出 fd 类型的判断：

```
if (out_fd 是 FD_PIPE) {
    // 读文件内容 → pipewrite 写入管道
    for (...) {
        read(in_fd, buf, n);
        pipewrite(out_pipe, buf, n);
    }
} else if (out_fd 是 FD_REG) {
    // 文件→文件：现有逻辑
}
```

参考 `kernel/fs/pipe.c` 中的 `pipewrite()` 和 `piperead()` 函数，以及 `kernel/syscall/sysfile.c` 中 splice(76) 的现有 FD_PIPE 处理逻辑（第 1161-1190 行附近）。

---

## 六、调试技巧总结

> 这次调试最大收获：**不要猜代码，让内核自己告诉你它做了什么。**

| 步骤 | 操作 | 耗时 |
|------|------|------|
| 1 | syscall.c 加一行 printf | 30 秒 |
| 2 | 编译 + 跑 QEMU | ~5 秒 |
| 3 | grep `busybox: syscall` 过滤 | 1 秒 |
| 4 | 对照 syscall.h 查号 → 看到 sendfile | 1 分钟 |

没有任何"可能是 XX 问题"的猜测环节。syscall 序列直接把 cat 调了 sendfile 而不是 write 的事实拍在脸上。

以后遇到类似问题（进程行为异常、管道不通、文件操作奇怪），直接开 syscall 日志。比读源码快一百倍。
