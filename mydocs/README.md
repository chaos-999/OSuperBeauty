# mydocs/ — 我的学习文档

本目录用于存放从 xv6 到 SpringOS 参赛的学习资料。

| 文件 | 内容 | 生成时间 |
|------|------|---------|
| `01-structure-guide.md` | SpringOS 文件结构与 xv6 对比分析，每个文件夹/文件干嘛的 | 2026-05-25 18:57 |
| `02-beginners-guide.md` | 参赛入门完整教程：要做什么、怎么改、测试怎么跑、FAQ | 2026-05-25 18:29 / 更新 18:41 |
| `03-file-catalog.md` | 完整文件/目录一览表：所有文件内容概要、syscall 清单、架构分布 | 2026-05-31 18:22 |
| `04-online-eval-guide.md` | 线上评测完整指南：评测流程、当前状态检查、需要改哪些文件、新手逐行讲解 | 2026-05-31 19:41 |
| `05-git-quickstart-guide.md` | Git 零基础快速上手：从安装到 PR/MR，覆盖 GitHub/GitLab/Gitee 平台差异 | 2026-06-01 10:11 |
| `06-autotest-guide.md` | autotest-for-oskernel 本地评测完整指南：环境准备、步骤拆解、评测全链路走读、常见问题 | 2026-06-01 10:46 |
| `07-basic-test-fix.md` | 🔧 RISC-V Basic 测试修复报告：诊断 0 分根因、修复 init-rv.c/init-la.c、后续待修复项 | 2026-06-01 15:01 |
| `08-score-analysis.md` | 📊 Basic 测试逐题得分分析：32 题逐题评分对照、RISC-V 96%/LoongArch 100%、后续 P0-P5 修改清单 | 2026-06-01 18:38 |
| `09-competition-strategy.md` | 🏆 初赛晋级形势分析：237 队分层画像、分数线估算、最优攻略路线（P0-P4）、时间线建议 | 2026-06-01 18:59 |
| `10-docker-test-flow.md` | 🔁 Docker 测评工作流优化：gzip 残留报错根因、Makefile 一键测评目标、目录权限只读设置 | 2026-06-02 15:18 |
| `11-prelim-code-slim.md` | ✂️ 初赛代码瘦身：QEMU vs 板子文件全量分类、可安全删除清单、Makefile 清理建议、保守/激进两套方案 | 2026-06-02 20:50 |
| `12-makefile-qemu-busybox.md` | 📖 Makefile + QEMU 入门教程 + busybox 调试实录：逐行讲 Makefile、逐参数讲 QEMU、记录 busybox 死循环的发现过程和解决方案 | 2026-06-02 22:21 |
| `13-busybox-pipe-sendfile-debug.md` | 🔍 Busybox 管道静默失败调试：syscall 日志插桩方法、从日志定位 cat 用 sendfile(71) 而非 write(64) 的完整过程、sendfile 修复方向 | 2026-06-03 14:25 |
| `14-sendfile-ppoll-pipe-fix.md` | 🔧 sendfile + ppoll 修复实录：sendfile 16MB count cap、ppoll 睡 pipe 频道消除时序竞争、管道打通全流程 | 2026-06-03 15:57 |
| `15-procfs-design.md` | 🗂️ procfs 虚拟文件系统实现方案：VFS 架构分析、逐步实现指南（7 阶段）、与 EXT4 隔离、支持 busybox ps | 2026-06-03 20:02 |
| `16-console-output-race.md` | 🔍 控制台输出竞争完整诊断与修复：根因三层分析（consolewrite 无锁/timer 抢断/测试集 printf 分段 write）→ 方案评估 → 行缓冲实现 → 验证 | 2026-06-05 07:59 |
| `17-buddy-panic-extent-double-free.md` | ⚠️ **治标方向**：buddy_free panic 诊断与修复：ext4 extent tree split 失败 → error path 对 path 二次释放 → buddy 将 panic 降级为 warning（根因未修） | 2026-06-05 11:12 |
| `18-musl-test-gap-analysis.md` | 🔍 musl 测试缺失分析：三层根因（init 代码路径/动态扫描器被禁用/LA execve 缺 auxv）、LA busybox-musl 静默崩溃诊断、修复方案 | 2026-06-06 19:02 |
| `19-la-musl-debug-full.md` | 🔬 LA musl 测试调试全记录：auxv 修复→pcalau12i 二进制 bug 定位→测评环境问题→glibc sleep namei 分析→最终修复汇总 | 2026-06-07 00:17 |
| `20-glibc-vs-musl.md` | 📚 glibc 与 musl 区别详解：启动初始化、syscall 发起、GOT 策略、内存分配、信号处理五大维度对比，本次调试中差异的实际影响 | 2026-06-07 00:35 |
| `21-ltp-syscall-list.md` | 📋 LTP 测试集 syscall 完整清单：360 个测试 syscall 按类别分组、用例数统计、SpringOS 实现状态对照、构建排除说明 | 2026-06-07 11:25 |
| `22-execve-enoexec-fix.md` | 🔧 execve 非 ELF 文件返回 ENOEXEC 修复实录：libctest run-dynamic.sh "Operation not permitted" 根因（内核 -1 → musl → EPERM → busybox 不 fallback）→ 4 处修改 → xv6 vs musl 返回值约定分析 | 2026-06-07 13:12 |
| `23-proc-self-exe-fix.md` | 🔧 libctest 脚本执行链完整修复实录：EPERM→ENOEXEC（exec.c）→ busybox ash tryexec() 源码分析→发现用 /proc/self/exe 而非 /bin/sh→内核 5 文件实现 /proc/self/exe 虚拟路径→完整链路打通 | 2026-06-07 14:24 |
| `24-proc-self-exe-clone-fix.md` | 🔧 /proc/self/exe 完整实现与 clone 修复：5 文件内核改动→发现 fork 子进程 exec_path 为空→定位 clone() 缺失拷贝→修复后 exec_path 正常→完整验证链路→lua 测试 9/11 通过 | 2026-06-07 15:00 |
| `25-syscall-errno-interp-fix.md` | 🔧 系统调用 errno 语义修复 + 动态链接器支持：~190 处 syscall `return -1` → `-ERRNO`，execve AT_PHDR 修正，PT_INTERP 动态链接器加载，ld.so 嵌入内核 | 2026-06-09 03:23 |
| `26-libctest-kernel-fixes.md` | 🔧 libc-test 内核修复：AT_PHDR 虚拟地址修正，clone CLONE_SETTLS / CLONE_THREAD / CLONE_CHILD_CLEARTID，rt_sigtimedwait 实现 | 2026-06-09 15:36 |
| `27-libctest-musl-full.md` | ⚠️ **部分过时**：libc-test musl 全量测试：直接 exec 方案实现 82/82 PASS，完整诊断 runtest.exe 失败根因（sigtimedwait/waitpid 链路）→ runtest.exe 根因已被 `40` 修正 | 2026-06-09 16:36 |
| `28-unimplemented-features-roadmap.md` | 🗺️ 未实现功能实现路线图：pthread/FUTEX/socket 等 4 个模块的逐步实现方案，含代码示例和优先级建议 | 2026-06-09 16:42 |
| `29-p2p3p4-implementation-and-false-pass.md` | 🔧 内核 panic 修复 & 假 PASS 诊断：施工记录 + 诊断报告 | 2026-06-10 |
| `30-libctest-classification-false-pass.md` | 📊 libc-test 全测试分类 & 假 PASS 归因：96 个测试逐类分析 | 2026-06-10 |
| `31-presentfailure-fix-guide.md` | 🔧 逐测试修复指南：基于 presentfailure 日志，每个假 PASS 的根因和修复方案 | 2026-06-11 |
| `32-ltp-full-manual.md` | 📚 LTP 测试套件完整手册：分类、数量、测试目标 | 2026-06-12 |
| `33-ltp-dynamic-linking-problem.md` | 🔍 LTP 动态链接问题：诊断与解决方案 | 2026-06-13 |
| `34-eval-debug-summary.md` | ⚠️ **部分过时**：测评调试总结 (6/25-6/28)：所有问题、根因、修复方案的汇总文档（其中 ext4_journal inc_ref 结论沿用 `37`，已被 `39` 修正） | 2026-06-28 15:17 |
| `35-initcode-12kb-overflow-fix.md` | 🔧 Initcode 12KB 溢出修复：uvmfirst 加第 4 页 + p->sz 动态计算，解决 LA basic test 全部崩溃 | 2026-06-28 15:20 |
| `36-rv-bb-testcode-fix.md` | 🔧 RV busybox argv 损坏修复：bb_testcode 全局→局部变量 | 2026-06-28 15:21 |
| `37-ext4-lru-use-after-free.md` | ❌ **结论错误**：RV ext4 块缓存 LRU 树 Use-After-Free 分析：ext4_journal inc_ref 绕过 LRU 维护（journal 已关闭，真正原因是 ext4 并发重入，见 `39`） | 2026-06-28 15:21 |
| `38-la-busybox-musl-silent-exit.md` | 🔍 LA busybox-musl 静默退出诊断：musl pcalau12i 工具链 bug，非内核问题 | 2026-06-28 15:22 |
| `39-ext4-concurrent-lru-corruption.md` | 🔍 RV ext4 LRU 红黑树崩溃/卡死定位：ext4 挂载锁未安装导致并发重入，修复 fstat/statx 引用泄漏 | 2026-11-11 |
| `40-runtest-timeout-and-sigkill.md` | 🔍 libc-test runtest.exe 全 FAIL/卡死复盘：sigtimedwait tick 10Hz/100Hz 换算错误 + SIGKILL 未唤醒睡眠进程 | 2026-11-11 |
| `41-rv-dynamic-linker-pt-interp.md` | 🔧 RV 动态链接缺失支持修复：解析 PT_INTERP、加载解释器到 0x40000000、设置 AT_BASE/AT_ENTRY、兼容 /musl/lib 与 /glibc/lib 路径 | 2026-11-12 |
| `42-elf-dynamic-linker-primer.md` | 📚 ELF 与动态链接零基础入门：ELF header/program header/PT_LOAD/PT_INTERP、静态 vs 动态链接、GOT/PLT/RELA、AT_PHDR/AT_ENTRY/AT_BASE，并逐项对应 41 的实现 | 2026-11-12 |

