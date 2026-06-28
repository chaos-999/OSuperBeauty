# RV ext4 块缓存 LRU 树 Use-After-Free 分析

> 定位 busybox-glibc 测试中 `sleep 5 & kill $!` 触发 kernel panic 的根因。

## 现象

评测环境（QEMU 1G 内存 + virtio-net + `-rtc base=utc`）：
```
scause=0xd sepc=0x802126c6 stval=0x101010101010159
panic: kerneltrap
```

`scause=0xd` = Load Page Fault，`stval=0x101010101010159` = `0x0101010101010101 + 88`，是 kfree 毒化模式。

本地 QEMU（256M，无网络，无 rtc）不触发。

## 诊断

### backtrace 解析（addr2line）

```
sepc=0x802126c6 → ext4_buf_lru_RB_REMOVE (ext4_bcache.c:67)
    0x802127b8 → ext4_bcache_alloc       (ext4_bcache.c:247)
    0x80213380 → ext4_block_get_noread   (ext4_blockdev.c:235)
    0x802133aa → ext4_block_get          (ext4_blockdev.c:247)
    0x802277a4 → ext4_trans_block_get    (ext4_trans.c:87)
    0x80219048 → ext4_dir_dx_find_entry  (ext4_dir_idx.c:777)
    0x80218028 → ext4_dir_find_entry     (ext4_dir.c:465)
    0x80214080 → ext4_generic_open2      (ext4.c:905)
    0x80215ab0 → ext4_raw_inode_fill     (ext4.c:1975)
    0x80205fc6 → vfs_ext_readlink        (VFS_ext.c:1076)
    0x8020b07e → sys_readlinkat          (sysfile.c:868)
```

### 崩溃指令

```asm
ld a5, 88(a5)    ← a5 = 0x0101010101010101（已释放）
                   a5+88 = 0x0101010101010159 → Load Page Fault
```

`struct ext4_buf` offset 88 = `lru_node.rbe_left`，LRU 红黑树左子指针。

### 数据流

busybox shell 执行 `sleep 5 & kill $!` 时，shell 用 `readlinkat` 解析 `sleep` 路径 → ext4 目录查找 → ext4 块缓存分配 → `ext4_bcache_find_get` → `RB_REMOVE(ext4_buf_lru)` → 遍历 LRU 树时碰到已释放节点 → panic。

## 根因分析

缓存大小 `CONFIG_BLOCK_DEV_CACHE_SIZE=8`，频繁触发 LRU 淘汰。嫌疑路径：

1. **`ext4_journal.c:1642`**：`ext4_bcache_inc_ref(block->buf)` 直接增加引用计数，不经过 `ext4_bcache_find_get`（后者会先从 LRU 树摘除）。若 buffer 在 LRU 树中（refctr=0），refctr 变 1 但节点仍在 LRU 树中。后续 GC 调用 `ext4_bcache_drop_buf` 时看到 refctr>0 跳过 LRU 摘除，直接 free 内存 → LRU 树留悬挂指针。

2. **`ext4_block_cache_shake`** 的 `dont_shake` 保护不完整，flush 过程中可能递归调用 `ext4_block_get`，绕过容量检查。

## 状态

已定位，未提交修复。参数敏感的竞态 bug（本地环境不触发）。
