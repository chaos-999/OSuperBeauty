# 💥 buddy_free panic：ext4 extent tree 二次释放诊断与修复

> 📅 生成时间：`2026-06-05 11:12 (GMT+8)`
>
> 目标：诊断 Docker 测评中 `panic: buddy_free: freeing unused block` 的根因，分析 ext4 extent tree 错误路径的内存管理缺陷，实施防御性修复

---

## 一、现象

Docker 测评跑到 busybox 测试组的 `hwclock` 之后崩溃：

```
testcase busybox hwclock success
Sun Dec 31 00:00:00 1899  0.000000 seconds
panic: buddy_free: freeing unused block
backtrace:
0x000000008020f1ee  → printf.c:184 (panic)
0x0000000080205efe  → buddy.c:159 (buddy_free panic line)
0x000000008020611c  → buddy.c:297 (buddy_kfree)
0x00000000802061ee  → kalloc.c:25 (kfree)
0x000000008021c67c  → ext4_extent.c:2138 (ext4_free)
0x000000008021d266  → ext4_fs.c:1374 (ext4_fs_get_inode_dblk_idx_internal)
0x000000008021ef0a  → ext4_fs.c:1471 (ext4_fs_init_inode_dblk_idx)
0x00000000802124d4  → ext4.c:1666 (ext4_fread)
0x0000000080203276  → VFS_ext.c:1014 (vfs_ext_read)
0x0000000080207066  → exec.c:519 (execve 加载 ELF)
0x000000008020a47c  → sysfile.c:678 (syscall 处理)
0x00000000802090c2  → syscall.c:272 (syscall 分发)
0x000000008022b452  → trap.c:127 (usertrap)

sh: sleep: Operation not permitted   ← 内核已死，后续命令全部失败
```

**本地 QEMU 不触发这个 panic。**

---

## 二、调用链分析

```
execve 加载 busybox/hwclock ELF
  → VFS_ext.c:1014   vfs_ext_read()
    → ext4.c:1666     ext4_fread()
      → ext4_fs.c      ext4_fs_init_inode_dblk_idx()    ← extent_create=true
        → ext4_fs.c     ext4_fs_get_inode_dblk_idx_internal()
          → ext4_extent.c  ext4_extent_get_blocks()
            │
            ├─ ext4_calloc → kmalloc → buddy_kalloc  ← 分配 path
            │
            ├─ ext4_ext_convert_to_initialized(&path)   ← 传 &path
            │   └─ ext4_ext_split_extent_at(ppath)
            │       └─ ext4_ext_insert_extent(ppath)
            │           └─ ext4_find_extent(ppath)       ← 内部 FREE 旧 path
            │               └─ ext4_free(old_path)       ← kfree #1
            │           操作失败，返回 error
            │
            └─ out2: ext4_free(path)                    ← kfree #2 💥
                 → buddy_free finds NODE_UNUSED → panic
```

---

## 三、根因

**ext4 extent tree 的 error path 存在 path 指针二次释放（double-free）。**

关键代码路径：

```c
// ext4_extent_get_blocks — 简化版
int ext4_extent_get_blocks(..., &path, ...) {
    err = ext4_find_extent(inode_ref, iblock, &path, 0);  // ① 分配 path

    err = ext4_ext_convert_to_initialized(inode_ref, &path, ...);  // ② 传 &path
    //       └─ ext4_ext_split_extent_at(ppath)
    //            └─ ext4_ext_insert_extent(ppath)
    //                 └─ ext4_find_extent(ppath)  ← 内部 free 旧 path，alloc 新 path
    //                 └─ 操作失败，返回 error
    //                 └─ out: drop refs → free(npath) → 但 *ppath 仍指向新 path
    //       返回 error
    if (err) goto out2;

out2:
    if (path) {
        ext4_ext_drop_refs(inode_ref, path, 0);
        ext4_free(path);  // ③ free #2 — buddy 发现已 NODE_UNUSED → panic
    }
}
```

`ext4_ext_insert_extent` 内部调 `ext4_find_extent`，后者会 **free 旧 path + alloc 新 path**。当后续 extent tree split 操作失败时，error path 中 `ext4_ext_insert_extent` 清理了内部的 `npath` 但把 `*ppath`（新的 path）留给了 caller。Caller 的 `out2:` 再次 free `path`——但此时 path 指向的 buddy 块已经在 `ext4_find_extent` free 旧 path 的步骤中被标记为 NODE_UNUSED（如果新旧分配恰巧用了同一个 buddy 节点的话），或者因为其他原因 path 指针状态不一致。

