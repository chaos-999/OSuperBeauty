# 线上评测完整指南：从本地仓库到比赛提交

> 📅 生成时间：`2026-05-31 19:41 (GMT+8)`
>
> 目标：新手友好地解释线上评测做什么、SpringOS 当前差什么、需要改哪些文件、每一步为什么这么改

---

## 一、线上评测到底在做什么？

### 1.1 一句话版

评测机把你的内核和官方提供的 ext4 测试镜像一起启动，看你的内核能不能正确运行镜像里的测试程序，然后抓取屏幕输出打分。

### 1.2 完整时间线

```
评测机执行的操作                你的内核里发生的事情
───────────────                ──────────────────

① git clone 你的仓库
   过滤掉 .xxx 隐藏文件

② make all                    → 你的 Makefile 编译出 kernel-rv
                                  (ELF 格式内核)

③ qemu-system-riscv64 \
   -kernel kernel-rv \         → OpenSBI → boot/rv/entry.S
   -drive file={fs.img} ...       → boot/rv/start.c → boot/main.c
                                  ↓
                              内核初始化完成
                                  ↓
                              forkret() 里调用 filesystem_init()
                              挂载 ext4 磁盘镜像 ←── 评测机挂载的 fs.img
                                  ↓
                              启动第一个用户进程 init
                                  ↓
                              ★ init 扫描磁盘找 *_testcode.sh
                              ★ 逐个执行测试脚本
                              ★ 每次执行输出结果到屏幕
                                  ↓
                              全部跑完后调用 shutdown()
                                  ↓
④ QEMU 进程退出               ← 评测机检测到退出
                                  ↓
⑤ judge_basic.py 等脚本
   解析屏幕输出 → 生成分数
```

### 1.3 磁盘镜像里有什么？

评测机挂载的 ext4 镜像 `fs.img` 的结构（由 testsuits-for-oskernel 构建生成）：

```
fs.img (ext4, 无分区表, ~4GB)
/
├── glibc/                         ← glibc 编译的测试（目录）
│   ├── basic_testcode.sh          ← basic 组测试脚本
│   ├── busybox_testcode.sh        ← busybox 组测试脚本
│   ├── lua_testcode.sh            ← lua 组测试脚本
│   ├── ... 其他 *_testcode.sh ...
│   ├── busybox                    ← busybox ELF 二进制
│   ├── basic/                     ← basic 测试二进制目录
│   │   ├── run-all.sh
│   │   ├── brk       (测试 brk syscall)
│   │   ├── chdir     (测试 chdir)
│   │   ├── clone     (测试 clone)
│   │   ├── ... 共 32 个测试程序 ...
│   │   └── yield
│   ├── lua/                       ← Lua 解释器
│   ├── libc-test/                 ← libc 兼容性测试
│   └── lib/                       ← libc.so 等运行时库
│
└── musl/                          ← musl 编译的测试（目录）
    └── (相同结构，测试程序用 musl 编译)
```

### 1.4 测试脚本长什么样？

以 `glibc/basic_testcode.sh` 为例：

```bash
./busybox echo "#### OS COMP TEST GROUP START basic-glibc ####"
cd ./basic
./run-all.sh
cd ..
./busybox echo "#### OS COMP TEST GROUP END basic-glibc ####"
```

`run-all.sh` 遍历 32 个测试程序逐个执行：

```bash
#!/bin/sh
for i in brk chdir clone close ... yield; do
    echo "Testing $i :"
    ./$i
done
```

每个测试程序运行时会打印带格式的输出（如 `========== START test_brk ==========`），评测脚本靠正则匹配这些输出来打分。

---

## 二、SpringOS 当前能做什么、差什么

### 2.1 能做什么 ✅

| 评测要求 | 当前状态 |
|----------|---------|
| `make all` → 生成 `kernel-rv` + `kernel-la` | ✅ `Makefile:560` 已实现 |
| 内核引导（OpenSBI → 入口 → 初始化） | ✅ `boot/rv/entry.S` → `main.c` 正常 |
| 挂载 ext4 磁盘 | ✅ `forkret()` → `filesystem_init()` → `vfs_ext_mount()` |
| 读取目录内容（getdents64） | ✅ `kernel/syscall/sysfile.c:801` 已实现 |
| ~90 个 Linux 兼容 syscall | ✅ `include/syscall/syscall.h` 定义完整 |
| 执行用户程序（execve） | ✅ `kernel/proc/exec.c` |
| 输出测试标记格式 | ✅ 代码里有 `#### OS COMP TEST GROUP START ... ####` |
| 关机命令 | ✅ `shutdown()` syscall |

### 2.2 差什么 ❌

