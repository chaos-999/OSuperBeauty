# libcbench-musl 27 项全部跑通：线程/VM/信号/文件系统的完整修复

> 记录 `make qemu`（RISC-V）下 libcbench-musl 从「线程崩溃、regex 异常、stdio 极慢」
> 到「27 项全部输出 `time:` 并正常关机」的完整定位与修复过程。
>
> 结论：问题不是单点，而是 **clone ABI、线程页表陷阱帧、set_tid_address 语义、
> 线程组堆/退出语义、用户栈容量、fstat 语义** 这一整条链。按本文修改后，
> RISC-V 完整通过；LoongArch 可编译、可启动（LA 下 libcbench 本身极慢，见 §8）。

---

## 1. 背景与目标

- 平台：`make qemu` = **RISC-V**（`make qemu-la` 才是 LoongArch）。
- 测例：`/musl/libcbench_testcode.sh` → `/musl/libc-bench`（musl 静态链接）。
- 目标：27 个 benchmark 全部输出 `time: ...`，不 panic、不卡死，最后正常关机。

评分脚本 `/home/chaos/oscomp-testdata/judge_libcbench-musl.py`：

- 只解析 `time: <秒>`，**不解析** `virt/res/dirty`；
- 单项分数：`res/baseline`，`>=1` 时 `2 - 1/score`，否则记 `1.0`；
- 也就是说 **只要该项跑出时间，最差也有 1 分**；比 baseline 快才可能超过 1 分。
- `config.json`：`qemu.timeout = 3600` 秒。

`virt/res/dirty` 为 0 是因为本内核未实现 `/proc/self/smaps`，不影响评分。

---

## 2. 问题定位链（按出现顺序）

### 2.1 `sys_clone` 参数顺序必须按架构区分

musl 的 `__clone` 汇编：

```
RISC-V:    clone(flags, stack, ptid, tls, ctid)
LoongArch: clone(flags, stack, ptid, ctid, tls)
```

原实现只按一种顺序取参，`make qemu`（RV）下 `tls/ctid` 互换，线程 TLS 错误。

修复：`kernel/syscall/sysproc.c` 中按 `RISCV` / `LOONGARCH` 分别 `argaddr`。

### 2.2 线程退出不回收，2500 次 `pthread_create` 耗尽 `NPROC`

`pthread_join()` 不走 `wait4()`，线程退出后 `ZOMBIE` 永久占槽。
RV 版 `b_pthread_createjoin_*` 循环 2500 次。

修复：`scheduler()` 回收 `ZOMBIE && tid != tgid` 的线程；
同时 `wait4()` 与 `reparent()` **跳过线程**（线程的“父进程”不是 wait 对象，
不能被 init 误回收）。

### 2.3 `b_regex_search` 的 260KB 栈数组超过用户栈

`libc-bench/regex.c`：`char buf[260000];`，原用户栈 `USTACK_PAGE=32`（128KB）。

修复：`USTACK_PAGE = 256`（1MB）。同时把用户栈整体下移 512MB
（`USTACK = MAXVA - 512MB - USTACK_PAGE*PGSIZE`），给线程栈 mmap 留空间，
避免 2500 个线程栈从 `MMAPEND` 向下增长时压到主用户栈。

### 2.4 `fstat` 按路径 stat，`tmpfile()` 已被 unlink

`tmpfile()`：`open(O_CREAT|O_EXCL)` 后立即 `unlink()`，再 `fdopen()`。
原 `filestat/filestatx` 用 `f->f_path` 做 `vfs_ext_stat`，文件已 unlink，必然失败。

修复：`kernel/fs/vfs/file.c` 改用 `vfs_ext_fstat(f, &st)`，基于打开文件的 inode。

> 注：musl 的 `__fdopen()` 固定使用 `BUFSIZ`，并不依赖 `st_blksize`；
> 这个修复是正确性修复（对已删除打开文件 fstat 应成功），
> 但它不是 stdio 200 秒的原因（见 §8）。

### 2.5 `sys_mprotect` 拒绝尚未缺页的 VMA 页 → `pthread_create` 失败

musl `pthread_create` 创建线程栈：

```c
map = mmap(NULL, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
mprotect(map + guard, size - guard, PROT_READ|PROT_WRITE);
```

mmap 只登记 VMA，页是惰性分配的；原 `sys_mprotect` 对每个页要求
`PTE_V`，否则返回 `-ENOMEM`，于是线程创建在 clone 之前就失败。

