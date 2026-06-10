# 30 -- libc-test 全测试分类 & 假 PASS 归因 & 修复难度分析

> 一份覆盖所有 96 个当前运行测试的分类手册。告诉你每类测试在测什么、哪些是假的 PASS、以及修好它们各自要花多少力气。

---

## 目录

1. [全景:96 个测试的六大分类](#1-全景96-个测试的六大分类)
2. [分类详解](#2-分类详解)
3. [假 PASS 归因:9 个骗子是谁](#3-假-pass-归因9-个骗子是谁)
4. [未运行的测试](#4-未运行的测试)
5. [修复难度分析](#5-修复难度分析)
6. [建议进攻路线](#6-建议进攻路线)

---

## 1. 全景:96 个测试的六大分类

libc-test 不是 96 个随机测试--它们按测试目标可以清晰地分成六类。理解这些分类后,你就知道每个测试在考内核的什么能力。

```
96 个测试
├── 🧮 纯算法/数据 (35)  - 不依赖任何 syscall,纯 C 标准库函数
├── 📋 格式化 I/O (12)   - printf/scanf 系列,间接依赖 TLS/浮点格式化
├── 🔤 字符/编码 (11)   - locale、宽字符、编码转换
├── 📁 文件 I/O (8)     - 文件操作 syscall:open/read/write/lseek/stat
├── 🔧 进程/系统 (8)    - 进程生命周期、信号、资源限制
├── 🌐 网络 (1)         - socket/bind/listen/accept/connect...
└── 🧵 pthread (0)      - 全部排除(会卡死),共约 11 个
```

**为什么这么分?** 因为不同分类的测试对内核的要求完全不同:
- 🧮 纯算法类只要 C 编译器正确 + 栈正常就行--根本不调 syscall
- 📋 格式化类需要 TLS 正确初始化(之前 AT_PHDR 修复正是为此)
- 📁 文件类需要 ext4 文件系统 + VFS 层完整
- 🌐 网络类需要完整的 socket 协议栈

### 各类的真/假 PASS 分布

| 分类 | 数量 | 真 PASS | 假 PASS | 假 PASS 率 |
|------|------|---------|---------|-----------|
| 🧮 纯算法/数据 | 35 | 35 | 0 | 0% |
| 📋 格式化 I/O | 12 | 12 | 0 | 0% |
| 🔤 字符/编码 | 11 | 11 | 0 | 0% |
| 📁 文件 I/O | 8 | 2 | 6 | **75%** |
| 🔧 进程/系统 | 8 | 5 | 3 | **38%** |
| 🌐 网络 | 1 | 0 | 1 | 100% |

> **-ENOSYS 修复已应用:** socket 和 statvfs 的 errno 已从 EPERM 修正为 ENOSYS。daemon_failure、rlimit_open_files、utime 的 EPERM 来自已实现 syscall 的 `return -1`,不是未知 syscall 路径的问题。
| 🧵 pthread | 0 | - | - | - |

**结论:假 PASS 全部集中在 📁 文件 I/O、🔧 进程/系统、🌐 网络这三类--恰好都是需要内核 syscall 支持的分类。**

---

## 2. 分类详解

### 🧮 纯算法/数据(35 个,全部真 PASS)

这些测试只验证 C 标准库的纯计算能力--字符串操作、内存操作、搜索排序、数学函数。它们**完全不调 syscall**,只要程序能正常加载执行就默认通过。

| 测试名 | 测什么 |
|--------|--------|
| argv | 命令行参数传递(argc/argv) |
| basename | 路径名 basename 提取 |
| dirname | 路径名 dirname 提取 |
| env | 环境变量读取(getenv) |
| qsort | 快速排序 |
| random | 随机数生成 |
| search_hsearch | 哈希表搜索(hsearch) |
| search_insque | 链表插入(insque/remque) |
| search_lsearch | 线性搜索(lsearch) |
| search_tsearch | 二叉搜索树(tsearch) |
| setjmp | 非局部跳转(setjmp/longjmp) |
| memstream | 内存流(open_memstream) |
| udiv | 整数除法 |
| tgmath | 泛型数学宏 |
| inet_pton | IP 地址字符串转二进制 |
| string | 字符串函数综合 |
| string_memcpy | memcpy 边界情况 |
| string_memmem | memmem 子串搜索 |
| string_memset | memset 边界情况 |
| string_strchr | strchr 边界情况 |
| string_strcspn | strcspn 边界情况 |
| string_strstr | strstr 子串搜索 |
| inet_ntop_v4mapped | IPv4-mapped IPv6 地址转换 |
| inet_pton_empty_last_field | inet_pton 边界情况 |
| iswspace_null | iswspace(NULL) 行为 |
| lrand48_signextend | lrand48 符号扩展 |
| malloc_0 | malloc(0) 行为 |
| mbsrtowcs_overflow | 多字节到宽字符转换溢出 |
| memmem_oob_read | memmem 越界读 |
| memmem_oob | memmem 越界 |
| putenv_doublefree | putenv 双重释放 |
| fflush_exit | fflush 在 exit 时的行为 |
| fgets_eof | fgets 遇到 EOF |
| fgetwc_buffering | fgetwc 缓冲行为 |
| setvbuf_unget | setvbuf 与 ungetc 交互 |

> ✅ 这 35 个全部是真 PASS -- 不存在假 PASS。

**为什么它们全过了?** 因为前期修复了 AT_PHDR(TLS 初始化),glibc/musl 的核心数据结构能正常初始化。这些纯算法测试不碰文件系统、不创建线程、不做网络通信--内核只需要做好 ELF 加载就够了。

---

### 📋 格式化 I/O(12 个,全部真 PASS)

printf/scanf 系列函数。它们自己不怎么调 syscall,但内部依赖 TLS(浮点格式化缓冲区)和 locale 数据。

| 测试名 | 测什么 |
|--------|--------|
| snprintf | 格式化字符串拼接 |
| sscanf | 格式化字符串解析 |
| sscanf_long | sscanf 长整数解析 |
| sscanf_eof | sscanf EOF 处理 |
| swprintf | 宽字符格式化输出 |
| strftime | 时间格式化 |
| printf_1e9_oob | printf 大数越界 |
| printf_fmt_g_round | printf %g 舍入 |
| printf_fmt_g_zeros | printf %g 零值 |
| printf_fmt_n | printf %n 格式化 |
| scanf_bytes_consumed | scanf 字节计数 |
| scanf_match_literal_eof | scanf 字面量匹配 EOF |
| scanf_nullbyte_char | scanf 空字节处理 |

> ✅ 全部真 PASS。格式化 I/O 路径经过 AT_PHDR 修复后已完全正常工作。

---

### 🔤 字符/编码(11 个,全部真 PASS)

locale、宽字符、编码转换等。部分依赖 `/usr/share/locale` 数据(不存在时回退默认行为,测试允许这种回退)。

| 测试名 | 测什么 |
|--------|--------|
| clocale_mbfuncs | C locale 下的多字节函数 |
| mbc | 多字节字符 |
| fnmatch | 文件名通配匹配 |
| fscanf | 格式化文件输入 |
| fwscanf | 宽字符格式化文件输入 |
| iconv_open | 字符编码转换器打开 |
| iconv_roundtrips | 编码转换往返测试 |
| uselocale_0 | uselocale 空指针行为 |
| wcsstr | 宽字符串子串搜索 |
| wcstol | 宽字符串转长整数 |
| strverscmp | 版本号字符串比较 |

> ✅ 全部真 PASS。

---

### 📁 文件 I/O(8 个,5 个假 PASS 🔴)

这些测试直接依赖内核的**文件系统 syscall**:open、read、write、lseek、stat、fstat、futimens、mkstemp 等。

| 测试名 | 测什么 | 状态 | 假 PASS 原因 |
|--------|--------|------|-------------|
| fdopen | fdopen 文件描述符转 FILE* | ✅ 真 PASS | - |
| rewind_clear_error | rewind 清除错误标志 | ✅ 真 PASS | - |
| mkdtemp_failure | mkdtemp 失败路径 | ✅ 真 PASS | - |
| **stat** | stat() 获取文件信息 | ⚠️ 假 PASS | /dev/null 不存在 |
| **utime** | futimens 修改文件时间戳 | ⚠️ 假 PASS | futimens 已注册但返回 -1 → EPERM |
| **ftello_unflushed_append** | fopen/fwrite/fflush 追加写 | ⚠️ 假 PASS | mkstemp 临时文件创建失败 |
| **lseek_large** | lseek 大偏移量(>2GB) | ⚠️ 假 PASS | lseek 返回值 32 位有符号,溢出 |
| **statvfs** | statvfs 文件系统信息 | ⚠️ 假 PASS ✅ errno 已修正 | syscall 43 未实现,ENOSYS 正确 |
| **socket** | socket 网络通信 | ⚠️ 假 PASS ❌→✅ | 8 个网络 syscall 未实现,errno 已修正为 ENOSYS |

**这个分类假 PASS 率 75%**(6/8)--文件系统是最复杂的子系统,缺失的功能点最多。其中 socket 和 statvfs 的 errno 已借 -ENOSYS 修复得到修正。

> ⚠️ **socket 归在文件 I/O 类而非网络类**,因为当前测试列表中网络测试只有它一个,且其失败症状与文件 I/O 测试同源(syscall 未实现)。

---

### 🔧 进程/系统(8 个,3 个假 PASS 🔴)

这些测试依赖于**进程生命周期管理**(fork/exit/wait)、**信号**、**资源限制**(rlimit)等系统能力。

| 测试名 | 测什么 | 状态 | 假 PASS 原因 |
|--------|--------|------|-------------|
| clock_gettime | 获取系统时钟 | ✅ 真 PASS | - |
| time | time() 获取时间戳 | ✅ 真 PASS | - |
| tls_align | TLS 对齐检查 | ✅ 真 PASS | - |
| sigprocmask_internal | sigprocmask 信号掩码 | ✅ 真 PASS | - |
| ungetc | ungetc 字符回退 | ✅ 真 PASS | - |
| **daemon_failure** | daemon() 失败路径 | ⚠️ 假 PASS | 期望 EMFILE(24) 但收到 EPERM(1) |
| **rlimit_open_files** | rlimit 文件数限制 | ⚠️ 假 PASS | dup 超出限制时期望 EMFILE 但收到 EPERM |
| **syscall_sign_extend** | syscall 参数符号扩展 | ⚠️ 假 PASS | read 返回值被符号扩展破坏 |

**注意:** daemon_failure 和 rlimit_open_files 的 EPERM 问题**不是它们自身 syscall 的实现问题**,而是调用链中某个未实现 syscall 返回了 `-1`(→EPERM),污染了 errno。修复 `-1 → -ENOSYS` 后,这两个测试可能变成真 PASS。

---

### 🌐 网络(1 个,1 个假 PASS 🔴)

| 测试名 | 测什么 | 状态 | 假 PASS 原因 |
|--------|--------|------|-------------|
| **socket** | socket/bind/listen/accept/connect/sendto/recvfrom | ⚠️ 假 PASS | 8 个网络 syscall 全部未实现 |

所有网络 syscall(198,200,201,202,203,204,206,207,208)全部返回 `-1`(→EPERM)。

---

### 边界/回归测试(其余 ~20 个,全部真 PASS)

这些是 libc-test 中针对特定 libc bug 的回归测试,覆盖各种边界情况。它们多数不依赖 syscall:

| 测试名 | 测什么 |
|--------|--------|
| dn_expand_empty | dn_expand 空输入 |
| dn_expand_ptr_0 | dn_expand 指针偏移 |
| fpclassify_invalid_ld80 | fpclassify 非法 long double |
| getpwnam_r_crash | getpwnam_r 崩溃回归 |
| getpwnam_r_errno | getpwnam_r errno 回归 |
| mkstemp_failure | mkstemp 失败路径 |
| regex_backref_0 | 正则表达式反向引用 |
| regex_bracket_icase | 正则 bracket 大小写 |
| regex_ere_backref | ERE 反向引用 |
| regex_escaped_high_byte | 正则转义高字节 |
| regex_negated_range | 正则否定范围 |
| regexec_nosub | regexec 无子匹配 |
| strtod / strtod_simple | strtod 浮点解析 |
| strtof | strtof 浮点解析 |
| strtol / strtold | 字符串转整数/长双精度 |
| strptime | 字符串解析时间 |
| wcsncpy_read_overflow | wcsncpy 越界读 |
| wcsstr_false_negative | wcsstr 假阴性回归 |

> ✅ 全部真 PASS。

---

## 3. 假 PASS 归因:9 个骗子是谁

### 按根因分组(-ENOSYS 修复后更新)

> 🔧 **-ENOSYS 修复已应用:** `kernel/syscall/syscall.c` 未知 syscall 路径从 `return -1` 改为 `return -ENOSYS`。socket 和 statvfs 的 errno 已从 EPERM 修正为 ENOSYS。

```
9 个假 PASS
├── 🐛 已修复:未知 syscall 返回 ENOSYS (2) ✅ 已解决
│   ├── socket         ← 8 个网络 syscall,现在正确返回 ENOSYS ✅
│   └── statvfs        ← syscall 43(statfs),现在正确返回 ENOSYS ✅
│
├── 🐛 已实现 syscall 返回 -1 而非正确 errno (3) ← 新发现!
│   ├── daemon_failure ← 预期 EMFILE(24),某已实现 syscall 返回 -1 → EPERM
│   ├── rlimit_open_files ← 预期 EMFILE(24),dup/open 返回 -1 而非 -EMFILE
│   └── utime          ← futimens 已注册但实现返回 -1 → EPERM
│
├── 📁 文件功能缺失 (3)
│   ├── stat           ← /dev/null 设备节点不存在
│   ├── ftello_unflushed_append ← mkstemp 临时文件创建失败
│   └── lseek_large    ← lseek 返回值 int 类型溢出
│
└── 🔧 syscall 参数处理 (1)
    └── syscall_sign_extend ← syscall 返回值符号扩展 bug
```

### 详细分析

#### 1. socket(网络)✅ -ENOSYS 修复已生效

```
最新输出:8 次 "unknown sys call 198/200/201/202/203/204/206/207/208"
      errno 已从 "Operation not permitted" (EPERM)
      变为 "Function not implemented" (ENOSYS) ✅

根因:全部 8 个网络 syscall 未注册,但 errno 现在正确了
待修:需完整实现网络栈才能让测试通过

涉及 syscall:socket(198), bind(200), listen(201), accept(202),
             connect(203), getsockname(204), sendto(206),
             recvfrom(207), setsockopt(208)

状态:仍为假 PASS(syscall 未实现,但 errno 已修正)
```

#### 2. statvfs(文件系统信息)✅ -ENOSYS 修复已生效

```
最新输出:statvfs("/") 返回 "Function not implemented" (ENOSYS) ✅
     errno="Function not implemented" 替代了之前的 "Operation not permitted"

根因:syscall 43 (statfs/fstatfs) 未实现,但 errno 现在正确了
状态:仍为假 PASS(syscall 未实现,但 errno 已修正)
```

#### 3. daemon_failure(daemon 失败路径)🔴 新发现:已实现 syscall 返回 -1

```
最新输出:daemon should have failed with 24 [EMFILE] got 1 [Operation not permitted]

根因(更新):-ENOSYS 修复后仍报 EPERM。
     这说明 daemon() 调用链中的某个 syscall 不是"未实现",
     而是已注册但返回了 -1(→EPERM)。
     daemon() 会依次调:fork → setsid → chdir → close 所有 fd → dup2
     问题很可能在 close/dup2 在超出界限时返回 -1 而非 -EBADF
     和/或 getrlimit/setrlimit 未实现(且返回 -1)
```

#### 4. rlimit_open_files(文件数限制)🔴 新发现:dup 返回 -1 而非 -EMFILE

```
最新输出:dup(1) failed with Operation not permitted, wanted EMFILE
     "more fds are open than rlimit allows: fd=127, limit=42"

根因(更新):-ENOSYS 修复后仍报 EPERM。
     说明内核的 open/dup 在文件描述符耗尽时返回的是 -1,
     而非 -EMFILE(24)。rlimit 检查本身是存在的(limit=42 被正确打印),
     但返回值写错了。

待修:open/dup 在超过 rlimit 时应 return -EMFILE 而非 -1
```

#### 5. utime(文件时间戳)🔴 新发现:futimens 已注册但返回 -1

```
最新输出:futimens(-1, ...) 返回 "Operation not permitted" (EPERM)

根因(更新):futimens syscall 已在 syscalls[] 表中注册(不是未知 syscall),
     但其实现函数返回 -1 而非正确的 errno。
     这说明 futimens 的入口函数存在,但内部参数校验或实现返回了 -1。

待修:futimens 实现中所有 return -1 应改为 return -EBADF/-EINVAL 等正确 errno
```

#### 6. stat(文件信息)

```
症状:stat("/dev/null", &st) 失败,errno=ENOENT

根因:文件系统里没有 /dev/null 这个设备节点
     不是 syscall 实现问题,是文件系统内容问题
```

#### 6. utime(文件时间戳)

```
症状:futimens(-1, UTIME_OMIT) 返回 -1, errno=EPERM
     后续 stat 发现时间戳为 0(应该是非零值)

根因:futimens/utimensat syscall 未完整实现
     当前可能只做了参数校验,没有实际更新时间戳
```

#### 7. ftello_unflushed_append(追加写 + 定位)

```
症状:open(tmp_file, O_WRONLY) 失败,errno=ENOENT
     → ftello 返回 -1 → fflush/fclose 都失败

根因:mkstemp/mkdtemp 创建的临时文件路径问题
     可能 /tmp 目录不存在或临时文件命名机制不完整
```

#### 8. lseek_large(大文件偏移)

```
症状:lseek(fd, 0x80000000, SEEK_SET) → 0xffffffffffffffff (应为正值)
     lseek(fd, 0x100000001, SEEK_SET) → 0x1 (丢失高位)

根因:lseek 返回值类型是 int(有符号 32 位)
     0x80000000 = 2^31,被当作负数符号扩展到 64 位
     需要改为 off_t(64 位有符号类型)
```

#### 9. syscall_sign_extend(参数符号扩展)

```
症状:syscall(SYS_read, fd, buf, 1) 返回 1 而不是 0
     "read 1 instead of 0"

根因:syscall() 函数在传递参数时,
     32 位到 64 位的符号扩展处理有 bug
     某个本应作为 unsigned 的参数被当作 signed 扩展了
```

---

## 4. 未运行的测试

### 已排除的测试

当前 `init-rv.c` 的测试列表中**没有包含任何 pthread 测试**,因为它们在当前内核上会卡死(HANG),阻塞整个测试流程。

已确认排除的测试(约 11 个):

| 测试 | 类型 | 卡死原因 |
|------|------|---------|
| pthread_cancel | functional | CLONE_SIGHAND 已修,但可能还需要 futex 同步完成 |
| pthread_cancel_points | functional | 同上 |
| pthread_cond | functional | 依赖 futex + 信号投递 |
| pthread_tsd | functional | 依赖完整的 TLS 析构 |
| pthread_robust_detach | regression | 同上 |
| pthread_cancel_sem_wait | regression | 同上 |
| pthread_cond_smasher | regression | 同上 |
| pthread_condattr_setclock | regression | 同上 |
| pthread_exit_cancel | regression | 同上 |
| pthread_once_deadlock | regression | 同上 |
| pthread_rwlock_ebusy | regression | 同上 |

### 可能遗漏的测试

libc-test 标准套件还有少量测试(如 `pthread_atfork`、`pthread_self` 等)也未包含。这些可以逐步补充到测试列表中。

---

## 5. 修复难度分析

### 难度评分标准

| 星级 | 含义 | 时间估计 |
|------|------|---------|
| ★ | 一行改动,立即见效 | <5 分钟 |
| ★★ | 一个函数或局部改动 | 30 分钟 ~ 2 小时 |
| ★★★ | 需要理解子系统,改多处 | 2~4 小时 |
| ★★★★ | 需要设计新子系统 | 4~8 小时 |
| ★★★★★ | 完整的子系统实现 | 1~2 天 |

### 逐个分析

#### ✅ -ENOSYS 修复(已完成 ✅,之前影响 2 个假 PASS)

| 难度 | ★ |
|------|---|
| 位置 | `kernel/syscall/syscall.c:272` |
| 改动 | `p->trapframe->a0 = -1` → `p->trapframe->a0 = -ENOSYS` |
| 效果 | socket → EPERM→ENOSYS ✅ / statvfs → EPERM→ENOSYS ✅ |
| 状态 | **已修复,errno 已正确** |

---

#### 🔴 已实现 syscall 返回 -1 而非正确 errno(影响 3 个假 PASS)← 新发现

| 难度 | ★★ |
|------|-----|
| 位置 | `kernel/syscall/sysfile.c`(open/dup/futimens 等),`kernel/syscall/sysproc.c`(getrlimit) |
| 改动 | 在已实现 syscall 中将 `return -1` 替换为正确 errno(-EMFILE/-EBADF/-EINVAL) |
| 影响 | daemon_failure → EPERM→EMFILE / rlimit_open_files → EPERM→EMFILE / utime → EPERM→EBADF |

> **-ENOSYS 修复后,剩余的 EPERM 全部来自已实现 syscall 的 `return -1`。** 需要用文档 25(errno 修复)的方法,逐个排查 daemon() 调用链、dup/open 边界检查、futimens 实现中的返回值。

---

#### lseek 64 位偏移

| 难度 | ★★ |
|------|-----|
| 位置 | `kernel/fs/VFS_ext.c` vfs_ext_lseek + sysfile.c lseek syscall |
| 改动 | 返回值类型从 `int` 改为 `int64`(或 `off_t`),确保 64 位偏移量不透传符号扩展 |
| 影响 | lseek_large → 真 PASS |

**为什么是 ★★ 而非 ★?** 改返回值类型可能影响调用链上的函数签名,需要全局搜索确认没有其他地方依赖 `int` 返回值。

---

#### /dev/null 设备节点

| 难度 | ★★ |
|------|-----|
| 位置 | 用户态 init-rv.c 或内核设备初始化 |
| 改动 | 创建 `/dev/null` 设备节点(可以是一个丢弃所有写入、读取返回 EOF 的简单设备) |
| 影响 | stat → 真 PASS |

**方案:** 在 init 程序中用 `mknod` 创建,或在内核中注册一个简单的字符设备。xv6 的 console.c 是一个不错的参考--`/dev/null` 比 console 更简单(读返回 0,写丢弃数据返回 n)。

---

#### syscall 参数符号扩展

| 难度 | ★★ |
|------|-----|
| 位置 | `kernel/syscall/syscall.c` 的 `argint`/`argaddr` 或用户态 `syscall()` 函数 |
| 改动 | 检查 32→64 位参数传递时是否有无符号→有符号的错误扩展 |
| 影响 | syscall_sign_extend → 真 PASS |

**需要排查的点:** `argint()` 返回的是 `int`(32 位有符号),而有些参数应该是 `unsigned int`。检查是否有参数被错误地符号扩展。

---

#### mkstemp 临时文件

| 难度 | ★★★ |
|------|------|
| 位置 | 内核文件系统层 + init-rv.c |
| 改动 | 确保 `/tmp` 目录存在且可写,mkstemp 路径拼装正确 |
| 影响 | ftello_unflushed_append → 真 PASS |

**排查方向:** 先确认 `/tmp` 目录是否存在、是否有写权限。然后检查 mkstemp 的模板路径拼装是否正确(musl 的 mkstemp 依赖 `getpid`+随机数+`open(O_CREAT|O_EXCL)`)。

---

#### futimens/utimensat

| 难度 | ★★★ |
|------|------|
| 位置 | `kernel/syscall/sysfile.c` |
| 改动 | 完整实现 futimens/utimensat:解析 timespec 参数、更新 inode 的 atime/mtime |
| 影响 | utime → 真 PASS |

**核心逻辑:**
```c
// futimens(fd, times[2])
// times[0] = access time, times[1] = modification time
// UTIME_NOW → 设为当前时间
// UTIME_OMIT → 保持不变
// 需要 ext4 层支持设置 inode 时间戳
```

---

#### 文件描述符 limit / rlimit

| 难度 | ★★★ |
|------|------|
| 位置 | `kernel/syscall/sysfile.c`(open/dup)+ `kernel/proc/proc.c`(rlimit 存储) |
| 改动 | 1) 在 proc 中存储 rlimit 值 2) open/dup 时检查是否超限 3) 返回 -EMFILE 而非 -1 |
| 影响 | rlimit_open_files → 真 PASS + daemon_failure 可能受益 |

**注意:** daemon_failure 的 EMFILE vs EPERM 问题可能部分来自缺少 rlimit 检查,部分来自 -1→EPERM 污染。先修 -ENOSYS,再判断还需要什么。

---

#### 网络栈(socket/bind/listen/accept/connect/sendto/recvfrom)

| 难度 | ★★★★★ |
|------|--------|
| 位置 | 新建 `kernel/fs/socket.c` |
| 改动 | 实现 9 个网络 syscall(仅需本地回环 127.0.0.1) |
| 影响 | socket → 真 PASS |

虽然难度最高,但 libc-test 的 socket 测试只需要**本地回环**(数据在内核内存中传递,不经过网卡)。不需要完整的 TCP/IP 协议栈。详见文档 28 模块四。

---

### 难度总览

| 假 PASS 测试 | 难度 | 最快路线 |
|-------------|------|---------|
| socket + statvfs | ✅ 已完成 | -1 → -ENOSYS(errno 已修正) |
| daemon_failure + rlimit_open_files + utime | ★★ | 已实现 syscall 中 return -1 → 正确 errno |
| lseek_large | ★★ | 改返回值类型为 int64 |
| stat | ★★ | 创建 /dev/null 节点 |
| syscall_sign_extend | ★★ | 检查 argint/argaddr 符号处理 |
| ftello_unflushed_append | ★★★ | 确保 /tmp 存在 + mkstemp 正确 |
| socket(完整修复) | ★★★★★ | 本地回环 socket 框架 |

---

## 6. 建议进攻路线

### 三步走(更新:-ENOSYS 已修)

```
第零步 ✅ 已完成 (1 行,已应用)
└── ✅  -1 → -ENOSYS
     效果:socket/statvfs errno 修正为 ENOSYS
    ↓

第一步 (2 小时) -- 清理已实现 syscall 的 -1 返回
├── ★★   dup/open 边界检查:return -1 → return -EMFILE
├── ★★   futimens 实现:return -1 → 正确 errno
├── ★★   lseek 64 位:改返回值类型为 int64
└── ★★   /dev/null 设备节点
    预期:daemon_failure + rlimit_open_files + utime + lseek_large + stat → 5 个变真 PASS
    ↓

第二步 (2-4 小时)
├── ★★   syscall_sign_extend(符号扩展排查)
└── ★★★  mkstemp 临时文件路径
    预期:2 个变真 PASS
    ↓

第三步 (4-12 小时)
└── ★★★★★ 网络栈(本地回环 socket)
    预期:最后 1 个变真 PASS(socket 功能通过,不光 errno 正确)
```

### 完成后

```
全部 96 个测试 → 真 PASS(假 PASS 清零)
                   ↓
          补上 pthread 测试(约 11 个)
                   ↓
        libc-test 全量 ≈ 107 个全 PASS 🎉
```

---

> **一张图记住所有假 PASS：** socket 和 statvfs 的 errno 已修正（✅ 已完成）。daemon_failure、rlimit_open_files、utime 都是已实现 syscall 返回了 -1 而非正确 errno——需要在 sysfile/sysproc 中逐个清理。lseek_large 是类型不够宽；stat 是缺个设备文件；ftello 是临时文件路径；syscall_sign_extend 是符号扩展。不是 9 个独立大问题，拆开看每个都不复杂。
