# RV busybox argv 损坏修复

> `init-rv.c` 中 `bb_testcode` 全局数组被踩踏，导致 busybox exec 时 argv 乱码。

## 现象

busybox-musl/glic 的 `execve("busybox", bb_testcode, NULL)` 成功加载 busybox，但 busybox 输出：
```
�+: applet not found
```

argv[0] 应为 `"busybox"`，实际为乱码 `�+`。

## 诊断

在 `exec.c` 页表切换后添加诊断：
```c
printf("exec_diag: path=%s argc=%d\n", path, argc);
for (int j = 0; j < argc; j++)
    printf("exec_diag: argv[%d]=%s\n", j, argv[j]);
```

发现 **内核侧 `kern_str` 已经是烂的**——问题在 `sys_execve` 的 `fetchaddr`/`fetchstr` 阶段，而非 `execve` 栈复制。

`argc=4` 而非预期的 3，`argv[0]="�+"`，`argv[1]=""`，`argv[3]=""`（不应存在）。

## 根因

`bb_testcode` 是全局变量：
```c
char *bb_testcode[10] = {"busybox", "sh", "busybox_testcode.sh", NULL};
```

符号表显示 `bb_testcode` 与 `basic_name[32]`（256 字节）在数据段紧邻。64 次 basic test 的 fork/exec 循环过程中，某处内存踩踏破坏了 `bb_testcode[0]` 和 `bb_testcode[1]` 的指针值。

## 修复

将 `bb_testcode` 从全局改为 `test_pre()` 局部变量：

```c
void test_pre() {
    char *bb_testcode_local[] = {"busybox", "sh", "busybox_testcode.sh", NULL};
    ...
    execve("busybox", bb_testcode_local, NULL);
}
```

每次调用时在栈上初始化，不受之前 fork/exec 循环的内存状态影响。

## 备注

LA 不需要此修复——LA 的 busybox-musl 的问题是 musl 编译的 busybox 工具链 bug（`pcalau12i`），非数据段踩踏。