修复：`sys_mprotect` 只修改已存在的 PTE；并遍历 VMA 表，更新覆盖该区间的
`v->prot`，让后续缺页按新权限映射。

### 2.6 线程共用页表后，`TRAPFRAME` 指向别人的 trapframe（核心 bug）

`CLONE_VM` 线程直接 `np->pagetable = p->pagetable`。但 `TRAPFRAME`
（`MAXVA-3*PGSIZE`）在页表里原本映射的是**父进程**的 trapframe：

- 子线程第一次 `userret` 时，trampoline 从 `TRAPFRAME` 恢复的是父线程的
  `a0/sp/epc`；
- 于是子线程拿到父线程的 `a0=pid`，**没有走 clone 的子分支**，
  而是继续执行 `pthread_create` 的父分支，调用 `sched_setscheduler`（未知系统调用 119）；
- 最终父子状态互相踩踏，出现 `panic: release`、`unknown sys call 119`。

修复：在 `usertrapret()` 中，若当前进程属于「共享页表的线程组」
（新增字段 `p->tg_shared`），把当前进程页表的 `TRAPFRAME` PTE 重指向
`p->trapframe`，并刷新 TLB。

- RV：`*tfpte = PA2PTE(p->trapframe) | PTE_R | PTE_W | PTE_V; sfence_vma();`
- LA：`*tfpte = PA2PTE(p->trapframe) | PTE_NX|PTE_P|PTE_W|PTE_MAT|PTE_D|PTE_V`
  （`LA2K1000` 另加 `PTE_U` 且不带 `PTE_NX`）。
  **LA 的 `PTE_V`（bit 0）和 `PTE_P`（bit 7）是两个独立位**，重写 PTE 时漏掉
  `PTE_V` 会让映射失效：LA 上表现为 `pthread_create` 无声卡死。
  不需要额外 `invtlb`，`uservec.S` 的 `userret` 在切换 `PGDL` 后已经做过全量刷新。

> 条件不能只判断 `p->pt_shared`：**线程组的 leader 自己 `pt_shared=0`**，
> 但它和线程共用同一张页表，同样需要重映射（只判断 `pt_shared` 会让
> `b_malloc_thread_stress` 卡死）。因此在创建线程时同时给 leader 和
> 线程置 `tg_shared=1`，普通进程保持 0。
>
> 也不能无条件执行：LA 上无条件重映射（即使 PTE 值不变，只多做一次
> `invtlb`）会导致系统在 `fs_mount done` 后启动卡死。

### 2.7 `sys_set_tid_address` 把 TID 写进用户内存 → `pthread_join` 死锁

Linux 的 `set_tid_address(addr)` 语义：**只记录 addr**，线程退出时内核把
`*addr` 清 0 并 `futex_wake`。原实现却把 `p->pid` `copyout` 到 `addr`。

musl 启动时调用 `set_tid_address(&__thread_list_lock)`，并把
`__thread_list_lock` 当作线程链表锁。内核把 TID 写进去后，该锁看起来
永远被主线程持有：

```
worker: __pthread_exit → __tl_lock 读到的值 == 主线程 tid
        → futex_wait(&__thread_list_lock, tid) 永久睡眠
main:   __tl_unlock 只递减 tl_lock_count，不清锁
        → pthread_join 永久睡眠
```

现象就是 `b_pthread_uselesslock`、`b_pthread_createjoin_*` 卡死。

修复：`sys_set_tid_address` 只保存 `p->clear_child_tid`，不再 `copyout`，
返回 `p->tid`。

### 2.8 线程组 `brk` 各自独立 → `mappages: remap`

线程直接 `brk` 时用的是各自 `p->sz`，两个线程扩堆会映射同一段虚拟地址，
`mappages()` 直接 `panic("mappages: remap")`。

修复：`struct proc` 增加 `leader` 指针；线程继承 leader
（`np->leader = p->leader ? p->leader : p`）；`growproc()` 与 `sys_brk()`
统一以 `leader->sz` 为当前堆顶，映射成功后再同步回本地 `p->sz`。

### 2.9 线程组退出语义：leader 不能在兄弟线程还活着时释放共享 mm

`b_pthread_create_serial1` 创建 2500 个线程**不 join**。RV 的 bench 子进程
（leader）在循环结束后 `exit()`，但此时还有少量线程仍在运行；原实现立刻：