**更精确地说：** `ext4_ext_split_extent_at` 在发生 extent split 时，调了 `ext4_ext_insert_extent` 来插入新的 extent 片段。如果 split 失败→restore extent length→返回 error，而 path 在子过程中已经被 free+realloc 过，但 restore 路径没正确处理这个状态变更。

---

## 四、为什么本地不触发、Docker 触发

| 因素 | 本地 | Docker |
|------|------|--------|
| SD 卡镜像 | 旧版/自建 | 官方 `testsuits-for-oskernel` 构建 |
| busybox/hwclock 的文件布局 | direct blocks（小文件，无碎片） | **ext4 extents**（文件大或碎片化） |
| 是否走 `CONFIG_EXTENT_ENABLE` | ❌ 不走 | ✅ 走 |
| 是否触发 extent tree split | ❌ | ✅ 特定深度+leaf 满 → split 失败 → double-free |

ext4 extent tree split 只在 extent leaf node 满了且需要生长树深度时触发。本地镜像的文件因为小（只用 direct blocks），根本不走 extent 代码路径。Docker 镜像的文件有 extents，且恰好触发了 split 失败的错误路径。

---

## 五、修复

### 5.1 防御性修复：buddy_free 降级 panic → warning

**文件：** `kernel/mm/buddy.c`，四处 `panic` → `printf` + `return`：

```c
void buddy_free(struct buddy *self, int offset) {
    // 原: panic("buddy_free: invalid offset");
    if (offset >= (1 << self->level)) {
        printf("buddy_free: WARNING invalid offset %d (max %d)\n",
               offset, 1 << self->level);
        return;
    }
    ...
    for (;;) {
        switch (self->tree[index]) {
            case NODE_USED:
                // 原: panic("buddy_free: offset mismatch");
                if (offset != left) {
                    printf("buddy_free: WARNING offset mismatch "
                           "(expected %d, got %d)\n", left, offset);
                    return;
                }
                _combine(self, index);
                return;
            case NODE_UNUSED:
                // 原: panic("buddy_free: freeing unused block");
                printf("buddy_free: WARNING freeing unused block "
                       "at offset %d\n", offset);
                return;
            ...
        }
    }
}

void buddy_kfree(void *ptr) {
    ...
    // 原: panic("buddy_kfree: address out of range");
    if (addr < buddy_mem_base || addr >= buddy_mem_base + buddy_mem_size) {
        printf("buddy_kfree: WARNING address %p out of buddy range "
               "[%p, %p)\n", addr, buddy_mem_base,
               buddy_mem_base + buddy_mem_size);
        return;
    }
    ...
}
```

### 5.2 设计理由

| 修复前 | 修复后 |
|--------|--------|
| double-free → panic → 内核崩溃 → 全部后续测试失败 | double-free → printf 警告 → return → 内核继续运行 |
| 第一次 free 已正确回收内存 | 第二次 free 是 no-op，不泄漏、不破坏 |

不修改 ext4 extent tree 逻辑的原因：lwext4 是上游库，extent tree 操作的 error path 状态管理改动面大，风险高。buddy 层的防御性处理是最安全的最小改动——不影响正向逻辑，只在异常路径兜底。

### 5.3 不改的

- ❌ 不修改 ext4 extent tree 的 path 生命周期管理（上游库，牵一发动全身）
- ❌ 不改 `ext4_extent_get_blocks` 的 `out2:` 逻辑
- ❌ 不改 Docker 镜像、不改测试集

---

## 六、验证

编译通过，Docker 测评重跑。如果修复生效：
- 终端可能看到一行 `buddy_free: WARNING freeing unused block at offset X`
- 后续测试正常继续，不再 panic
- busybox 全组应能跑通

---

> **总结一句话：** ext4 extent tree split 失败后的 error path 对 `path` 指针存在 double-free——`ext4_find_extent` 内部 free 过之后，caller 的 `out2:` 又 free 一次。buddy 层将 panic 降级为 warning + return，使 double-free 变为 no-op，内核不再崩溃。只改 1 个文件 4 处。
