# busybox (LoongArch64, static, stripped) 初步反汇编分析

样本：/home/OSuperBeauty/busybox  (2,065,912 B, md5 3bc4005d0212307e9d217c1cf69b0324)
类型：ELF64 LSB EXEC, LoongArch, statically linked, stripped
版本字符串：BusyBox v1.33.1 (2025-06-14 15:07:02 Aisa) @ file 0x1f228b -> va 0x1201f228b

## 1. 生成的文件

| 文件 | 大小 | 说明 |
|---|---|---|
| busybox.disasm        | 22,594,019 B (448,793 行) | objdump -d 全量（含原始字节） |
| busybox.disasm.noraw  | 18,106,159 B (448,793 行) | objdump -d --no-show-raw-insn |
| busybox.entry.disasm  | 5,412 B | 入口 0x1201b640c 前后各 64 条指令 |

反汇编覆盖 .text 0x120000120–0x1201b6568（0x1b6448 B），无 (bad)/unknown 指令。

## 2. 段映射（文件偏移 <-> 虚拟地址）

```
LOAD0  off 0x000000  va 0x120000000  filesz 0x1f7888  R E   (.text .rodata .eh_frame)
       => va = off + 0x120000000
LOAD1  off 0x1f7e48  va 0x1201fbe48  filesz 0x4e2 memsz 0x1d50 RW (.got .data .bss)
       => va = off + 0x120004000
```
注意：RW 段的加数不是 0x120000000 而是 0x120004000（差 0x4000，页对齐造成）。
.rodata：off 0x1b6570 -> va 0x1201b6570，大小 0x41184；所有 strings 偏移都落在这里。

## 3. 入口点与入口反汇编

ELF e_entry = 0x1201b640c（位于 .text 末尾前 0x15c 字节处）

```
1201b640c: move      $fp, $zero
1201b6410: move      $a0, $sp
1201b6414: pcalau12i $a1, -131510        ; 编码 0x1bbfc945
1201b6418: addi.d    $a1, $a1, 0         ; 编码 0x02c000a5
1201b641c: bstrins.d $sp, $zero, 0x3, 0x0
1201b6420: b         -1794780  # 0x120000144
```
0x120000144 处（_start_c 形态，逐条对应 busybox.disasm 中 0x120000144 起的指令）：
```
120000144: ldptr.w     $a1, $a0, 0       ; a1 = p[0] = argc —— 覆盖上面那个 a1
120000148: pcalau12i   $a4, 508
12000014c: pcalau12i   $a3, 508
120000150: addi.d      $a2, $a0, 8       ; argv = p + 1
120000154: pcalau12i   $a0, 508
120000158: move        $a5, $zero
12000015c: ld.d        $a4, $a4, -128    ; GOT 槽 (fini)
120000160: ld.d        $a3, $a3, -280    ; GOT 槽 (init)
120000164: ld.d        $a0, $a0, -152    ; a0 = GOT[0x1201fbf68] = main = 0x120001874
120000168: b           0x120189aac       ; libc 启动入口 __libc_start_main(a0..a5)
```

## 4. pcalau12i 统计与正确性判定

- .text 中 pcalau12i 共 **22,524** 条（pcaddi 仅 3 条）。
- 能按 “pcalau12i Rd,hi20 + 紧接着用 Rd 作 base 的 imm12 指令” 配对的：22,394 对。
- 这 22,394 对中目标落在镜像外的：**1 对**，即入口：
  `pc=0x1201b6414, imm20=-131510, addi.d +0`
  target = (0x1201b6000) + (-131510<<12) + 0 = **0x100000000** —— 非法地址。
  其余 22,393 对全部落在 0x120000000–0x1201fdb98（镜像）内。
- hi20 == 0 且 lo12 == 0 的 45 对都指向自身所在页，正常。

结论：
1) **不存在全局性的 pcalau12i/GOT 偏移错误**。若工具链把 la.pcrel 普遍算错，echo 也不可能工作；实测 22,393/22,394 对目标全部合法。
2) 入口这一条确属异常（编译/链接期对 `la.pcrel $a1, main` 的重定位算出了 0x100000000），但它是**死代码**：
   0x120000144 第一条指令 `ldptr.w $a1, $a0, 0` 立即用 argc 覆盖 a1，`main` 是从 GOT
   0x1201fbf68 读出来的（= 0x120001874）。所以它**不是 sh 静默退出的原因**。
3) .got 共 53 槽（0x1201fbe58–0x1201fc000）：除 8 个为 0 的槽外全部指向镜像内；
   GOT[0x1201fbf68]=0x120001874(main)，GOT[0x1201fbf80]=0x12018a070，GOT 自身一致性好。

