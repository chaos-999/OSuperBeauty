# RV ext4 并发重入导致 LRU 红黑树损坏

> 定位 `CONFIG_BLOCK_DEV_CACHE_SIZE` 从 512 改回 16 后，busybox 测试稳定出现的
> `ext4_buf_lru_RB_REMOVE_COLOR` 崩溃/静默卡死。
>
> 结论：**不是缓存容量问题，也不是 journal 问题。根因是 lwext4 的挂载点锁从未安装，
> 多进程并发进入 ext4，破坏块缓存的 LRU 红黑树。**

---

## 1. 结论摘要

- `kernel/fs/lwext4/ext4.c` 里的 `EXT4_MP_LOCK()/EXT4_MP_UNLOCK()` 只有在
  `mp->os_locks` 非空时才生效。
- 仓库里 `ext4_mount_setup_locks()` **只有定义，没有任何调用者**，所以这些锁实际全是空操作。
- 本内核在 timer 中断的 `kerneltrap()` 中会 `yield()`，块 I/O 也会睡眠，因此多个进程可以
  同时进入 ext4。
- ext4 的块缓存 `lba_root` / `lru_root` 两棵红黑树没有任何并发保护：
  - `ext4_bcache_find_get()` 中的 `RB_REMOVE(lru)`
  - `ext4_bcache_free()` 中的 `RB_INSERT(lru)`
- 并发下的双重 `RB_REMOVE` / 交错 `RB_INSERT` 会打坏 LRU 树；后续
  `ext4_buf_lru_RB_REMOVE_COLOR` 访问到非法 NULL 兄弟节点，表现为 Load Page Fault；
  如果树成环，则表现为静默卡死。

所以：

- `CONFIG_BLOCK_DEV_CACHE_SIZE=16` 只是让 shake/LRU 操作更频繁，**更容易命中竞态窗口**；
- 改成 512 只是把“立即崩溃”变成“卡死/更晚崩溃”，**没有修复问题**；
- 真正修复是：**串行化所有 ext4 操作**。

---

## 2. 现象

配置：

```c
// include/fs/ext4/ext4_config.h
#define CONFIG_BLOCK_DEV_CACHE_SIZE 16
```

本地 QEMU（`-smp 1`）复跑时，busybox 组出现两类失败：

### 2.1 panic：LRU 红黑树空指针

```text
scause=0xd sepc=0x80216796 stval=0x70
panic: kerneltrap
```

反汇编：

```asm
80216794:   ld a5,96(a1)     # a5 = elm->lru_node.rbe_right
80216796:   lw a3,112(a5)    # a3 = a5->lru_node.rbe_color
```

`struct ext4_buf` 偏移：

```text
lru_node = 88
lru_node.rbe_left   = 88
lru_node.rbe_right  = 96
lru_node.rbe_parent = 104
lru_node.rbe_color  = 112
```

`stval=0x70=112`，说明 `a5` 为 `NULL`，`RB_REMOVE_COLOR` 把 NULL 当成了合法兄弟节点。
旧日志中的 `stval=0x58=88` 同理，是 `lru_node.rbe_left` 偏移，不是“缓存容量”的证据。

### 2.2 静默卡死

同样代码在 512 配置或不同时序下可能不 panic，而是停在：

```text
testcase busybox sort test.txt | ./busybox uniq success
...
testcase busybox find -name "busybox_cmd.txt" success
```

之后无输出、无 panic。树被破坏成环时，`RB_MIN` / `RB_NEXT` 遍历会死循环。

---

## 3. 问题发现脉络与误诊复盘

这一节按时间顺序记录“一开始以为什么、后来如何被证据推翻、最后如何定位”。

### 3.1 起点：沿用根目录 MISDIAGNOSIS-REPORT 和文档 37 的结论

最初的判断来自两份材料：

- 根目录 `MISDIAGNOSIS-REPORT.md`
- `mydocs/37-ext4-lru-use-after-free.md`

当时的结论是：

1. `CONFIG_BLOCK_DEV_CACHE_SIZE=16` 太小；
2. `ext4_block_cache_shake()` 频繁淘汰；
3. `ext4_block_flush_buf()` 里的 journal `end_write` 回调在 shake 期间重入；
4. 引用计数记账失衡，导致已释放的 `ext4_buf` 仍留在 LRU 树里；
5. 建议把缓存从 16 改成 512 “止血”。

