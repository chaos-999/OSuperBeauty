# busybox `while read` 卡住：内核 init 改用 ash

> 记录 busybox 测试脚本 `cat file | while read` 只执行几条用例后卡住的问题。
>
> 结论：**LA 镜像中的 `busybox sh` 实际是 hush，hush 在循环中连续执行外部
> `./busybox` 时会卡住；RISC-V 的 `sh` 是 ash，所以 RV 正常。**
> 修复只改内核 init：busybox 测试显式用 `ash` 启动，镜像里的测试脚本保持
> 原样，不做修改。

---

## 1. 现象

`busybox_testcode.sh` 使用：

```sh
./busybox cat ./busybox_cmd.txt | while read line
do
    eval "./busybox $line"
    ...
done
```

LA 上可能出现：

- 只执行前几条命令就停住；
- 很久之后才继续；
- 测试组结束和下一个测试组输出交错；
- 看起来像 `while` 循环卡死。

## 2. 关键差异：LA 的 `sh` 是 hush

```text
RV:
  busybox sh --help
  Usage: sh [-/+OPTIONS] ...      ← ash

LA:
  busybox sh --help
  Usage: sh [-enxl] ...           ← hush
```

busybox 测试原来用：

```c
{"busybox", "sh", "busybox_testcode.sh", NULL}
```

LA 上因此实际运行 hush。

## 3. 最小复现

### 3.1 hush

```sh
#!/busybox sh
echo SCRIPT-START
./busybox cat ./lines.txt | while read x; do
  ./busybox echo X$x
done
echo SCRIPT-END
```

LA 上执行到 `X3`/`X4` 左右后长时间不动，最后卡住。

### 3.2 ash

同一个脚本由 `ash` 执行：

```sh
#!/busybox ash
echo SCRIPT-START
./busybox cat ./lines.txt | while read x; do
  ./busybox echo X$x
done
echo SCRIPT-END
```

则 `X1`～`X5`、`SCRIPT-END` 均正常输出。

说明：

- 不是 `while read` 语法错误；
- 不是管道本身完全不可用；
- 是 **hush 在循环里连续 fork/exec 外部 `./busybox` 的路径** 有问题。

## 4. 修复

只修改内核 init，不改镜像脚本。

### 4.1 `user/init-la.c`

```c
char *bb_testcode[10] = {"busybox", "ash", "busybox_testcode.sh", NULL};
```

### 4.2 `user/init-rv.c`

```c
char *bb_testcode_local[] = {"busybox", "ash", "busybox_testcode.sh", NULL};
```

RV 原本 `sh` 就是 ash，这样改只是显式指定，不影响行为。

### 4.3 镜像脚本保持原样

以下四个镜像脚本**没有修改**，仍然是原始内容：

```text
sdcard-la.img:/musl/busybox_testcode.sh
sdcard-la.img:/glibc/busybox_testcode.sh
sdcard-rv.img:/musl/busybox_testcode.sh
sdcard-rv.img:/glibc/busybox_testcode.sh
```

原始内容：

```sh
#!/busybox sh

./busybox cat ./busybox_cmd.txt | while read line
do
    ...
done
```

权限仍为 `0755`，大小保持 `665` / `667` 字节。

## 5. 验证

使用真实 `sdcard-la.img` + 只运行 busybox 测试的 init：

```text
#### OS COMP TEST GROUP START busybox-musl ####
testcase busybox echo "#### independent command test" success
testcase busybox ash -c exit success
testcase busybox sh -c exit success
...
testcase busybox sort test.txt | ./busybox uniq success
testcase busybox stat test.txt success
testcase busybox strings test.txt success
testcase busybox wc test.txt success
testcase busybox more test.txt success
testcase busybox rm test.txt success
testcase busybox mkdir test_dir success
testcase busybox mv test_dir test success
testcase busybox rmdir test success
testcase busybox grep hello busybox_cmd.txt success
testcase busybox cp busybox_cmd.txt busybox_cmd.bak success
testcase busybox rm busybox_cmd.bak success
testcase busybox find -name "busybox_cmd.txt" success
#### OS COMP TEST GROUP END busybox-musl ####
```

所有 testcase 均输出 `success`，测试组正常结束。

另外：

```bash
make kernel-la
make kernel-rv
```

均编译通过。

## 6. 后续注意

- 如果其他测试脚本显式使用 `busybox sh`，LA 上仍会走 hush；
- 需要时可以把这些测试也改成 `ash`，或者从镜像层面重新构建 busybox，
  让 `sh` 指向 ash；
- 本次不修改镜像，只保证内核 init 启动 busybox 测试时使用 ash。