## 5. applet 表（比字符串更有用的定位手段）

- `applet_names` 字符串块：file 0x1e90f6 -> **va 0x1201e90f6**，共 **397** 个 applet 名（"[" "[[ " "acpid" ... "zcip"）。
- `applet_main` 函数指针表：**va 0x1201b66a0**，397 项，逐项校验**全部**是 .text 内地址。
- 索引验证（证明表对齐正确）：`true`(idx 343)->0x120141da4 返回 0；`false`(idx 83)->0x12012ee80 返回 1；`echo`(idx 70)->0x12012cda0（解析 -n/-e 的循环，符合 echo_main）。
- **`sh` = idx 292 -> 0x1200cf480；`hush` = idx 122 -> 0x1200cf480（同一个函数）**
  => 该二进制是 `CONFIG_SH_IS_HUSH`，`busybox sh` 走的是 **hush**，不是 ash（与常见 ash 假设不同，排查时不要盯 ash）。
- `ash` = idx 10 -> 0x1200c1a3c（真正的 ash_main：引用 HISTFILE/.ash_history/"syntax error at '%s'"）。
- hush_main(0x1200cf480) 关键引用（已核对 xref）：
  - 0x1200cf52c -> 0x1201cea30 "HUSH_VERSION"
  - 0x1200cf668/694 -> 0x1201cea40 "HOSTNAME"
  - 0x1200cf6ac -> 0x1201ce9f0 "IFS"；0x1200cf6b8 -> 0x1201ce8c8 "PATH"
  - 0x1200cf6cc -> 0x1201f23ae "/sbin:/usr/sbin:/bin:/usr/bin"
  - 0x1200cf72c -> 0x1201cea50 getopt 串 **"+cexinsl"**
  - 0x1200cf874 -> 0x1201cea60 "/etc/profile"；0x1200cf8a8 -> "HOME"；0x1200cf8cc -> 0x1201cea78 ".profile"
  - 0x1200cf954 -> 0x1201cea88 "-c"；0x1200cfc70 -> 0x1201cea90 "HISTFILE"
  - 0x1200cf770 的选项跳转表 0x1201cf448（22 项，'c'..'x'）**全部**指向 .text 内合法 case
    （'c'->0x1200cf784，'e'/'n'/'x'->0x1200cf7bc，'i'->0x1200cf7e0，'s'->0x1200cf798，'l'->0x1200cf7ac，其余->0x1200cf7dc 报用法）。
- 全局 hush 状态结构在 0x1201fc378（.data/.bss，hush_main 内高频 `ld.d $t0,...| addi 888` 访问）。

## 6. 关键字符串（file off -> va = off + 0x120000000）

| 字符串 | file off | va | xref（引用点） |
|---|---|---|---|
| "sh"（applet 名） | 0x1e98e3 | 0x1201e98e3 | applet_names 表 |
| "ash"（applet 名） | 0x1e9135 | 0x1201e9135 | applet_names 表 |
| ": applet not found" | 0x1b7728 | 0x1201b7728 | 0x120001860（main/分派逻辑） |
| "you have no permission to run this applet" | 0x1b73f0 | 0x1201b73f0 | 0x120000d04 |
| "can't execute '%s'" | 0x1baac8 | 0x1201baac8 | 0x120021cf4 |
| "No such file or directory" | 0x1dfa5d | 0x1201dfa5d | —（通用 libc 串） |
| "can't access tty; job control turned off" | 0x1cc8a0 | 0x1201cc8a0 | 0x1200adbc0（hush） |
| "HISTFILE" | 0x1cc700 | 0x1201cc700 | 0x1200c1f8c/0x1200c2000/0x1200c2018（ash） |
| "syntax error at '%s'" | 0x1ce5e8 | 0x1201ce5e8 | 0x1200c2540（ash） |
| "BusyBox v1.33.1 (...)" | 0x1f228b | 0x1201f228b | 版本串 |

## 7. 判断与建议

- pcalau12i/GOT：**除入口那条死代码外无异常**，不支持“工具链 pcalau12i/GOT 普遍算错”的假设。
- sh 静默退出应到 **hush** 代码里找：从 0x1200cf480 起（hush_main），重点看 getopt32(0x12018d9ec, "+cexinsl")
  之后 argv[optind] 脚本文件的分支、以及 0x1200cf650 附近（`li.w $a0,1; bl 0x1200c3d84`）那条可能直接退出的路径。
- 下一步可查：hush_main 中 open/read 脚本文件的调用点、`exit`/`_exit` 调用点，以及 `setjmp/die_jmp`(hush 的 die_func) 相关全局。
