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

## 3. 为什么之前的判断是误诊

之前把问题归因于“16 块缓存太小 -> shake 频繁 -> LRU 树被 use-after-free”，并建议改成 512。
这个结论不成立：

1. `CONFIG_JOURNALING_ENABLE=0`，journal 回调路径根本不参与，`jbd_trans_end_write`
   不是嫌疑路径。
2. 改成 512 后 panic 消失，但测试仍在同一逻辑区域卡死，说明只是改变了时序和症状。
3. `ext4_buf_lru_RB_REMOVE_COLOR` 的偏移只能证明“树已经坏了”，不能证明“是谁弄坏的”。
   真正原因在更上层：多个进程同时在操作这棵树。

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
