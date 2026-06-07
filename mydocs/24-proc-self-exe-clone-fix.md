# 🔧 /proc/self/exe 完整实现实录：从 fork 到 clone 的 exec_path 追踪

> 📅 生成时间：`2026-06-07 15:00 (GMT+8)`
>
> 目标：完整记录 `/proc/self/exe` 内核实现过程，包括 fork/clone 双重 exec_path 拷贝、namei VFS 拦截、ENOEXEC fallback 链路打通，以及 busybox ash tryexec() 源码分析。

---

## 零、背景

busybox ash 的 ENOEXEC fallback（参见 `22-execve-enoexec-fix.md`）触发后，ash 需要重新 exec 自己来作为脚本解释器：

```c
// busybox shell/ash.c tryexec()
if (cmd != bb_busybox_exec_path && errno == ENOEXEC) {
    argv[0] = (char*) cmd;                  // "./run-dynamic.sh"
    cmd = bb_busybox_exec_path;             // "/proc/self/exe"
    argv--;
    argv[0] = (char*) "ash";
    goto repeat;                            // execve("/proc/self/exe", ...)
}
```

注释原文：*"do not use $SHELL, user's shell, or /bin/sh; just call ourselves."*

SpringOS 没有实现 `/proc/self/exe`，导致第二次 exec 失败 → "not found"。本文记录完整的内核实现过程。

---

## 一、实现方案

### 1.1 进程结构体新增字段

**`include/proc/proc.h`：**

```c
char exec_path[MAXPATH];     // 记录进程当前可执行文件的路径
```

位于 `cwd` 之后、`name` 之前。

### 1.2 execve 时记录路径

**`kernel/proc/exec.c`（RISCV 版本）：**

```c
// 页表切换成功后、释放锁之前
safestrcpy(p->exec_path, path, MAXPATH);
```

`path` 是 execve 的第一个参数（用户传入的文件路径，如 `"busybox"` 或 `"/init"`）。

### 1.3 allocproc 初始化

**`kernel/proc/proc.c:allocproc()`：**

```c
p->exec_path[0] = '\0';
```

### 1.4 fork 拷贝（★ 第一版）

**`kernel/proc/proc.c:fork()`：**

```c
strcpy(np->exec_path, p->exec_path);
```

### 1.5 namei VFS 拦截

**`kernel/fs/vfs/ops.c:namei()`：**

```c
if (strncmp(path, "/proc/self/exe", 15) == 0
    && (path[15] == '\0' || path[15] == '/')) {
    struct proc *p = myproc();
    if (p->exec_path[0] != '\0') {
        get_absolute_path(p->exec_path, p->cwd.path, name);
        return vfs_ext_namei(name);
    }
    return 0;  // 或 fallback
}
```

关键点：`exec_path` 存的是相对路径（如 `"busybox"`），需要 `get_absolute_path()` 结合进程 cwd 转成绝对路径再查 ext4。

---

## 二、调试过程：exec_path 为空的根因

### 2.1 症状

实现完上述 5 处后，日志显示：

```
execve: pid=2 name=busybox exec_path='busybox'     ← 正常保存
execve: pid=3 name=busybox exec_path='./busybox'    ← 正常保存
namei: /proc/self/exe hit, pid=4 exec_path=''        ← fork 子进程为空！
```

pid=3 的 exec_path 是 `'./busybox'`，pid=4 是 pid=3 的 fork 子进程。如果 fork 拷贝了 exec_path，pid=4 应该继承 `'./busybox'`，但它是空的。

### 2.2 排除法定位：三步推理

**第 1 步：确认 execve 保存没问题。**
日志中 `execve: pid=3 name=busybox exec_path='./busybox'` 说明保存代码正常执行。

**第 2 步：确认是不是 fork 的问题。**
在 `fork()` 函数里加 debug：

```c
printf("fork: parent pid=%d exec_path='%s' → child pid=%d exec_path='%s'\n",
       p->pid, p->exec_path, np->pid, np->exec_path);
```

重新跑，日志里这行只出现了一次：

```
fork: parent pid=1 exec_path='' → child pid=2 exec_path=''
```

pid=1 是 init 进程（exec_path 为空很正常——它是内核直接启动的 initcode，不是通过 execve 加载的），它 fork 出 pid=2 然后 pid=2 exec busybox。

**但 pid=3 → pid=4 的 fork 没出现！** 说明 busybox 内部创建子进程时根本没走 `fork()` 函数。

**第 3 步：查 clone。**
SpringOS 里创建进程只有两个入口：`fork()` 和 `clone()`。既然不是 `fork()`，那就是 `clone()`。打开 `clone()` 一看：

```c
// clone() 拷贝了哪些？
np->cwd.fs = p->cwd.fs;             // ✅ cwd
strcpy(np->cwd.path, p->cwd.path);  // ✅ cwd 路径
strcpy(np->name, p->name);          // ✅ 进程名
// np->exec_path                     // ❌ 没有！
```

