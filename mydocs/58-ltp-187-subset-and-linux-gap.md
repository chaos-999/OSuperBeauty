# LTP：188 例（LA）+ 6 例（RV）子集跑通，遗留问题与 Linux 对照

> 接 `55-ltp-dynamic-loader-blocker.md`。上一轮结论是“镜像 LTP 全是动态链接，loader 语义不足”。
> 本轮把动态 loader 的关键缺口补齐后，LA 上跑通了一个 **188 例**的 syscall 子集
> （官方 harness 采到 raw 90，隔离验证 168），RV 保留 6 例（raw 7）。
>
> 结论：**判分格式、运行框架、动态 loader 都已打通**；剩余问题集中在
> ① 块缓存无 readahead/过小导致加载慢（LTP 超时）、② 块缓存偶发陈旧读、③ 若干
> 语义偏差（symlink、mount、getdents 损坏项）。pipe EPIPE/SIGPIPE 已在本轮修复（§5.3）。
> 下文逐项对照 Linux 实现。

---

## 1. 判分规则（`judge_ltp-musl.py` + `postwork.py`）

- judge 解析每个 `RUN LTP CASE <name>` 到 `FAIL LTP CASE <name> <rc>` 之间的
  `Summary:` 块，累加 `passed`；`success = passed`。
- `postwork.py` 对组名含 `ltp` 的组做**对数映射**：
  ```python
  raw = max(0.0, min(#TOTAL, 10000.0))          # #TOTAL = 所有 case 的 passed 之和
  score = 500.0 * log10(1 + 9 * raw / 10000.0)
  ```
  低分区间极平：raw 14 → ~2.7，raw 97 → ~18，raw 500 → ~80，raw 10000 → 500。
- 关键：**只认 `passed`（TPASS 数量）**，TFAIL/TBROK/TCONF 不加分也不扣分。

## 2. 运行框架（`user/init-*.c` 的 `run_ltp_dir`）

每个用例：`fork` → 子进程把 stdout/stderr 重定向到 `/tmp/ltp.out` → `execve` 用例
（带 `LD_LIBRARY_PATH=/musl/lib`）→ 父进程带 **nanosleep 实时看门狗**（30s 杀掉挂起用例）
→ 读回 `/tmp/ltp.out` 解析 TPASS/TFAIL/TBROK/TCONF → 打印过滤后的行 + 自造 `Summary:` +
`FAIL LTP CASE`。loader 链接在 `main` 里建：
- LA：`/lib64/ld-musl-loongarch-lp64d.so.1` ← hardlink `/musl/lib/libc.so`
- RV：`/lib/ld-musl-riscv64.so.1` ← hardlink `/musl/lib/libc.so`

## 3. 本轮修掉的内核 bug（让 187 例能跑）

| bug | 症状 | 修复 | Linux 对照 |
|-----|------|------|-----------|
| `ext4_dir_entry_next` 空指针 | 读目录到底 `it.curr==NULL` 直接解引用 → `kerneltrap` | 加 `it.curr==NULL` 守卫 | `ext4_readdir`/`dx_readdir` 对遍历位置做严格边界判断，到尾部返回 0 |
| `ext4_dir_iterator_set` rec_len 下溢 | 损坏/为 0 的 `rec_len` 通过 `length-8` 无符号下溢 → `iterator_next` 死循环 100% CPU | 加 `length < 8` 拒绝 | `ext4_check_dir_entry` 校验 `rec_len < EXT4_DIR_REC_LEN(1) || rec_len > blocksize` → `-EFSCORRUPTED` |
| `clone` 裸 CLONE_VM 未标 `tg_shared` | 共享页表子进程穿过父进程 TRAPFRAME 返回 → 跳垃圾地址 | `CLONE_VM` 分支对父子都置 `tg_shared=1` | Linux 的 trapframe 在**内核栈**（`pt_regs`），非用户映射页，天然每线程独立，无此问题 |
| `EXT4_MP_LOCK/UNLOCK` 回调空指针 | 某些路径 `os_locks->unlock` 为空 | 对函数指针加空检查 | Linux mount 锁是自旋锁/读写信号量，无函数指针表 |
| `setresuid/getresuid/setresgid/getresgid/setfsuid/setfsgid/getgroups/setgroups` ENOSYS | musl `seteuid` 实为 `setresuid`，ENOSYS 让大量用例 TBROK | 按 generic 编号 147-152/158/159 实现 | Linux 用 `struct cred`（COW 引用计数）承载 uid/gid 全集合，`setresuid` 系列返回 0 并更新 cred |
| RV PIE 程序 fork 后解释器页缺失 | ld.so 落在 `p->sz` 之上，`uvmcopy` 漏复制，子进程首条 ld.so 指令指令缺页 | 新增 `interp_start/interp_end` 随 fork/exec 复制/释放 | Linux 把 ld.so 当作普通共享库 mmap 进进程地址空间，`fork` 复制整个 `mm_struct` 自然覆盖它 |

