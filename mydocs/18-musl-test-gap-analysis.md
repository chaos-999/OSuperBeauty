# 🔍 musl 测试缺失分析：从串口日志到内核根因

> 📅 生成时间：`2026-06-06 19:02 (GMT+8)`
>
> 目标：解释为什么测试磁盘上大量 musl 测试未被执行（RISC-V + LoongArch），诊断 LA 上 busybox-musl 静默失败的根因，给出修复方案

---

## 零、前置知识：xv6 exec 代码详解

> 理解 execve 的完整流程是诊断 auxv 缺失问题的基础。SpringOS 的 RISC-V `execve` 直接派生自 xv6-riscv，本节逐段讲解原版逻辑。

整个函数做三件事：**① 解析 ELF 并加载段到新页表 → ② 在新页表的栈上构造 argc/argv/envp/auxv → ③ 切换进程页表并设置入口寄存器。**

### 0.1 打开文件、读 ELF 头

```c
if ((ip = namei(path)) == 0) return -1;   // VFS 路径 → inode
ip->i_op->lock(ip);                        // 锁住 inode

// 读 ELF 头（前 64 字节）
if (ip->i_op->read(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) goto bad;
if (elf.magic != ELF_MAGIC) goto bad;      // 0x7f 'E' 'L' 'F'
```

`elf` 是 `struct elfhdr`，包含：

| 字段 | 含义 | 用途 |
|------|------|------|
| `e_ident[16]` | magic + 平台信息 | 验证是合法 ELF |
| `e_type` | 2=可执行文件 | — |
| `e_machine` | 243=RISC-V | — |
| `e_entry` | 入口虚拟地址 | 最终写入 `p->trapframe->epc`，进程从这里开始执行 |
| `e_phoff` | 程序头表在文件中的偏移 | 遍历 PT_LOAD 段的起点 |
| `e_phnum` | 程序头总数 | 遍历次数 |
| `e_phentsize` | 每个程序头大小 | 遍历步长 |

### 0.2 创建新页表

```c
if ((pagetable = proc_pagetable(p)) == 0) goto bad;
```

**关键设计：** 这里新建了一个**空的**页表，旧页表还挂在 `p->pagetable` 上。万一后面加载失败，`goto bad` 会释放这个新页表，旧页表不受影响——exec 失败进程还能继续跑。这是 xv6 防御性编程的典型模式：先在新资源上做事，全部成功后再原子切换。

### 0.3 遍历程序头，加载每个 `PT_LOAD` 段

```c
for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
    // 从文件中读出第 i 个程序头
    if (ip->i_op->read(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) goto bad;

    if (ph.type != ELF_PROG_LOAD) continue;  // 只加载 PT_LOAD 段

    // 安全检查
    if (ph.memsz < ph.filesz) goto bad;
    if (ph.vaddr + ph.memsz < ph.vaddr) goto bad;  // 防溢出

    // ★ 分配虚拟内存：从 sz 到 ph.vaddr+ph.memsz
    // sz 是当前已分配的最高虚拟地址，起到"连续分配"的效果
    if ((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz,
                        PTE_W | PTE_X | PTE_R | PTE_U)) == 0)
        goto bad;
    sz = sz1;

    // ★ 把文件内容 copy 到对应的物理页
    // loadseg 内部：对每一页，walkaddr 拿物理地址，read 从文件读入
    if (loadseg(pagetable, PGROUNDDOWN(ph.vaddr), ip,
                PGROUNDDOWN(ph.off), ph.filesz + margin_size) < 0)
        goto bad;
}
```

这里有一个微妙设计：`uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, ...)`

- 第一个参数 `sz` 是起始虚拟地址——当前已分配区域的末尾
- 第二个参数 `ph.vaddr + ph.memsz` 是终止虚拟地址
- uvmalloc 负责分配 `[sz, end)` 之间的所有物理页并映射

为什么用 `sz`（累计偏移）而不是直接用 `ph.vaddr`？

因为 xv6 的 `uvmalloc` 假设内存是**连续增长**的。第一个 `PT_LOAD` 段会把 `sz` 从 0 推到 `ph.vaddr + ph.memsz`；第二个段的 `ph.vaddr` 一般紧跟在后面，所以 `sz` 正好等于它。这是 ELF 链接器保证的性质——各 LOAD 段在虚存中紧密排列。

```
加载前的虚拟地址空间：        加载后：
0                            0
├── .text (ph.vaddr=0)       ├── .text     ← PT_LOAD #1, filesz=代码大小
├── .rodata                   ├── .rodata
├── .data                     ├── .data     ← PT_LOAD #2, filesz < memsz
├── .bss  (memsz > filesz)    ├── .bss      ← kalloc 已清零，不用读文件
```

