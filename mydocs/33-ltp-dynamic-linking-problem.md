# LTP 动态链接问题：诊断与解决方案

> 记录 SpringOS 在运行 LTP 测试套件时遇到的动态链接兼容性问题。

---

## 问题概述

LTP (Linux Test Project) 测试二进制是**动态链接**的 PIE 可执行文件，需要动态链接器 `/lib/ld-musl-riscv64.so.1` 才能运行。运行时报错：

```
Error relocating ltp/testcases/bin/abort01: unsupported relocation type 64
```

---

## 已完成的内核修复

在解决 LTP 问题的过程中，已实现以下内核改进：

| 修改文件 | 修改内容 | 效果 |
|---------|---------|------|
| `kernel/proc/exec.c` | 添加 symlink 跟随 | execve 能解析符号链接 |
| `kernel/proc/exec.c` | 实现 PT_INTERP 加载 | 支持动态链接可执行文件 |
| `kernel/fs/VFS_ext.c` | vfs_ext_stat 跟随 symlink | stat/access 能检查 symlink 目标权限 |
| `kernel/mm/vm.c` | freewalk 释放残留叶页 | 进程退出时不再 kernel panic |
| `user/init-rv.c` | 创建 /lib 和 /bin 符号链接 | 提供动态链接器路径和命令 |

---

## 根本原因分析

### 评测镜像中的 ABI 不匹配

| 组件 | 位置 | Float ABI | 类型 |
|------|------|-----------|------|
| musl libc.so | `/musl/lib/libc.so` | **soft-float** (flags=0x0) | 动态链接器 |
| LTP 二进制 | `/musl/ltp/testcases/bin/*` | **double-float** (flags=0x4) | 测试程序 |
| busybox | `/musl/busybox` | soft-float | 静态链接（不受影响） |
| glibc ld | `/glibc/lib/ld-linux-riscv64-lp64d.so.1` | double-float | glibc 链接器 |

**核心矛盾：** LTP 二进制编译时使用 hard-float musl，但镜像中只提供了 soft-float 的 musl libc.so。soft-float 的动态链接器拒绝/无法正确处理 hard-float 的二进制。

### LTP 二进制的实际 relocation 类型

通过 `readelf -r` 分析 brk01 和 abort01：

```
333 R_RISCV_RELATIVE   (type 3)  — 加上基地址
205 R_RISCV_JUMP_SLOT  (type 5)  — PLT 跳转表
 11 R_RISCV_64         (type 2)  — 64位绝对地址
```

这些都是**标准 relocation 类型**，理论上任何 musl 版本都应支持。报错的 "type 64" 实际可能是 musl 因为 ABI 不匹配而产生的误导性错误信息。

---

## 解决方案选项

### 方案 A：放弃 LTP 动态链接测试（推荐，零工作量）

**思路：** 接受 LTP 动态链接测试无法通过的现实，专注于其他能得分的测试。

**优点：**
- 零额外内核开发工作
- basic 测试、libc-test（静态链接）、busybox 测试已经能跑
- 评测分数来自多个测试组，LTP 只是其中之一

**缺点：**
- 丢失 LTP 测试分数

**适用场景：** 时间紧迫，需要把精力放在其他高性价比改进上。

---

### 方案 B：内核级最小 ELF Relocator（中等难度）

**思路：** 不使用 musl 动态链接器，由内核直接完成 relocation 并映射 libc。

**实现步骤：**

1. 在 exec.c 中，当检测到 PT_INTERP 但动态链接器加载失败（或 ABI 不匹配）时，回退到内核自行处理
2. 内核处理主程序的 relocations：
   - `R_RISCV_RELATIVE`：直接 `*(addr) += load_base`
   - `R_RISCV_64`：写入绝对地址
   - `R_RISCV_JUMP_SLOT`：解析到 libc 中的对应符号
3. 加载 libc.so 到内存（不通过 musl ldso）
4. 手动解析符号表，填充 GOT

**实现难点：**
- 需要在内核中实现 ELF 符号解析（遍历 `.dynsym`、`.dynstr`、哈希表）
- 需要处理 libc 自身的 relocations
- 需要正确设置 TLS（`__thread` 变量）
- JUMP_SLOT entries 指向 libc 中的函数，需要知道 libc 的加载地址和符号表

**代码量估计：** ~300-500 行内核代码

**风险：** 
- 实现不完整可能导致部分测试 segfault
- TLS 初始化如果错误，大量功能会异常

---

### 方案 C：替换为兼容的 musl libc（简单但需要外部资源）

**思路：** 交叉编译一个 double-float 的 musl libc.so，在内核启动时写入文件系统。

**步骤：**

```bash
# 在开发机上交叉编译 musl (hard-float)
git clone https://git.musl-libc.org/cgit/musl
cd musl
./configure --target=riscv64-linux-gnu \
    CFLAGS="-march=rv64gc -mabi=lp64d" \
    --prefix=/opt/musl-rv64-hf
make -j$(nproc)
# 产物：lib/libc.so (≈900KB)
```