#### 问题 1：init 跑的是现场赛模式，不是初赛模式

```c
// user/init-rv.c main() 当前末尾：
    test_on_site();   // ← 只跑 git 测试，比赛用不上
    shutdown();

// test_pre()   ← 初赛 basic 测试逻辑，被注释掉了
// test_final() ← 互操作测试，也被注释掉了
```

#### 问题 2：硬编码测试名称，不能自适应

`test_pre()` 用了一个写死的数组 `basic_name[]` 列出 32 个测试名。如果测试集增加或改名，这就不对了。比赛要求**动态扫描磁盘**找到所有测试脚本然后执行。

#### 问题 3：没有扫描 `*_testcode.sh` 的逻辑

比赛要的是：扫描磁盘 → 找到所有 `xxxxx_testcode.sh` → 用 `busybox sh` 执行。当前代码是直接 exec 单个测试二进制，不通过脚本。

#### 问题 4：Makefile 缺少 `-no-reboot`

当前 RISC-V QEMU 选项没有 `-no-reboot`，内核 panic/shutdown 后 QEMU 可能重启而不是退出，导致评测超时。

```makefile
# 当前 QEMUOPTS_RV (Makefile 631行)：
QEMUOPTS_RV = -machine virt -bios default -kernel kernel-rv -m 1G -smp $(CPUS) -nographic
QEMUOPTS_RV += -drive file=sdcard-rv.img,if=none,format=raw,id=x0
QEMUOPTS_RV += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS_RV += -rtc base=utc
# ← 缺 -no-reboot
```

---

## 三、需要改哪些文件

一共 **2 个文件**需要改动，核心工作量在 `init-rv.c`。

### 3.1 `user/init-rv.c` — 重写测试发现和执行逻辑（核心改动）

**改动目标：**

替换 `main()` 里的 `test_on_site()` 调用，改为动态扫描 `/glibc/` 和 `/musl/` 目录，找到所有 `*_testcode.sh` 脚本，逐个用 `busybox sh` 执行。

**为什么这么改：**

- 比赛要求"主动扫描磁盘，依次运行每个测试点"
- 比赛镜像结构固定：测试脚本在 `/glibc/` 和 `/musl/` 子目录
- 用 `getdents64()` 扫描目录 → 找到 `*_testcode.sh` → `execve("busybox", ...)` 执行
- 不硬编码测试名称，增加或减少测试组自动适应

**改动思路（伪代码）：**

```c
// 新增：扫描某个目录，执行其中所有 *_testcode.sh
void run_dir_testscripts(const char *dirpath) {
    int fd = openat(AT_FDCWD, dirpath, O_RDONLY, 0);
    if (fd < 0) return;  // 目录不存在就跳过

    // 用 getdents64 读取目录项，找出所有 *_testcode.sh
    char buf[4096];
    struct linux_dirent64 *de;
    int n;
    while ((n = getdents64(fd, (void*)buf, sizeof(buf))) > 0) {
        // 遍历每个目录项
        char *p = buf;
        while (p < buf + n) {
            de = (struct linux_dirent64 *)p;
            // 检查文件名是否以 "_testcode.sh" 结尾
            if (endswith(de->d_name, "_testcode.sh")) {
                // chdir 到脚本所在目录，用 busybox sh 执行
                chdir(dirpath);
                char *busybox = "busybox";
                char *argv[] = {busybox, "sh", de->d_name, NULL};
                int pid = fork();
                if (pid == 0) {
                    execve(busybox, argv, NULL);
                    exit(1);
                }
                wait(0);
            }
            p += de->d_reclen;
        }
    }
    close(fd);
}

int main() {
    // ... 原有的 console、/tmp 初始化代码 保持不变 ...
    
    // ★ 新增：扫描并运行测试脚本
    run_dir_testscripts("/glibc");
    run_dir_testscripts("/musl");

    printf("All tests completed, shutting down system...\n");
    shutdown();
    return 0;
}
```

**需要注意的技术细节：**

1. `getdents64(fd, buf, len)` 返回的 `linux_dirent64` 结构包含 `d_name`（文件名）和 `d_type`（文件类型）。只需要文件名，不需要递归进子目录。

2. `busybox` 的路径：chdir 到 `/glibc/` 后，`busybox` 就在当前目录（`./busybox`），`execve("busybox", ...)` 会正确找到。

3. `endswith()` 函数：当前 `user/ulib.c` 没有这个函数，可以自己写一个简单的：
   ```c
   int endswith(const char *s, const char *suffix) {
       int slen = strlen(s), sulen = strlen(suffix);
       return slen >= sulen && strcmp(s + slen - sulen, suffix) == 0;
   }
   ```