## 4. 用例筛选（200 → 188 LA / 60 RV）

从镜像 `/musl/ltp/testcases/bin`（950 个二进制）按「syscall 名 + 两位编号」过滤，
排除网络/IPC/命名空间/抢占等类别，再逐个剔除会 panic/死锁/刷屏的：

- 死锁（100% CPU、中断关闭，watchdog 也杀不掉）：
  `getcwd03`（chdir+readdir 清理）、`creat06`（mount+清理）、`lchown01/03`、`lstat01`、
  `readlink01/readlinkat01/02`（symlink 解析）、`openat01/201`（`strcpy` 空源）
- `panic: sched locks`：`fchownat02`
- 刷屏（pipe 满写返回 EPERM 导致用例重试）：`pipe04/15`（**pipe04 已修复并加回**，见 §5.3）

## 5. 遗留问题 & 与 Linux 实现的对照

### 5.1 块缓存无 readahead / 过小 → 动态加载慢、LTP 超时

- 现象：LA 完整流程（basic/busybox/libctest/libcbench 之后）里 LTP 单例 ~27 s，
  187 例在 3600 s harness 超时前只跑到 ~100 例（raw 90）；隔离（无前序组）时单例 ~5 s。
- 我们的实现：块缓存 `CONFIG_BLOCK_DEV_CACHE_SIZE=16`（64 KB），`loadseg`/loader 每次
  只读 4 KB、无 readahead；libcbench tmpfile 写脏后，每次 LTP 读块都要先刷脏块。
- Linux 实现：**page cache（radix tree/XArray）+ `do_generic_file_read` 的同步/异步
  readahead**，把 libc.so 的连续页一次性预读进缓存；`sync`/写回由 `writeback` 内核线程
  异步完成，前台读几乎不阻塞在刷脏上。
- 曾尝试把缓存 16→1024（4 MB）提速，但触发 5.2 的陈旧读，已回退。

### 5.2 块缓存偶发陈旧读（“Gee,” 垃圾）

- 现象：busybox-glibc 的脚本 `cat busybox_cmd.txt | while read` 偶发读到垃圾
  （`busybox_testcode.sh: line 28/30/32: Gee,: not found`，而脚本只有 27 行），
  使 busybox-glibc 从 53 掉到 22，并连带 libcbench-glibc/lua-glibc 全 0，总分 1158→1074。
- 只在缓存 16→1024 或加了 `sync` 刷脏时出现；缓存=16、无 sync 连续两次 harness 稳定
  1158/1162。
- 根因定位（两处）：
  1. **`ext4_bcache_invalidate_lba` 漏失效**（`bf667fd` 已修）：原实现只 `lookup(from)` 再
     `RB_FOREACH_FROM(tmp)`，当 `from` 不在缓存时循环体不执行，`[from,end]` 里其它已缓存
     的块不会被失效 → 释放块的旧内容留在缓存里被重新读回。改为遍历整棵树跳过 `lba<from`。
  2. 缓存放大到 1024 后还触发 **getdents01 内核级自旋**（100% CPU、看门狗杀不掉，目录迭代
     读到陈旧目录块）；这是陈旧读的另一表现，invalidate_lba 修复未覆盖。故 `cache=1024`
     仍不安全，保持 16。