- `vmaunmap()` 释放所有 mmap 区（包括这些线程的栈）；
- `proc_freepagetable()` 释放共享页表；
- 兄弟线程继续在已释放的页表/栈上运行 → 内核在调度或 userret 时永久卡住。

修复：`exit()` 中，若当前是 leader（`tid == tgid`），先：

1. 把同组其他线程 `killed=1` 并 `wakeup(chan)`；
2. `yield()` 循环等待它们变成 `ZOMBIE/UNUSED`；
3. 再做文件/VMA/页表清理并置 `ZOMBIE`。

同时 `sys_exit_group` 调用同一个 `exit()`，语义与 Linux 的 `exit_group` 一致。

### 2.10 `NVMA` 太小，2500 个未 join 的线程栈把 VMA 表撑爆

原来 `NVMA=16`，`pthread_create_serial1` 每次 `mmap` 一个线程栈且不释放，
第 17 个就 `panic("mmap: no free vma")`。

修复：`NVMA = 3072`（RV 2500 + 余量，LA 只需 10）。
代价是 `struct proc` 增大，BSS 约 19.6MB，`buddy_init` 仍剩 107MB 可用。

---

## 3. 修改清单

| 文件 | 修改 |
|---|---|
| `kernel/syscall/sysproc.c` | `sys_clone` 按架构取参；`sys_mprotect` 允许惰性页并更新 VMA；`sys_set_tid_address` 不写用户内存；`sys_brk` 使用 leader `sz` |
| `kernel/proc/proc.c` | 线程 ZOMBIE 回收；`reparent`/`wait4` 跳过线程；leader/`brk` 共享；`growproc` 用 leader `sz`；`exit()` 等兄弟线程退出后再清理；创建线程时给整个组置 `tg_shared`；`freeproc` 清 `leader`/`tg_shared` |
| `include/proc/proc.h` | `struct proc` 增加 `struct proc *leader`、`int tg_shared` |
| `kernel/trap/rv/trap.c` | `usertrapret` 中 `tg_shared` 时重映射 TRAPFRAME |
| `kernel/trap/la/trap.c` | 同上（LA 版 PTE 标志 + `invtlb`，且必须条件执行） |
| `include/memlayout.h` | `USTACK_PAGE 32→256`；用户栈下移 512MB（3 处 RV/LA/VF2） |
| `include/param.h` | `NVMA 16→3072` |
| `kernel/fs/vfs/file.c` | `filestat/filestatx` 改用 `vfs_ext_fstat` |

---

## 4. 验证方法

### 4.1 必须全量 clean 构建

该 Makefile 的 `-include kernel/*.d` **只覆盖 `kernel/` 顶层**，
`kernel/proc/*.d`、`kernel/trap/rv/*.d` 等不在其中；因此**改头文件后
子目录不会自动重编**，会出现「结构体布局不一致」的诡异问题（例如
`sig_handlers` 写到了旧偏移，clone 读到 NULL）。

```bash
find . -name '*-rv.o' -delete; find . -name '*-rv.d' -delete
find . -name '*-la.o' -delete; find . -name '*-la.d' -delete
make kernel-rv kernel-la -j4
```

### 4.2 RISC-V 完整运行

```bash
qemu-system-riscv64 -machine virt -bios default \
  -kernel kernel-rv -m 1G -smp 1 -nographic \
  -drive file=/home/chaos/oscomp-testdata/sdcard-rv.img,if=none,format=raw,id=x0,snapshot=on \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -rtc base=utc -no-reboot
```

结果：`GROUP START` → 27 条 `time:` → `GROUP END` →
`All tests completed, shutting down system...`，无 panic。

### 4.3 LoongArch

`kernel-la` 可 clean 编译；用
`/home/chaos/oscomp-testdata/sdcard-la.img.gz` 解压后的镜像可正常启动并跑
busybox 测例。另外用 32MB 最小镜像单独验证了 LA 线程路径：
`pthread_create → worker → pthread_join → malloc/memset` 全部正常
（修复 `PTE_V` 之前会卡在 `pthread_create`）。

LA 下 libcbench 的 malloc/内存写性能极低（见 §8），未能完整跑完，
不作为本次 RV 目标的阻塞项。

---

## 5. 调试经验（重要）

1. **内核 `printf` 是“按进程缓冲”的**：`console_flush(p)` 只在 `freeproc()`
   里调用。调试调度/退出问题时，`printf` 的输出顺序会严重失真，
   后打印的内容可能先出现。需要即时输出时，直接写 UART：
   ```c
   static void udbg(const char *s) {
       while (*s) *(volatile unsigned char *)0x10000000 = *s++;
   }
   ```
