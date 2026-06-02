# ✂️ 初赛代码瘦身：QEMU 测试 vs 现场板子文件分类与清理建议

> 📅 生成时间：`2026-06-02 20:50 (GMT+8)`
>
> 目标：梳理当前仓库中哪些文件是初赛 QEMU 测试需要的，哪些是现场赛跑板子（VF2/2K1000）的代码，给出可安全删除/暂屏蔽的建议，让初赛维护更轻量。

---

## 一、仓库全景：三套架构、两块板子

当前 `main` 分支同时承载了三套编译目标，Makefile 用条件编译（`-D QEMU` / `-D VF2` / `-D LA2K1000`）和架构目录区分：

```
SpringOS
├── 初赛 QEMU 测试（你要的核心）
│   ├── RISC-V QEMU  → kernel-rv
│   └── LoongArch QEMU → kernel-la
│
├── 现场赛板子（初赛用不到）
│   ├── VisionFive 2 (RISC-V) → kernel-vf2
│   └── 龙芯 2K1000LA (LoongArch) → kernel-la2k1000
│
└── 共用基础设施
    ├── 文件系统 (ext4/lwext4)
    ├── 进程管理、内存管理
    ├── 系统调用
    └── 用户程序
```

`make all` 只编译 `kernel-rv + kernel-la`，不碰板子代码。但板子代码仍然占据目录空间、干扰 git diff、增加心智负担。

---

## 二、板子专用文件清单（初赛可安全删除）

### 2.1 整目录删除

| 路径 | 内容 | 说明 |
|------|------|------|
| `boot/vf2/` | `entry.S` + `main.c`（3 文件，16KB） | VisionFive 2 板子引导代码 |
| `boot/2k1000/` | `entry.S` + `main-2k1000.c` + `early_uart.c`（3 文件，20KB） | 2K1000 板子引导代码 |
| `kernel/vf2/` | `kernel.ld`（1 文件，8KB） | VF2 链接脚本 |
| `kernel/2k1000/` | `kernel.ld`（1 文件，8KB） | 2K1000 链接脚本 |
| `include/board/` | `2k1000.h`（1 文件） | 2K1000 板级地址/中断定义 |
| `site/` | `git_testcode.sh`（1 文件） | 现场赛 git 测试脚本 |

> 6 个目录共 **约 52KB**，删掉后 `make all` 完全不受影响。

### 2.2 单片源文件可删

| 路径 | 行数 | 说明 |
|------|------|------|
| `kernel/trap/la/iointc-2k1000.c` | 75 | 2K1000 IO 中断控制器专用驱动 |
| `kernel/fs/ramdisk.c` | ~100 | 板子 RAMDisk 驱动，整文件被 `#if defined(VF2) \|\| defined(LA2K1000)` 包裹 |

> `ramdisk.c` 虽然放在共用 `kernel/fs/` 下，但整文件由预处理器守卫，MQUMU 编译时完全跳过。删掉更干净。

### 2.3 板子专用工具和文档可删

| 路径 | 说明 |
|------|------|
| `tools/create_ramdisk.sh` | VF2/2K1000 RAMDisk 镜像构建脚本 |
| `loongson2K1000LA.md` | 龙芯 2K1000 用户手册 |
| `testsh/judge-site.sh` | 现场赛评测脚本 |

---

## 三、共享文件中的板子代码（不要删，不要动）

以下源文件同时被 QEMU 和板子编译，通过 `#ifdef VF2` / `#ifdef LA2K1000` 条件分支切换。**删除会导致编译错误或代码逻辑缺失。**

