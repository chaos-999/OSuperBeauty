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
| `17-buddy-panic-extent-double-free.md` | 💥 buddy_free panic 诊断与修复：ext4 extent tree split 失败 → error path 对 path 二次释放 → buddy 将 panic 降级为 warning | 2026-06-05 11:12 |
| `18-musl-test-gap-analysis.md` | 🔍 musl 测试缺失分析：三层根因（init 代码路径/动态扫描器被禁用/LA execve 缺 auxv）、LA busybox-musl 静默崩溃诊断、修复方案 | 2026-06-06 19:02 |
| `19-la-musl-debug-full.md` | 🔬 LA musl 测试调试全记录：auxv 修复→pcalau12i 二进制 bug 定位→测评环境问题→glibc sleep namei 分析→最终修复汇总 | 2026-06-07 00:17 |
| `20-glibc-vs-musl.md` | 📚 glibc 与 musl 区别详解：启动初始化、syscall 发起、GOT 策略、内存分配、信号处理五大维度对比，本次调试中差异的实际影响 | 2026-06-07 00:35 |
| `21-ltp-syscall-list.md` | 📋 LTP 测试集 syscall 完整清单：360 个测试 syscall 按类别分组、用例数统计、SpringOS 实现状态对照、构建排除说明 | 2026-06-07 11:25 |
| `22-execve-enoexec-fix.md` | 🔧 execve 非 ELF 文件返回 ENOEXEC 修复实录：libctest run-dynamic.sh "Operation not permitted" 根因（内核 -1 → musl → EPERM → busybox 不 fallback）→ 4 处修改 → xv6 vs musl 返回值约定分析 | 2026-06-07 13:12 |
| `23-proc-self-exe-fix.md` | 🔧 libctest 脚本执行链完整修复实录：EPERM→ENOEXEC（exec.c）→ busybox ash tryexec() 源码分析→发现用 /proc/self/exe 而非 /bin/sh→内核 5 文件实现 /proc/self/exe 虚拟路径→完整链路打通 | 2026-06-07 14:24 |
| `24-proc-self-exe-clone-fix.md` | 🔧 /proc/self/exe 完整实现与 clone 修复：5 文件内核改动→发现 fork 子进程 exec_path 为空→定位 clone() 缺失拷贝→修复后 exec_path 正常→完整验证链路→lua 测试 9/11 通过 | 2026-06-07 15:00 |
| `25-syscall-errno-interp-fix.md` | 🔧 系统调用 errno 语义修复 + 动态链接器支持：~190 处 syscall `return -1` → `-ERRNO`，execve AT_PHDR 修正，PT_INTERP 动态链接器加载，ld.so 嵌入内核 | 2026-06-09 03:23 |
| `26-libctest-kernel-fixes.md` | 🔧 libc-test 内核修复：AT_PHDR 虚拟地址修正，clone CLONE_SETTLS / CLONE_THREAD / CLONE_CHILD_CLEARTID，rt_sigtimedwait 实现 | 2026-06-09 15:36 |
| `27-libctest-musl-full.md` | ✅ libc-test musl 全量测试：直接 exec 方案实现 82/82 PASS，完整诊断 runtest.exe 失败根因（sigtimedwait/waitpid 链路） | 2026-06-09 16:36 |
| `28-unimplemented-features-roadmap.md` | 🗺️ 未实现功能实现路线图：pthread/FUTEX/socket 等 4 个模块的逐步实现方案，含代码示例和优先级建议 | 2026-06-09 16:42 |