这个结论看起来很完整，但它是从“崩溃点在 LRU 树”反推出来的，**没有先验证代码路径是否真的存在**。

### 3.2 第一个反证：journal 是关闭的

先检查编译开关：

```bash
grep -n 'CONFIG_JOURNALING_ENABLE' include/fs/ext4/ext4_config.h
# #define CONFIG_JOURNALING_ENABLE 0
```

`CONFIG_JOURNALING_ENABLE=0`，意味着：

- journal 模块不参与；
- `jbd_trans_end_write()` 这条 `end_write` 回调路径根本不会执行；
- 把 journal 重入列为首要嫌疑是**事实性错误**。

同时检查：

```bash
grep -n 'CONFIG_DEBUG_ASSERT\|CONFIG_HAVE_OWN_ASSERT' include/fs/ext4/ext4_config.h
# 1
# 1
```

`ext4_assert()` 是启用的。如果 refcount 断言真的被违反，日志里应该先看到
`assertion failed`，但实际日志中没有。这进一步削弱了“refcount 记账失衡触发断言”的说法。

### 3.3 第二个反证：调大缓存并没有修复问题

实际把 `CONFIG_BLOCK_DEV_CACHE_SIZE` 从 16 改成 512 后：

- panic 确实消失了；
- 但测试仍然在 `sort test.txt | ./busybox uniq` 附近卡死；
- 失败形态在 panic / hang 之间漂移；
- 失败测试组在 musl / glibc 之间漂移。

这说明“缓存容量”不是根因，只是改变了内存布局和时序。
如果根因真是“shake 太频繁”，扩大 32 倍缓存应该显著降低失败概率，而不是换一种症状继续 100% 失败。

### 3.4 第三个反证：`stval=0x58/0x70` 只能证明树坏了

旧分析用：

```text
stval=0x58 = offsetof(ext4_buf, lru_node)
```

来证明“LRU 树被 shake 破坏”。但结构体偏移只能说明**崩溃时访问的是 LRU 节点**，
不能说明是谁、在哪一步把树弄坏的。

这次本地复现拿到了新的崩溃点：

```text
scause=0xd sepc=0x80216796 stval=0x70
```

反汇编：

```asm
80216794:   ld a5,96(a1)     # a5 = elm->lru_node.rbe_right
80216796:   lw a3,112(a5)    # a3 = a5->lru_node.rbe_color
```

`lru_node.rbe_color` 的偏移正好是 112（0x70），`stval=0x70` 说明 `a5` 是 NULL。
也就是 `RB_REMOVE_COLOR` 把一个 NULL 兄弟节点当成合法节点解引用了。

这是“树已经损坏”的现场，仍然不是“谁弄坏树”的证明。

### 3.5 转向：为什么同一逻辑点会 panic/hang 漂移？

接下来不再从“缓存大小”入手，而是问：

> 为什么同样的代码、同样的命令，有时 panic，有时卡死，有时甚至跑到更后面？

这种“同点不同形态”更像：

- 内存破坏；
- 并发竞态；
- 时序相关 bug。

于是开始检查内核是否真的允许 ext4 并发执行。

### 3.6 关键发现一：内核态可以被 timer 中断抢占

`kernel/trap/rv/trap.c`：

```c
if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();
```

timer 中断在 `kerneltrap()` 中也会让出 CPU。
同时 virtio 块 I/O 会 `sleep`，所以进程 A 在 ext4 操作中间被切走、进程 B 继续进入 ext4，
是完全可发生的。

### 3.7 关键发现二：ext4 挂载锁从未安装

`kernel/fs/lwext4/ext4.c` 中所有公共 API 都写了：

```c
EXT4_MP_LOCK(mp);
...
EXT4_MP_UNLOCK(mp);
```

但宏的实现是：

```c
if ((_m)->os_locks)
    (_m)->os_locks->lock();
```

而 `ext4_mount_setup_locks()` 只有定义，没有调用者：

```bash
grep -rn 'ext4_mount_setup_locks' kernel include
# 只有定义
```

也就是说：**所有 ext4 公共 API 实际上都是无锁的。**

### 3.8 关键发现三：块缓存 LRU 树本身没有并发保护

`kernel/fs/lwext4/ext4_bcache.c`：

