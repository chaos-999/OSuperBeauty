# ELF 与动态链接零基础入门：从一本书到 41 号文档

> 这篇文档写给完全没学过 ELF、链接器、GOT/PLT 的读者。
>
> 目标：先用生活化的方式讲清楚“一个程序文件是怎么变成正在运行的进程的”，
> 再把这些概念一一对应到 `mydocs/41-rv-dynamic-linker-pt-interp.md`
> 里的内核代码。
>
> 如果只想看修复代码，可以跳到第 8 节；如果想从头理解，按顺序读即可。

---

## 1. 先看一个最朴素的问题

写 C 程序时，我们得到的是一个文件，比如：

```text
entry-dynamic.exe
```

它只是磁盘上的字节。CPU 不会直接“运行一个文件”，CPU 只会：

1. 从某个内存地址取指令；
2. 执行指令；
3. 继续取下一个地址的指令。

所以“运行程序”这件事，内核必须做几件事：

```text
把文件里的代码/数据搬到内存
告诉 CPU 从哪个地址开始执行
把命令行参数、环境变量放到栈上
```

ELF 就是描述“文件里的哪些字节应该搬到内存哪个地址”的格式。
动态链接则是“程序运行时还需要另一个文件（动态链接器/共享库）配合”的机制。

---

## 2. ELF 是什么

ELF 全称 **Executable and Linkable Format**，中文常叫“可执行与可链接格式”。

你可以把它理解成一本书：

```text
ELF 文件
├── 封面/目录        ← ELF header
├── 章节摆放说明      ← program headers（加载器看）
├── 章节内容          ← .text / .data / .rodata ...
└── 索引/符号表       ← section headers（链接器/调试器看）
```

### 2.1 ELF header：书的封面

ELF 文件最开头是一个固定格式的头部，里面有几个关键字段：

| 字段 | 含义 | 类比 |
|------|------|------|
| `e_ident` | 魔数、位数、大小端、ABI | 书的语言、版本 |
| `e_type` | 文件类型：`ET_EXEC` / `ET_DYN` | 是“固定住址的书”还是“可搬动的书” |
| `e_entry` | 程序入口地址 | 从第几页开始读 |
| `e_phoff` | program header 表在文件中的偏移 | 目录在第几页 |
| `e_phentsize` | 每个 program header 多大 | 每条目录多长 |
| `e_phnum` | 有多少个 program header | 目录有多少条 |

本仓库 `include/elf.h` 里的结构：

```c
struct elfhdr {
    uint magic;      // 0x464C457F，也就是 "\x7FELF"
    uchar elf[12];
    ushort type;     // ET_EXEC / ET_DYN
    ushort machine;
    uint version;
    uint64 entry;    // e_entry
    uint64 phoff;    // e_phoff
    uint64 shoff;
    uint flags;
    ushort ehsize;
    ushort phentsize;
    ushort phnum;
    ...
};
```

### 2.2 Program header：给“加载器”看的目录

程序运行时，内核关心的是 **segment（段）**，不是 section。

每个 program header 描述“文件中的一段字节，应该放到内存的哪里”：

| 字段 | 含义 |
|------|------|
| `p_type` | 这个段是什么类型，例如 `PT_LOAD`、`PT_INTERP` |
| `p_offset` | 这段内容在文件中的偏移 |
| `p_vaddr` | 应该放到内存的哪个虚拟地址 |
| `p_filesz` | 文件中有多少字节 |
| `p_memsz` | 内存中需要多少字节（可以比 filesz 大，多出来的是 BSS） |
| `p_flags` | 读/写/执行权限 |

在本仓库里：

```c
struct proghdr {
    uint32 type;
    uint32 flags;
    uint64 off;     // p_offset
    uint64 vaddr;   // p_vaddr
    uint64 paddr;
    uint64 filesz;  // p_filesz
    uint64 memsz;   // p_memsz
    uint64 align;
};
```

最重要的两个类型：

