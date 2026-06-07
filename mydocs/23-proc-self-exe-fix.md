# 🔧 libctest 脚本执行链修复实录：从 EPERM 到 /proc/self/exe

> 📅 生成时间：`2026-06-07 14:24 (GMT+8)`
>
> 目标：完整记录 libctest 测试中 `./run-dynamic.sh` 从"Operation not permitted"到正常执行的诊断与修复全过程，包括 ENXIO 返回值修复、busybox ash 源码分析、`/proc/self/exe` 内核实现。

---

## 一、问题链

libctest 测试加入 `test_pre()` 后，串口输出经历了两层错误：

```
阶段一：libctest_testcode.sh: line 4: ./run-dynamic.sh: Operation not permitted
阶段二：libctest_testcode.sh: line 4: ./run-dynamic.sh: not found
```

每个阶段对应一个独立的根因，需要逐层修复。

---

## 二、第一层：EPERM → ENOEXEC（exec 返回值修复）

### 2.1 调用链

```
init → busybox sh libctest_testcode.sh
  → 脚本内 ./run-dynamic.sh
    → fork + execve("./run-dynamic.sh")
      → 内核读文件 → ELF magic 不匹配 → return -1
        → musl __syscall_ret(-1) → errno = EPERM
          → busybox ash: EPERM ≠ ENOEXEC → 不 fallback → 报错
```

### 2.2 根因

SpringOS（xv6 改）的 `execve` 对所有失败返回 `-1`。musl 的 `__syscall_ret()` 把 `-1` 解析为 `-EPERM`（errno=1）。busybox ash 只在收到 `ENOEXEC` 时才 fallback 把脚本当 shell 文件解释执行——其他 errno 一律报错。

### 2.3 修复：`kernel/proc/exec.c`（4 处）

```c
// ① 引入 errno 头
#include "errno.h"

// ② 默认 errno 变量
int i, off, ret = -1;   // 默认 -1 兜底其他错误路径

// ③ 非 ELF 文件返回正确 errno
if (elf.magic != ELF_MAGIC) {
    ret = -ENOEXEC;      // → musl 解析为 errno=8
    goto bad;
}

// ④ 文件不存在返回正确 errno
if ((ip = namei(path)) == 0) {
    return -ENOENT;       // → musl 解析为 errno=2
}

// ⑤ bad 标签使用 ret 变量
bad:
    return ret;           // 不再硬编码 -1
```

**效果：** execve 非 ELF 返回 `-ENOEXEC` → musl `errno=8` → busybox ash 触发 ENOEXEC fallback。

---

## 三、第二层：ENOEXEC fallback 失败（/proc/self/exe 缺失）

### 3.1 错误现象

ENOEXEC 修复后，busybox ash 不再报 "Operation not permitted"，改为报 "not found"。

### 3.2 源码分析：busybox ash 的 `tryexec()`

阅读 `/home/chaos/testsuits-for-oskernel/busybox/shell/ash.c` 第 8102-8157 行：

```c
static void
tryexec(IF_FEATURE_SH_STANDALONE(int applet_no,) const char *cmd,
        char **argv, char **envp)
{
    // ... FEATURE_SH_STANDALONE 路径（OSCOMP 编译中已禁用）...

 repeat:
    execve(cmd, argv, envp);                    // ① 正常 exec

    if (cmd != bb_busybox_exec_path && errno == ENOEXEC) {
        // ② 收到 ENOEXEC → 不用 /bin/sh，不用 $SHELL，重新 exec 自己
        argv[0] = (char*) cmd;                  // argv[0] = "./run-dynamic.sh"
        cmd = bb_busybox_exec_path;             // cmd = 自己的路径
        argv--;
        argv[0] = (char*) "ash";                // 以 ash 模式启动
        goto repeat;                            // ③ 重新 exec
    }
}
```

关键的注释原文：

> *"do not use $SHELL, user's shell, or /bin/sh; just call ourselves."*

**busybox ash 的 ENOEXEC fallback 不是调 `/bin/sh`，是重新 exec 自己。** 而 `bb_busybox_exec_path` 在编译时写死为：

```c
// libbb/messages.c:30
const char bb_busybox_exec_path[] = CONFIG_BUSYBOX_EXEC_PATH;
// CONFIG_BUSYBOX_EXEC_PATH = "/proc/self/exe"  ← 所有 config 的默认值
```

### 3.3 根因

SpringOS 没有实现 `/proc/self/exe`：

