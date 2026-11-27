# LTP：187 例（LA）+ 6 例（RV）子集跑通，遗留问题与 Linux 对照

> 接 `55-ltp-dynamic-loader-blocker.md`。上一轮结论是“镜像 LTP 全是动态链接，loader 语义不足”。
> 本轮把动态 loader 的关键缺口补齐后，LA 上跑通了一个 **187 例**的 syscall 子集
> （官方 harness 采到 raw 90，隔离验证 168），RV 保留 6 例（raw 7）。
>
> 结论：**判分格式、运行框架、动态 loader 都已打通**；剩余问题集中在
> ① 块缓存无 readahead/过小导致加载慢（LTP 超时）、② 块缓存偶发陈旧读、③ 若干
> 语义偏差（pipe EPIPE、symlink、mount、getdents 损坏项）。下文逐项对照 Linux 实现。

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

## 4. 用例筛选（200 → 187 LA / 60 RV）

从镜像 `/musl/ltp/testcases/bin`（950 个二进制）按「syscall 名 + 两位编号」过滤，
排除网络/IPC/命名空间/抢占等类别，再逐个剔除会 panic/死锁/刷屏的：

- 死锁（100% CPU、中断关闭，watchdog 也杀不掉）：
  `getcwd03`（chdir+readdir 清理）、`creat06`（mount+清理）、`lchown01/03`、`lstat01`、
  `readlink01/readlinkat01/02`（symlink 解析）、`openat01/201`（`strcpy` 空源）
- `panic: sched locks`：`fchownat02`
- 刷屏（pipe 满写返回 EPERM 导致用例重试）：`pipe04/15`

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
  1158/1162。疑似 lwext4 块缓存在大容量下的 LRU/up-to-date 维护 bug，根因未最终定位。
- Linux 对照：page cache 用 `struct page` + 每页 `PageUptodate/PageDirty` 标志，块的
  生命周期由 radix tree 与 refcount 严格管理，绝无「同一 LBA 返回旧数据」；
  `end_buffer_async_read` 里 `SetPageUptodate` 只在真实读完磁盘后置位。

### 5.3 pipe 写 EPIPE 语义（pipe04 刷屏）

- 我们的 `pipewrite` 在 `readopen==0` 时 `return -1`（→ 用户态 EPERM）；LTP pipe04 期望
  `EPIPE`（并触发 SIGPIPE），拿到 EPERM 后子进程重试刷屏。
- Linux：读端关闭时 `pipe_write` 返回 `-EPIPE` 并向本线程发 `SIGPIPE`（`kill_fasync`）。

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

## 6. 结论与下一步建议

- LTP 判分是「TPASS 数量」的对数映射，**多跑用例收益递减**（raw 100→18 分，500→80）。
- 真正卡分的是 5.1/5.2 的块缓存：修好 readahead + 陈旧读后，动态加载从 ~27s 降到秒级，
  LA 187 例能跑满（raw ~168，+~12 分），RV 也能扩到几十例；但块缓存陈旧读是 lwext4
  层面的深坑，性价比中等、风险高。