- Linux 对照：page cache 用 `struct page` + 每页 `PageUptodate/PageDirty` 标志，块的
  生命周期由 radix tree 与 refcount 严格管理，绝无「同一 LBA 返回旧数据」；
  `end_buffer_async_read` 里 `SetPageUptodate` 只在真实读完磁盘后置位。

### 5.3 pipe 写 EPIPE 语义（pipe04 刷屏）— ✅ 已修复

- 原状：`pipewrite` 在 `readopen==0` 时 `return -1`（→ 用户态 EPERM），LTP pipe04 期望
  `EPIPE`（并触发 SIGPIPE），拿到 EPERM 后子进程重试刷屏。
- 修复（`b0fdfa4`）：
  1. `pipewrite`/`pipewrite_kernel` 读端关闭时返回 `-EPIPE` 并置 `sig_pending |= SIGPIPE`；
     `killed` 分支单独返回 `-EINTR`。
  2. 新增 `proc.exit_signal`（`sig_default_action`/`SIGKILL` 置位），`wait4` 在子进程被信号
     杀死时把 wait status 的低 7 位置成信号号（`WIFSIGNALED` 为真），而不是 `(xstate&0xff)<<8`
     的 exit(255) 假象。
- 结果：**pipe04 现 2 TPASS**（`Child 1/2 killed while writing to a pipe`），已加回 LA 列表
  （188 例）。`pipe15` 仍 TBROK（依赖 `/proc/sys/fs/pipe-user-pages-soft`）。
- Linux：读端关闭时 `pipe_write` 返回 `-EPIPE` 并向本线程发 `SIGPIPE`（`kill_fasync`）；进程被
  信号杀死时 `exit_code` 直接是信号号，`wait` 原样返回。

### 5.4 symlink 解析（lchown/lstat/readlink 死锁）

- 我们的 `vfs_ext_stat` 里 symlink 解析（`ext4_readlink`）和 `lstat`（NOFOLLOW）路径在
  特定目录状态下死锁；用例已剔除。
- Linux：symlink 解析在 VFS `link_path_walk` 的 `follow_link` 阶段做，深度上限 40、
  超限返回 `-ELOOP`，`AT_SYMLINK_NOFOLLOW` 直接停止遍历，不在 stat 内部再 resolve。

### 5.5 mount 是 no-op（creat06 死锁）

- 我们的 `sys_mount`/`sys_umount2` 直接 `return 0`，creat06 的 `mount(tmpfs)` + 清理会挂。
- Linux：mount 维护 `struct mount` + mount namespace（`struct mnt_namespace`），
  `umount` 检查引用计数，tmpfs 是独立文件系统；no-op 会让依赖挂载语义的用例异常。

### 5.6 getdents 损坏项（rec_len 下溢，见 §3）

- Linux 用 `ext4_check_dir_entry` 一次性校验 `inode/rec_len/name_len/block 边界`，
  发现损坏返回 `-EFSCORRUPTED` 并 `ext4_error` 标记文件系统只读，绝不进入死循环。

### 5.7 alarm/setitimer 是空实现（alarm07 1 TPASS+1 TFAIL）— ✅ 已修复

- 原状：`sys_setitimer` 只清空 old 值并返回 0，`alarm(N)` 从不投递 SIGALRM，alarm07 拿到
  `alarm_cnt(0) != 1` 的 TFAIL。