```c
#define ELF_PROG_LOAD   1   // 需要加载到内存的代码/数据
#define ELF_PROG_INTERP 3   // 动态链接器路径
```

### 2.3 Section 和 Segment 的区别

刚学 ELF 最容易混的是这两个词。

- **Section（节）**：给链接器、调试器看的细粒度内容，比如 `.text`、`.data`、`.rodata`、`.symtab`。
- **Segment（段）**：给操作系统加载器看的粗粒度内容，一个 segment 可以包含多个 section。

可以这样记：

```text
Section 是“书的章节”
Segment 是“装订成册的几大块”
```

内核 `execve()` 只需要看 **program header / segment**，不需要看 section。

### 2.4 `ET_EXEC` 和 `ET_DYN`

| 类型 | 含义 | 地址特点 |
|------|------|----------|
| `ET_EXEC` | 固定地址可执行文件 | 文件里写死了 `p_vaddr`，例如从 `0x10000` 开始 |
| `ET_DYN` | 可重定位文件（共享库、PIE） | 可以被加载到任意基址，实际地址 = 基址 + `p_vaddr` |

本仓库测试镜像里的：

```text
/musl/entry-dynamic.exe      ET_EXEC，入口约 0x2e2e8，第一个 LOAD 在 0x10000
/musl/lib/libc.so            ET_DYN，可以被放到任意基址，例如 0x40000000
```

`ET_DYN` 里的地址都是“相对地址”，所以运行时需要加一个 **load base（加载基址）**。

---

## 3. 静态链接 vs 动态链接

### 3.1 静态链接：所有东西都装进一本书

静态可执行文件把所有需要的库代码都复制进自己文件里。

```text
entry-static.exe
├── 自己的代码
├── libc 的代码
├── libm 的代码
└── 启动代码
```

优点：运行时不依赖别的文件。
缺点：文件大；多个程序重复包含同一份 libc。

### 3.2 动态链接：程序只带一张“借书清单”

动态可执行文件不把 libc 复制进来，而是记录：

```text
我需要 libc.so
我某些函数地址还不知道，运行时请动态链接器帮我填
```

运行时需要另一个文件：**动态链接器**（dynamic linker / interpreter）。

```text
entry-dynamic.exe
├── 自己的代码
├── 一张“我需要 libc.so”的清单（DT_NEEDED）
├── 一张“这些地址需要重定位”的表（RELA）
└── PT_INTERP：请先运行 /lib/ld-musl-riscv64-sf.so.1
```

### 3.3 一个生活化比喻

把运行程序比作“读一本外文书”：

- **静态链接**：把所有参考资料翻译好，装订进同一本书。
- **动态链接**：书里只写“遇到外语请查《词典》”，
  而《词典》就是动态链接器/共享库。

内核负责把书和词典都放到桌上，然后先让“词典”开始工作。
词典查完所有生词，再跳回书的正文。

---

## 4. 动态链接里最重要的三个东西

### 4.1 `PT_INTERP`：先运行谁

动态 ELF 的一个 program header 会写：

```text
PT_INTERP
  p_offset = ...
  p_filesz = ...
```

这段内容是一个字符串，例如：

```text
/lib/ld-musl-riscv64-sf.so.1
```

它的意思是：

> 不要直接执行我，请先加载并运行这个动态链接器。

### 4.2 `GOT` 和 `PLT`：地址填表

动态程序调用 `printf()` 时，编译时并不知道 `printf()` 最终在内存哪里。
所以生成代码时先留一个“地址表格”：

```text
GOT（Global Offset Table）：
  [0] = 以后填 printf 的真实地址
  [1] = 以后填 malloc 的真实地址
  ...
```

调用时先跳到一个小跳板 **PLT（Procedure Linkage Table）**：

```text
main:
    call printf@plt

printf@plt:
    load GOT[printf]
    jump 到 GOT[printf] 里的地址
```

程序刚启动时，GOT 里还没有真实地址。
动态链接器的工作就是：

```text
根据 RELA 重定位表，把 GOT 里的地址填对
```

