# libcbench：RV/LA 双架构双 libc 全通过 + 官方分数

> 接 `54-libcbench-performance-and-official-score.md`。本文是 libcbench 的**收尾文档**：
> 在 `54` 之后又修掉了 LA 上 musl 启动即崩（`libc.auxv=0`）和线程组 OOM 两个致命问题，
> 使 **RV/LA 两架构、glibc/musl 两组全部 27/27 完成**，官方 docker harness 的
> libcbench 合计稳定在 **115.5–116.1**。
>
> 结论：libcbench 已达成目标，不再需要投入。

---

## 1. 最终官方分数（harness `Accpted`，取 `harness_1162.log`）

| 组 | RV | LA |
|----|----|----|
| libcbench-glibc | 30.90 | 28.94 |
| libcbench-musl  | 27.25 | 28.87 |

四组合计约 **116**。对照基线（ea0d57c，libcbench 四组全部 0 分），净增 ~116。

> judge 公式（`generate_score`）：每个 benchmark `score = baseline/res`；
> `>=1` 时 `2-1/score`，否则 1.0。27 项满分 54/组。我们多数项在 1.0 下限附近，
> pthread create/join 因为线程数少（LA 上 `#ifdef __loongarch64` 只跑 10 次）能到 1.9。

## 2. 本轮（`54` 之后）新增的两个致命修复

### 2.1 `prlimit64` 空实现 → glibc 线程栈数 GB → LA OOM

- 症状：LA libcbench 在 `b_pthread_createjoin_serial2` 阶段 `panic: sig_init: out of memory`
  （或 `vmalazytouch: kalloc`），隔离的 2500 线程 create/join + fork 却正常。
- 根因：`sys_prlimit64` 只 `return 0` 不写用户结构。glibc 启动时 `getrlimit(RLIMIT_STACK)`
  读到栈上垃圾（LA 上约 4.8 GB），于是默认 pthread 栈变成数 GB。
  每次 `pthread_create` 的 mmap 都会为这个巨型 VMA 预创建页表级（LA 每 2 MB 一个页表页），
  一个线程 ≈ 9 MB 内核页表，50 个并发线程就把 512 MB 物理内存打穿。
- 修复：`sys_prlimit64` 按 Linux 语义返回 `RLIMIT_STACK=8 MiB`、`NPROC`、`NOFILE`。
  修完后线程栈回到 8 MiB，serial2 时间从几十秒回到 0.1 s 级。

### 2.2 `mmap` 取址向全地址空间最低 VMA 下沉 → musl TLS 落进程序镜像

- 症状：LA libcbench **musl** 组启动即崩（`libc.auxv=0`，mallocng `get_random_secret`
  解引用 NULL），busybox-musl 同样 `a_crash`。
- 根因：`sys_mmap` 非固定映射把游标降到「全地址空间最低 VMA」（musl 在 brk 堆附近有个
  `MAP_FIXED|PROT_NONE` 保留区），于是 `__init_tls` 的 `mmap(NULL, tls_size, RW)` 被分到
  **程序镜像/BSS 页之内**（`0x120010000`）。之后 `free()` 对该组 `__munmap` 把这块 BSS 页
  解除映射 → errno/TLS 所在页没了 → musl 崩。
- 修复：mmap 游标从 `USTACK` 向下，只跳过**与候选区间真正重叠**的 VMA，低地址的
  MAP_FIXED 保留区不再把游标拖下去。

## 3. 顺带修掉（libcbench / LTP 共用）

- **LA TLB refill 死循环**：中间页表级缺失时（NULL 指针即可）`lddir` 得 0，旧代码
  `addi -1` 后 `ldpte/tlbfill` 装垃圾项 → 无限 refill，整机 100% CPU。现逐级检查 `V` 位，
  缺失时填无效 TLB 项，重试产生正常 PIL/PIS 交给 usertrap。
- **RV PIE `AT_PHDR` 哨兵**：`first_load_vaddr==0` 当“未设置”，PIE 首 LOAD vaddr=0 时
  AT_PHDR 指向第二个段；改用 `first_load_set` 标志。
- **RV 动态解释器跨 fork 复制**：PIE 程序 ld.so 落在 `p->sz` 之上，`uvmcopy` 漏复制，
  子进程第一条 ld.so 指令即指令缺页；新增 `interp_start/interp_end` 随 fork/exec 复制/释放。

## 4. 遗留（可接受，不再投入）

- 各 benchmark 的 `score` 大多卡在 1.0 下限：malloc/stdio/string 在 TCG 下比 baseline 慢
  （baseline 是平台参考机），要越过下限需要把 malloc（页缺页路径）、stdio tmpfile
  （ext4 块缓存 + 无 readahead）做深，性价比低。
- LA musl `b_stdio_putcgetc` 单项 293 s 是最大单项时间，根因同 stdio：16 块块缓存 +
  4 KB 单块读 + 无 readahead（见 `58-ltp-...` 的 Linux 对照）。
