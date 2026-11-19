# `/dev/null` stat 修复：`libctest stat` 从假 PASS 变为真 PASS

> 记录 libc-test 用例 `stat` 的根因定位、最小修复与完整回归验证。
>
> 结论：**`kernel/fs/VFS_ext.c` 的 `vfs_ext_stat()` 原实现只会查 ext4 目录项；
> `/dev/null` 只在 `sys_openat()` 中被合成为虚拟文件，磁盘上没有对应 inode，
> 所以 `stat("/dev/null")` 返回 `-ENOENT`。**
>
> 本次只在 `vfs_ext_stat()` 中为 `/dev/null` 合成一个字符设备 `kstat`；
> `sys_openat()`、`sys_fstatat()`、镜像脚本、测试代码和其他内核文件均未改动。

---

## 1. 现象

`stat` 是 libc-test 的 `functional` 类用例。在原始 RISC-V 串口日志
`~/os_serial_out_rv.txt` 中，静态和动态两次运行都失败：

```text
941:========== START entry-static.exe stat ==========
942:src/functional/stat.c:27: stat("/dev/null",&st)==0 failed: errno = No such file or directory
943:FAIL stat [status 1]
944:========== END entry-static.exe stat ==========

1358:========== START entry-dynamic.exe stat ==========
1359:src/functional/stat.c:27: stat("/dev/null",&st)==0 failed: errno = No such file or directory
1360:FAIL stat [status 1]
1361:========== END entry-dynamic.exe stat ==========
```

该用例的相关源码为：

```c
if (TEST(stat(".",&st)==0, "errno = %s\n", strerror(errno))) {
    TEST(S_ISDIR(st.st_mode), "\n");
    TEST(st.st_nlink>0, "%ju\n", (uintmax_t)st.st_nlink);
    /* ... 时间断言 ... */
}

if (TEST(stat("/dev/null",&st)==0, "errno = %s\n", strerror(errno))) {
    TEST(S_ISCHR(st.st_mode), "\n");
}
```

因此要真正通过，必须同时满足：

1. `stat("/dev/null", &st) == 0`；
2. `S_ISCHR(st.st_mode)` 为真，即 `st_mode` 必须是字符设备类型。

早期文档
[`31-presentfailure-fix-guide.md`](31-presentfailure-fix-guide.md) 第 3 节、
[`30-libctest-classification-false-pass.md`](30-libctest-classification-false-pass.md)
和 [`29-p2p3p4-implementation-and-false-pass.md`](29-p2p3p4-implementation-and-false-pass.md)
都把它归为“虚拟设备节点只在 open 路径存在，stat 路径缺失”的问题。

## 2. 根因链条

### 2.1 musl 把 `stat()` 转换成 `fstatat()`

RISC-V 64 位 Linux ABI 下：

```text
stat("/dev/null", &st)
  → fstatat(AT_FDCWD, "/dev/null", &st, 0)   // syscall 79
```

`kernel/syscall/syscall.c` 中已注册：

```c
[SYS_fstatat] sys_fstatat,
```

所以问题不是 syscall 未实现。

### 2.2 `sys_fstatat()` 直接走 VFS/ext4 查询

`kernel/syscall/sysproc.c` 的 `sys_fstatat()` 关键路径：

```c
struct filesystem *fs = get_fs_from_path(pathname);
if (fs->type == EXT4) {
    struct kstat kbuf;
    const char *dirfd_path =
        (dirfd == AT_FDCWD) ? myproc()->cwd.path : myproc()->ofile[dirfd]->f_path;
    char absolute_path[MAXPATH];
    get_absolute_path(pathname, dirfd_path, absolute_path);

    int r = vfs_ext_stat(absolute_path, &kbuf);
    if (r < 0) {
        /* 只对末尾 '/' 做一次重试 */
        ...
        if (r < 0) return r;
    }
    copyout(myproc()->pagetable, st, (char *)(&kbuf), sizeof(kbuf));
}
```