`clone()` 拷贝了 `cwd`、`name`、文件描述符、VMA……唯独漏了 `exec_path`。

**为什么 busybox 走 clone 而不是 fork？** 因为 busybox 链接的 C 标准库（musl 或 glibc）的 `fork()` 函数底层实现就是 `clone(SIGCHLD, 0)`：

```c
// musl 的 fork.c（简化）
pid_t fork(void) {
    return __syscall(SYS_clone, SIGCHLD, 0);
}
```

用户态写的是 `fork()`，进入内核走的是 `SYS_clone` → `sys_clone()` → `clone()`。所以只修 `fork()` 不够，`clone()` 也要修。
namei: /proc/self/exe hit, pid=4 exec_path=''       ← fork 子进程为空！
```

### 2.2 fork 加 debug 打印

```c
printf("fork: parent pid=%d exec_path='%s' → child pid=%d exec_path='%s'\n",
       p->pid, p->exec_path, np->pid, np->exec_path);
```

输出：

```
fork: parent pid=1 exec_path='' → child pid=2 exec_path=''
```

父进程 pid=1（init）的 exec_path 为空，子进程自然也空。但这不影响——子进程 exec busybox 后会覆盖。

**更关键的：** busybox（pid=3）fork 的子进程 pid=4 的 exec_path 也是空的，而此时 pid=3 的 exec_path 是 `'./busybox'`。

### 2.3 根因：clone() 没有拷贝 exec_path

musl/glibc 的 `fork()` 实际上调用的是 `clone(SIGCHLD, 0)`，走的是 `sys_clone` → `clone()` 路径。SpringOS 的 `clone()` 函数拷贝了 `cwd`、`name`、文件描述符等，**但没有拷贝 `exec_path`**。

对比 fork 和 clone：

```c
// fork() — 有 exec_path 拷贝
strcpy(np->exec_path, p->exec_path);    // ✅ 已加

// clone() — 缺失！
strcpy(np->name, p->name);              // ✅ 有
// exec_path 拷贝                        // ❌ 没有！
```

### 2.4 修复：clone 加 exec_path 拷贝

**`kernel/proc/proc.c:clone()`：**

```c
strcpy(np->name, p->name);
strcpy(np->exec_path, p->exec_path);    // ★ 新增
```

---

## 三、验证结果

修复后日志：

```
fork: parent pid=1 exec_path='' → child pid=2 exec_path=''
execve: pid=2 name=busybox exec_path='busybox'
execve: pid=3 name=busybox exec_path='./busybox'
namei: /proc/self/exe hit, pid=4 exec_path='busybox' cwd='/musl'
namei: /proc/self/exe → absolute='/musl/busybox'     ✅
execve: pid=4 name=exe exec_path='/proc/self/exe'
execve: pid=5 name=lua exec_path='./lua'
testcase lua date.lua success
```

完整链路：

```
busybox ash 遇到 ./test.sh
  → fork (clone) → child 继承 exec_path='busybox'  ✅
  → child: execve("./test.sh") → ENOEXEC
  → child: execve("/proc/self/exe", {"ash", "./test.sh"})
  → namei("/proc/self/exe") → exec_path='busybox'
  → get_absolute_path("busybox", "/musl") → "/musl/busybox"
  → vfs_ext_namei("/musl/busybox") → busybox inode ✅
  → busybox 以 ash 模式启动 → 解释执行 test.sh ✅
```

---

## 四、改动文件总览（6 个文件）

| 文件 | 改动 | 行数 |
|------|------|:--:|
| `include/proc/proc.h` | `char exec_path[MAXPATH]` 字段 | 1 |
| `kernel/proc/proc.c:allocproc` | `p->exec_path[0] = '\0'` | 1 |
| `kernel/proc/proc.c:fork` | `strcpy(np->exec_path, p->exec_path)` + debug printf | 3 |
| `kernel/proc/proc.c:clone` | `strcpy(np->exec_path, p->exec_path)` | 1 |
| `kernel/proc/exec.c` | `safestrcpy(p->exec_path, path, MAXPATH)` + debug printf | 2 |
| `kernel/fs/vfs/ops.c:namei` | `/proc/self/exe` 拦截 + 绝对路径解析 + fallback | ~15 |

**关键是发现 `clone()` 也需要拷贝——fork 和 clone 是 SpringOS 里创建进程的两个入口，缺一不可。**

---

## 五、教训

- **fork ≠ 唯一进程创建路径**：musl/glibc 的 `fork()` 通过 `clone()` 实现。新增进程状态字段时，两个函数都要处理。
- **`/proc/self/exe` 是 Linux 标准机制**：不用 `/bin/sh`，不用猜 busybox 在哪——内核自己记录 exec 路径，自然正确。
- **相对路径问题**：exec_path 存的是用户传入的原始路径（如 `"busybox"`），VFS 拦截时必须结合进程 cwd 做 `get_absolute_path()` 转换。
