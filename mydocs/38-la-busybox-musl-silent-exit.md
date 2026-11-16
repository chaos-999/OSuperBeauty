# LA busybox-musl 静默退出诊断

> sdcard 中 musl 编译的 busybox LA 二进制无法正常执行测试脚本。

> **修正（2026-11-16）：本文原来的 `pcalau12i` 结论是错的。**
> `pcalau12i` 的计算应为
> `(0x120000148 + 508 * 0x1000) & ~0xfff = 0x1201FC000`，
> 随后 `ld.d $a4, $a4, -128` 得到 `0x1201FBF80`，
> 该地址落在 `.got`（`0x1201FBE58`～`0x1201FC000`）内，二进制本身正常；
> 用 qemu-user 也能正常执行该 busybox。
>
> 真正原因是内核 `uvmcopy()` 只复制 `[0, p->sz)` 和固定用户栈，
> 没有复制 `mmap()`/VMA 已经缺页加载的页。`fork()` 后子进程的匿名
> mmap 区域变成全零页；musl 的 malloc/ash 命令状态正好在这些区域，
> 于是 shell fork 出来的子进程丢失命令状态，直接 `exit(0)`，既不
> `execve` 也不输出。glibc busybox 的状态主要走 brk/栈，所以不受影响。
>
> 修复见 `kernel/mm/vm.c:uvmcopy()`：遍历父进程有效 VMA，把已经映射的
> 页复制到子页表；未触碰的页保持懒加载。修复后 musl busybox 脚本可完整
> 输出，仅剩 `sleep` 无独立可执行文件这一已知问题。完整记录见
> `46-fork-mmap-vma-copy.md`。

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

**不是 busybox 二进制/工具链 bug，而是内核 `fork()` 没有复制 `mmap()` 区域。**

关键证据：

1. 反汇编 `pcalau12i` 后，三个 GOT 地址（`0x1201FBF80`、`0x1201FBEE8`、`0x1201FBF68`）都在 `.got` 范围内，偏移没有算错。
2. 用 qemu-user 直接跑 `/musl/busybox sh busybox_testcode.sh`，shell、`ls`、`cat` 均正常。
3. 内核里 fork 出来的子进程只执行 `set_tid_address → rt_sigprocmask×2 → exit_group(0)`，没有 `execve`，也没有 `openat/getdents`；说明它还没开始执行命令就退出了。
4. `sys_mmap()` 只登记 VMA，页在缺页时才分配；`uvmcopy()` 只复制 `[0,p->sz)` 和 USTACK 栈页，漏掉了 VMA 页。musl 的 malloc/ash 命令状态在匿名 mmap 区域，fork 后变成全零，子进程因此丢失 `cmdentry`/argv 状态并直接 `exit(0)`。glibc 版本的状态主要位于 brk/栈，所以“看起来正常”。

## 解决方案

已修复 `kernel/mm/vm.c:uvmcopy()`：在复制普通内存和用户栈之后，遍历父进程有效 VMA，把**已经映射**的页复制到子页表；未触碰的页继续懒加载。

同时注意：

- 当前 `user/init-la.c` 的 `busybox-musl` 组实际 `chdir(bb_path_glibc)`，串口日志里的“musl”是 glibc 跑出来的；验证 musl 时改回 `chdir(bb_path_musl)`。
- 脚本中 `sh -c 'sleep 5' & ./busybox kill $!` 仍会失败，因为 LA 的 musl busybox 未开 `FEATURE_SH_STANDALONE`，且 `/musl/sleep` 不存在。可创建 `sleep -> busybox` 链接或重新编译 busybox 时开启 standalone；这与本次 fork 问题无关。