2. **spinlock 卡死定位**：在 `acquire()` 自旋里加计数器，超过阈值打印锁名，
   比盲目加 print 有效。
3. **最小镜像复现**：`mke2fs -q -t ext4 -b 4096 -d <dir> t.img 32M`
   可以造一个 32MB 镜像，只放一个静态测试程序；把 init 改成直接
   `execve("/t")`，能把 400MB 的 sdcard 测例缩小到秒级迭代。
4. `boot/rv/initcode.o` 通过 `.incbin "../user/initcode-rv"` 嵌入 init，
   make 不会跟踪 `.incbin` 依赖；改 `user/init-rv.c` 后需要
   `rm -f boot/rv/initcode.o`。

---

## 6. 评分口径与残余性能

- 27 项均输出 `time:`，按 judge 公式每项至少 1 分。
- 多数项目比 baseline 慢（QEMU 模拟），得 1.0；
  `b_malloc_*`、`b_pthread_*` 等已接近 baseline。
- `b_stdio_putcgetc*` 各约 200 秒（见 §7），仍是 1.0 分，
  总耗时约 7 分钟，远低于 `qemu.timeout=3600`。
- `virt/res/dirty=0` 是缺少 `/proc/self/smaps`，不影响评分。

---

## 7. stdio 200 秒的真实原因（性能项，非正确性）

`libc-bench/stdio.c` 对 `tmpfile()` 做 500 万次 `putc` + 500 万次 `getc`。
musl 的 `FILE` 缓冲为 `BUFSIZ`，因此实际是约 4883 次 1KB 写 + 4883 次 1KB 读。

用最小复现（`t2.c`）测得：

```
N=500000  putc=17.47s  getc=1.56s
```

把 stdio 缓冲改成 64KB 后 `putc` 仍为 16.9s（写次数从 488 降到 8），
说明瓶颈**不是系统调用次数**，而是块设备/文件系统写入吞吐：

- 本内核 `BSIZE = 512`，bio 层每个请求只传 512 字节；
- 文件系统块 4KB，需要 8 次设备请求；
- lwext4 写路径还叠加日志事务、位图、inode 更新。

估算有效写吞吐约 30KB/s，因此 5MB 约 170 秒。

这条属于**性能优化**（把物理块大小提到 4KB、或加页缓存/写合并），
改动面大、回归风险高；而按评分公式它无论如何都是 1.0 分，
所以本次 **不做**，仅记录为已知限制。

---

## 8. 已知限制

1. **LA libcbench 极慢**：LA QEMU TCG 下 `b_malloc_sparse` 的前 400 次
   `malloc(4000)+memset` 耗时约 45 秒（约 9 次/秒），而 LA 的线程
   create/join 单独测试是正常的。属于 LA 内存/缺页/写路径的性能问题，
   与本次 RV 目标的修复无关；要定位需单独做 LA 侧的性能剖析。
2. **stdio 写吞吐**：见 §7。
3. 线程组退出采用「leader 轮询等待兄弟线程」的实现，
   在单核（`-smp 1`）下正确；多核下 `TRAPFRAME` 共享页表重映射本身
   也需要每核独立的 trapframe 方案，当前实现面向赛题单核配置。

---

## 8.1 附：非 libcbench 需求的合入

PR #3 修复了 `/dev/zero` 的 `read` 语义（libc-test 的 `syscall_sign_extend`），
libcbench 本身不使用 `/dev/zero`，因此不影响本文的 27 项结论。
合入时未采纳 PR 中的 `kernel-rv` 二进制（基于旧 main、缺少本文的线程/VM 修复），
已本地重新编译；详细对照见 `52-libcbench-kernel-requirements.md` 第 13 节。

## 9. 复现命令速查

```bash
# 构建
make kernel-rv kernel-la -j4

# RISC-V 跑 libcbench（真实镜像）
qemu-system-riscv64 -machine virt -bios default -kernel kernel-rv -m 1G -smp 1 \
  -nographic \
  -drive file=/home/chaos/oscomp-testdata/sdcard-rv.img,if=none,format=raw,id=x0,snapshot=on \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -rtc base=utc -no-reboot

# 统计
grep -c 'time:' serial.log     # 期望 27
grep -c '^b_'   serial.log     # 期望 27
```