对 `/dev/null` 来说：

```text
get_fs_from_path("/dev/null") → EXT4
absolute_path                 → "/dev/null"
vfs_ext_stat("/dev/null")     → ext4_raw_inode_fill("/dev/null") → -ENOENT
```

### 2.3 `/dev/null` 只存在于 open 特例里

`kernel/syscall/sysfile.c` 的 `sys_openat()` 有虚拟设备拦截：

```c
if (!strcmp(path, "/etc/localtime") || ... ||
    !strcmp(path, "/dev/null")) {
    if ((f = filealloc()) == NULL || (fd = fdalloc2(f, 0)) < 0) {
        return -1;
    }
    f->f_type = 9;              // 虚拟“零设备”文件
    f->f_pos = 0;
    f->f_flags = flags;
    strcpy(f->f_path, path);
    return fd;
}
```

但镜像里的 ext4 根目录并没有 `/dev`：

```text
debugfs -R 'ls -l /dev' sdcard-rv.img
/dev: File not found by ext2_lookup
```

所以 `open("/dev/null")` 能成功，而 `stat("/dev/null")` 查不到磁盘 inode。
这就是典型的“虚拟设备节点只在 open 路径存在，其他 VFS 操作缺失”。

## 3. 修复方案选择

文档 31 给出了两个方向：

| 方案 | 做法 | 取舍 |
|------|------|------|
| A | 在 `sys_fstatat()` 中拦截 `/dev/null` | 改动小，但只修 syscall 入口；`statx()` 等其他入口仍可能漏掉 |
| B | 用 `mknod` 在镜像里创建真实设备节点 | 规范，但依赖 ext4 设备 inode、镜像内容和更多 VFS 路径支持 |

本次没有直接照搬 A，而是放在 `vfs_ext_stat()` 里，原因是当前 VFS 层已经有同类虚拟路径特例：

```c
if (strcmp(statpath, "/sbin/ls") == 0) {
    return vfs_ext_stat("/musl/busybox", st);
}
if (strcmp(statpath, "/sbin/sleep") == 0) {
    return vfs_ext_stat("/musl/busybox", st);
}
```

把 `/dev/null` 的 metadata 合成放在这里，可以：

- 让 `fstatat()` 和其他调用 `vfs_ext_stat()` 的路径得到一致结果；
- 保留 `sys_fstatat()` 现有的路径解析、末尾 `/` 重试和 `copyout` 逻辑；
- 不改 `sys_openat()`，避免影响已通过的 open/read/write 用例；
- 不改镜像，不依赖评测方镜像里是否存在 `/dev` 目录。

## 4. 修复

### 4.1 改动位置

唯一源码改动在 `kernel/fs/VFS_ext.c` 的 `vfs_ext_stat()`，位置在符号链接解析之后、
ext4 inode 查询之前：

```diff
     statpath = resolved_stat;
 
+    /* /dev/null is synthesized by sys_openat() and has no ext4 inode.
+     * Report it as a character device so stat()/fstatat() can find it too. */
+    if (strcmp(statpath, "/dev/null") == 0) {
+        memset(st, 0, sizeof(*st));
+        st->st_dev = 1;
+        st->st_ino = 999;
+        st->st_mode = 0020000 | 0666; /* S_IFCHR | 0666 */
+        st->st_nlink = 1;
+        st->st_rdev = 0x0103;         /* major 1, minor 3 */
+        st->st_blksize = 4096;
+        return 0;
+    }
+
     if (strcmp(statpath, "/sbin/ls") == 0) {
         return vfs_ext_stat("/musl/busybox", st);
     }
```

### 4.2 字段含义