```c
if (!buf->refctr) {
    buf->lru_id = ++bc->lru_ctr;
    RB_REMOVE(ext4_buf_lru, &bc->lru_root, buf);   // 无锁
    ...
}
ext4_bcache_inc_ref(buf);
```

以及：

```c
if (!buf->refctr) {
    RB_INSERT(ext4_buf_lru, &bc->lru_root, buf);   // 无锁
    ...
}
```

典型竞态：

```text
进程 A: find_get(buf) 看到 refctr == 0
进程 A: RB_REMOVE(lru, buf)
        <-- timer 中断，yield
进程 B: find_get(buf) 仍看到 refctr == 0
进程 B: 再次 RB_REMOVE(lru, buf)        // 同一节点被删两次
```

或者两个进程同时 `ext4_bcache_free()`，同时 `RB_INSERT`，旋转/重平衡交错。
这正好可以解释后续 `RB_REMOVE_COLOR` 的 NULL 兄弟节点，以及树成环后的 hang。

### 3.9 决定性证据：重入检测抓到两个不同进程同时进入 ext4

为了确认“并发重入”不是猜测，在 ext4 公共 API 入口临时加了检测钩子：

```c
static volatile struct proc *ext4_dbg_owner;

static void ext4_dbg_mp_lock(void)
{
    struct proc *me = myproc();
    if (ext4_dbg_owner)
        printf("EXT4_MP_REENTRY: dbg_owner=%p now=%p\n",
               (void *)ext4_dbg_owner, (void *)me);
    ext4_dbg_owner = me;
}
```

实际串口输出：

```text
EXT4_MP_REENTRY: dbg_owner=0x0000000080288308 now=0x0000000080288e60
EXT4_MP_REENTRY: dbg_owner=0x0000000080288308 now=0x0000000080286c58
...
```

出现位置包括：

- `sh -c 'sleep 5' & ./busybox kill $!`
- `sort test.txt | ./busybox uniq`
- `cat test.txt` / `cut -c 3 test.txt` 等管道命令

这些 `struct proc *` 指针不同，说明**父子进程/管道进程确实在同时进入 lwext4**。

### 3.10 额外发现：fstat/statx 绕过锁且泄漏 inode 引用

继续审查所有 ext4 入口时发现：

```c
int vfs_ext_fstat(struct file *f, struct kstat *st)
{
    ...
    int r = ext4_fs_get_inode_ref(&file->mp->fs, file->inode, &ref);
    ...
    return EOK;   // 没有 ext4_fs_put_inode_ref(&ref)
}
```

`vfs_ext_statx()` 同样：

- 直接调用低层 `ext4_fs_get_inode_ref()`，不经过 `EXT4_MP_LOCK`；
- 永不调用 `ext4_fs_put_inode_ref()`，导致 inode table 缓存块引用泄漏。

在 16 块缓存下，这会长期钉住缓存块，压缩可用缓存，使 shake 更频繁，
进一步放大并发竞态窗口。这也能解释“跳过 basic 测试集时问题有时不出现”：
basic 测试里的 `fstat` 等调用改变了缓存压力与时序。

### 3.11 最终定位

把所有证据串起来：

1. ext4 挂载锁从未安装，`EXT4_MP_LOCK` 是空操作；
2. 内核态 timer 中断会 `yield()`，块 I/O 会睡眠；
3. 多个进程可同时进入 lwext4；
4. `ext4_bcache_find_get()` / `ext4_bcache_free()` 对 LRU 红黑树的
   `RB_REMOVE` / `RB_INSERT` 没有并发保护；
5. LRU 树被双重删除或交错旋转打坏；
6. 后续 `RB_REMOVE_COLOR` 访问 NULL 兄弟节点 → panic；
   树成环 → hang；
7. `CONFIG_BLOCK_DEV_CACHE_SIZE=16` 只是让 shake/LRU 操作更频繁，
   更容易命中竞态窗口，不是根因。

### 3.12 这一轮排查中的错误清单