在 RISC-V 上，PLT 最终会跳到 GOT 中保存的地址。
如果动态链接器根本没运行，GOT 里可能是 0 或非法值，
CPU 就会跳到：

```text
0xfffffffffffffffe   // 也就是 -2
```

这就是 41 号文档里看到的 `sepc=-2`。

### 4.3 `DT_NEEDED` 和 `PT_DYNAMIC`

动态 ELF 还有一个 `PT_DYNAMIC` 段，里面是一张张表：

| 标记 | 作用 |
|------|------|
| `DT_NEEDED` | 需要哪个共享库，例如 `libc.so` |
| `DT_SYMTAB` | 符号表在哪里 |
| `DT_STRTAB` | 字符串表在哪里 |
| `DT_RELA` | 重定位表在哪里 |
| `DT_JMPREL` | PLT 重定位表在哪里 |
| `DT_PLTGOT` | GOT 在哪里 |

动态链接器读这些表，完成：

```text
找到 libc.so
找到需要重定位的符号
把地址写进 GOT
```

---

## 5. 一个动态程序从 execve 到 main 的全过程

下面用“内核视角”走一遍。

### 5.1 内核读 ELF header

```text
execve("/musl/entry-dynamic.exe")
  → 读 ELF header
  → 知道 e_phoff / e_phnum / e_entry
```

### 5.2 内核扫描 program header

主程序通常有：

```text
PT_INTERP  → 动态链接器路径
PT_LOAD    → 代码段
PT_LOAD    → 数据段
PT_DYNAMIC → 动态链接信息
```

内核要做两件事：

1. 把主程序的 `PT_LOAD` 搬到内存；
2. 记住 `PT_INTERP` 的路径，稍后加载解释器。

### 5.3 加载主程序

```text
主程序第一个 LOAD：p_vaddr = 0x10000
主程序第二个 LOAD：p_vaddr = 0x57000
```

内核根据 `p_offset/p_filesz` 把文件内容复制到对应虚拟地址。

### 5.4 加载解释器

解释器是 `ET_DYN`，内核要选一个不冲突的基址，比如：

```text
interp_base = 0x40000000
```

然后把解释器的每个 `PT_LOAD` 放到：

```text
interp_base + p_vaddr
```

例如：

```text
libc.so 的 p_vaddr = 0
  → 实际加载到 0x40000000

libc.so 的入口 e_entry = 0x65b08
  → 实际入口 = 0x40000000 + 0x65b08
```

### 5.5 建立用户栈

用户栈通常在很高的地址，往低地址增长。
栈上要放：

```text
argc
argv[0], argv[1], ...
NULL
envp[0], envp[1], ...
NULL
auxv（辅助向量）
```

`auxv` 是内核传给用户态程序的“小纸条”，其中最关键的三项：

| auxv 项 | 含义 |
|---------|------|
| `AT_PHDR` | 主程序 program header 在内存中的地址 |
| `AT_ENTRY` | 主程序真正的入口地址 |
| `AT_BASE` | 解释器被加载到的基址 |

### 5.6 内核跳到解释器，而不是 main

这一点是初学者最容易忽略的：

```text
有 PT_INTERP 时：
    CPU 先执行解释器入口
    解释器完成重定位
    解释器再跳到 AT_ENTRY（主程序入口）
```

可以画成这样：

```text
内核
  → 跳到 interp_base + libc.e_entry
      → musl _dlstart
          → 重定位 entry-dynamic 的 GOT/PLT
          → 跳转到 AT_ENTRY（entry-dynamic 的 _start）
              → __libc_start_main
                  → main()
```

### 5.7 如果没有解释器会发生什么

如果内核忽略 `PT_INTERP`，直接跳到主程序 `elf.entry`：

```text
entry-dynamic 的 _start
  → 调用 __libc_start_main@plt
      → 读 GOT
          → GOT 里是 0 / 非法地址
              → 跳到 0xfffffffffffffffe
                  → Instruction Page Fault
```