```
ash ENOEXEC fallback:
  → execve("/proc/self/exe", {"ash", "./run-dynamic.sh"})
  → namei("/proc/self/exe") → 文件不存在 → ENOENT
  → shellexec 错误处理 → "not found"
```

### 3.4 修复：内核实现 `/proc/self/exe`（5 个文件）

#### ① `include/proc/proc.h` — 进程结构体加字段

```c
char exec_path[MAXPATH];     // 记录进程当前执行的二进制路径
```

#### ② `kernel/proc/proc.c:allocproc()` — 初始化

```c
p->exec_path[0] = '\0';
```

#### ③ `kernel/proc/proc.c:fork()` — 子进程继承

```c
strcpy(np->exec_path, p->exec_path);
```

#### ④ `kernel/proc/exec.c` — exec 时记录

```c
safestrcpy(p->exec_path, path, MAXPATH);  // 页表切换后、释放锁之前
```

#### ⑤ `kernel/fs/vfs/ops.c:namei()` — VFS 拦截

```c
struct inode *namei(char *path) {
    char name[EXT4_PATH_LONG_MAX];

    // /proc/self/exe → 返回当前进程可执行文件的 inode
    if (strncmp(path, "/proc/self/exe", 15) == 0
        && (path[15] == '\0' || path[15] == '/')) {
        struct proc *p = myproc();
        if (p->exec_path[0] != '\0')
            return vfs_ext_namei(p->exec_path);
        return 0;
    }

    get_absolute_path(path, myproc()->cwd.path, name);
    return vfs_ext_namei(name);
}
```

---

## 四、修复后的完整调用链

```
libctest_testcode.sh → busybox ash 解析执行
  │
  ├─ 遇到 ./run-dynamic.sh
  │    fork()
  │    ├─ execve("./run-dynamic.sh")
  │    │    → 内核读文件 → ELF magic 不匹配
  │    │    → ret = -ENOEXEC → musl errno=8
  │    │
  │    ├─ ash: errno==ENOEXEC → 触发 fallback
  │    │    cmd = "/proc/self/exe"
  │    │    argv = {"ash", "./run-dynamic.sh"}
  │    │
  │    └─ execve("/proc/self/exe", {"ash", "./run-dynamic.sh"})
  │         → namei("/proc/self/exe")
  │         → 拦截 → myproc()->exec_path = "/glibc/busybox"
  │         → vfs_ext_namei("/glibc/busybox") → busybox inode ✅
  │         → 加载 busybox ELF，以 ash 模式启动
  │         → 解释执行 ./run-dynamic.sh ✅
  │
  └─ run-dynamic.sh 内再调 ./run-static.sh → 同样链路 → 逐层打通 ✅
```

---

## 五、改动文件总览

| 文件 | 改动内容 | 行数 |
|------|---------|:--:|
| `kernel/proc/exec.c` | `#include "errno.h"` + ret 变量 + `-ENOEXEC` + `-ENOENT` + `return ret` + exec_path 记录 | ~8 |
| `include/proc/proc.h` | `char exec_path[MAXPATH]` 字段 | 1 |
| `kernel/proc/proc.c` | allocproc 初始化 + fork 继承 | 2 |
| `kernel/fs/vfs/ops.c` | namei `/proc/self/exe` 拦截 | ~8 |
| `user/init-rv.c` | lua + libctest 测试代码（testcode 数组 + test_pre） | ~70 |
| `user/init-la.c` | lua + libctest 测试代码（glibc only） | ~40 |

**完全没有：** 拷贝文件、创建 symlink、创建 `/bin/sh`、修改 busybox 源码。

---

## 六、知识沉淀

### 6.1 xv6 内核 → musl errno 约定

```
Linux 内核 syscall 返回值约定：
  成功     → ≥ 0（或 void）
  失败     → -ERRNO（如 -ENOEXEC=-8, -ENOENT=-2）

musl __syscall_ret():
  r ∈ [0, ∞)        → 成功，直接返回 r
  r ∈ [-4095, -1]   → 失败，errno = -r，返回 -1
```

SpringOS 从 xv6 继承的 `return -1` 习惯在 musl/glibc 环境下必须改为 `return -ERRNO`。

### 6.2 /proc/self/exe 的语义

Linux 的 `/proc/self/exe` 不是磁盘文件，是内核动态生成的虚拟路径：

- 任何进程访问它 → 返回"当前进程正在执行的 ELF 文件的 inode"
- `execve("/proc/self/exe")` ≈ 重新执行自己
- 这是 busybox、bash 等 shell 实现脚本 fallback 的标准机制