| 错误判断 | 为什么错 | 如何被推翻 |
|----------|----------|------------|
| 16 块缓存太小导致 LRU 树损坏 | 512 仍然失败，只是症状变成 hang | 512 复跑对照 |
| journal `end_write` 在 shake 中重入 | `CONFIG_JOURNALING_ENABLE=0` | 查编译开关 |
| 断言没生效、refcount 错误被静默吞掉 | `CONFIG_DEBUG_ASSERT=1`，日志无 assertion failed | 查配置和日志 |
| `stval=0x58` 证明 shake 打坏树 | 只证明树已坏，不能证明破坏者 | 新崩溃点 0x70 + 反汇编 |
| 先考虑优化缓存/淘汰策略 | 真正问题是并发，不是容量 | 重入检测日志 |
| 以为只是单线程 refcount 记账问题 | 单线程路径无法解释同点 panic/hang 漂移 | 两个不同进程的 `EXT4_MP_REENTRY` |

最终修复见第 7 节。

---

## 4. 并发根源

### 4.1 挂载点锁从未安装

`kernel/fs/lwext4/ext4.c`：

```c
#define EXT4_MP_LOCK(_m)                    \
    do {                                    \
        if ((_m)->os_locks)                 \
            (_m)->os_locks->lock();         \
    } while (0)
```

`ext4_mount_setup_locks()` 在 `ext4.c:675` 中定义，但修复前全仓库没有调用点：

```bash
grep -rn 'ext4_mount_setup_locks' kernel include
# 只有定义
```

因此 `ext4_fopen2`、`ext4_fread`、`ext4_fwrite`、`ext4_dir_entry_next`、
`ext4_rename` 等所有带 `EXT4_MP_LOCK` 的 API，实际上都没有锁。

### 4.2 内核态可被抢占

`kernel/trap/rv/trap.c:220`：

```c
if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();
```

timer 中断发生在内核态时也会让出 CPU。此外 virtio 块 I/O 会 `sleep`。
所以进程 A 在 ext4 操作中间被切走、进程 B 继续进入 ext4，是正常可发生的。

### 4.3 块缓存 LRU 树没有并发保护

`kernel/fs/lwext4/ext4_bcache.c:217`：

```c
struct ext4_buf *
ext4_bcache_find_get(struct ext4_bcache *bc, struct ext4_block *b, uint64_t lba)
{
    struct ext4_buf *buf = ext4_buf_lookup(bc, lba);
    if (buf) {
        if (!buf->refctr) {
            buf->lru_id = ++bc->lru_ctr;
            RB_REMOVE(ext4_buf_lru, &bc->lru_root, buf);   // 无锁
            if (ext4_bcache_test_flag(buf, BC_DIRTY))
                ext4_bcache_remove_dirty_node(bc, buf);
        }
        ext4_bcache_inc_ref(buf);
        ...
    }
}
```

`ext4_bcache.c:307`：

```c
if (!buf->refctr) {
    RB_INSERT(ext4_buf_lru, &bc->lru_root, buf);           // 无锁
    ...
}
```

典型竞态：

```text
进程 A: find_get(buf) 看到 refctr == 0
进程 A: RB_REMOVE(lru, buf)
         <-- timer 中断，yield
进程 B: find_get(buf) 仍看到 refctr == 0
进程 B: 再次 RB_REMOVE(lru, buf)       // 同一个节点被删两次
```

或者两个进程同时 `ext4_bcache_free()`，同时 `RB_INSERT`，旋转和重平衡交错：

```text
进程 A: RB_INSERT(lru, bufA)
         <-- 被抢占，树处于旋转中间状态
进程 B: RB_INSERT(lru, bufB)
```

两种情况都会让 `rbe_left/rbe_right/rbe_parent/rbe_color` 关系失效。
之后的 `RB_REMOVE_COLOR` 就会把 NULL 当合法节点解引用，或者在遍历时成环。

---

## 5. 重入检测实验

为确认“并发重入”不是猜测，临时在 ext4 公共 API 入口加了检测钩子
（只记录，不阻塞），记录当前进入 ext4 的 `struct proc *`：

```c
static volatile struct proc *ext4_dbg_owner;

static void ext4_dbg_mp_lock(void)
{
    struct proc *me = myproc();
    if (ext4_dbg_owner)
        printf("EXT4_MP_REENTRY: dbg_owner=%p now=%p\n",
               (void *)ext4_dbg_owner, (void *)me);
    ext4_dbg_owner = me;
}
```

实际串口输出：

```text
EXT4_MP_REENTRY: dbg_owner=0x0000000080288308 now=0x0000000080288e60
EXT4_MP_REENTRY: dbg_owner=0x0000000080288308 now=0x0000000080288e60
EXT4_MP_REENTRY: dbg_owner=0x0000000080288308 now=0x0000000080286c58
...
```

