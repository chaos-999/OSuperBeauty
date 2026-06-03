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
