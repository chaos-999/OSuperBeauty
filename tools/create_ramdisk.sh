#!/bin/bash

# 创建 EXT4 RAMDisk 镜像并转换为 C 头文件

set -e  # 遇到错误立即退出

SRC_DIR="$1"           # 可选：指定要打包为 ramdisk 的源目录（递归拷贝）
IMG_SIZE=8              # 默认 8MB；若指定目录，将按目录实际大小自适应放大
IMG_FILE=ramdisk.img
HEADER_FILE=include/ramdisk_img.h
MNT_DIR=ramdisk_mnt

if [ -n "$SRC_DIR" ] && [ -d "$SRC_DIR" ]; then
  # 估算目录大小并加余量（+20% + 16MB 对齐），最小 8MB
  DU_MB=$(du -sm "$SRC_DIR" | awk '{print $1}')
  [ -z "$DU_MB" ] && DU_MB=1
  EXTRA_MB=$(( DU_MB / 5 + 16 ))
  IMG_SIZE=$(( DU_MB + EXTRA_MB ))
  if [ $IMG_SIZE -lt 8 ]; then IMG_SIZE=8; fi
  echo "[ramdisk] Source dir: $SRC_DIR (du=${DU_MB}MB) => image=${IMG_SIZE}MB"
else
  echo "[ramdisk] No source dir provided; fallback to auto-pick user/_*-rv binaries"
fi

echo "=== Creating ${IMG_SIZE}MB RAMDisk image for VF2 ==="

# 清理之前的文件
rm -f $IMG_FILE $HEADER_FILE
umount $MNT_DIR 2>/dev/null || true
rm -rf $MNT_DIR

# 1. 创建空镜像文件
echo "Step 1: Creating ${IMG_SIZE}MB disk image..."
dd if=/dev/zero of=$IMG_FILE bs=1M count=$IMG_SIZE

# 2. 格式化为 EXT4 (与 lwext4 兼容)
echo "Step 2: Formatting with EXT4..."
mkfs.ext4 -O ^metadata_csum -F -b 4096 -L ramdisk $IMG_FILE

# 3. 挂载并复制内容
echo "Step 3: Mounting and copying contents..."
mkdir -p $MNT_DIR
# Prefer loopback mount, fall back if needed
mount $IMG_FILE $MNT_DIR

if [ -n "$SRC_DIR" ] && [ -d "$SRC_DIR" ]; then
  echo "Copying directory tree from $SRC_DIR ..."
  if command -v rsync >/dev/null 2>&1; then
    rsync -a "$SRC_DIR"/ "$MNT_DIR"/
  else
    echo "[ramdisk] rsync not found, falling back to cp -a"
    cp -a "$SRC_DIR"/. "$MNT_DIR"/
  fi
else
  # 复制 README（可选）
  if [ -f "README" ]; then
      cp README $MNT_DIR/
  fi
  # 复制用户程序 (RISC-V 版本) 到根目录，去掉开头的下划线和 -rv 后缀
  echo "Copying user programs..."
  for prog in user/_*-rv; do
      if [ -f "$prog" ]; then
          name=$(basename "$prog" | sed 's/^_//' | sed 's/-rv$//')
          echo "  Copying $prog -> $name"
          cp "$prog" $MNT_DIR/"$name"
      fi
  done
fi

# 列出复制的顶层内容
echo "Files in ramdisk (top-level):"
ls -la $MNT_DIR/ || true

# 确保 /init 存在（initcode 会 exec "/init"）。优先使用 user/_init-rv，与 qemu 行为一致
if [ ! -f "$MNT_DIR/init" ]; then
  if [ -f "user/_init-rv" ]; then
    echo "Copying user/_init-rv -> /init"
    cp "user/_init-rv" "$MNT_DIR/init"
  elif [ -f "$MNT_DIR/sh" ]; then
    echo "Creating /init from /sh"
    cp "$MNT_DIR/sh" "$MNT_DIR/init"
  elif [ -f "$MNT_DIR/bin/sh" ]; then
    echo "Creating /init from /bin/sh"
    cp "$MNT_DIR/bin/sh" "$MNT_DIR/init"
  else
    echo "[WARN] /init not found and no sh available; system may fail to boot userland." >&2
  fi
fi

# 4. 卸载文件系统
echo "Step 4: Unmounting..."
umount $MNT_DIR
rm -rf $MNT_DIR

# 5. 保留原始镜像（用于 objcopy 二进制嵌入）
echo "Step 5: Keeping raw image for binary embedding..."
echo "=== RAMDisk creation completed ==="
echo "Generated image: $IMG_FILE (size: $(stat -c%s $IMG_FILE) bytes)"
echo "RAMDisk ready for VF2 build!"