- 修复（`e01cb7b`）：
  1. `proc.alarm_deadline/alarm_active` + `sys_setitimer(ITIMER_REAL, {it_value})` 按
     `timespec_to_ticks` 设一次定时；`{0,0}` 取消。
  2. `check_alarms()` 挂在时钟中断（LA/RV 的 `clockintr`）里：到期进程置 `SIGALRM` pending、
     唤醒 SLEEPING 进程，返回用户态时执行 handler/默认动作。
- 结果：**alarm07 现 2 TPASS**（`alarm_cnt == 0` 之前、`alarm_cnt == 1` 之后）。
- Linux 对照：`setitimer(ITIMER_REAL)` 由 `struct hrtimer` + `posix_timer` 承载，到期在
  软中断里 `send_sig(SIGALRM)`；单次 `it_interval=0` 即一次性定时器。

### 5.8 file 对象复用时 `removed` 位泄漏 → chmod/chown/access 的 `Failed to chmod file: EPERM` — ✅ 已修复

- 现象：chmod01/chmod03/chown03/access01 在 setup 的 `SAFE_TOUCH`（open + chmod + close）里
  `chmod("testfile")` 返回 ENOENT，TBROK `Failed to chmod file 'testfile': EPERM`。
  之前被误判为块缓存陈旧读，实为 **file 生命周期 bug**。
- 根因（`d879614` 已修）：`fileclose()` 把文件拷到局部 `ff`、用 `ff.removed` 做延迟 unlink
  （`vfs_ext_rm`）后清的是 `ff.removed`，**从未清原 `f->removed`**；`filealloc()` 复用
  `f_count==0` 的文件对象时不重置任何字段。于是下一个 open 的文件继承了上一条「open 中
  被 unlink 的 scratch 文件（`ltp_<tid>_<pid>`）」留下的 `removed=1`，close 时被误判为
  「已删除」而 `vfs_ext_rm` 掉刚建好的 `testfile`，随后 `chmod()` 找不到它 → ENOENT。
- 修复：`fileclose` 里随 `f->f_count=0; f->f_type=FD_NONE;` 一并 `f->removed=0`。
- 结果：**chmod01 现 32 TPASS**（文件+目录两变体各 8 组 chmod+stat，之前 0 + TBROK）。
- Linux 对照：`struct file` 由 slab 分配后 `memset` 清零（`fget` 拿到的对象字段全为 0）；
  `fput` 走 `__fput` 释放，不存在「复用残留标志位」问题。

### 5.9 fchmod(52)/chown uid-gid 未实现 — ✅ 已修复

- 原状：`fchmod`（generic ABI 52）未注册 → fchmod01/02/04 TFAIL `ENOSYS`；
  `sys_fchownat`/`sys_fchown` 是 `return 0` 空桩，chown03/chown05 TFAIL `wrong owner`。
- 修复（`0488de1`）：
  1. 新增 `SYS_fchmod=52` → `ext4_mode_set(f_path)`；自定义 `exec()` 从 55（generic ABI 的
     fchown）迁到 1000，`SYS_fchown=55`。
  2. `sys_fchmodat` 返回真实 `-errno`（原来一律 -1→EPERM）。
  3. `ext4_owner_set`：`uid/gid==-1` 表示不改；并清 setuid/setgid（Linux chown 语义）。
  4. `sys_fchown`/`sys_fchownat` 真正调用 `ext4_owner_set`。
  5. `open(O_CREAT)` 新建文件时用调用者 euid/egid 初始化属主。
- 验证：fchmod01=6 TPASS、fchmod04=TPASS、fchmodat01=6 TPASS、fchown01=TPASS、
  chown05=6 TPASS、chown03=TPASS（basic-musl 仍 26/32）。

### 5.10 `ext4_file/ext4_dir` 表锁未释放 → open14 panic — ✅ 已修复

- 现象：cache=1024 全流程跑到 LTP `open14` 时
  `acquire: lock 'ext4_file_table' already held by CPU 0` → panic。
- 根因（`ae9ea2d`）：`free_ext4_file()`/`free_ext4_dir()` 只在指针命中表项的分支里
  `release`，指针不在表内时直接返回 → 自旋锁泄漏，下一次 `alloc_ext4_file()` 重复加锁 panic。