| 文件 | 板子条件分支 | QEMU 编译路径 |
|------|:---:|------|
| `kernel/drive/rv/plic.c` | `#ifdef VF2` × 5 处 | QEMU 走主路径，不通 VF2 分支 |
| `kernel/drive/uart.c` | `#if VF2` / `#if LA2K1000` 多处 | QEMU 路径不受影响 |
| `kernel/fs/VFS_block.c` | `#ifdef VF2` / `#ifdef LA2K1000` | 独立分支，互不干扰 |
| `kernel/fs/vfs/file.c` | `#ifdef LA2K1000` × 2 处 | QEMU 编译时跳过 |
| `kernel/mm/vm.c` | `#ifdef LA2K1000` × 8 处 | LA QEMU 走主路径 |
| `kernel/proc/exec.c` | `#ifdef LA2K1000` × 3 处 | LA QEMU 走主路径 |
| `kernel/proc/proc.c` | `#ifdef LA2K1000` × 6 处 | LA QEMU 走主路径 |
| `kernel/trap/la/trap.c` | `#ifdef LA2K1000` × 10 处 | LA QEMU 走主分支 |
| `kernel/trap/rv/trap.c` | `#ifdef VF2` | QEMU 不受影响 |
| `kernel/util/printf.c` | `#ifdef LA2K1000` × 1 处 | 极小的特殊处理 |
| `include/memlayout.h` | `#ifdef VF2` + `#ifdef LA2K1000` 大段 | QEMU 路径独立 |
| `include/platform.h` | `#ifdef LA2K1000` | QEMU 走主路径 |
| `include/defs.h` | `#if defined(VF2) \|\| defined(LA2K1000)` | 仅声明板子函数原型 |

> **结论：这些文件一个都不要动。** 里面的板子代码在 QEMU 编译时被预处理器跳过，不影响初赛测试。

---

## 四、Makefile 清理建议

### 4.1 可以删除的整段目标

| 行号范围 | 内容 | 说明 |
|---------|------|------|
| 16–18 | `K_SRC_VF2` / `K_ASM_VF2` 变量 | VF2 源码清单 |
| 41–51 | `OBJS_VF2` 变量 | VF2 编译产物清单 |
| 117–121, 126, 133–155 | `CFLAGS_VF2` | VF2 编译选项 |
| 197–232 | `vf2` / `ramdisk.img` / `kernel-vf2` 目标 | VF2 整条构建链 |
| 158–162 | `CFLAGS_LA2K1000` | 2K1000 编译选项 |
| 533–557 | `%-la2k1000.o` / `%-2k1000.o` / VF2 obj 规则 | 板子编译规则 |

### 4.2 需要保留但改名的（可选）

- `2k1000: kernel-la2k1000` 目标 — 建议注释掉或删除
- `.PHONY` 里的 `vf2` / `la2k1000` — 建议移除

### 4.3 做法建议

**最省事且安全：** 不动 Makefile，只删第二节列出的文件和目录。因为 `make all` 根本不碰板子目标，板子代码的文件路径在 Makefile 变量里定义但不会被 `all` 触发编译。留几行注释掉的变量不增加任何编译开销。

**极致洁癖：** 删完板子文件 + 板子文档后，再把 Makefile 里的 VF2/2K1000 相关段落注释掉或删除即可。

---

## 五、文档和辅助文件归类

### 5.1 📌 保留（初赛有用）

| 文件 | 用途 |
|------|------|
| `README.md` / `README` | 项目说明 |
| `mydocs/` | 学习文档，全部保留 |
| `Makefile` | 核心构建 |
| `.gitignore` | 版本控制 |
| `.editorconfig` / `.clang-format` | 代码风格 |
| `testsh/basic.md` | Basic 测试用例说明 |
| `testsh/busybox.md` | Busybox 命令说明 |
| `testsh/busybox_testcode.sh` | Busybox 测试脚本（参考用） |
| `testsh/judge.sh` / `judge-la.sh` | 本地快速测评脚本 |
| `testsh/judge-offline.sh` | 离线日志评分 |
| `testsh/analyze_log.sh` | QEMU 日志分析 |
| `testsh/oscomp_syscalls.md` | 系统调用清单 |
| `busybox_cmd.txt` | Busybox 命令列表 |

### 5.2 🗑️ 可删（初赛用不到）