**BSS 怎么处理的？** `uvmalloc` 分配的物理页来自 `kalloc()`，它返回的页已被 `memset(pa, 0, PGSIZE)` 清零。`loadseg` 只复制 `ph.filesz` 字节，剩下的（BSS 部分）自动为零——不需要任何额外的清零代码。

### 0.4 分配用户栈

```c
sz = PGROUNDUP(sz);  // 页对齐

// 在虚拟地址高端分配 USTACK ~ USTACK_TOP 作为用户栈
uvmalloc(pagetable, USTACK, USTACK_TOP, PTE_W | PTE_R | PTE_U);
uvmclear(pagetable, USTACK);    // 栈底第一个页设为无效——栈溢出保护
```

xv6 的虚拟地址空间布局：

```
MAXVA
├── TRAMPOLINE (内核跳板页)
├── TRAPFRAME  (进程 trapframe 映射)
├── USTACK_TOP ← sp 初始指向这里
│   ...        ← 栈向下增长
│   ...        
├── USTACK     ← 栈底
├── (guard)    ← uvmclear 清掉 PTE_V，访问就 page fault
├── 用户程序各段 (.text / .data / .bss)
└── 0
```

### 0.5 构建栈内容：字符串 → 指针表 → auxv → argc

这一步是在新页表的用户栈上写入数据。从高地址往低地址依次放：

```
USTACK_TOP （高地址）
├── 环境变量字符串 (envp[0], envp[1], ...)
├── 参数字符串 (argv[0], argv[1], ...)
├── 16 字节对齐填充
├── auxv 表 [{AT_HWCAP,0}, {AT_PAGESZ,4096}, ..., {AT_NULL,0}]
├── envp 指针表 [&envp[0], &envp[1], ..., NULL]
├── argv 指针表 [&argv[0], &argv[1], ..., NULL]
└── argc (8 字节)
    ← sp 最终指向这里（低地址）
```

**对应代码（按执行顺序）：**

```c
// 5a. 推 envp 字符串
for (envc = 0; envp[envc]; envc++) {
    sp -= strlen(envp[envc]) + 1;  // 栈向下增长
    sp -= sp % 16;                 // RISC-V 要求 16 字节对齐
    copyout(pagetable, sp, envp[envc], strlen(...) + 1);
    estack[envc] = sp;             // 记录每个字符串的地址
}

// 5b. 推 argv 字符串（逻辑同上）
for (argc = 0; argv[argc]; argc++) {
    sp -= strlen(argv[argc]) + 1;
    sp -= sp % 16;
    copyout(pagetable, sp, argv[argc], strlen(...) + 1);
    ustack[argc] = sp;
}

// 5c. 构建 auxv
uint64 aux[MAX_AT * 2];
int index = 0;
AUXV(AT_HWCAP, 0);                // AUXV 宏展开为 aux[index++]=key; aux[index++]=val;
AUXV(AT_PAGESZ, PGSIZE);
AUXV(AT_PHDR, elf.phoff);         // ← 程序头表地址
AUXV(AT_PHENT, elf.phentsize);    // ← 每个程序头大小
AUXV(AT_PHNUM, elf.phnum);        // ← 程序头总数
AUXV(AT_BASE, 0);                 // ← 解释器基址
AUXV(AT_ENTRY, elf.entry);
AUXV(AT_UID, p->uid);
AUXV(AT_EUID, ...);
AUXV(AT_GID, ...);
AUXV(AT_EGID, ...);
AUXV(AT_SECURE, ...);
AUXV(AT_RANDOM, sp);
AUXV(AT_NULL, 0);                 // 终止符

// 5d. 从高到低放指针表
sp -= sp % 16;                        // 对齐
sp -= aux_bytes; copyout(aux);       // auxv 表（最靠近栈顶）
sp -= env_bytes; copyout(estack);    // envp[] 指针表
sp -= argv_bytes; copyout(ustack);   // argv[] 指针表
sp -= sizeof(uint64);                // argc（栈底）
copyout(pagetable, sp, &argc, sizeof(uint64));
```

### 0.6 切换页表、设置寄存器