这就是 41 号文档修复前的现象：

```text
scause=0xc
sepc=0xfffffffffffffffe
```

`-2` 不是某个系统调用返回的错误码，而是 **CPU 的 PC 真的跑到了 -2**。

---

## 6. 和 41 号文档的对应关系

41 号文档讲的是“怎么改内核”，这一节把每个代码点和上面的概念对应起来。

### 6.1 `ELF_PROG_INTERP`

概念：`PT_INTERP` 是“先运行谁”的 program header。

41 中代码：

```c
#define ELF_PROG_INTERP 3
```

对应位置：

```text
mydocs/41-rv-dynamic-linker-pt-interp.md  4.1 节
```

### 6.2 `exec_open_interp()`

概念：解释器路径可能写成 `/lib/...`，但镜像实际文件在 `/musl/lib` 或 `/glibc/lib`。

41 中代码：

```c
static struct inode *exec_open_interp(const char *path) {
    struct inode *ip = namei((char *)path);
    ...
    const char *dirs[] = {"/musl/lib/", "/glibc/lib/"};
    ...
    if (strncmp(base, "ld-musl-riscv64", 15) == 0)
        return namei("/musl/lib/libc.so");
}
```

它做的事情：

```text
先找 /lib/ld-musl-riscv64-sf.so.1
找不到就找 /musl/lib/ld-musl-riscv64-sf.so.1
再找 /glibc/lib/...
最后 fallback 到 /musl/lib/libc.so
```

对应 41 的 4.2 节。

### 6.3 `exec_load_interp()`

概念：解释器是 `ET_DYN`，要加载到独立基址，并且处理 `PT_LOAD`、文件拷贝、BSS。

41 中代码：

```c
interp_base = 0x40000000;
exec_load_interp(pagetable, lip, interp_base, &interp_entry);
```

函数内部做：

```text
读解释器 ELF header
遍历解释器的 PT_LOAD
  kalloc 页面
  mappages 到 interp_base + p_vaddr
  从文件拷贝 p_filesz 字节
  清零 BSS
入口 = interp_base + el_entry
```

对应 41 的 4.3 节。

### 6.4 `AT_BASE`

概念：解释器需要知道自己被加载到哪，才能做自重定位。

41 中代码：

```c
AUXV(AT_BASE, has_interp ? interp_base : 0);
```

对应 41 的 4.4 节。

### 6.5 `AT_ENTRY` 和 `AT_PHDR`

概念：

- `AT_PHDR`：主程序 program header 在哪；
- `AT_ENTRY`：主程序入口在哪。

41 中代码：

```c
AUXV(AT_PHDR, first_load_vaddr + elf.phoff);
AUXV(AT_ENTRY, elf.entry);
```

解释器读这两个值，完成主程序重定位后跳回 `AT_ENTRY`。

### 6.6 入口切换

概念：有解释器时，先执行解释器，不是主程序。

41 中代码：

```c
p->trapframe->epc = has_interp ? interp_entry : elf.entry;
```

对应 41 的 4.4 节最后。

### 6.7 `uvmfirst()` 第 3 页

概念：用户栈顶是 `4 * PGSIZE`，即使 initcode 很小，也会用到第 3 页。

41 中代码：

```c
mappages(pagetable, 3 * PGSIZE, PGSIZE, (uint64)mem,
         PTE_W | PTE_R | PTE_X | PTE_U);
```

对应 41 的 4.5 节。

---

## 7. 地址空间长什么样

修复后的动态进程大致是这样：

```text
高地址
  ┌──────────────────────────┐
  │ 用户栈 USTACK_TOP        │  argc/argv/envp/auxv
  │ 用户栈                   │
  ├──────────────────────────┤
  │ ...                      │
  ├──────────────────────────┤
  │ 解释器 /musl/lib/libc.so │  interp_base = 0x40000000
  │ 入口 = 0x40000000+0x65b08│
  ├──────────────────────────┤
  │ ...                      │
  ├──────────────────────────┤
  │ 主程序数据段             │  0x57000 ...
  │ 主程序代码段             │  0x10000 ...
  └──────────────────────────┘
低地址
```

