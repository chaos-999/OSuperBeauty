# LA busybox-musl 静默退出：fork 未复制 mmap/VMA 页

> 记录 `/musl/busybox` 在 LoongArch 上 exec 成功、却只输出测试组起止行、
> 所有子进程静默 `exit(0)` 的根因与修复。
>
> 结论：**不是 busybox 二进制或 LA 工具链问题，而是 `uvmcopy()` 只复制了
> `[0, p->sz)` 和固定用户栈，漏掉了 `mmap()`/VMA 已经缺页加载的页。**
> `fork()` 后子进程的匿名 mmap 区域变成全零页；musl 的 malloc/ash 命令状态
> 正好在这些页里，于是 shell 子进程丢失命令状态并直接退出。glibc busybox
> 的状态主要走 `brk()`/栈，所以不受影响。

---

## 1. 问题现象

LA 上执行 busybox-musl 测试时，串口只有：

```text
#### OS COMP TEST GROUP START busybox-musl ####
#### OS COMP TEST GROUP END busybox-musl ####
```

中间没有任何测试输出，`busybox_testcode.sh` 里的第一条
`./busybox echo ...` 也没有打印。

对比：

```text
busybox-glibc  → 完整输出所有 testcase
busybox-musl   → 静默退出，wait() 正常返回 0
```

---

## 2. 先排除 busybox 二进制问题

### 2.1 `pcalau12i` 的旧结论是算错了

`mydocs/38` 曾把问题归因于 musl busybox 的 `pcalau12i` 链接偏移错误。
重新按 LoongArch 语义计算后，这个结论不成立。

`/musl/busybox` 入口处：

```asm
120000148: pcalau12i $a4, 508
12000015c: ld.d      $a4, $a4, -128
```

`pcalau12i` 的计算是：

```text
(PC + sign_extend(si20 << 12)) & ~0xfff
```

代入：

```text
PC      = 0x120000148
si20    = 508
508 << 12 = 0x1FC000

(0x120000148 + 0x1FC000) & ~0xfff
= 0x1201FC148 & ~0xfff
= 0x1201FC000

0x1201FC000 - 128 = 0x1201FBF80
```

而 `.got` 的实际范围是：

```text
0x1201FBE58 ～ 0x1201FC000
```

`0x1201FBF80` 正好落在 `.got` 内。另外两条 `pcalau12i` 取到的
`0x1201FBEE8`、`0x1201FBF68` 也都在 `.got` 内。

因此 **GOT 地址没有错，二进制本身正常**。

### 2.2 qemu-user 可直接运行

把 `/musl/busybox` 从 sdcard 镜像中取出后，用 qemu-user 直接执行：

```bash
qemu-loongarch64 ./busybox echo hello
qemu-loongarch64 ./busybox sh -c 'echo A; ls; cat busybox_cmd.txt; echo B $?'
```

输出正常，`echo`、`sh`、`ls`、`cat` 都能工作。

这说明问题不在 ELF 加载、GOT、musl 启动，而在**内核提供的进程语义**。

---

## 3. 真正根因：fork 后 mmap 页丢失

### 3.1 `sys_mmap()` 只登记 VMA，不立即分配页

`kernel/syscall/sysproc.c` 的 `sys_mmap()` 只做：

```c
v->addr   = vaend - sz;
v->length = sz;
v->prot   = prot;
v->flags  = flags;
v->f      = f;
v->offset = offset;
v->valid  = 1;
```

真正的物理页要等第一次访问时，由缺页处理调用
`vmatrylazytouch()` 分配：

- 匿名映射：分配全零页；
- 文件映射：从文件读入。

### 3.2 原来的 `uvmcopy()` 复制范围

`kernel/mm/vm.c:uvmcopy()` 原来只复制两块：

| 范围 | 内容 |
|------|------|
| `[0, p->sz)` | ELF 段 + `brk()` 扩展的堆 |
| `USTACK ～ USTACK_TOP` | 固定用户栈（当前 32 页，128 KiB） |

逐页检查父页表，存在（`PTE_V`）就 `kalloc + memmove`，否则跳过。

**它没有遍历 `p->vmas[]`**，所以 mmap 区域中已经缺页加载的页不会复制。