4. 关于 `execve` vs `exec`：当前 `ulib.c` 的 `exec(char *name)` 只接受一个参数。运行 busybox 需要传多个参数（`{"busybox", "sh", "script.sh"}`），所以必须用 `execve`。

### 3.2 `Makefile` — 加一行 `-no-reboot`

**位置：** 约 631 行，`QEMUOPTS_RV` 定义区

**改动：**

```makefile
# 在 QEMUOPTS_RV += -rtc base=utc 下面加一行：
QEMUOPTS_RV += -rtc base=utc
QEMUOPTS_RV += -no-reboot          # ← 新增这行
```

加上后，内核调用 `shutdown()` 或 `panic()` 后 QEMU 直接退出，评测机才能检测到进程结束。

> 这行影响的是你本地 `make qemu` 的行为。比赛评测用的是自己的 QEMU 命令（已经带 `-no-reboot`），所以这行**只是为了方便你本地调试**，不是线上必须的。

### 3.3 不需要改的

| 文件 | 原因 |
|------|------|
| `boot/main.c` | `filesystem_init()` 在 `forkret()` 中自动调用，已经能挂载 ext4 |
| `kernel/fs/` | ext4 + VFS 层完整，`getdents64` 已实现 |
| `kernel/syscall/` | 90 个 syscall 已注册，basic 测试所需的全覆盖 |
| `include/syscall/syscall.h` | syscall 编号已对标 Linux 5.10 |
| `kernel/trap/` | 信号 + trap 机制完整 |

---

## 四、完整评测链路走一遍

以 basic 测试为例，从开机到关机的每一步：

```
第 0 步：评测机准备
  ├── git clone 你的仓库 (过滤 .xxx 隐藏文件)
  ├── cd 你的仓库 && make all
  │   └── 生成 kernel-rv (ELF)
  └── 准备 fs.img (官方 ext4 测试镜像)

第 1 步：启动 QEMU
  qemu-system-riscv64
    -machine virt -bios default
    -kernel kernel-rv
    -m 256M -smp 4 -nographic
    -drive file=fs.img,...           ← 测试镜像挂到 /dev/vda
    -no-reboot
    -rtc base=utc

第 2 步：OpenSBI → 内核入口
  OpenSBI (bios default) 初始化硬件 →
  跳转到 0x80200000 (boot/rv/entry.S) →
  boot/rv/start.c (RISC-V 平台初始化) →
  boot/main.c (多核启动 + 内核子系统初始化)

第 3 步：首次调度 → forkret() → 文件系统挂载
  scheduler() 选择第一个进程 →
  forkret() 首次执行 →
    filesystem_init()
      └── 注册 ext4 文件系统
      └── fs_mount(&ext4_fs, ...)
          └── vfs_ext_mount()
              └── vfs_ext4_blockdev_create(ROOTDEV) ← 绑定 virtio 磁盘
              └── ext4_mount("/dev/vda", "/")        ← 挂载 ext4 到根目录
      └── 设置当前进程 cwd = "/"

第 4 步：加载 init 进程
  usertrapret() → 返回用户态 →
  initcode.S → exec("/init") → 执行 user/init-rv.c

第 5 步：init 执行测试（★ 你改的代码在这）
  main():
    ├── openat("console") / dup(0) / dup(0)     ← 设置 stdio
    ├── mkdir("/tmp")
    │
    ├── run_dir_testscripts("/glibc")            ← ★ 新增
    │   ├── openat("/glibc", O_RDONLY)
    │   ├── getdents64(fd, buf, 4096)
    │   │   → 找到: basic_testcode.sh
    │   │   → 找到: busybox_testcode.sh
    │   │   → ...
    │   │
    │   ├── chdir("/glibc")
    │   ├── fork() → execve("busybox", {"busybox","sh","basic_testcode.sh"})
    │   │   └── basic_testcode.sh 内部：
    │   │       busybox echo "#### OS COMP TEST GROUP START basic-glibc ####"
    │   │       cd ./basic && ./run-all.sh
    │   │         ├── ./brk   → 屏幕输出 "========== START test_brk ========== ..."
    │   │         ├── ./chdir → ...
    │   │         └── ./yield → ...
    │   │       busybox echo "#### OS COMP TEST GROUP END basic-glibc ####"
    │   ├── wait(0)  ← 等脚本执行完
    │   │
    │   ├── fork() → execve("busybox", {"busybox","sh","busybox_testcode.sh"})
    │   ├── wait(0)
    │   └── ... (lua_testcode.sh 等，如果有的话)
    │
    ├── run_dir_testscripts("/musl")             ← ★ 新增（同上）
    │
    └── shutdown()                                ← ★ 全部完成后关机

第 6 步：QEMU 退出 → 评测
  shutdown() → sbi_shutdown() → QEMU 进程退出
  judge_basic.py 解析串口输出 → 正则匹配 → JSON 分数
```