注意：

- 主程序由自己的 `PT_LOAD` 决定地址；
- 解释器固定放在 `0x40000000`；
- 用户栈在很高地址；
- `p->sz` 仍然只表示主程序的范围，解释器是单独映射的（41 的“已知限制”里有说明）。

---

## 8. 一张表总结所有名词

| 名词 | 一句话解释 | 在 41 中的位置 |
|------|------------|----------------|
| ELF | 描述程序如何加载的文件格式 | 全文 |
| ELF header | 文件封面：类型、入口、program header 位置 | `struct elfhdr` |
| program header | 加载器看的目录：每段放哪 | `struct proghdr` |
| `PT_LOAD` | 需要搬到内存的代码/数据 | 主程序加载循环 |
| `PT_INTERP` | 动态链接器路径 | `ELF_PROG_INTERP` |
| `PT_DYNAMIC` | 动态链接信息表 | 解释器使用 |
| `ET_EXEC` | 固定地址可执行文件 | entry-dynamic.exe |
| `ET_DYN` | 可重定位文件/共享库 | libc.so |
| 动态链接器 | 启动时给程序填 GOT/PLT 的程序 | musl libc.so / glibc ld-linux |
| `AT_PHDR` | 主程序 program header 地址 | 4.4 节 |
| `AT_ENTRY` | 主程序入口 | 4.4 节 |
| `AT_BASE` | 解释器加载基址 | 4.4 节 |
| GOT | 存放函数/数据真实地址的表 | 第 4.2 节 |
| PLT | 跳转到 GOT 的小跳板 | 第 4.2 节 |
| RELA | 重定位表 | 第 4.3 节 |
| `scause=0xc` | 取指令时页错误 | 第 1 节 |
| `sepc=-2` | PC 跑到非法地址 | 第 5.7 节 |

---

## 9. 常见问题

### Q1：为什么静态程序不需要 `PT_INTERP`？

因为静态程序已经把 libc 代码复制进自己文件里，没有“运行时再找词典”的需求。

### Q2：为什么动态程序不能直接跳到 `main`？

因为 `main` 之前还有启动代码、GOT/PLT 重定位、共享库初始化。
解释器必须先完成这些工作。

### Q3：为什么解释器要放在 `0x40000000`？

因为主程序通常从 `0x10000` 附近开始，用户栈在很高地址。
`0x40000000` 是一个中间的空闲区域，不容易冲突。

### Q4：为什么 `AT_BASE` 不能是 0？

0 表示“没有解释器”。解释器需要知道自己的真实加载基址，
才能把内部的相对地址转换成绝对地址。

### Q5：为什么 `execve` 返回后不是直接执行 `main`？

`execve` 只是把新程序映像装好并设置 `trapframe`。
真正开始执行是在从系统调用返回用户态时，CPU 跳到 `trapframe->epc`。

### Q6：41 文档里的 `fork()` 限制是什么意思？

解释器映射在 `0x40000000`，但没有算进 `p->sz`。
`fork()` 的 `uvmcopy()` 只复制 `0..p->sz`，所以动态进程 fork 出来的子进程
可能没有解释器页。当前测试主要是在 exec 后直接运行，不受影响；
如果以后要支持动态进程 fork，需要额外记录解释器范围并复制。

---

## 10. 一句话总结

ELF 是“程序文件怎么搬到内存”的说明书；
动态链接是“程序运行时还要请一个解释器来填地址表”；
41 号文档做的事就是：

```text
读 PT_INTERP
  → 找到并加载解释器
  → 设置 AT_BASE/AT_ENTRY
  → 先跳到解释器
  → 解释器重定位后再回到主程序
```

理解了这条链，再看 41 里的 `exec_open_interp()`、`exec_load_interp()`、
`AT_BASE` 和 `trapframe->epc`，就不会觉得它们是孤立的魔法了。
