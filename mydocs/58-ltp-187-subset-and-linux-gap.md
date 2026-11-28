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

### 5.14 RV 内核挂死根因：partial mprotect 覆盖整个 VMA（已修）

- 现象：`chmod01`、`clone01` 在 RV 上 100% CPU 自旋，LTP 的 30s alarm 也杀不掉。
- 根因：`pthread_create` 映射线程栈后，对栈顶 guard page 做 `mprotect(PROT_NONE)`（**部分范围**）。
  `sys_mprotect` 却把**整个 VMA 的 `prot` 改成 0**，把线程栈也变成 PROT_NONE；RV 的
  `vmatrylazytouch` 对「PTE 已存在」的页做权限升级时，`v->prot==0` 没加任何权限却
  `return 1`，于是写栈→缺页→假装成功→再写→再缺页，**无限缺页循环**。
- 修复（`666900c`）：
  1. `sys_mprotect` 仅当 mprotect 覆盖**整个 VMA** 时才更新 `v->prot`；部分范围只改 PTE。
  2. `vmatrylazytouch`：`v->prot` 无任何权限时返回 0（不再假装成功）。
- 结果：`chmod01` 从自旋 → **32 TPASS**；RV 19 用例合计 **84 TPASS**（之前 52），
  全流程 557s。LA 侧同函数修复后行为更正确（LA 的 vmatrylazytouch 无该升级路径，本来就不循环）。

## 6. 结论与下一步建议

- LTP 判分是「TPASS 数量」的对数映射，**多跑用例收益递减**（raw 100→18 分，500→80）。
- 真正卡分的是 5.1/5.2 的块缓存：修好 readahead + 陈旧读后，动态加载从 ~27s 降到秒级，
  LA 187 例能跑满（raw ~168，+~12 分），RV 也能扩到几十例；但块缓存陈旧读是 lwext4
  层面的深坑，性价比中等、风险高。

### 5.15 第二轮：按 Linux 语义补齐（RV/LA 已跑同一份 191 例）

**真实 Linux 参照**：把镜像里的 191 个 RISC-V musl 测试二进制原样拿到
Ubuntu 22.04（内核 6.8）上用 qemu-user 跑（特权容器，真 root + loop 设备），
得到 **1190 TPASS / 19 TFAIL / 20 TBROK / 38 TCONF**。这就是这套用例在 Linux
上的参考上限，也说明 LA/RV 原来 302/287 的差距主要来自内核实现而不是测试本身。

**本轮修复**（RV/LA 共享，commit `944bc98`、`8005ca2`）：

| 修复 | 效果（每架构） |
|------|----------------|
| fork 时物理共享 `MAP_SHARED` 页 | LTP 子进程的结果计数能被父进程看到；access01 25→199 |
| pipe 缓冲 512→PGSIZE(4096) | pipe11 0→70（PIPE_BUF 语义） |
| `/proc/sys/kernel/pid_max` 桩 | getpid01 0→100，getppid01 setup |
| `clock_getres` + `CLOCK_*_COARSE` | setitimer01 |
| access/faccessat 按 uid/gid 位 + 目录搜索位 | access01 199/199（与 Linux 一致） |
| `open(O_CREAT, mode)` 应用创建模式 | fstat02 |
| filealloc/fileclose 槽位全清零、/dev/null 写入、关闭前槽位存活检查 | 消除 filewrite panic / 重复关闭 ext4_handle 卡死 |
| read/write 的 kmalloc 上限、-ENOMEM | 消除大 count read 的 panic |
| NPROC 128→512 + init 回收孤儿 | 后段用例不再因进程表满而 fork 失败 |
| waitpid WNOHANG、RV 每例 30s SIGKILL 看门狗 | 全流程不再被单个用例卡死 |

**实测（比赛 QEMU 命令、干净镜像）**：
- LA：191/191 用例，**651 TPASS**（access01 199、getpid01 100、pipe11 70），0 panic
- RV：191/191 用例，**见最终测评日志**，0 panic

对比第一轮：LA 302→651（2.15×），RV 287→同量级。剩余差距主要是不支持的
socket/SysV IPC/loop 设备/mount 和少量信号语义，见 §5.1–5.14。