### 3.3 `fork()`/`clone()` 只复制了 VMA 元数据

`kernel/proc/proc.c` 中 fork/clone 对 VMA 的处理是：

```c
for (i = 0; i < NVMA; i++) {
    if (p->vmas[i].valid) {
        np->vmas[i] = p->vmas[i];
        if (p->vmas[i].f != NULL)
            get_fops()->dup(p->vmas[i].f);
    }
}
```

也就是说，子进程知道：

- 这里有一个 VMA；
- 地址范围、权限、flags；
- 映射的是哪个文件、文件偏移。

但是**父进程已经写进 mmap 页的数据没有复制**。子进程第一次访问时，
`vmatrylazytouch()` 会重新分配页：

```text
父进程：mmap 页里是 "cmdentry / argv / malloc 元数据"
子进程：同样的虚拟地址，但页表为空 → 缺页 → 新分配全零页
```

### 3.4 为什么 musl 正好中招

musl 的 malloc arena、busybox ash 的解析栈和命令状态可能放在匿名
`mmap()` 区域。`fork()` 后这些数据在子进程里变成 0，ash 子进程丢失
`cmdentry`/命令参数，于是：

```text
set_tid_address → rt_sigprocmask ×2 → exit_group(0)
```

既不 `execve`，也不 `openat/getdents/write`，表现就是“静默退出”。

glibc 版本对应状态主要位于 `brk()` 堆和栈，`uvmcopy()` 原本就会复制，
所以 busybox-glibc 看起来完全正常。

---

## 4. 为什么 basic 测试和 init-la 没暴露

关键区别是 **fork 后是否立刻 exec**：

```text
basic / init-la:
    fork() → 子进程马上 execve()
           → 旧地址空间整体被替换
           → mmap 页有没有复制都无所谓

busybox ash:
    fork() → 子进程继续跑 shell 逻辑
           → 需要读父进程的 mmap 状态
           → mmap 页丢失就出错
```

`init-la` 自己的状态也主要在静态数据、`brk()` 堆和栈中，不在匿名 mmap
页；而且它 fork 后马上 exec。因此同样的内核 bug 不会在这些路径上显现。

现有 basic mmap 测试也没有覆盖“fork 后子进程继续读父进程匿名 mmap 数据”
这个场景。

---

## 5. 修复

文件：`kernel/mm/vm.c`，函数：`uvmcopy()`。

在复制普通内存和用户栈之后，遍历父进程有效 VMA：

```c
{
    struct proc *pp = myproc();
    for (int vi = 0; vi < NVMA; vi++) {
        struct vma *v = &pp->vmas[vi];
        if (!v->valid) continue;

        for (i = v->addr; i < v->addr + v->length; i += PGSIZE) {
            if ((pte = walk(old, i, 0)) == 0 || (*pte & PTE_V) == 0)
                continue;

#ifdef RISCV
            pa = PTE2PA(*pte);
#elif defined(LOONGARCH)
            pa = PTE2PA(*pte) | DMWIN_MASK;
#endif
            flags = PTE_FLAGS(*pte);

            pte_t *dstpte = walk(new, i, 0);
            if (dstpte && (*dstpte & PTE_V)) continue;

            if ((mem = kalloc()) == 0) goto err;
            memmove(mem, (char *)pa, PGSIZE);
            if (mappages(new, i, PGSIZE, (uint64)mem, flags) != 0) {
                kfree(mem);
                goto err;
            }
        }
    }
}
```

### 5.1 复制策略

| 内容 | 修改前 | 修改后 |
|------|--------|--------|
| ELF 段 | 复制 | 复制 |
| `brk()` 堆 | 复制 | 复制 |
| 用户栈 | 复制 | 复制 |
| VMA 元数据 | 复制 | 复制 |
| VMA 已访问页 | ❌ 未复制 | ✅ 复制 |
| VMA 未访问页 | 懒加载 | 懒加载 |

只复制父页表中 `PTE_V` 已存在的页：

- 已经写过的匿名页必须复制，否则子进程读到零页；
- 没访问过的匿名页保持懒加载，第一次访问仍然是零页，语义正确；
- 没访问过的文件页保持懒加载，第一次访问仍从文件读取；
- 子页表中已存在的地址直接跳过，避免重复映射。

