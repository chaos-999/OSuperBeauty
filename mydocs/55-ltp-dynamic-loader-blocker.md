# LTP 运行可行性排查：阻塞点是动态 ELF loader

> 目标：让官方镜像里的 LTP `syscalls` 套件真正跑起来并计分。
> 结论：**运行框架已经打通，但镜像里的 LTP 二进制全部是动态链接的，当前内核的
> 动态 ELF loader 兼容性不足**，这不是补几个 syscall 能解决的。libcbench 不受
> 影响（仍为 115.5–116.1）。

---

## 1. 镜像里的 LTP 是什么形态

- `/glibc/ltp/testcases/bin`：约 1262 个用例
- `/musl/ltp/testcases/bin`：约 1132 个用例
- `runtest/syscalls` 清单 1799 行；参考机实际 1407 个用例（1140 pass）
- 二进制形态：
  - glibc: PIE + 动态链接，解释器 `/lib/ld-linux-riscv64-lp64d.so.1`
  - musl : PIE + 动态链接，解释器 `/lib/ld-musl-riscv64.so.1`
- 镜像**没有 `/lib`**；实际 loader/libc 在 `/glibc/lib`、`/musl/lib`

## 2. 已实现并保留在树里的内核改进

这些改动本身是正确的，且 libcbench 回归通过（RV 27/27 + 27/27）：

- `sys_mmap` 支持 `MAP_FIXED`：替换/裁剪重叠 VMA，按固定地址建映射
  （动态 loader 映射 ELF 段必需）
- `sys_mprotect` 对**部分重叠**的 VMA 也更新 `prot`（原来要求整段包含）
- RV `vmatrylazytouch`：页已存在但权限不符时，就地升级 PTE 权限而不是
  `mappages` panic
- `INIT_PAGES=32` + `INIT_STACK_TOP`：init 二进制变大（>16KB）时不再与初始栈重叠
- 11 个兼容 syscall（当前 init 未调用，供后续 LTP 使用）：
  `prctl/getrusage/sched_setaffinity/sched_getaffinity/personality/
   mlock/munlock/mlockall/munlockall/membarrier/rseq/faccessat2/
   preadv/pwritev`

## 3. 实际测试结果

### 3.1 glibc LTP：两种加载方式都不行

1. **走 PT_INTERP（内核直接把控制权交给 ld.so）**：
   ```
   Inconsistency detected by ld.so: rtld.c: 1310:
   rtld_setup_main_map: Assertion `GL(dl_rtld_map).l_libname' failed!
   ```
2. **loader 当主程序（`ld.so --library-path /glibc/lib <test>`）**：
   在补上 `MAP_FIXED` 后已经能完成段映射，但 ld.so 在
   `_dl_map_object_deps` / phdr 扫描里进入无限循环（`epc=0x6662` 反复出现），
   `--help` 打印完后也卡在 exit 路径。说明 link_map/auxv 语义仍不完整。

### 3.2 musl LTP

- **LA 上可以真正执行**（日志里能看到 `TPASS`），因为内核的
  “解释器找不到时回落到 `/musl/lib/libc.so`” 路径对它有效。
- **RV 上不行**：
  ```
  Error relocating access01: unsupported relocation type 64
  ```
  镜像里 RV 的 musl loader/libc 版本比二进制新（TLSDESC relocation），
  与 LA 的情况不同。

### 3.3 判分格式（顺带确认）

- `judge_ltp-musl.py` 统计每个用例 `Summary:` 里的 `passed N`
- `judge_ltp-glibc.py` 统计彩色 `TPASS` 行
- 官方 `run.py` 走 `parse_serial_out_new` → `judge_ltp-*`，因此 init 必须输出
  `RUN LTP CASE <name>` / `FAIL LTP CASE <name> : <rc>` 和组名 `ltp-glibc|ltp-musl`

## 4. 继续做 LTP 需要什么

要让 glibc LTP 跑起来，需要补齐 **Linux 兼容的动态 ELF 加载语义**：

1. `execve` 动态程序：正确的 `AT_PHDR/AT_ENTRY/AT_BASE/AT_EXECFN`、解释器映射
   与 `ld.so` 自身 `l_libname` 的识别；
2. 完整的 `mmap/mprotect` 语义：`MAP_FIXED`（已做）、RELRO、文件段权限升级、
   `MAP_DENYWRITE` 等；
3. TLS / `rseq` / `set_robust_list` 等 glibc 启动依赖；
4. 逐项回归（每个改动都需跑一次 LTP 子集）。

工作量评估：不是“加几个 syscall”，而是一个独立的 loader 兼容性项目，
建议单独排期；在此之前，libcbench 的 115.5–116.1 分保持为可交付结果。

## 5. 当前状态

- `user/init-{rv,la}.c` 已恢复为**只跑 libcbench** 的已验证版本；
- 内核保留上述通用改进，RV 回归：`libcbench-glibc 27/27 = 30.904`、
  `libcbench-musl 27/27 = 27.274`，`done=0`，无 panic/assertion；
- 官方 harness 历史结果：`Accpted`，score 115.5–116.1。