### 5.16 第三轮：busybox 回退、RV 内存上限、ext4 关闭死循环（已修）

- **busybox 53→1 的回退（commit `961bb5e`）**：`6f0eb51` 让 fork 子进程继承父进程的
  信号处理器表。busybox ash 为管道/命令替换 fork 的子 shell 继承了 ash 的 SIGCHLD
  处理器，处理器抢先 reap 掉 applet，子 shell 的 `wait4` 得到 ECHILD，`$?` 变成
  255，于是所有外部 applet 命令都被判 fail。回退为子进程 SIG_DFL（kill12 少 1
  TPASS），busybox 恢复 51–53 分（共 +约 210 分）。
- **RV 只能用 51MB 内存（commit `f09ae5a`）**：RISC-V 的 `PHYSTOP` 还是
  `0x88000000`(128MB)，NPROC=512 后内核 BSS 到 `0x84c42000`，buddy 只管理
  **51MB**（LA 是 512MB）。所以 pipe11 的 50 个并发 fork 在 uvmcopy/kalloc 处失败
  （fork→EPERM，35/70），mem02 的 64MB 分配无结果。把 PHYSTOP 提到 `0xC0000000`
  （QEMU 1G）、`MAX_BUDDY_LEVEL=18`，buddy 管理 **947MB**，pipe11 70/70。
- **readlink01 内核死循环（commit `8ce9a03`）**：路径式 close 可能留下
  valid=1、mp=0 的槽位，`ext4_fclose` 的 `ext4_assert(mp)` 失败路径是 `while(1)`，
  SIGKILL 也杀不掉，整个测评卡死。关闭前同时检查槽位存活与 mp，必要时释放槽位。

**最终测评（official_eval_10.log，verdict Accpted，score 1236）**：
- LA：191/191 用例，**634 TPASS**
- RV：191/191 用例，**630 TPASS**（差 4，0.6%）
- busybox：LA 53/53，RV 51/53
- 残留小差异：chmod01 受 30s 看门狗影响 LA 偶尔只跑到 16/32；confstr01(4)、
  setitimer01(3)、prctl03/readlink01/symlink01(各2) 等 RV 单独失败。

### 5.17 第四轮：看门狗/输出收集/setitimer/NPROC（eval13，score 1237）

- **chmod01 被 30s 看门狗截断**：LA 只跑到 16/32。默认看门狗 30s→75s（mem02 单独 240s：
  它的 15000 次 valloc 在 TCG 下要几分钟），两边 chmod01 都完整 32。
- **readlink01/prctl03 在 RV 上 0 结果**：测试的 fork 子进程在父进程退出后才 flush 输出，
  runner 读文件太早。每个用例结束后 200ms settle 再读，两边都恢复 2 TPASS。
- **setitimer01**：只实现了一次性 timer、old value 全零。补上 it_interval 的保存/返回和
  周期重装后，两边都从 3/0 变成 **7 TPASS**。
- **NPROC 512→256**：PHYSTOP 修复后 fork 失败的原因是内存(51MB)而不是 proc 表；512 让
  每次切换扫描 512 个 proc，libctest/libcbench 的 pthread 组慢 2-4 倍。256 足够
  （pipe11 的 50 个并发子进程 + 残留），RV 组阶段从 ~40 分钟降到 ~35 分钟。

**eval13（official_eval_13.log，verdict Accpted，score 1237）**：
- LA：191/191，**654 TPASS**；RV：191/191，**646 TPASS**（差 8，1.2%）
- 已对齐：chmod01 32/32、pipe11 70/70、setitimer01 7/7、readlink01 2/2、prctl03 2/2、mem02 5/5
- 残留 8 个 TPASS：confstr01(4)、atof01(1)、clone04/fptest01/fptest02/gettid02/mmap03/prctl01
  各 1（RV 为 0），RV 侧多 fstatfs02/getdents01/munmap02 各 1。属于架构/浮点/单个
  syscall 语义差异，不影响两边跑同一份 191 例。