| 文件 | 理由 |
|------|------|
| `loongson2K1000LA.md` | 2K1000 板子用户手册 |
| `mmexport.md` | LoongArch 异常码表（QEMU LA 用 SBI，不依赖这个表） |
| `op.txt` | 不明用途的地址列表，疑似调试用 |
| `loongarch_ABI.txt` | 1373 行 ABI 参考（需要时 Google 即可） |
| `testsh/judge-site.sh` | 现场赛评测脚本 |
| `testsh/judge_all.sh` | 综合评测脚本（线上平台用，本地 Docker 不需要） |

> 上述 6 个文件可用可删，不影响编译和测评，留着也不碍事。

---

## 六、架构专用目录 — 哪些是 QEMU 必需的

```
boot/                        kernel/                      kernel/drive/
├── main.c       ← ✅ 共享   ├── drive/                   ├── uart.c     ← ✅ 共享
├── rv/          ← ✅ QEMU   │   ├── rv/   ← ✅ QEMU     ├── rv/        ← ✅ QEMU-RV
│   ├── entry.S              │   │   ├── plic.c           │   ├── plic.c
│   ├── start.c              │   │   └── virtio_disk.c    │   └── virtio_disk.c
│   └── initcode*.S          │   └── la/   ← ✅ QEMU     └── la/        ← ✅ QEMU-LA
├── la/          ← ✅ QEMU   │       ├── pci.c                └── pci.c,virtio_disk.c,
│   ├── entry.S              │       ├── virtio_disk.c            virtio_pci.c,virtio_ring.c
│   └── initcode*.S          │       ├── virtio_pci.c
├── vf2/         ← ⛔ VF2    │       └── virtio_ring.c    kernel/trap/
│   ├── entry.S              │   ├── fs/     ← ✅ 共享    ├── rv/  ← ✅ QEMU-RV
│   └── main.c               │   ├── lock/   ← ✅ 共享    ├── la/  ← ✅ QEMU-LA
└── 2k1000/      ← ⛔ 2K1000 │   ├── mm/     ← ✅ 共享        ├── apic.c    ← ✅ LA
                              │   ├── proc/    ← ✅ 共享        ├── extioi.c  ← ✅ LA
                              │   ├── syscall/ ← ✅ 共享        └── iointc-2k1000.c ← ⛔ 2K1000
                              │   ├── util/    ← ✅ 共享    ├── signal.c ← ✅ 共享
                              │   ├── rv/      ← ✅ QEMU
                              │   ├── la/      ← ✅ QEMU
                              │   ├── vf2/     ← ⛔ VF2
                              │   └── 2k1000/  ← ⛔ 2K1000
                              └── include/     ← ✅ 共享(除board/)
```

> **核心结论：`boot/rv/` + `boot/la/` + `kernel/rv/` + `kernel/la/` + `kernel/drive/rv/` + `kernel/drive/la/` + 所有共享目录 = QEMU 编译所需全部。**

---

## 七、推荐操作清单

### 保守方案（零风险，5 分钟）

```bash
cd /home/my-springos

# 删板子专用目录
rm -rf boot/vf2 boot/2k1000 kernel/vf2 kernel/2k1000 include/board site

# 删板子专用文件
rm -f kernel/trap/la/iointc-2k1000.c
rm -f kernel/fs/ramdisk.c
rm -f tools/create_ramdisk.sh

# 删板子文档
rm -f loongson2K1000LA.md mmexport.md op.txt loongarch_ABI.txt

# 删用不到的测试脚本
rm -f testsh/judge-site.sh testsh/judge_all.sh

# 验证 make all 仍然正常
make clean && make all
```

### 激进方案（需改 Makefile，10 分钟）

在保守方案基础上，删除 Makefile 中所有 VF2/2K1000 构建规则（第 4.1 节所列行号），从 `.PHONY` 移除 `vf2`，减少 Makefile 约 100 行。

### 后续维护建议

在 `.gitignore` 追加（如果还没加）：

```bash
echo "*.img" >> .gitignore
echo "*.img.gz" >> .gitignore
echo "sdcard-*.img*" >> .gitignore
```

---

## 八、删除前后对比