| 字段 | 值 | 原因 |
|------|----|------|
| `st_dev` | `1` | 设备号非零，与 ext4 使用的合成 `st_dev` 保持一致 |
| `st_ino` | `999` | 虚拟节点需要一个稳定、非零的 inode 号 |
| `st_mode` | `0020000 \| 0666` | Linux `S_IFCHR` 是八进制 `0020000`；加上 `0666` 权限后满足 `S_ISCHR()` |
| `st_nlink` | `1` | 合理的硬链接计数 |
| `st_rdev` | `0x0103` | 字符设备 major=1、minor=3，即 Linux `/dev/null` |
| `st_blksize` | `4096` | 与常见块大小一致 |
| 其他字段 | `0` | `memset()` 清零，避免栈上残留值 |

`struct kstat` 中 `st_mode` 在偏移 16 字节，现有 `copyout(..., sizeof(kbuf))`
足以覆盖 musl `struct stat` 前半部分的全部关键字段；因此不需要改动 `struct kstat`
定义或扩大 copyout 长度。

### 4.3 未改动的内容

以下文件/逻辑均保持原样：

```text
kernel/syscall/sysfile.c        # sys_openat() 的虚拟设备处理
kernel/syscall/sysproc.c        # sys_fstatat()
kernel/fs/vfs/file.c            # filestat() / filestatx()
user/init-rv.c / user/init-la.c # 最终状态未改动
sdcard-rv.img / 测试脚本         # 未改动
```

## 5. 编译与验证

### 5.1 编译

```bash
make kernel-rv -j4
```

通过。项目使用 `-Wall -Werror`，无编译告警。

二进制：

```text
上游 main 原始 kernel-rv SHA-256 前缀: 29c3f69dd0c54814fcd39ff9
本次重建 kernel-rv SHA-256 前缀:       04c09a1417ffeb6c2c49d3c9
```

注意：从旧分支切到上游 `main` 后必须先 `make clean` 再构建。直接增量构建会复用
旧架构/旧配置遗留的 `.o` 文件，得到的内核在启动阶段 `panic: kerneltrap`；全量重建
后与上游基线行为一致。

### 5.2 rebase 后定向单测

提交 PR 前把分支 rebase 到当前上游 `main` 后，上游的 `user/init-rv.c` 已经改成
**只运行 libcbench-musl**（`test_pre()` 在 `main()` 中被注释掉），因此无法再用普通完整
QEMU 日志证明 libctest `stat` 通过。为此临时修改 `user/init-rv.c`，让 init 只 `fork`+
`execve("entry-static.exe", {"entry-static.exe", "stat", NULL})` 并打印子进程退出码，
在 rebase 后的新内核上运行，结果：

```text
TEMP stat raw=2 exit=0
```

`exit=0` 即 `entry-static.exe stat` 子进程退出码为 0（用例通过），无 `panic`。先前的
旧基线单测也曾得到同样的 `exit=0`；两者共同证明本次源码补丁在 rebase 前后都生效。

验证完成后已从备份恢复 `user/init-rv.c`，并重新执行 `make clean` + 全量构建最终
`kernel-rv`；用 `strings kernel-rv | grep 'TEMP stat'` 复查无输出，确认临时测试代码
没有进入最终二进制，且哈希回到 `04c09a1417ffeb6c2c49d3c9`。

### 5.3 完整回归（rebase 前同源码补丁）

下面的完整 libctest 回归是在 rebase 前的旧基线上、使用与本次**完全相同**的
`vfs_ext_stat()` 源码补丁跑出来的。rebase 后由于上游 init 不再运行 libctest，
改用第 5.2 节的定向单测验证同一补丁，没有再跑一遍全量 libctest。

使用官方镜像解压副本，以 QEMU 完整运行：

```bash
qemu-system-riscv64 \
  -machine virt -bios default \
  -kernel kernel-rv \
  -m 1G -smp 1 -nographic \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -rtc base=utc -no-reboot
```

日志：`/tmp/full_after.log`，QEMU 退出码 `0`，正常关机。

静态用例：

```text
918:========== START entry-static.exe stat ==========
919:Pass!
920:========== END entry-static.exe stat ==========
```

动态用例：

