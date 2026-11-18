# `/dev/zero` read 修复：`syscall_sign_extend` 从假 PASS 变为真 PASS

> **合并说明（本仓库）**：本文由 PR #3（zhangAngel360）贡献。合入时做了区分处理：
> 只采纳源码修复与本文档，**没有采纳 PR 中的 `kernel-rv` 二进制**
> （该二进制基于旧 main，缺少本仓库 libcbench 线程/VM 修复，已本地重新编译）；
> 源码实现改为 `static const` 零缓冲并补充了注释；LoongArch 一并重新编译。
> 文中 `/home/angel/...`、`/tmp/libctest-verify-angel/...` 为贡献者验证环境的路径。

> 记录 libc-test 用例 `syscall_sign_extend` 的根因定位、最小修复与完整回归验证。
>
> 结论：**`kernel/fs/vfs/file.c` 的 `fileread()` 中，`f_type == 8`（`/dev/zero`）分支
> 原实现直接 `return 0`，把读取 `/dev/zero` 当成 EOF；正确行为应是向用户缓冲区写入
> `n` 个零字节并返回 `n`。**
>
> 本次只改这一个分支；`/dev/null`（`f_type == 9`）分支、镜像脚本、其他内核代码均未改动。

---

## 1. 现象

`syscall_sign_extend` 是 libc-test 的 `regression` 类用例。该测试绕过 libc 包装函数，
直接用 `syscall(SYS_read, fd, buf, 1)` 读取 `/dev/zero`，检查 syscall 参数符号扩展是否
正确，同时检查读出的字节是否为零。

在原始内核下，完整日志里静态和动态两次运行都出现相同失败：

```text
src/regression/syscall-sign-extend.c:21: (r = syscall(SYS_read, fd, buf, 1)) == 1 failed: No error information
src/regression/syscall-sign-extend.c:23: read 1 instead of 0
FAIL syscall_sign_extend [status 1]
```

注意：在旧版评测汇总里这个用例可能被记为“PASS”，但那是测试框架在失败时用文件输出
造成的假 PASS，实际逐用例日志如上所示是失败的。分类依据见
[`31-presentfailure-fix-guide.md`](31-presentfailure-fix-guide.md) 第 10 节、
[`30-libctest-classification-false-pass.md`](30-libctest-classification-false-pass.md)。

## 2. 根因链条

### 2.1 `/dev/zero` 如何被识别

`kernel/syscall/sysfile.c` 的 open 路径对 `/dev/zero` 做了特殊处理：

```c
if (!strcmp(path, "/dev/zero")) {
    if ((f = filealloc()) == NULL || (fd = fdalloc2(f, 0)) < 0) {
        return -1;
    }

    f->f_type = 8;
    f->f_pos = 0;
    f->f_flags = flags;
    strcpy(f->f_path, path);
    return fd;
}
```

因此 `/dev/zero` 对应的 `struct file` 使用 `f_type == 8` 标识。

### 2.2 read 调用链

```text
用户 syscall(SYS_read, fd, buf, 1)
  → kernel/syscall/sysfile.c:sys_read()
  → get_fops()->read(f, p, n)
  → kernel/fs/vfs/file.c:fileread(f, addr, n)
  → f_type == 8 分支
```

### 2.3 原实现

修复前 `fileread()` 的相关分支为：

```c
} else if (f->f_type == 9) {
    char a = 0;
    copyout(myproc()->pagetable, addr, (char *)&a, sizeof(char));
    return 0;
} else if (f->f_type == 8) {
    return 0;
} else {
    panic("fileread");
}
```

这里把两类设备混为一谈了：

| `f_type` | 设备 | 正确 read 语义 | 原实现 |
|----------|------|----------------|--------|
| 9 | `/dev/null` | 返回 0，表示 EOF | 正确 |
| 8 | `/dev/zero` | 写入 `n` 个 `0x00`，返回 `n` | 错误：直接返回 0 |

因此测试得到的不是“读取 1 字节成功”，而是“读到 0 字节”。`buf[0]` 没有被写入，
保持栈上残留值，第二条断言也随之失败。

## 3. 修复