这些地址是不同进程的 `struct proc *`。出现位置包括：

- `sh -c 'sleep 5' & ./busybox kill $!`
- `sort test.txt | ./busybox uniq`
- `cat test.txt` / `cut -c 3 test.txt` 等管道命令

也就是说：**父子进程/管道进程确实在同时进入 lwext4**。
修复后不再出现该重入日志。

---

## 6. 第二个问题：fstat/statx 引用泄漏并绕过锁

`kernel/fs/VFS_ext.c` 中：

```c
int vfs_ext_fstat(struct file *f, struct kstat *st)
{
    ...
    int r = ext4_fs_get_inode_ref(&file->mp->fs, file->inode, &ref);
    ...
    return EOK;   // 没有 ext4_fs_put_inode_ref(&ref)
}
```

`vfs_ext_statx()` 同样。

问题有两层：

1. `ext4_fs_get_inode_ref()` 会通过 `ext4_block_get()` 增加 inode table 块的 `refctr`；
   不 put 就会永久泄漏该缓存块的引用。
2. `ext4_fs_get_inode_ref()` 是低层函数，不自己加 `EXT4_MP_LOCK`；VFS 这里直接调用，
   等于又开了一个无锁并发入口。

在 16 块缓存下，泄漏的 inode table 块会长期钉在缓存里，进一步压缩可用缓存，
让 `ext4_block_cache_shake()` 更频繁，放大并发竞态窗口。这也解释了：
**为什么跳过 basic 测试集时问题有时不出现**——basic 测试里的 `fstat` 等调用会制造泄漏和缓存压力，
改变后续 busybox 的 shake 频率与时序。

---

## 7. 解决方案

### 7.1 在 VFS 层安装 ext4 挂载锁

`kernel/fs/VFS_ext.c` 新增睡眠锁（ext4 操作可能跨磁盘 I/O，不能用自旋锁）：

```c
static struct sleeplock ext4_mount_sleeplock;

static void vfs_ext4_mp_lock(void)
{
    acquiresleep(&ext4_mount_sleeplock);
}

static void vfs_ext4_mp_unlock(void)
{
    releasesleep(&ext4_mount_sleeplock);
}

static const struct ext4_lock vfs_ext4_mp_locks = {
    .lock = vfs_ext4_mp_lock,
    .unlock = vfs_ext4_mp_unlock,
};

int vfs_ext4_init(void)
{
    initlock(&extlock, "ext4 fs");
    initsleeplock(&ext4_mount_sleeplock, "ext4 mount");
    ext4_device_unregister_all();
    ext4_init_mountpoints();
    return 0;
}
```

挂载成功后真正安装：

```c
r = ext4_mount(DEV_NAME, fs->path, false);
if (r != EOK) {
    ...
} else {
    r = ext4_mount_setup_locks(fs->path, &vfs_ext4_mp_locks);
    if (r != EOK) {
        printf("ext4_mount_setup_locks failed: %d\n", r);
        ext4_umount(fs->path);
        vfs_ext4_blockdev_destroy(vbdev);
        goto out;
    }
    ext4_get_sblock(fs->path, (struct ext4_sblock **)(&(fs->fs_data)));
}
```

`vfs_ext_umount()` 也用同一把锁串行化：

```c
acquiresleep(&ext4_mount_sleeplock);
r = ext4_umount(fs->path);
releasesleep(&ext4_mount_sleeplock);
```

### 7.2 修复 fstat/statx 泄漏

`vfs_ext_fstat()`：

```c
acquiresleep(&ext4_mount_sleeplock);
int r = ext4_fs_get_inode_ref(&file->mp->fs, file->inode, &ref);
if (r != EOK) {
    releasesleep(&ext4_mount_sleeplock);
    return -r;
}

... 读取 ref.inode 字段 ...

r = ext4_fs_put_inode_ref(&ref);
releasesleep(&ext4_mount_sleeplock);
if (r != EOK)
    return -r;
return EOK;
```

`vfs_ext_statx()` 同样处理。

同时把 `ext4_inode_get_size(sb, ...)` 改成仅在 `sb_ok` 时调用，避免
`ext4_get_sblock()` 失败时解引用 NULL。

### 7.3 修复 ext4 内部两处锁使用错误

这两处在“锁是空操作”时没有影响，但一旦安装真锁就会出问题：