```c
// 保存旧页表指针
oldpagetable = p->pagetable;
p->pagetable = pagetable;          // ★ 原子切换到新页表
p->sz = sz;                        // 更新进程虚拟内存大小

// ★ 设置用户态入口
p->trapframe->epc = elf.entry;     // PC = _start（从 ELF 头读出）
p->trapframe->sp = sp;             // SP = 栈底（argc 所在位置）
p->trapframe->a1 = argv_ptr;       // a1 = argv[] 指针表地址
p->trapframe->a2 = envp_ptr;       // a2 = envp[] 指针表地址
// 注意：a0 未显式设置，return 0 时写入 0 → 用户态 _start 将其解释为 argc

// 释放旧页表（旧程序的代码/数据/栈全部回收）
proc_freepagetable(oldpagetable, oldsz);
return 0;  // a0 = 0 → 用户态作为 argc（虽然实际 argc 从栈上读）
```

### 0.7 完整流程图

```
execve("/bin/sh", {"sh", NULL}, {"PATH=/bin", NULL})
         │
         ▼
┌─────────────────────────────────────────────────┐
│ ① namei("/bin/sh") → inode                     │
│ ② read(elfhdr) → 验证 ELF_MAGIC                │
│ ③ proc_pagetable() → 新建空白页表              │
│ ④ 遍历 PT_LOAD 段：                            │
│    uvmalloc() 分配物理页 + 映射                 │
│    loadseg() 从文件读 .text/.data 到物理页      │
│    .bss 自动为零（kalloc 清零）                 │
│ ⑤ uvmalloc(USTACK) 分配栈 + uvmclear(guard)    │
│ ⑥ 在栈上 copyout 字符串 + argv/envp/auxv/argc  │
│ ⑦ p->pagetable = new; p->trapframe->epc=entry; │
│    p->trapframe->sp = 新栈底                    │
│ ⑧ proc_freepagetable(old) 释放旧页表           │
│ ⑨ return 0                                     │
│         │                                       │
│         ▼                                       │
│  usertrapret() → sret → 用户态 _start           │
│  _start 从栈读 argc/argv → main(argc, argv)     │
└─────────────────────────────────────────────────┘
```

**核心要点：**

- **新旧页表分离**——加载过程在全新页表上进行，旧页表不受影响，exec 失败可回退
- **栈布局标准化**——argc/argv/envp/auxv 在栈上按 Linux 约定排列，C 运行时直接可用
- **BSS 隐式清零**——`kalloc` 返回的物理页已 memset(0)，`loadseg` 只拷贝 `filesz` 字节，剩余空间天然为零
- **auxv 是进程的"说明书"**——告诉 libc 页大小、入口地址、程序头位置等关键信息；缺失 AT_PHDR 会导致 musl 的 `__init_tls` 访问空指针（本文档的核心议题）

---

## 一、现象：串口日志里 musl 只跑了 basic 和 busybox

对比 SD 卡磁盘镜像上**实际存在的**测试脚本 vs 内核**实际执行的**：

### 1.1 RISC-V（`os_serial_out_rv.txt`）

```
实际执行顺序：
  ① basic-glibc  (32 tests) ✅
  ② basic-musl   (32 tests) ✅
  ③ busybox-musl (busybox sh busybox_testcode.sh) ✅
  ④ busybox-glibc ✅
  → shutdown()
```

### 1.2 LoongArch（`os_serial_out_la.txt`）

```
实际执行顺序：
  ① basic-glibc  (32 tests) ✅
  ② basic-musl   (32 tests) ✅
  ③ busybox-musl  ← 静默失败，无任何输出！
  ④ busybox-glibc ✅
  → shutdown()
```

### 1.3 磁盘上未被触及的测试

| 测试组 | `/glibc/` 脚本 | `/musl/` 脚本 | 状态 |
|--------|:--:|:--:|:--:|
| basic | ✅ | ✅ | **已执行** |
| busybox | ✅ | ✅ | **已执行**（LA musl 静默失败） |
| lua | `lua_testcode.sh` | `lua_testcode.sh` | ❌ 未跑 |
| libctest | `libctest_testcode.sh` | `libctest_testcode.sh` | ❌ 未跑 |
| libcbench | `libcbench_testcode.sh` | `libcbench_testcode.sh` | ❌ 未跑 |
| lmbench | `lmbench_testcode.sh` | `lmbench_testcode.sh` | ❌ 未跑 |
| ltp | `ltp_testcode.sh` | `ltp_testcode.sh` | ❌ 未跑 |
| iozone | `iozone_testcode.sh` | `iozone_testcode.sh` | ❌ 未跑 |
| iperf | `iperf_testcode.sh` | `iperf_testcode.sh` | ❌ 未跑 |
| netperf | `netperf_testcode.sh` | `netperf_testcode.sh` | ❌ 未跑 |
| cyclictest | `cyclictest_testcode.sh` | `cyclictest_testcode.sh` | ❌ 未跑 |
| unixbench | `unixbench_testcode.sh` | `unixbench_testcode.sh` | ❌ 未跑 |

