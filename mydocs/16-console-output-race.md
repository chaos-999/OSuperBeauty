# 🔍 控制台输出竞争：从根因诊断到行缓冲修复

> 📅 生成时间：`2026-06-05 07:59 (GMT+8)`
>
> 目标：完整记录 yield 测试控制台输出字符交错的诊断过程、根因分析、方案评估和实施

---

## 一、现象

三个子进程并发 `printf()`，UART 输出行被截断拼接：

```
# 期望：
 I am child process: 38. iteration 0.
 I am child process: 39. iteration 1.
 I am child process: 40. iteration 2.

# 实际：
 I am child process: 40. iteration 2 I am child process: 38...
```

`judge_basic.py` 匹配不到恰好 15 行 → **yield 测试 0/4 分**。

---

## 二、根因分析

### 三层缺陷

**层 1：`consolewrite()` 无锁（`kernel/fs/console.c:66`）**

```c
int consolewrite(...) {
    for (i = 0; i < n; i++) {
        either_copyin(&c, ...);
        PUTCHAR(c);  // → uartputc(c)，无外层锁保护
    }
}
```

**层 2：`uartputc()` 锁粒度为单字符（`kernel/drive/uart.c:118`）**

```c
void uartputc(int c) {
    acquire(&uart_tx_lock);  // 只保护单个字符放入缓冲区
    uart_tx_buf[...] = c;
    uartstart();
    release(&uart_tx_lock);  // 每字符释放
}
```

三个子进程的字符在 UART 缓冲区自由交错写入。

**层 3：用户态 `printf()` 拆成 5 次 `write()`**

测试集 `testsuits-for-oskernel/basic/user/lib/stdio.c` 的实现：

```c
void printf(const char *fmt, ...) {
    for (;;) {
        out(f, " I am child process: ", 23);  // write() ①
        printint(pid);                         // write() ②
        out(f, ". iteration ", 11);            // write() ③
        printint(i);                           // write() ④
        out(f, ".\n", 2);                      // write() ⑤
    }
}
```

一次逻辑上的 `printf()` 产生 **5 次独立的系统调用**。即使内核在 `consolewrite()` 加锁，5 次 `write()` 之间锁释放，timer 抢断照样使三个进程的 15 次 `write()` 交错。

**层 4：timer interrupt 在用户态抢断**

```c
// kernel/trap/rv/trap.c — kerneltrap
if (which_dev == 2 && myproc() && myproc()->state == RUNNING)
    yield();  // 每 ~10ms 强制让出 CPU
```

抢断发生在**用户态两次 `write()` 之间**——此时 `cons.write_lock` 已释放，锁管不到。

### LoongArch 为何没暴露

| 架构 | `PUTCHAR` 实现 | 每字符开销 | 一行 45 字符总时间 | 被 timer 打断概率 |
|------|--------------|-----------|------------------|:---:|
| RISC-V | `SBI_PUTCHAR(c)` → ecall → M 态 | ~5μs | ~225μs | 高 |
| LoongArch | `uartputc_sync(c)` → 直接 MMIO | ~0.5μs | ~22.5μs | 低 (10×) |

纯属调度运气，不是 LA 实现更正确。

### 完整调用链

```
yield.c: printf("%d...", pid, i)
  → stdio.c: out()×3 + printint()×2 → 5 次 write()
    → ecall → sys_write → filewrite → consolewrite
      → (cons.write_lock) → for each char: PUTCHAR(c)
        → SBI_PUTCHAR(c) → ecall → OpenSBI → UART THR
```

---

## 三、方案评估

| 方案 | 改动 | 可靠性 | 副作用 | 结论 |
|------|------|:---:|------|:---:|
| A: 内核行缓冲 | 4 文件 ~30 行 | ✅ 100% | 无 | **选择** |
| B: 增大 timer 间隔 | 1 行 | ⚠️ ~80% | 全局调度变慢 | 不推荐 |
| C: 延迟 kerneltrap yield | ~10 行 | ❌（抢断在用户态） | 破坏调度公平性 | 不适用 |
| D: 修改测试集 | 不可改 | — | — | 不可行 |

---

## 四、行缓冲方案设计与实现

### 设计思路

每个进程一个私有 `console_buf[256]`，`write()` 时不直接输出，先往 buf 里攒，遇到 `\n`（或 buf 满）才在 `cons.write_lock` 下整行 flush。timer 随便抢——buf 是进程私有的，多进程各自攒各自的。

### 代码变更

**① `include/proc/proc.h` — struct proc 加两个字段：**