- 总耗时 57 分钟：~40 分钟是 basic/libctest/libcbench/lua 基准组（`b_stdio_putcgetc`
  单次 LA 98s、RV 78s，TCG 固定开销），LTP 阶段 LA ~10 分钟、RV ~19 分钟。

### 5.18 eval13 剩余 8 TPASS 差异的逐项归因（含本机证据）

用本机 Ubuntu 22.04（同一批二进制、qemu-user、真 root+loop）作参照
（`hostltp/sweep/results.tsv`，列：rc/TPASS/TFAIL/TBROK/TCONF）：

| 用例 | LA | RV | 本机 Linux | 归因 |
|------|----|----|-----------|------|
| confstr01 | 36 | 32 | **32** | **测试二进制差异**：LA 的二进制多编译了 `POSIX_V7_THREADS_CFLAGS/LDFLAGS` 两个 case（各 2 个 TPASS），串口里 LA 有这两行、RV 没有。本机 x86 也只有 32。不是内核。 |
| atof01 | 4 | 3 | **0（4 TFAIL）** | 浮点 `atof` 解析差异，本机也失败；与内核无关。 |
| fptest01/02 | 1/1 | 0/0 | **0（各 1 TFAIL）** | 浮点测试，本机也失败。 |
| clone04 | 1 | 0 | **0（SIGSEGV）** | musl 的 `clone(NULL stack)` 包装器差异（LTP 标了 musl-git fa4a8abd）；本机 qemu-user 同样 SIGSEGV。 |
| gettid02 | 1 | 0 | **11** | RV 线程栈 guard 缺页循环（`PF loop va=3fdfeff000 pc=40059e7c -> SIGSEGV`），是 RV 线程栈处理的遗留差异；LA 也只能拿到 1 TPASS。 |
| mmap03 | 1 | 0 | **1** | RV 上该二进制静默 exit 0、无任何结果输出；本机/LA 正常。单独重跑同样 0，属 RV 特例。 |
| prctl01 | 1 | 0（评测） | **2** | 单独重跑 RV 得到 1 TPASS+1 TFAIL（与 LA 相同），全量评测时受负载/超时影响变成 0，属波动。 |
| fstatfs02/getdents01/munmap02 | 0/2/0 | 1/3/1 | — | RV 反而多的 3 个 TPASS，抵消后净差 8。 |

**结论**：191 例列表相同、两边都完整跑完；剩余 8 TPASS（1.2%）中 6 个来自测试二进制/
浮点/用户态 musl 差异（本机 Linux 同样如此），另 2 个（gettid02/mmap03）是 RV 线程栈
与单测二进制特例，不属于 syscall 缺失。内核层面的对齐已经完成。

### 5.19 eval14（1237）与 init 调试块移除（919e241）

- **发现 `init-rv.c` 的 `main()` 里残留了 07:43 调试用的六用例块**
  （`readlink01/prctl03/confstr01/setitimer01/mmap03/proc01`），每次 RV 启动都会先跑它：
  `setitimer01` 会挂到框架 30s 超时并留下自旋子进程，白耗 ~30–60s 且污染串口。
  `919e241` 已删除该块，启动直接进入完整 `test_pre`。
- **eval14（official_eval_14.log，verdict Accpted，score 1237）**：
  LA 191/191，**655 TPASS**；RV 191/191，**647 TPASS**（差 8）。
- **gettid02 / mmap03 的确定性机理证据**（RV trap 诊断打印 + 页表/VMA）：
  - `gettid02`：`LAZY pid=19 name=gettid02 va=3fdfefebf8 vma=1 prot=0` —— pthread 栈的
    guard 页是 `PROT_NONE`，线程 sp 落到 guard 页，属 RV 线程栈布局下真实的 guard 缺页
    SIGSEGV；LA 的栈布局/缺页路径不同，故 LA 只拿到 1 TPASS。
  - `mmap03`：`LAZY pid=15/20 name=mmap03 va=3fdfeff000 vma=1 prot=4` ×8 +
    `PF loop ... pc=40059e7c -> SIGSEGV` —— 测试在 USTACK 下映射 `PROT_EXEC` 文件页，
    RV 的 mmap 游标没有让开父进程已继承的匿名 `data` VMA，exec 映射落在同一页，
    `memset(data)` 写 PROT_EXEC 页失败并反复缺页直到看门狗。属 RV VMA 重叠/线程栈
    处理的真实差异，需重做 RV mmap VMA 冲突检测与 clone 栈语义。
  - 其余 6 个差异（confstr01/atof01/fptest01/02/clone04 + RV 多的三项）已用本机
    Ubuntu 参照证明为测试二进制/浮点/用户态 musl 差异（见 §5.18），净差 8。