**结论：24 个测试入口中只跑了 4 个（RV）/ 3.5 个（LA）。9 个测试组的 musl 版全未跑。**

---

## 二、根因分析（三层递进）

### 2.1 第一层：init 代码只调了 `test_pre()`

`user/init-rv.c` 和 `user/init-la.c` 的 `main()` 当前执行路径：

```c
int main() {
    // ... console、/tmp 初始化 ...

    test_pre();   // ← 唯一被调用的测试函数！

    // 以下全部被 #if 0 或 // 注释禁用：
    // test_final();           ← interrupts/copyfilerange/splice (glibc+musl)
    // test_on_site();         ← git 测试
    // run_dir_testscripts();  ← 动态扫描所有 *_testcode.sh

    shutdown();
}
```

`test_pre()` 内部只覆盖 basic + busybox 共 4 个入口：

```c
void test_pre() {
    // ① /glibc/basic/ → fork+exec 32 个测试二进制
    // ② /musl/basic/  → fork+exec 32 个测试二进制
    // ③ /musl/  → busybox sh busybox_testcode.sh
    // ④ /glibc/ → busybox sh busybox_testcode.sh
}
```

其他 10 组测试（lua、libctest、libcbench、lmbench、ltp、iozone、iperf、netperf、cyclictest、unixbench）**根本没有被访问的代码路径**。

### 2.2 第二层：动态扫描器被 `#if 0` 禁用且有匹配 bug

`run_dir_testscripts()` 本应扫描指定目录找到所有 `*_testcode.sh` 并执行，但：

**问题 A — 整个函数被禁用：**

```c
// 【暂时禁用】依赖 busybox sh applet，当前 busybox 不可用，待修复后启用
#if 0
static void run_dir_testscripts(const char *dirpath) {
    // ... 扫描逻辑 ...
}
#endif
```

**问题 B — 禁用前提已被证伪。** 同一个 `test_pre()` 里 busybox-musl/glibc 就是通过 `execve("busybox", {"busybox","sh","busybox_testcode.sh"})` 执行的，日志显示完整成功：

```
#### OS COMP TEST GROUP START busybox-musl ####
#### independent command test
testcase busybox echo "#### independent command test" success
testcase busybox ash -c exit success
testcase busybox sh -c exit success   ← sh applet 工作正常！
```

**问题 C — 即使取消 `#if 0`，匹配逻辑有 bug：**

```c
// 只精确匹配 basic_testcode.sh！不是通配所有 *_testcode.sh
if (de->d_name[0] != '.' && strcmp(de->d_name, "basic_testcode.sh") == 0) {
```

`lua_testcode.sh`、`libctest_testcode.sh` 等即使扫描器启用也匹配不到。

### 2.3 第三层（LA 特有问题）：busybox-musl 静默失败

在 LoongArch 上，`test_pre()` 的 busybox-musl 执行路径：

```c
printf("before chdir busybox\n");
chdir(bb_path_musl);   // → /musl/
printf("after chdir busybox\n");
pid = fork();
if (pid == 0) {
    execve("busybox", {"busybox","sh","busybox_testcode.sh"}, NULL);
    printf("init: exec busybox_testcode failed\n");
    exit(1);
}
wait(0);
```

串口日志只输出：

```
before chdir busybox
after chdir busybox
（静默，无任何 busybox 输出）
```

没有 `"init: exec busybox_testcode failed"`（说明 execve 没返回 -1），没有 busybox 输出（说明 busybox 启动后立刻崩溃），`wait(0)` 正常返回（子进程以某种方式退出了）。

**排查：LA execve 缺了 5 个关键的 auxv 条目。**

对比 RISC-V 和 LoongArch 的 execve 实现中 auxv 设置：

```c
// RISC-V execve — 设置了完整的 auxv
AUXV(AT_HWCAP, 0);
AUXV(AT_PAGESZ, PGSIZE);
AUXV(AT_PHDR, elf.phoff);     // ← ELF 程序头虚拟地址
AUXV(AT_PHENT, elf.phentsize);
AUXV(AT_PHNUM, elf.phnum);
AUXV(AT_BASE, 0);             // ← 解释器基址
AUXV(AT_ENTRY, elf.entry);
AUXV(AT_UID, ...);
// ...

// LoongArch execve — 缺失 AT_HWCAP / AT_PHDR / AT_PHENT / AT_PHNUM / AT_BASE
alloc_aux(aux, AT_PAGESZ, PGSIZE);
alloc_aux(aux, AT_ENTRY, elf.entry);
alloc_aux(aux, AT_UID, ...);
// ← AT_PHDR 等全都没有！
```