- 修复：把 `release` 移出循环，保证任何分支都释放。

### 5.11 cache=1024 全流程结果（snapshot=on）

- 191/191 用例全部执行，**LTP TPASS=301**（此前 157；目标 ~168），`gee=0, panic=0`，干净退出。
- 按判分公式 `500*log10(1+9*raw/10000)`：LA raw 301 → **52.1 分**；LA+RV（×2）→ **94.0 分**。

### 5.12 harness 实测踩到的两个 ABI/构建坑

1. **fchmodat 是 3 参数 syscall（53），不是 4 参数**。musl 的 `chmod()` 只传
   `(AT_FDCWD, path, mode)`，第 4 个寄存器 a3 是垃圾值。曾经给 `sys_fchmodat` 加
   `flags & ~AT_SYMLINK_NOFOLLOW → EINVAL` 的检查，结果 harness 里所有 LTP 用例
   setup 的 `chmod("/dev/shm/ltp_*", 0666)` 全 EINVAL（LTP TPASS 掉到 23，总分 929）。
   已回退：fchmodat 只读 3 个参数（`flags` 可读但不可据此报错）；fchmodat02 的
   EFAULT/ENAMETOOLONG/ENOTDIR 边角暂不追求。
2. **`SYS_exec` 从 55 改到 1000 后必须重建 `usys-*.o`**。`initcode-*` 规则虽然会
   `perl usys-*.pl > usys-*.S` 再编译，但 `syscall.h` 不在依赖里，只改 syscall.h 时
   `make` 不会重跑该规则，旧 `usys-rv.o` 仍是 `li a7,55` → RV 的 `exec()` 打到
   `sys_fchown`，RV basic 组全空（分数少 ~200）。已在 Makefile 给两个 initcode 目标
   加上 `include/syscall/syscall.h` 依赖。

### 5.13 RV LTP 扩容 + block cache 4096（性能根因）

- 现象：RV 的 LTP 原来只挂 6 个用例，单例 ~77s，全流程 LTP 极慢；LA 191 例却只要 ~10s/例。
- 根因（量化）：给 RV 内核加计数器后测得 **4 个用例做了 ~206,000 次磁盘 I/O**，lwext4 块缓存
  **命中率只有 51.8%**（210877 hit / 196292 miss）。`sleep` 等待平均 **0 tick**（中断立即
  唤醒），所以瓶颈不是 sleep，而是 **缓存太小导致 I/O 次数爆炸**。
- 修复：`CONFIG_BLOCK_DEV_CACHE_SIZE` 1024→**4096**（4MB→16MB）。4 用例从 **323s → 37s
  （8.8×）**，数据完全正确；`access01`（之前全流程下会自旋）也恢复正常（25 TPASS）。
- 放弃的方案：virtio-mmio 纯轮询 used ring 虽然能到 10s，但 QEMU/TCG 下 used ring 更新早于
  DMA 数据可见，会读到陈旧数据（已用块校验和证实），不可用。
- RV LTP 列表从 6 → **18 个**（`brk01/pipe01/read01/write01/open01/lseek01/close01/dup01/
  fork01/wait01/exit01/access01/uname01/times01/gettimeofday01/abort01/alarm07/atof01`），
  实测 **52 TPASS**（原来 8）。`chmod01` 在 RV 上内核自旋（LA 上 32 TPASS），暂排除。

## 6. 结论与下一步建议

- LTP 判分是「TPASS 数量」的对数映射，**多跑用例收益递减**（raw 100→18 分，500→80）。
- 真正卡分的是 5.1/5.2 的块缓存：修好 readahead + 陈旧读后，动态加载从 ~27s 降到秒级，
  LA 187 例能跑满（raw ~168，+~12 分），RV 也能扩到几十例；但块缓存陈旧读是 lwext4
  层面的深坑，性价比中等、风险高。