`kernel/fs/lwext4/ext4.c:1501`，`ext4_ftruncate_no_lock()` 内部重复解锁：

```c
// 修复前
if (r != EOK) {
    EXT4_MP_UNLOCK(file->mp);
    return r;
}

// 修复后
if (r != EOK) {
    return r;       // 由 ext4_ftruncate() 统一解锁
}
```

`ext4_removexattr()` 错误路径误把 UNLOCK 写成 LOCK：

```c
// 修复前
if (r != EOK) {
    EXT4_MP_LOCK(mp);
    return r;
}

// 修复后
if (r != EOK) {
    EXT4_MP_UNLOCK(mp);
    return r;
}
```

---

## 8. 验证

配置保持初始值：

```c
// include/fs/ext4/ext4_config.h:128
#define CONFIG_BLOCK_DEV_CACHE_SIZE 16
```

编译：

```bash
make kernel-rv -j$(nproc)
```

QEMU 全量复跑：

```bash
qemu-system-riscv64 -machine virt -bios default -kernel kernel-rv \
  -m 1G -smp 1 -nographic \
  -drive file=sdcard-rv.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -rtc base=utc -no-reboot
```

结果：

```text
#### OS COMP TEST GROUP START basic-glibc ####
#### OS COMP TEST GROUP END   basic-glibc ####
#### OS COMP TEST GROUP START basic-musl ####
#### OS COMP TEST GROUP END   basic-musl ####
#### OS COMP TEST GROUP START busybox-musl ####
#### OS COMP TEST GROUP END   busybox-musl ####
#### OS COMP TEST GROUP START busybox-glibc ####
#### OS COMP TEST GROUP END   busybox-glibc ####
#### OS COMP TEST GROUP START libctest ####
#### OS COMP TEST GROUP END   libctest ####
#### OS COMP TEST GROUP START libctest ####
#### OS COMP TEST GROUP END   libctest ####

All tests completed, shutting down system...
Shutdown system call invoked, shutting down xv6...
```

全程：

- 无 `panic`
- 无 `scause`
- 无卡死
- 无 `EXT4_MP_REENTRY`

---

## 9. 经验总结

1. **先确认锁是否真的安装。** 有 `EXT4_MP_LOCK` 宏不代表有锁；`os_locks == NULL` 时它什么都不做。
2. **看到红黑树崩溃时，先查并发，再查容量。** 红黑树本身不会因为“缓存小”而自己坏掉。
3. **同一逻辑点 panic/hang 漂移，优先怀疑竞态或内存破坏。** 容量参数变化往往只是改变时序。
4. **低层 ext4 API 也可能绕过锁。** `ext4_fs_get_inode_ref()` 这类函数不自己加锁，VFS 直接调用时必须补锁并配对 put。
5. **sleep lock 用于文件系统整体操作，spinlock 不适合跨块 I/O 的临界区。**

---

## 10. 证据清单

```bash
# 1. 修复前：setup 只有定义，没有调用者
grep -rn 'ext4_mount_setup_locks' kernel include

# 2. 内核态 timer 中断会 yield
grep -n 'which_dev == 2' kernel/trap/rv/trap.c
# 151: usertrap
# 220: kerneltrap

# 3. LRU 树无锁操作
grep -n 'RB_REMOVE(ext4_buf_lru\|RB_INSERT(ext4_buf_lru' \
     kernel/fs/lwext4/ext4_bcache.c
# 173/227: RB_REMOVE
# 307:     RB_INSERT

# 4. 结构体偏移
# lru_node=88
# lru_node.rbe_left=88 / rbe_right=96 / rbe_parent=104 / rbe_color=112
# 与 stval=0x58 / 0x70 对应

# 5. 崩溃函数
grep -n 'ext4_buf_lru_RB_REMOVE_COLOR' kernel-rv.sym
# 8021678a ext4_buf_lru_RB_REMOVE_COLOR
```

**一句话总结：**
把“16 块缓存太小导致 LRU 树被 shake 打坏”的错误判断，修正为
“**ext4 锁从未安装，父子/管道进程并发重入 lwext4，并发 RB_REMOVE/RB_INSERT
破坏了共享 LRU 红黑树**”；修复方式是为所有 ext4 操作安装真正的睡眠锁，
并补上 fstat/statx 的 inode 引用释放。