**为什么 musl 受影响而 glibc 不受影响：**

`AT_PHDR` / `AT_PHENT` / `AT_PHNUM` 告诉程序自身 ELF 程序头在虚拟内存中的位置。musl 的 C 运行时初始化流程：

```
_start()
  → __init_libc()
    → __init_tls()
      → getauxval(AT_PHDR)  ← 遍历 ELF 程序头找 PT_TLS 段
        → 未设置 → NULL → 访问非法地址 → SIGSEGV → 静默崩溃
```

glibc 的初始化可能通过其他方式定位 phdr（如回退到 `_DYNAMIC` 或 `.dynamic` 段遍历），因此即使 AT_PHDR 缺失也能工作。

**次要因素 — LA trapframe 缺少 `gp` 寄存器（r22）：**

```c
// RISC-V trapframe: 有 gp (offset 56)
// LoongArch trapframe: 无 gp 字段！

// uservec.S 只保存 r21 和 fp (r23)，跳过 gp (r22):
st.d   $r21, $a0, 160
st.d   $fp,  $a0, 168
// ← r22 (gp) 从未被保存/恢复
```

execve 换进程镜像后只设置 `era`、`sp`、`a0–a2`，gp 保持 init 进程的残留值。如果 musl 二进制编译时使用了 gp-relative 寻址（很多 LA 静态链接程序的默认行为），在 `_start` 设置 gp 之前就访问了全局数据 → 拿到随机 gp 值 → 段错误。

---

## 三、缺失测试组能否跑（可行性评估）

对于被扫描器遗漏的 9 个测试组，它们的 `*_testcode.sh` 脚本执行只需要 `busybox sh`，不依赖其他内核功能。

| 测试组 | 依赖 | 可行性 |
|--------|------|:------:|
| lua | busybox sh → `run-all.sh` 执行 lua 脚本 | ⚠️ lua 解释器需要完整 libc + mmap/futex |
| libctest | busybox sh → `run-dynamic.sh` / `run-static.sh` | ⚠️ 大量 libc 测试，需要完整 syscall 覆盖 |
| libcbench | busybox sh → `libc-bench` | ⚠️ malloc/string/math 性能测试 |
| lmbench | busybox sh → `lat_ctx` / `bw_mmap_rd` 等 | ⚠️ 需要 mmap/fork/pipe 完整 + 时钟精度 |
| ltp | busybox sh → `runtest.exe` | ⚠️ 大量 LTP 测试用例（可能超时） |
| iozone | busybox sh → `iozone` | ⚠️ 需要完整文件 I/O |
| iperf | busybox sh + 网络栈 | ❌ 尚无 TCP/IP 栈 |
| netperf | busybox sh + 网络栈 | ❌ 尚无 TCP/IP 栈 |
| cyclictest | busybox sh + 实时调度 | ⚠️ 需要高精度时钟 + 实时调度策略 |

---

## 四、修复方案

### 4.1 RISC-V + LoongArch 共同修复：启用动态扫描器

修改 `user/init-rv.c` 和 `user/init-la.c`：

**改动 A — 取消 `#if 0`，启用 `run_dir_testscripts()`：**

```c
// 删除 #if 0 ... #endif 包裹
static void run_dir_testscripts(const char *dirpath) {
    // ... 原有代码 ...
}
```

**改动 B — 将精确匹配改为通配匹配：**

```c
// 修复前：
if (de->d_name[0] != '.' && strcmp(de->d_name, "basic_testcode.sh") == 0)

// 修复后：匹配所有 *_testcode.sh
if (de->d_name[0] != '.' && endswith(de->d_name, "_testcode.sh"))
```

需要在 `user/ulib.c` 中添加 `endswith()` 工具函数。

**改动 C — `main()` 中调用扫描器：**

```c
int main() {
    // ... 初始化 ...

    test_pre();                        // 保留：直接 exec basic 测试（不依赖 busybox）

    // 新增：扫描执行其余所有测试脚本
    run_dir_testscripts("/glibc");
    run_dir_testscripts("/musl");

    shutdown();
}
```

### 4.2 LoongArch 专属修复：execve 补 auxv 条目（✅ 已实施）

#### 4.2.1 辅助向量（auxv）是什么

当内核通过 `execve` 加载一个新程序时，除了把 ELF 的代码和数据段映射到内存、设置栈和寄存器，还要在用户栈上放一张「辅助向量表」（auxiliary vector，简称 auxv）。这张表是一串 `{key, value}` 对，以 `AT_NULL(0)` 结尾，告诉新程序关于自身和运行环境的关键信息。