```c
char console_buf[256];    // 进程私有行缓冲
int  console_buf_pos;     // 当前缓冲位置（allocproc memset 归零）
```

**② `kernel/fs/console.c` — 改写 `consolewrite()` + 新增 `console_flush()`：**

```c
void console_flush(struct proc *p) {
    if (p->console_buf_pos == 0) return;
    acquire(&cons.write_lock);
    for (int j = 0; j < p->console_buf_pos; j++)
        PUTCHAR(p->console_buf[j]);
    release(&cons.write_lock);
    p->console_buf_pos = 0;
}

int consolewrite(int user_src, uint64 src, int n) {
    struct proc *p = myproc();
    for (int i = 0; i < n; i++) {
        char c;
        if (either_copyin(&c, user_src, src + i, 1) == -1) break;
        if (p->console_buf_pos < (int)sizeof(p->console_buf) - 1)
            p->console_buf[p->console_buf_pos++] = c;
        if (c == '\n' || p->console_buf_pos >= (int)sizeof(p->console_buf) - 1)
            console_flush(p);
    }
    return n;
}
```

**③ `include/defs.h` — 声明：**`void console_flush(struct proc*);`

**④ `kernel/proc/proc.c` — `freeproc()` 兜底：**

```c
void freeproc(struct proc *p) {
    sig_cleanup(p);
    console_flush(p);  // ← 在释放页表之前刷出残留（防止最后一行不带 \n 丢数据）
    proc_freepagetable(p->pagetable, p->sz);
    ...
}
```

### 并发工作原理

```
进程 A: write① " I am child" → A.console_buf (无\n)
进程 B: write① " I am child" → B.console_buf (无\n)
  ⚡ TIMER → yield → 切到 C
进程 C: write① " I am child" → C.console_buf
  ⚡ TIMER → yield → 回到 A
进程 A: write② "38" → A.console_buf
  ...
进程 A: write⑤ ".\n" → 遇到 \n → [拿锁] 刷 A.console_buf 全部 [放锁]
  输出: " I am child process: 38. iteration 0.\n" ← 整行！
进程 B: write⑤ ".\n" → 刷 B.console_buf 全部
  输出: " I am child process: 39. iteration 1.\n"
```

---

## 五、验证

行缓冲实施后重新编译运行，yield 输出：

```
 I am child process: 38. iteration 0.
 I am child process: 39. iteration 1.
 I am child process: 40. iteration 2.
 ...
（恰好 15 行，每行完整无交错）

RISC-V basic-glibc: 32/32 ✅ (原 31/32)
RISC-V basic-musl:  32/32 ✅ (原 31/32)
```

---

## 六、设计原理

### 为什么放在控制台而不是通用写路径

| I/O 类型 | 消费者 | 行缓冲是否合理 | 原因 |
|---------|--------|:---:|------|
| 控制台 | 人眼 | ✅ | 人期望按行阅读 — Unix line discipline 标准做法 |
| 文件 | 程序 | ❌ | POSIX 只保单次 write 原子，多次交错是正确行为 |
| 管道 | 程序 | ❌ | 会改变消息语义 |

行缓冲只在 `consolewrite()` 中生效，不影响文件/管道路径。

### 内核职责边界

```
用户态职责：一次 printf() 应该是一次 write() 还是多次
内核职责：  保证单次 write() 原子 + 控制台提供行缓冲 (line discipline)
不在职责：  跨系统调用的原子性、猜测用户态语义、为输出改调度器
```

行缓冲不是 hack——它是 Unix **line discipline** 的最小子集。Linux 内核的 `n_tty.c`（~2500 行）正是做这件事的：在硬件和用户进程之间加一层语义层，让控制台输出对人类可读。我们只用了它的输出端行缓冲子集（不做输入规范模式、信号生成、回声控制）。

---

## 七、文件 I/O 同样的场景是否需要类似处理

**不需要。** 文件写的消费者是程序，POSIX 只保证单次 `write()` 原子，不保证多次 `write()` 之间不被打断。如果用户程序在意写入顺序，应该自己加锁或一次 `write()` 写全。控制台是特例，因为它的"消费者"是人。

---

> **总结一句话：** 根因是测试集 `printf()` 分段 `write()` + timer 在用户态抢断，导致三次进程的输出字符交错。内核行缓冲（per-proc `console_buf[256]`，遇 `\n` 整行 flush）在控制台层解决了问题——不改测试集、不改调度器、不改系统调用层。4 文件 ~30 行代码，yield 从 0/4 变 4/4，RISC-V basic 满分。