### 5.20 最后 3 个 RV 差异的真实根因与修复（2026-11-28）

§5.19 把 mmap03 归因于 VMA 重叠/继承，这个判断不完整。用测试二进制反汇编
（`mmap03` 的 `memcmp(dummy, addr)` 读 `PROT_EXEC` 映射）+ trap 诊断确认了三个
真正的内核 bug，并已修复：

1. **mmap03：PROT_EXEC 页缺 PTE_R**
   - RV 懒缺页只按 `v->prot` 给 `PTE_X`，RISC-V 的 execute-only 页在 `memcmp`
     读取时反复缺页，watchdog 最后才发 SIGSEGV。
   - 修复：`vmatrylazytouch()` 和 `mprotect()` 中 `PROT_EXEC` 同时授予 `PTE_R`
     （与 LoongArch/本项目用户态语义一致）。
2. **gettid02：部分 mprotect 没有拆分 VMA + getpid/gettid 语义错误**
   - musl `pthread_create()` 先 `mmap(PROT_NONE)` 整个栈，再对 guard 以上的
     可用区做部分 mprotect；旧代码只在整段 VMA 被覆盖时更新 `v->prot`，可用栈
     区一直是 `PROT_NONE`，线程第一次压栈就 SIGSEGV。
   - 修复：新增 `vma_mprotect_range()`，按 Linux 语义把部分 mprotect 拆成
     head / middle / tail 三个 VMA，并复制文件引用。
   - 同时 `getpid()` 返回 `tgid`、`gettid()` 返回 `tid`（原来都返回 `p->pid`，
     10 个线程全部 TFAIL）。
3. **mmapstress01：进程槽复用未清零缺页计数 + rt_sigreturn 覆盖 a0**
   - `allocproc()` 复用时保留上一个进程的 `fault_cnt`，新进程第一次缺页就被
     watchdog 当成第 9 次相同缺页，直接 SIGSEGV；修复为清零
     `fault_va/fault_sepc/fault_cnt`。
   - SIGALRM 在 `wait()` 返回后投递时，`sys_rt_sigreturn()` 恢复的 `a0` 被
     `syscall()` 用 sigreturn 自己的返回值 0 覆盖，用户态看到 `wait()` 返回 0，
     报 `unknown child pid 0`；修复为 `rt_sigreturn` 不回写 `a0`。
4. **munmap 范围化（避免 mprotect 拆分的回归）**
   - mprotect 拆分后，旧 `sys_munmap()` 只处理第一个 VMA，线程栈每次退出都会
     留下 usable 那一半 VMA；libcbench 的 2500 线程测试会耗尽 `NVMA`。
   - 改为 `mmap_replace_range()` 按范围 unmap/裁剪/拆分，munmap02 仍为 1 TPASS。

**验证（小规模、逐项对比 eval14）**

- 4 用例隔离复测（连续两次）：mmap03=1，gettid02=11，mmapstress01=1；
  clone04 仍为 TBROK（LTP 20240524 tag `musl-git fa4a8abd`，本机 Ubuntu 参照
  同样失败，不是内核差异）。
- 20 个 mmap/mprotect/munmap/clone/gettid 用例与 eval14 逐项对比：
  只有 mmap03 +1、gettid02 +11、mmapstress01 +1，其余 17 项全部不变
  （munmap02 仍 1，mprotect01 仍 1+2TFAIL，mmap04/08 等仍保持原状）。
- LA 3 用例冒烟：mmap03=1，gettid02=11，mmapstress01=1，0 panic/0 PF loop。
- `make clean && make all` 成功；RV/LA 内核均为 fullflow=1、无调试打印，
  init 仍为完整 191 例。按用户要求不再跑大规模回归；正式测评由用户执行。
  预计 LTP：LA 655 / RV 650（净差 5，全部为已证明的测试二进制/浮点/用户态
  musl 差异）。