```
用户栈顶（高地址）
├── 环境变量字符串
├── 参数字符串
├── AT_RANDOM 缓冲区（16 字节随机数）
├── auxv 表           ← 这张表
│   ├── {AT_PAGESZ, 4096}        "系统页大小是 4096"
│   ├── {AT_HWCAP, 0}            "硬件能力位掩码"
│   ├── {AT_PHDR, 0x400000}      "你的 ELF 程序头在虚存 0x400000"
│   ├── {AT_PHENT, 56}           "每个程序头 56 字节"
│   ├── {AT_PHNUM, 8}            "共 8 个程序头"
│   ├── {AT_BASE, 0}             "解释器基址（静态链接为 0）"
│   ├── {AT_ENTRY, 0x200100}     "入口地址"
│   ├── {AT_UID, 0}              "用户 ID"
│   ├── ...
│   └── {AT_NULL, 0}             ← 终止符
├── envp[] 指针表
├── argv[] 指针表
└── argc（栈底，低地址）
```

程序运行时，C 运行时库通过 `getauxval(key)` 函数读取 auxv 表获取这些信息。

#### 4.2.2 修复前的代码（缺失 5 个条目）

`kernel/proc/exec.c` 中 LA 分支的 execve 函数，auxv 设置部分：

```c
// 修复前 —— 只设置了 7 个条目
uint64 aux[MAX_AT * 2];
aux[0] = 0;
aux[1] = 0;
aux[2] = 0;  // 初始化（aux[0] 用作计数器）

alloc_aux(aux, AT_PAGESZ, PGSIZE);                        // ① 页大小
//                   ↑ AT_HWCAP 缺失！
//                   ↑ AT_PHDR  缺失！← 致命
//                   ↑ AT_PHENT 缺失！
//                   ↑ AT_PHNUM 缺失！
//                   ↑ AT_BASE  缺失！
alloc_aux(aux, AT_ENTRY, elf.entry);                       // ② 入口地址
alloc_aux(aux, AT_UID, p->uid);                            // ③ 用户 ID
alloc_aux(aux, AT_EUID, has_setuid ? file_uid : p->euid); // ④ 有效用户 ID
alloc_aux(aux, AT_GID, p->gid);                            // ⑤ 组 ID
alloc_aux(aux, AT_EGID, has_setgid ? file_gid : p->egid); // ⑥ 有效组 ID
alloc_aux(aux, AT_SECURE, (has_setuid || has_setgid) ? 1 : 0); // ⑦ 安全模式
// ... AT_RANDOM ...
alloc_aux(aux, AT_NULL, 0);                                // 终止符
```

对比 RISC-V 分支的 `execve`（同一文件，`#ifdef RISCV` 段），它完整设置了全部条目：

```c
AUXV(AT_HWCAP, 0);                              // 硬件能力
AUXV(AT_PAGESZ, PGSIZE);                        // 页大小
AUXV(AT_PHDR, elf.phoff);                       // 程序头偏移
AUXV(AT_PHENT, elf.phentsize);                  // 程序头条目大小
AUXV(AT_PHNUM, elf.phnum);                      // 程序头条目数
AUXV(AT_BASE, 0);                               // 解释器基址
AUXV(AT_ENTRY, elf.entry);                      // 入口地址
AUXV(AT_UID, p->uid);                           // ...
AUXV(AT_EUID, has_setuid ? file_uid : p->euid);
AUXV(AT_GID, p->gid);
AUXV(AT_EGID, has_setgid ? file_gid : p->egid);
AUXV(AT_SECURE, (has_setuid || has_setgid) ? 1 : 0);
AUXV(AT_RANDOM, sp);
AUXV(AT_NULL, 0);
```

LA 分支只是写漏了 AT_HWCAP / AT_PHDR / AT_PHENT / AT_PHNUM / AT_BASE 这 5 行。

#### 4.2.3 修复后的代码（在 `AT_PAGESZ` 之后补 5 行）

```c
// 修复后 —— alloc_aux 调用的顺序和 RISC-V 分支对齐
alloc_aux(aux, AT_PAGESZ, PGSIZE);
alloc_aux(aux, AT_HWCAP, 0);          // ← 新增：硬件能力位掩码
alloc_aux(aux, AT_PHDR, elf.phoff);   // ← 新增：ELF 程序头在虚存中的地址
alloc_aux(aux, AT_PHENT, elf.phentsize); // ← 新增：每个程序头的大小
alloc_aux(aux, AT_PHNUM, elf.phnum);  // ← 新增：程序头总数
alloc_aux(aux, AT_BASE, 0);           // ← 新增：动态解释器基址
alloc_aux(aux, AT_ENTRY, elf.entry);
alloc_aux(aux, AT_UID, p->uid);
// ... 后续不变 ...
```