### 关键路径对照

| 事件 | 哪个文件负责 |
|------|-------------|
| 内核编译 | `Makefile` → `all: kernel-rv kernel-la` |
| 内核入口 | `boot/rv/entry.S` |
| 初始化 + 启动 | `boot/main.c` |
| ext4 挂载 | `kernel/proc/proc.c` (forkret) → `kernel/fs/vfs/fs.c` (filesystem_init) |
| 测试发现 | `user/init-rv.c` (run_dir_testscripts) ← **要改的** |
| 测试执行 | `user/init-rv.c` + busybox + 测试脚本 |
| 关机 | `user/init-rv.c` 调用 `shutdown()` |

---

## 五、怎么在本地验证？（测试镜像暂缺时的替代方案）

### 5.1 如果你有测试镜像

```bash
cd ~/my-springos
make kernel-rv
qemu-system-riscv64 \
  -machine virt -bios default \
  -kernel kernel-rv -m 256M -smp 4 -nographic \
  -drive file=fs.img,if=none,format=raw,id=x0 \
  -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
  -no-reboot -rtc base=utc
```

### 5.2 如果你还没有测试镜像（当前情况）

可以先用 SpringOS 自己的 `make qemu-sh` 验证内核能正常启动和挂载 ext4：

```bash
cd ~/my-springos
make qemu-sh     # 用 fs-rv.img（xv6 自带的文件系统 + shell）
```

看到 shell 提示符说明内核和文件系统正常。

### 5.3 最小化 basic 测试（不需要完整镜像）

也可以创建一个小 ext4 镜像，自己编译几个 basic 测试程序放进去验证。你已有 `riscv64-linux-gnu-gcc` 和 `~/testsuits-for-oskernel/basic/user/` 里的源码。

---

## 六、常见问题

### Q：比赛评测的 QEMU 版本和我不一样怎么办？

比赛用 QEMU 9.2.1，你本地是 6.2.0。主要区别在新增的设备和参数，但基本的 `-machine virt` + `-kernel` + `-drive` 逻辑是一样的。内核本身不感知 QEMU 版本。

### Q：myspringos 里看到有其他 team 的代码（摄像头OS、xv6-labs），会不会冲突？

不会。`my-springos` 是独立目录，`make` 命令在其中执行，不会引用其他目录的代码。比赛也是 `cd 你的仓库 && make all`。

### Q：如果 busybox 跑不起来，basic 测试还能过吗？

如果你用动态扫描方式，init 会尝试执行 `basic_testcode.sh`，这会用到 busybox。如果 busybox 因为某些 syscall 缺失而崩溃，脚本就跑不完。

**备选方案：** 可以在 `run_dir_testscripts()` 里加一个 fallback——如果 busybox 执行失败，退回到直接执行 `/glibc/basic/` 下的测试二进制。这样即使 busybox 出问题，basic 测试还是能跑。

### Q：测试脚本执行顺序重要吗？

不重要。官方原文：*"测试点的执行顺序与评分无关"*。只要串行跑完就行。

### Q：必须跑 `musl` 两套吗？

测评镜像有 glibc 和 musl 两套，但你**可以选择只跑一套**（只跑 glibc 的）。没跑的测试不计分，但需要在输出中打印 `#### OS COMP TEST GROUP START basic-musl ####` 和 `END` 标记。

当前设计是两套都跑，会分别计分。

---

## 七、改动清单汇总

| # | 文件 | 改什么 | 优先级 |
|---|------|--------|--------|
| 1 | `user/init-rv.c` | 替换 `main()` 中 `test_on_site()` 为动态扫描 `run_dir_testscripts("/glibc")` + `/musl` | 🔴 必须 |
| 2 | `Makefile` | `QEMUOPTS_RV` 后加 `-no-reboot`（方便本地调试） | 🟡 建议 |

改完后的效果：
- `make all` 编译 → 生成 `kernel-rv` / `kernel-la`
- 评测机挂载 `fs.img` 启动 QEMU → 内核自动扫描测试脚本 → 执行 → 输出结果 → 关机
- 本地 `make qemu` 也能正常启动（只要你准备了磁盘镜像）

---

> **总结一句话：** 就是把 init 里写死的 `test_on_site()`（只跑 git 测试）换成动态扫描 `/glibc/`、`/musl/` 下所有 `*_testcode.sh` 然后 `busybox sh` 执行。代码改动不到 50 行。