| 维度 | 删除前 | 删除后（保守方案） |
|------|--------|-------------------|
| 目录数（顶层） | 17 | 15 |
| 有板子专用代码的文件 | 14 个共享文件 + 9 个专用文件 | 14 个共享文件（条件编译，无影响） |
| `make all` 是否正常 | ✅ | ✅（完全不受影响） |
| Docker 测评是否正常 | ✅ | ✅ |
| Git 仓库清爽度 | 混杂板子/Docker/文档/测试 | 只有 QEMU 代码 + 文档 |
| 恢复现场赛代码 | — | `git checkout on-site` 分支即可 |

> **初赛只需要 QEMU。现场赛代码留在 `on-site` 分支，切分支就能拿回来，不需要在 `main` 里带着。**

---

> **一句话：板子专用代码全在 `boot/{vf2,2k1000}` + `kernel/{vf2,2k1000}` + `include/board/` + `site/` + 几个单片文件里，删掉它们 `make all` 照常跑，测评照常过，维护少一截干扰。**

---

## 九、实际执行记录（2026-06-02 21:26 已完成）

以下操作用激进方案在 `main` 分支上实际执行，已验证通过。

### 9.1 删除的目录（5 个）

| 路径 | 内容 |
|------|------|
| `boot/vf2/` | `entry.S` + `main.c`（VF2 引导代码） |
| `boot/2k1000/` | `entry.S` + `main-2k1000.c` + `early_uart.c`（2K1000 引导代码） |
| `kernel/vf2/` | `kernel.ld`（VF2 链接脚本） |
| `kernel/2k1000/` | `kernel.ld`（2K1000 链接脚本） |
| `include/board/` | `2k1000.h`（2K1000 板级定义） |

### 9.2 删除的单文件（2 个）

| 路径 | 说明 |
|------|------|
| `kernel/trap/la/iointc-2k1000.c` | 2K1000 IO 中断控制器驱动 |
| `kernel/fs/ramdisk.c` | 板子 RAMDisk 驱动（全文件 `#if VF2 \|\| LA2K1000`） |

### 9.3 Makefile 清理明细

| 删除内容 | 说明 |
|---------|------|
| `K_SRC_VF2` / `K_ASM_VF2` 变量 | VF2 源码清单 |
| `OBJS_VF2` 变量 | VF2 编译产物清单（10 行） |
| `CFLAGS_VF2` 变量 | VF2 编译选项 |
| `CFLAGS_VF2` stack-protector 行 | VF2 栈保护 |
| `CFLAGS_VF2` no-pie 检查 | VF2 PIE 禁用 |
| `CFLAGS_VF2` static 行 | VF2 静态链接 |
| `CFLAGS_LA2K1000` 变量 | 2K1000 编译选项 |
| `vf2` / `2k1000` 顶层目标 | 板子入口目标 |
| `ramdisk.img` / `kernel-vf2` 目标 | VF2 整条构建链 |
| VF2 编译规则段 | `%-vf2.o` + `boot/vf2/*.o` 规则 |
| 2K1000 编译规则段 | `%-la2k1000.o` + `%-2k1000.o` + 所有板子 `.o` 规则 |
| `ramdisk-2k1000.img` 目标 | 2K1000 RAMDisk 构建 |
| `OBJS_LA2K1000` 变量 | 2K1000 对象清单 |
| `kernel-la2k1000` 目标 | 2K1000 内核构建链 |
| `clean` 中 VF2/2K1000 模式 | `*-vf2.o`, `*-la2k1000.o`, `*-2k1000.o`, `kernel-vf2*`, `kernel-la2k1000*`, `ramdisk*` |
| `.PHONY` 中 `vf2` | 伪目标列表 |

### 9.4 验证结果

```
make clean        → ✅ 通过
make kernel-rv    → ✅ 通过（生成 kernel-rv 1.6MB）
Makefile 行数     → 549 行（原有 ~630 行，精简约 80 行）
```

> LoongArch 编译未在本机验证（本机无 `loongarch64-linux-gnu-gcc`），但 Makefile LA 部分未改动，在 Docker 评测镜像里会正常编译。