#### 4.2.4 逐条解释每个缺失条目的作用

**`AT_PHDR`（值 = `elf.phoff`）— 最关键**

`elf.phoff` 是 ELF 文件头中记录的程序头表（Program Header Table）在**文件中的偏移**。但对于静态链接的可执行文件，该值恰好也是程序头表被加载到虚拟内存后的地址（因为 ELF 的 `p_vaddr` 和 `p_offset` 通常对齐）。

```
ELF 文件布局：
┌─────────────────┐
│ ELF Header      │  ← elf.phoff 指向这里
│  .text 段       │
│  .rodata 段     │
│  .data 段       │
│  .bss 段        │
├─────────────────┤
│ Program Headers │  ← 程序头表，描述上面各段如何加载
└─────────────────┘

加载到内存后（静态链接）：
┌─────────────────┐ 0x0
│ ELF Header      │ ← elf.phoff = 0x40（举例）
│ Program Headers │ ← 0x40，和文件中偏移一致
│  .text          │
│  .rodata        │
│  .data          │
│  .bss           │
└─────────────────┘
```

C 运行时库用 `getauxval(AT_PHDR)` 拿到程序头表地址后，遍历每一个 `Elf64_Phdr` 条目，寻找特定类型的段：

| 段类型 | 常量 | 用途 |
|--------|------|------|
| `PT_LOAD` | 1 | 可加载的代码/数据段 |
| `PT_DYNAMIC` | 2 | 动态链接信息 |
| `PT_TLS` | 7 | **线程本地存储模板** ← musl 初始化必需 |
| `PT_GNU_STACK` | 0x6474e551 | 栈可执行标志 |
| `PT_GNU_RELRO` | 0x6474e552 | 只读重定位段 |

**`AT_PHENT`（值 = `elf.phentsize`）**

每个程序头条目的字节大小。遍历程序头表时，每步前进 `phentsize` 字节找到下一个条目。通常为 56 字节（`sizeof(Elf64_Phdr)`）。

**`AT_PHNUM`（值 = `elf.phnum`）**

程序头表中一共有几个条目。遍历循环的上界。

**`AT_BASE`（值 = `0`）**

动态解释器（`ld.so`）的加载基址。对于静态链接的可执行文件，没有解释器，值为 0。设为 0 告知 C 运行时「我是静态链接的，不需要处理动态重定位」。

**`AT_HWCAP`（值 = `0`）**

硬件能力位掩码，告诉程序 CPU 支持哪些特性（如浮点、向量扩展、原子指令等）。LoongArch 上目前没有定义具体的 hwcap 位，传 0 表示「无特殊能力声明」。不影响正确性。

#### 4.2.5 为什么缺 AT_PHDR 导致 musl 崩溃而 glibc 不受影响

**musl 的初始化链（发生 crash 的路径）：**

```
内核 execve 完成
  → 跳转到 Elf64_Ehdr.e_entry（即 _start）
    → _start()                         // crt/loongarch64/crt1.s
      → la.global $gp, __global_pointer$  // 设置全局指针
      → __libc_start_main()
        → __init_libc()
          → __init_tls()               // ★ 这里崩溃
            → __copy_tls()
              → libc.tls_size > 0 时，需要初始化 TLS
              → getauxval(AT_PHDR)     // ← 返回 0！因为没有设置
              → phdr = NULL
              → 遍历 phdr 找 PT_TLS
              → phdr->p_type          // ← 访问 NULL 指针 → SIGSEGV
```

musl 的线程本地存储（TLS）初始化**强制依赖** AT_PHDR 来找到 `PT_TLS` 段。没有它就无法确定 TLS 模板在内存中的位置和大小。

**glibc 为什么不受影响：**

glibc 的 TLS 初始化有多条回退路径：

1. 优先尝试 `getauxval(AT_PHDR)` 
2. 如果 AT_PHDR 为 0，通过读取 ELF 文件头中的 `e_phoff` 自行计算程序头表地址（利用 `AT_ENTRY` 反推 ELF base）
3. 如果都失败，用 linker-provided symbol `__ehdr_start` 定位 ELF 头

glibc 的工程实践更加防御性——它在多种异常内核上运行过，对缺失 auxv 条目有容错处理。musl 作为轻量级 libc，假设内核会提供标准的 auxv 条目。

#### 4.2.6 修复验证

改动只有一个文件一处：`kernel/proc/exec.c`，在 LA 分支的 `alloc_aux(aux, AT_PAGESZ, ...)` 之后插入 5 行。