然后将编译好的 libc.so 嵌入内核二进制（作为数据段），在 init 启动时写入 `/lib/ld-musl-riscv64.so.1`。

**优点：**
- 完全解决 ABI 兼容性问题
- 不需要修改内核的 ELF 加载逻辑
- 一旦有正确的 libc.so，所有动态链接程序都能跑

**缺点：**
- 内核二进制增大 ~1MB（嵌入 libc.so）
- 需要在开发机上搭建 RISC-V 交叉编译环境
- 需要确保 musl 版本支持 LTP 使用的所有 syscall

**实现方式：**
```c
// 在内核代码中声明嵌入的 libc.so 数据
extern char _binary_libc_so_start[];
extern char _binary_libc_so_end[];

// init-rv.c 中写入文件系统
int fd = openat(AT_FDCWD, "/lib/ld-musl-riscv64.so.1", O_CREAT|O_WRONLY, 0755);
write(fd, embedded_libc_data, embedded_libc_size);
close(fd);
```

---

### 方案 D：Patch musl 动态链接器跳过 ABI 检查（Hack）

**思路：** 在内核加载 musl libc.so 到内存后，通过二进制 patch 修改其代码，跳过 ABI 兼容性检查。

**原理：**
- musl ldso 在启动时检查被加载 ELF 的 flags
- 如果 flags 不匹配，输出错误并退出
- 找到这个检查的代码位置，NOP 掉

**步骤：**
1. 反汇编 `/musl/lib/libc.so`，找到 ABI 检查逻辑
2. 在 exec.c 加载 interpreter 后，patch 掉对应的指令
3. 让链接器强制继续处理

**优点：**
- 不需要外部编译
- 改动量小

**缺点：**
- 极其脆弱（依赖特定的 musl 编译版本和偏移量）
- 即使跳过 ABI 检查，soft-float 的 libc 函数可能在处理浮点参数时行为异常
- 不保证所有测试都能通过

**评估：** 不推荐，风险高且收益不确定。

---

### 方案 E：内核中实现静态 PIE 加载（绕过动态链接器）

**思路：** 对于只依赖 libc.so 的简单 PIE 可执行文件，内核直接做 relocation 而不使用外部 linker。

**适用条件：**
- 二进制的 NEEDED 只有 `libc.so`（大部分 LTP 测试满足）
- Relocations 只有 RELATIVE、JUMP_SLOT、R_RISCV_64

**实现核心逻辑：**

```c
// 在 exec.c 中，如果 interpreter 加载失败，回退到内核 relocator
if (has_interp && interp_load_failed) {
    // 1. 对主程序做 RELATIVE relocation
    //    遍历 .rela.dyn，对每个 R_RISCV_RELATIVE:
    //    写入 *reloc_addr += load_base (对 PIE 来说 load_base = 0)
    
    // 2. 加载 libc.so 到固定地址
    //    和加载 interpreter 类似，映射到 0x40000000
    
    // 3. 解析符号表，填充 JUMP_SLOT
    //    遍历主程序的 .rela.plt
    //    对每个 JUMP_SLOT：
    //      从主程序 .dynsym 获取符号名
    //      在 libc.so 的 .dynsym 中查找同名符号
    //      将 libc 中的地址写入主程序的 GOT
    
    // 4. 设置 TLS (如果有 PT_TLS 段)
    
    // 5. 跳转到主程序的 entry point
}
```

**工作量估计：** 500-800 行，复杂度高
**成功率估计：** 如果实现正确，大部分 LTP 简单测试可以通过

---

## 建议优先级

| 优先级 | 方案 | 原因 |
|--------|------|------|
| 1 | **方案 A** (放弃 LTP) | 零工作量，先确保其他测试得分 |
| 2 | **方案 C** (编译兼容 musl) | 最正确的解法，但需要交叉编译环境 |
| 3 | **方案 B/E** (内核 relocator) | 工作量大但能解决问题 |
| 4 | **方案 D** (patch musl) | 不推荐 |

---

## 当前可得分的测试

即使放弃 LTP，以下测试应该已经或即将能通过：

| 测试组 | 状态 | 备注 |
|--------|------|------|
| basic-glibc / basic-musl | ✅ 已通过 | 静态链接，32个系统调用测试 |
| libc-test (entry-static.exe) | 部分通过 | 静态链接，取决于 syscall 实现程度 |
| busybox 测试 | ✅ 已通过 | 静态链接 busybox |
| LTP | ❌ 动态链接兼容性问题 | 需要方案 B/C/E |
| iozone / lmbench / unixbench | 待测试 | 需检查是否动态链接 |

---

## 相关文件修改记录

```
kernel/proc/exec.c      — symlink 跟随 + PT_INTERP 加载
kernel/fs/VFS_ext.c     — vfs_ext_stat symlink 解析
kernel/mm/vm.c          — freewalk 安全释放叶页
user/init-rv.c          — /lib 和 /bin 符号链接创建
```