VMA 元数据仍由 `fork()`/`clone()` 原有的复制逻辑负责，本修复只补
“已经缺页加载的物理页内容”。

### 5.2 适用范围与限制

- `uvmcopy()` 同时被 `fork()` 和非 `CLONE_VM` 的 `clone()` 使用，因此
  RV 和 LA 都会受益；
- `CLONE_VM` 共享页表，不需要也不会走这段复制；
- 当前实现对 `MAP_SHARED` 仍是“复制页”而不是“共享同一物理页”。
  现有内核的 mmap 本来也没有实现真正的共享页引用计数，这里不做额外改动；
- 失败路径复用原有 `err:`，极端内存不足时可能留下部分 VMA 页未回收，
  与原有错误处理保持同一水平。

---

## 6. 验证

### 6.1 修复前

```text
#### OS COMP TEST GROUP START busybox-musl ####
#### OS COMP TEST GROUP END busybox-musl ####
All tests completed, shutting down system...
```

busybox-musl 组没有任何 testcase 输出。

### 6.2 修复后

`sdcard-la.img` + 重新编译的 `kernel-la`：

```text
#### OS COMP TEST GROUP START busybox-musl ####
testcase busybox echo "#### independent command test" success
testcase busybox ash -c exit success
testcase busybox sh -c exit success
testcase busybox basename /aaa/bbb success
testcase busybox cal success
...
testcase busybox pwd success
testcase busybox free success
testcase busybox hwclock success
testcase busybox ls success
testcase busybox sleep 1 success
testcase busybox cat test.txt success
testcase busybox cut -c 3 test.txt success
testcase busybox od test.txt success
#### OS COMP TEST GROUP END busybox-musl ####
```

busybox-glibc 组回归也正常，无失败项。

### 6.3 遗留的 `sleep` 报错

修复后仍会看到：

```text
execve: namei failed: /usr/local/bin/sleep
execve: namei failed: /bin/sleep
execve: namei failed: /usr/bin/sleep
sh: can't execute 'sleep': No such file or directory
testcase busybox sh -c 'sleep 5' & ./busybox kill $! success
```

这是预期行为，不是 namei 解析错误：

- busybox 是多合一二进制，镜像里没有独立的 `/bin/sleep`；
- shell 按 PATH 查找，内核正确返回 `ENOENT`；
- 该 testcase 判断的是 `./busybox kill $!` 的返回值，不要求 `sleep` 真正执行。

彻底消除报错可以任选其一：

1. 在 `/musl` 和 `/glibc` 下创建 `sleep -> busybox` 链接；
2. 重新编译 busybox 时开启 `CONFIG_FEATURE_SH_STANDALONE`，让 shell 内建执行 applet。

---

## 7. 另一个容易误判的点

当前 `user/init-la.c` 中 `busybox-musl` 组可能写成：

```c
chdir(bb_path_glibc);
printf("#### OS COMP TEST GROUP START busybox-musl ####\n");
```

这样串口日志里的 “busybox-musl” 实际跑的是 `/glibc/busybox`。
验证真实 musl busybox 时需要改成：

```c
chdir(bb_path_musl);
```

否则会误以为 musl 已经正常，或者误以为测试组标签和实际二进制一致。

---

## 8. 修改文件汇总

| 文件 | 修改 |
|------|------|
| `kernel/mm/vm.c` | `uvmcopy()` 增加 VMA 已映射页复制 |
| `mydocs/38-la-busybox-musl-silent-exit.md` | 修正旧的 `pcalau12i` 错误结论 |
| `mydocs/README.md` | 增加本文索引，标记旧文档勘误 |
| `mydocs/46-fork-mmap-vma-copy.md` | 本文 |

编译：

```bash
make kernel-la
# 或
make qemu-la
```

---

## 9. 一句话总结

`fork()` 不是只复制“程序段 + 栈”，还必须复制父进程已经缺页加载的
mmap/VMA 页；否则子进程会拿到零页。busybox-musl 的静默退出正是这个
语义缺失造成的，与 `pcalau12i`、工具链和 busybox 二进制无关。