### 5.21 NVMA 6144：mprotect 拆分导致 pthread 栈 VMA 翻倍的回归（本次正式测评暴露）

用户在 5227042 上跑正式测评时，RV 在 **libcbench-glibc** 的
`b_pthread_createjoin_serial2` 之后、`b_pthread_create_serial1` 阶段
`panic: mmap: no free vma`（现场另存 `rv_panic_vma_*.txt`）。

- 根因：`b_pthread_create_serial1` 一次创建 **2500 个不 join 的线程**；
  musl/glibc 的 pthread 栈都是先 `mmap(PROT_NONE)` 整段，再对 guard 以上的
  可用区 mprotect。5.20 的 VMA 拆分把每个栈变成 **2 个 VMA**
  （guard + usable），峰值 5000 个，超过 `NVMA=3072`，`sys_mmap` 直接 panic。
  这是 5.20 引入的回归，不是 LTP 用例问题。
- 修复：`NVMA 3072 -> 6144`（2500×2 + 1144 余量）。内核 BSS 从 ~39MB
  增至 ~77MB；在 1GB 内存下仍保留 ~920MB 给 buddy。
- 定向验证（都在 NVMA=6144 的干净构建上）：
  - RV `libcbench-glibc` 和 `libcbench-musl` 均跑完
    `b_pthread_create_serial1`、`b_pthread_uselesslock`，打印
    `#### OS COMP TEST GROUP END`，0 panic；
  - RV `mem02` 5/5、`gettid02` 11、`mmap03` 1、`mmapstress01` 1；
  - LA `mem02` 5/5、`gettid02` 11、`mmap03` 1、`mmapstress01` 1，0 panic。
- 之前那次正式测评因 RV panic 作废；提交目录里遗留的
  `sdcard-rv.img`/`sdcard-la.img` 已删除，重跑前不会再触发
  `gzip: ... already exists`。

### 5.22 最终正式测评 eval16（HEAD 00a6c63）：Accpted，1273

用户在 NVMA=6144 修复后重跑官方 docker 测评，结果：

```
{"verdict": "Accpted", "score": "1273", ...}
LTP:  LA 665 TPASS / RV 660 TPASS  （净差 5）
两个架构均 191/191 用例、10/10 组完成，0 panic
```

日志保留：

- `/tmp/opencode/official_eval_16.log`（完整 stdout，802KB）
- `/tmp/opencode/eval16_la_serial.txt` / `eval16_rv_serial.txt`
- `/tmp/opencode/eval16_console.log`

与 eval14（1237，LA 655 / RV 647）逐项对比：

| 项目 | eval14 | eval16 | 变化 |
|---|---:|---:|---:|
| 总分 | 1237 | **1273** | **+36** |
| LTP LA | 655 | **665** | +10 |
| LTP RV | 647 | **660** | +13 |
| basic-glibc-rv | 87 | 92 | +5 |
| basic-musl-rv | 87 | 92 | +5 |
| libcbench-glibc-rv | 22.22 | 30.40 | +8.18 |
| libcbench-musl-rv | 21.0 | 25.24 | +4.24 |
| libctest-musl-rv | 178 | 189 | +11 |

LTP 逐项变化只有：

```
LA gettid02   1 -> 11
RV gettid02   0 -> 11
RV mmap03     0 -> 1
RV mmapstress01 0 -> 1
RV getdents01 3 passed/3 failed -> 3 passed/0 failed
```

没有新增差异或回归。剩余 8 个差异仍是：

```
LA 多: atof01 +1, clone04 +1, confstr01 +4, fptest01 +1, fptest02 +1
RV 多: fstatfs02 +1, getdents01 +1, munmap02 +1
净差 = 8 - 3 = 5
```

这些差异此前已用本机 Ubuntu 参照/测试二进制检查证明为浮点、测试二进制枚举
（confstr01）和用户态 musl 差异，不是内核可修复项。NVMA=6144 之后
libcbench-glibc/musl 的四个 pthread 压测全部通过，正式测评不再 panic。