### 3.1 改动位置

唯一源码改动在 `kernel/fs/vfs/file.c` 的 `fileread()`：

```diff
     } else if (f->f_type == 9) {
         char a = 0;
         copyout(myproc()->pagetable, addr, (char *)&a, sizeof(char));
         return 0;
     } else if (f->f_type == 8) {
-        return 0;
+        // /dev/zero: every read returns n zero bytes (never EOF).
+        if (n <= 0) return 0;
+        char zeros[512] = {0};
+        int remain = n;
+        while (remain > 0) {
+            int chunk = remain < (int)sizeof(zeros) ? remain : (int)sizeof(zeros);
+            if (copyout(myproc()->pagetable, addr, zeros, chunk) < 0) return -1;
+            addr += chunk;
+            remain -= chunk;
+        }
+        return n;
     } else {
         panic("fileread");
     }
```

### 3.2 实现取舍

- `n <= 0` 时保持原来的 `return 0`，不扩大改动范围；
- 用 512 字节的零缓冲分块 `copyout`，比逐字节 `copyout` 更适合后续大块读取；
- 不用 `kalloc()` 分配整页，避免引入内存分配路径和额外失败处理；
- 拷贝失败时 `return -1`，与该文件周围既有错误风格一致，不额外引入 `-EFAULT`；
- 不改 `f_type == 9` 的 `/dev/null` 分支，也不改 `filewrite()`、`sysfile.c` 或镜像脚本。

## 4. 编译

```bash
make kernel-rv -j4
```

通过。项目使用 `-Wall -Werror`，无编译错误。

为做基线对比，还临时用 Git HEAD 中的原始 `file.c` 构建了一个基线内核：

```bash
git show HEAD:kernel/fs/vfs/file.c > /tmp/libctest-verify-angel/file-orig.c
cp /tmp/libctest-verify-angel/file-orig.c kernel/fs/vfs/file.c
make kernel-rv -j4
cp kernel-rv /tmp/libctest-verify-angel/kernel-rv-baseline

# 随后恢复修复版 file.c 并重新构建当前 kernel-rv
cp /tmp/libctest-verify-angel/file-fixed.c kernel/fs/vfs/file.c
make kernel-rv -j4
```

构建日志：

```text
/tmp/libctest-verify-angel/build-base.log
/tmp/libctest-verify-angel/build-fixed.log
```

当前 `kernel-rv` 为修复版，SHA-256 前缀 `608b62f69150283247ebd78d`；
基线内核 SHA-256 前缀 `39a9a0c4625ddb7b02f885da`。

## 5. 验证

### 5.1 运行环境

使用官方镜像：

```text
/home/angel/oscomp-testdata/sdcard-rv.img.gz
```

解压副本后，用文档记录的 QEMU 参数运行完整测试：

```bash
qemu-system-riscv64 \
  -machine virt -bios default \
  -kernel kernel-rv \
  -m 1G -smp 1 -nographic \
  -drive file=/tmp/libctest-verify-angel/sdcard-rv-full.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -rtc base=utc -no-reboot
```

最终完整验证使用的是**全新未精简的镜像副本**，不是调试时改过脚本的副本。

### 5.2 修复版结果

日志：

```text
/tmp/libctest-verify-angel/qemu-full.log
```

静态用例：

```text
1181:========== START entry-static.exe syscall_sign_extend ==========
1182:Pass!
1183:========== END entry-static.exe syscall_sign_extend ==========
```

动态用例：

```text
1620:========== START entry-dynamic.exe syscall_sign_extend ==========
1621:Pass!
1622:========== END entry-dynamic.exe syscall_sign_extend ==========
```

QEMU 正常退出（`exit: 0`），日志结尾包含：

```text
All tests completed, shutting down system...
Shutdown system call invoked, shutting down xv6...
```

### 5.3 基线结果

基线内核使用同样参数、同样全新镜像运行。日志：

```text
/tmp/libctest-verify-angel/qemu-baseline.log
```

静态用例：