```text
1332:========== START entry-dynamic.exe stat ==========
1333:Pass!
1334:========== END entry-dynamic.exe stat ==========
```

### 5.4 与提供的基线日志对比

| 指标 | `~/os_serial_out_rv.txt`（基线） | `/tmp/full_after.log`（修复后） |
|------|----------------------------------|--------------------------------|
| `START` / `END` 标记 | 281 / 281 | 281 / 281 |
| `Pass!` | 171 | 175 |
| `FAIL` 行数 | 46 | 42 |
| `stat`（静态） | FAIL | Pass! |
| `stat`（动态） | FAIL | Pass! |

`FAIL` 集合差异：

```diff
-FAIL stat [status 1]
-FAIL stat [status 1]
-FAIL syscall_sign_extend [status 1]
-FAIL syscall_sign_extend [status 1]
```

需要说明：

- 前两条 `stat` 失败消失是本次 `/dev/null` VFS 修复的直接结果；
- 后两条 `syscall_sign_extend` 失败消失来自当前 HEAD 已合入的
  [`50-dev-zero-read-syscall-sign-extend-fix.md`](50-dev-zero-read-syscall-sign-extend-fix.md)，
  不是本次修改造成的；
- 除上述四条外，其余失败集合保持一致，**没有新增失败用例**。

### 5.5 rebase 后内核整体回归

rebase 后全量重建的内核以第 5.3 节相同 QEMU 参数完整运行，日志
`/tmp/full_after_clean_rebase.log`：QEMU 退出码 `0`，输出 27 个 `time:` 标记
（libcbench 全部 benchmark），无 `panic`，最后正常关机：

```text
All tests completed, shutting down system...
Shutdown system call invoked, shutting down xv6...
```

说明本次源码补丁合入后内核整体行为正常，没有引入回归。

## 6. 如何复查

```bash
# 源码改动
git diff -- kernel/fs/VFS_ext.c

# 修复后完整日志中的 stat 用例
grep -a -n -B1 -A2 'entry-static.exe stat =' /tmp/full_after.log
grep -a -n -B1 -A2 'entry-dynamic.exe stat =' /tmp/full_after.log

# 原始日志中的失败
grep -a -n -B1 -A2 'entry-static.exe stat =' ~/os_serial_out_rv.txt

# 确认临时验证代码没有进入最终二进制
strings kernel-rv | grep 'TEMP stat' || echo 'clean'

# 当前工作区最终改动
git status --short
```

`/tmp/full_after.log` 是本次验证留下的临时产物；若环境清理后需要复现，按第 5.3 节的
构建和 QEMU 命令重跑即可。

## 7. 影响范围与注意事项

- 提交 PR 前已把分支 rebase 到当前上游 `main`。上游在 PR #3 之后新增了
  `51-libcbench-all-pass.md` 和 `52-libcbench-kernel-requirements.md`，因此本文在 PR
  中顺延为 `53-dev-null-stat-vfs-fix.md`；`kernel-rv` 也从上游 `main` 加本次源码修复
  重新构建，而不是沿用旧 `main` 上的二进制。
- 修改位于架构无关的 `kernel/fs/VFS_ext.c`，因此 RISC-V 与 LoongArch 重新编译后都会
  获得该修复；但本次只构建并完整验证了 `kernel-rv`。
- 本次只合成 `/dev/null` 的 stat 信息，没有把 `/dev/zero`、`/dev/console`、`/etc/localtime`
  等其他虚拟路径纳入特例，避免改变目前正常工作的目录/文件探测行为。
- `open("/dev/null")` 仍然使用 `sys_openat()` 原来的 `f_type == 9` 路径；`read()`/`write()`
  行为完全不变。
- 这不是在镜像中创建真实设备节点，因此 `ls -l /dev/null` 之类依赖目录项枚举的操作
  仍不会看到该节点；本次目标是修复 libc-test `stat` 用例，不引入镜像和 ext4 设备 inode
  的大范围改动。
