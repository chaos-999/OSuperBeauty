# 测评调试总结 — 2026-06-25 至 2026-06-28 问题与修复

> 本文档汇总了评分过程中发现的所有问题、根因定位和最终解决方案。

---

## 1. RV — busybox argv 损坏

**现象**：busybox-musl/glic 的 `execve("busybox", bb_testcode, NULL)` 返回后，busybox 运行但 `argv[0]` 是乱码 `�+`。

**诊断过程**：
- 在 `exec.c` 的 `execve()` 页表切换后添加诊断输出 `exec_diag`
- 发现内核侧 `kern_str` 已经是烂的 → 问题在 `sys_execve` 的 `fetchaddr`/`fetchstr` 阶段
- `bb_testcode` 是全局变量，64 次 fork/exec 循环后内存被踩

**根因**：`bb_testcode` 全局数组在 initcode 数据段中，与 `basic_name[32]` 紧邻。大量 fork/exec 过程中数据段被踩踏。

**修复**（`user/init-rv.c`）：
```c
// 旧：全局变量
// char *bb_testcode[10] = {"busybox", "sh", "busybox_testcode.sh", NULL};

// 新：test_pre() 内的局部变量
void test_pre() {
    char *bb_testcode_local[] = {"busybox", "sh", "busybox_testcode.sh", NULL};
    ...
    execve("busybox", bb_testcode_local, NULL);
}
```

---

## 2. RV — ext4 块缓存 panic

**现象**：`busybox-glibc` 测试中 `hwclock` 通过后，`sleep 5 & kill $!` 触发 kernel panic。

**两次不同 panic**：
- `panic: mappages: remap` — `libctest-musl` 末尾，页表重复映射
- `scause=0xd sepc=0x802126c6 stval=0x101010101010159` — `busybox-glibc` 中，ext4 块缓存 LRU 树 use-after-free

**诊断过程**：
- `addr2line` 解析 backtrace：`s>readlinkat → vfs_ext_readlink → ext4_dir_find_entry → ext4_bcache_alloc → ext4_buf_lru_RB_REMOVE`
- `stval=0x101010101010101` 是 `kfree` 的 0x01 毒化模式
- `struct ext4_buf` offset 88 = `lru_node.rbe_left` LRU 树指针被毒化

**根因**：`ext4_journal.c:1642` 的 `ext4_bcache_inc_ref(block->buf)` 直接增加引用计数，绕过了 `ext4_bcache_find_get`（该函数会先从 LRU 树摘除再加引用）。若 buffer 在 LRU 树中（refctr=0），refctr 变 1 但节点仍在 LRU 树里。后续 GC 释放时跳过 LRU 摘除，留下悬挂指针。

**状态**：已定位根因，未提交修复。参数敏感的非确定性 bug（本地 256M 内存、无网络环境不触发）。

---

## 3. LA — basic test 全部崩溃 (badi=29c06061)

**现象**：所有 basic test 子进程 fork/exec 后访问同一地址 `badi=29c06061`，触发 `vma lazy touch failed` → `usertrap unexpected trapcause 10004`。

**诊断过程**：
- 反复试验发现：修改 `init-la.c`（添加 printf、取消注释、加 for 循环）→ basic test 崩溃；恢复原始版本 → 正常
- 对比发现 initcode 二进制大小是决定性因素：
  - 正常时 initcode ≈ 12KB 以内
  - 加 libctest 代码后 initcode = 13040 字节，超出 12KB 临界值

**根因**：
1. `kernel/mm/vm.c` 的 `uvmfirst()` 硬编码只分配 **3 页（12KB）**，超过 12KB 的 initcode 数据被直接丢弃
2. `kernel/proc/proc.c` 的 `userinit()` 中 `p->sz = 3 * PGSIZE` 硬编码 12KB，fork 的 `uvmcopy` 只复制 12KB，子进程缺失超出部分的数据

**修复**（3 处）：

`kernel/mm/vm.c` — `uvmfirst` 增加第 4 页支持：
```c
// RV 和 LA 各增加一页
mem = kalloc();
memset(mem, 0, PGSIZE);
mappages(pagetable, 3 * PGSIZE, PGSIZE, (uint64)mem, ...);
if (sz > 3 * PGSIZE) {
    memmove(mem, src + 3 * PGSIZE, MIN(sz - 3 * PGSIZE, PGSIZE));
}
```

`kernel/proc/proc.c` — `userinit` 动态计算进程大小：
```c
// 旧：p->sz = 3 * PGSIZE;
// 新：
p->sz = PGROUNDUP((uint64)(initcode_end - initcode_start));
p->trapframe->sp = PGROUNDUP((uint64)(initcode_end - initcode_start));
```

---

## 4. LA — init-la.c 代码修复

**问题**：
- `libc_tests[]` 定义在 musl 块内部，glibc 块引用不到（作用域错误）
- musl 的 for 循环用 `i<96` hardcode

**修复**：
```c
// libc_tests[] 提到两个 section 前面
char *libc_tests[] = { ... NULL };

// musl
chdir(bb_path_musl);
printf("#### OS COMP TEST GROUP START libctest-musl ####\n");
for (int i = 0; libc_tests[i] != NULL; i++) { ... }

// glibc
chdir(bb_path_glibc);
printf("#### OS COMP TEST GROUP START libctest-glibc ####\n");
for (int i = 0; libc_tests[i] != NULL; i++) { ... }
```

同时删除了完全未使用的 `bb_cmds[][]` 和 `bb_test_success[]` 数组（约 800+ 字节 .rodata）。

---

## 5. LA — busybox-musl 静默退出

**现象**：busybox-musl exec 成功，但 `busybox_testcode.sh` 无任何输出，子进程 exit(0)。

**诊断**：通过 syscall 追踪发现 busybox sh 的 clone 子进程执行路径为 `set_tid_address → signal → exit(0)`，从未调用 execve。shell 内部 `find_command` 判定 `./busybox` 不可用。

**结论**：sdcard 中 musl busybox LA 二进制本身的工具链 bug（`pcalau12i` 相关），非内核问题。glibc busybox 正常。

---

## 修改文件清单

| 文件 | 修改内容 |
|------|----------|
| `kernel/mm/vm.c` | `uvmfirst` 增加第 4 页（RV + LA） |
| `kernel/proc/proc.c` | `userinit` 的 `p->sz` 和 `trapframe->sp` 改为动态计算 |
| `user/init-rv.c` | `bb_testcode` 全局 → 局部变量；banner 改名 |
| `user/init-la.c` | `libc_tests[]` 提前；删除 `bb_cmds`、`bb_test_success` |
| `kernel/proc/exec.c` | 已删诊断代码（仅调试用） |
| `kernel/trap/la/trap.c` | 已删诊断代码（仅调试用） |
