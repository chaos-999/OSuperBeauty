# LA busybox-musl 静默退出诊断

> sdcard 中 musl 编译的 busybox LA 二进制无法正常执行测试脚本。

## 现象

busybox-musl exec 成功加载，`busybox_testcode.sh` 脚本内容被正确读取，但**无任何测试输出**，所有子进程 exit(0)。

对比：busybox-glibc 正常输出所有测试结果。

## 诊断方法

### 1. 文件存在性验证

```bash
debugfs -R "stat /musl/busybox" sdcard-la.img
# Inode: 59, Mode: 0755, Size: 2065912 — 文件正常
debugfs -R "stat /musl/busybox_testcode.sh" sdcard-la.img
# 脚本文件存在且大小正确
```

### 2. exec 参数验证

在内核 execve 添加诊断 `exec_la_diag`：
```
path=busybox argc=3 argv[0]="busybox" argv[1]="sh" argv[2]="busybox_testcode.sh"
```
内核侧 argv 完全正确。

### 3. syscall 追踪

在 usertrap 添加追踪，busybox 子进程的 syscall 序列：

```
pid=75: set_tid_address → rt_sigprocmask×2 → exit(0)
pid=76: set_tid_address → rt_sigprocmask×2 → dup3(5→1) → close(5,4) → exit(0)
pid=77: set_tid_address → rt_sigprocmask×2 → dup3(4→0) → close(4) → exit(0)
pid=78: set_tid_address → rt_sigprocmask×2 → exit(0)
```

- shell 正确解析了管道命令（pid=76/77 设好了管道重定向）
- 但所有子进程**从未调用 execve（sc=221）**
- shell 内部 `find_command` 判定 `./busybox` 不可用，跳过 exec 容错退出

### 4. 文件存在性确认

`/musl/busybox` 文件本身正常（0755, 2MB），路径正确。

## 结论

**sdcard 中 musl busybox LA 二进制本身的工具链 bug**。

`init-la.c` 已有注释佐证：
```c
// --- lua test (glibc only; musl binary has pcalau12i bug) ---
```

musl 编译的 LA busybox 在地址加载指令 `pcalau12i` 上有 bug，导致 shell 运行时无法正确查找或执行 `./busybox`。glibc busybox 正常。

## 解决方案

- 重新用修正后的 LA 工具链编译 musl busybox
- 或使用 glibc busybox 替代 musl 版本