```text
1182:========== START entry-static.exe syscall_sign_extend ==========
1183:src/regression/syscall-sign-extend.c:21: (r = syscall(SYS_read, fd, buf, 1)) == 1 failed: No error information
1184:src/regression/syscall-sign-extend.c:23: read 1 instead of 0
1185:FAIL syscall_sign_extend [status 1]
1186:========== END entry-static.exe syscall_sign_extend ==========
```

动态用例同样失败，位于日志 `1623`–`1627` 行。

### 5.4 回归对比

| 指标 | 基线内核 | 修复版内核 |
|------|----------|------------|
| QEMU 退出码 | 0 | 0 |
| `START` / `END` 标记 | 281 / 281 | 281 / 281 |
| `Pass!` 行数 | 171 | 173 |
| `FAIL` 行数 | 46 | 44 |
| `testcase ... fail` 行数 | 1 | 1 |
| `syscall_sign_extend`（静态） | FAIL | Pass! |
| `syscall_sign_extend`（动态） | FAIL | Pass! |

`FAIL` 集合差异：

```diff
-FAIL syscall_sign_extend [status 1]
-FAIL syscall_sign_extend [status 1]
```

也就是说：**只少了两条目标用例失败，没有任何新增失败。**

完整清理后对比文件：

```text
/tmp/libctest-verify-angel/baseline-fail.txt
/tmp/libctest-verify-angel/fixed-fail.txt
/tmp/libctest-verify-angel/clean.diff
```

原始全文 diff 中还有一些 `gettimeofday` 数值、`test_yield` 子进程输出顺序、
busybox `sleep/kill` 条目位置互换等运行期噪声；其中 busybox 的 `success`/`fail`
计数在两份日志中完全一致，与本次 `/dev/zero` 改动无关。

### 5.5 当前二进制复查

基线构建后已重新构建修复版 `kernel-rv`。又用精简镜像复查当前二进制：

```text
/tmp/libctest-verify-angel/qemu-min-final.log
```

仍输出：

```text
========== START entry-static.exe syscall_sign_extend ==========
Pass!
========== END entry-static.exe syscall_sign_extend ==========
```

## 6. 如何查看这些测试结果

按以下路径直接核对：

```bash
# 源码改动
git diff -- kernel/fs/vfs/file.c

# 修复版完整日志中的目标用例
sed -n '1180,1184p;1619,1623p' /tmp/libctest-verify-angel/qemu-full.log | cat -v

# 基线完整日志中的目标用例
sed -n '1181,1187p;1622,1628p' /tmp/libctest-verify-angel/qemu-baseline.log | cat -v

# FAIL 集合差异（只需要目标用例两条消失）
diff -u \
  /tmp/libctest-verify-angel/baseline-fail.txt \
  /tmp/libctest-verify-angel/fixed-fail.txt || true

# 构建日志
tail -5 /tmp/libctest-verify-angel/build-fixed.log
tail -5 /tmp/libctest-verify-angel/build-base.log

# 当前 kernel-rv 复查
grep -a -A1 -B1 syscall_sign_extend \
  /tmp/libctest-verify-angel/qemu-min-final.log
```

`/tmp` 日志是本次验证留下的临时产物；若环境清理后需要复现，按第 4、5 节的构建和
QEMU 命令重跑即可。

## 7. 影响范围与注意事项

- 修改位于架构无关的 VFS 层文件 `kernel/fs/vfs/file.c`，因此 RISC-V 与 LoongArch
  重新编译后都会获得修复；但本次只构建并完整验证了 `kernel-rv`。
- `~/os_serial_out_la.txt` 是一次较早的 LoongArch 串口输出快照，没有更新。该文件记录
  的运行在 `iconv_open` 处 `panic: kerneltrap`，尚未执行到 `syscall_sign_extend`，
  因此不包含本次修复的验证证据，也不属于本次最小改动需要同步的产物。
- 快速调试时曾在 `/tmp` 的镜像副本里精简过测试脚本；最终完整回归使用全新镜像，仓库
  中的 `user/init-rv.c`、镜像内容和测试脚本均未改动。
- 本次改动未提交 commit；工作区包含 `kernel/fs/vfs/file.c` 和重建后的 `kernel-rv`。