## 勘误与错误方向标记

后续文档修正了早期文档中的部分结论，阅读旧文档时请以下表为准：

| 旧文档 | 标记 | 问题 | 以哪篇为准 |
|--------|------|------|------------|
| `37-ext4-lru-use-after-free.md` | ❌ **结论错误** | 把 ext4 LRU 树损坏归因于 `ext4_journal.c` 的 `ext4_bcache_inc_ref()` 和缓存 shake；但 `CONFIG_JOURNALING_ENABLE=0`，journal 路径不参与，真正原因是 ext4 挂载锁未安装导致的多进程并发重入 | `39-ext4-concurrent-lru-corruption.md` |
| `34-eval-debug-summary.md` | ⚠️ **部分过时** | 其中 ext4 LRU use-after-free 的“journal inc_ref”根因沿用自 `37`，同样不成立 | `39-ext4-concurrent-lru-corruption.md` |
| `27-libctest-musl-full.md` | ⚠️ **部分过时** | 对 `runtest.exe` 的分析停留在“sigtimedwait 睡在 `ticks` 上、wakeup 不匹配、waitpid 返回 EAGAIN”；当前实际根因是 tick 换算 10 Hz/100 Hz 错误，以及 SIGKILL 未唤醒睡眠进程 | `40-runtest-timeout-and-sigkill.md` |
| `17-buddy-panic-extent-double-free.md` | ⚠️ **治标方向** | 用 buddy warning 降级掩盖 ext4 extent path 的 double-free，属于避免 panic 的防御性处理，并未修复根因 | 暂无后续根因文档，使用前需重新评估 |
| 根目录 `MISDIAGNOSIS-REPORT.md` | ❌ **结论错误** | 该报告认为 `CONFIG_BLOCK_DEV_CACHE_SIZE` 16→512 是止血方案，并怀疑 journal `end_write`；这两点都已被推翻 | `39-ext4-concurrent-lru-corruption.md` |

> 说明：`⚠️` 表示文档中部分结论仍有效、但关键根因已被后续文档修正；`❌` 表示该文档的核心方向已被证明错误，不应再作为排查依据。