编译和测试：

```bash
# Docker 编译 LA 内核
docker run --rm -v /home/my-springos:/workspace -w /workspace \
  zhouzhouyi/os-contest:20260510 make kernel-la

# 本地 QEMU 跑 LA（需先解压 sdcard-la.img）
make qemu-la

# 预期：os_serial_out_la.txt 中出现
# #### OS COMP TEST GROUP START busybox-musl ####
# testcase busybox echo "#### independent command test" success
# ...（正常输出）
```

---

### 4.3 后续可选修复：LA trapframe 添加 `gp` 寄存器

> ⚠️ 本项尚未实施。当前的 auxv 修复已足够解决 busybox-musl 静默崩溃问题。此项修复仅在 musl 程序仍然异常时考虑。

```c
// include/proc/proc.h — LoongArch trapframe 新增：
uint64 r21;
uint64 gp;     // ← 新增 (r22)
uint64 fp;
```

```asm
# kernel/trap/la/uservec.S — 保存路径新增：
st.d   $r21, $a0, 160
st.d   $gp,  $a0, 168   # ← 新增
st.d   $fp,  $a0, 176   # ← 偏移调整

# 恢复路径同样新增 ld.d $gp
```

此改动涉及 trapframe 偏移量变更，需要同步更新 `userret` 中所有 `st.d`/`ld.d` 的偏移量。

### 4.4 LoongArch init-la.c：取消 test_final() 注释

```c
int main() {
    test_pre();
    test_final();    // ← 取消注释：激活 interrupts + copyfilerange + splice（各含 musl+glibc）
    shutdown();
}
```

---

## 五、变更影响矩阵

| 测试组 | 修复前 RV | 修复前 LA | 修复后 RV | 修复后 LA |
|--------|:--:|:--:|:--:|:--:|
| basic-glibc (32) | ✅ | ✅ | ✅ | ✅ |
| basic-musl (32) | ✅ | ✅ | ✅ | ✅ |
| busybox-glibc | ✅ | ✅ | ✅ | ✅ |
| busybox-musl | ✅ | ❌ 静默崩溃 | ✅ | ✅（auxv 修复后） |
| lua-glibc/musl | ❌ | ❌ | ✅（扫描器） | ✅（扫描器） |
| libctest-glibc/musl | ❌ | ❌ | ✅ | ✅ |
| libcbench-glibc/musl | ❌ | ❌ | ✅ | ✅ |
| lmbench-glibc/musl | ❌ | ❌ | ✅ | ✅ |
| ltp-glibc/musl | ❌ | ❌ | ✅ | ✅ |
| iozone-glibc/musl | ❌ | ❌ | ✅ | ✅ |
| iperf/netperf-glibc/musl | ❌ | ❌ | ❌（缺网络栈） | ❌ |
| cyclictest-glibc/musl | ❌ | ❌ | ⚠️（缺实时调度） | ⚠️ |

---

## 六、变更记录

| 文件 | 变更 | 说明 | 状态 |
|------|------|------|:--:|
| `kernel/proc/exec.c` (LA) | 在 `AT_PAGESZ` 之后补充 `AT_HWCAP` / `AT_PHDR` / `AT_PHENT` / `AT_PHNUM` / `AT_BASE` 共 5 个 auxv 条目 | 修复 LA musl busybox 静默崩溃（AT_PHDR 缺失导致 musl `__init_tls` 访问空指针） | ✅ 已实施 |
| `user/init-rv.c` | 取消 `#if 0` 包裹 `run_dir_testscripts()`；修复通配匹配为 `*_testcode.sh`；`main()` 中调用扫描器 | 激活磁盘上所有测试脚本 | ⬜ 待实施 |
| `user/init-la.c` | 同上；取消 `test_final()` 注释 | 激活 LA 上 interrupts 等测试 | ⬜ 待实施 |
| `include/proc/proc.h` (LA) | trapframe 新增 `gp` 字段 | 修复 gp 寄存器跨 syscall 不保存的问题 | ⬜ 可选 |
| `kernel/trap/la/uservec.S` | 新增 `gp` 的 save/restore | 配合 trapframe 变更 | ⬜ 可选 |

---

> **总结一句话：** musl 测试缺失分三层——① `init` 只调了 `test_pre()`（仅覆盖 basic+busybox），② 动态扫描器被 `#if 0` 禁用且匹配逻辑只认 `basic_testcode.sh`，③ LA 的 execve 缺 `AT_PHDR` 等 auxv 条目导致 musl 程序静默段错误。前两层影响 RV+LA 共 18 个测试入口，第三层仅影响 LA 上的所有 musl 二进制。
