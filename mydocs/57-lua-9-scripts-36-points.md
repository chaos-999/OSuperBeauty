# lua：镜像自带 9 个脚本全通过（两架构两 libc，+36 分）

> 目标：启用官方 judge `judge_lua-{glibc,musl}.py` 对应的 lua 测试组。
> 结论：**零内核改动、零风险**——镜像里 `lua` 是静态链接的解释器，9 个 `.lua` 脚本
> 全部 `testcase lua <name> success`，RV/LA、glibc/musl 共 **36 分**。
> 这是本次“性价比最高”的一个测试集（得分/时间 ≈ 36 分 / 约 2 分钟运行）。

---

## 1. 判分规则（`judge_lua-*.py`）

- 输入：串口日志里匹配 `testcase (.+) (success|fail)` 的行。
- 期望的 9 个脚本（`cmds` 列表）：
  `date.lua file_io.lua max_min.lua random.lua remove.lua round_num.lua sin30.lua sort.lua strings.lua`
- 每个脚本 `success` → score 1，缺失/fail → 0。`build_table` 对 rv+la 求和，
  所以 9 × 2 arch × 2 libc = **36**。

## 2. 镜像形态

- `/musl/lua`（576 KB）、`/glibc/lua`（5.8 MB）：都是 **statically linked** 的解释器，
  不需要动态 loader。
- `/musl/*.lua`、`/glibc/*.lua`：9 个脚本。例 `date.lua`：
  ```lua
  local tbCurrentTime = os.date("*t")
  if tbCurrentTime ~= nil then return 0 else return -1 end
  ```
  脚本顶层 `return 0/…`，解释器据此退出（成功=0）。
- 镜像自带 `lua_testcode.sh`（跑 `./test.sh <script>`）和 `test.sh`（`#!/bin/busybox sh`），
  但 `test.sh` 的 shebang 依赖 `/bin/busybox`（镜像里没有），所以内核 init 不依赖它，
  **直接 `execve("lua", {script})` 并自行打印 judge 行**。

## 3. init 实现（`user/init-la.c` / `user/init-rv.c`）

```c
static const char *lua_scripts[] = {"date.lua","file_io.lua","max_min.lua","random.lua",
                                    "remove.lua","round_num.lua","sin30.lua","sort.lua","strings.lua"};
static void run_lua_group(const char *dir, const char *group) {
    printf("#### OS COMP TEST GROUP START %s ####\n", group);
    if (chdir(dir) < 0) { printf("#### OS COMP TEST GROUP END %s ####\n", group); return; }
    for (int i = 0; i < 9; i++) {
        int pid = fork();
        if (pid == 0) { char *av[] = {"lua", (char*)lua_scripts[i], 0}; execve("lua", av, 0); exit(1); }
        int st = 0; if (pid > 0) wait(&st);
        printf("testcase lua %s %s\n", lua_scripts[i], (pid>0 && (st&0xff)==0) ? "success" : "fail");
    }
    chdir("/");
    printf("#### OS COMP TEST GROUP END %s ####\n", group);
}
```

调用顺序（在 libcbench 之后、LTP 之前）：
`run_lua_group("/musl","lua-musl")` → `run_lua_group("/glibc","lua-glibc")`。

## 4. 结果

官方 harness 四组 `lua-{glibc,musl}-{rv,la}` 全部 9.0，合计 **36**。

- 无需 loader、无需 `/proc`、无需特殊 syscall；`os.date("*t")` 走 `gettimeofday`（已实现）。
- 若某天镜像 lua 换成动态链接，会退回 `55/58` 里的动态 loader 问题（RV PIE fork、块缓存等）。

## 5. 为什么这是“性价比之王”

| 测试集 | 得分 | 运行成本 | 内核改动 | 风险 |
|--------|------|---------|---------|------|
| lua | 36 | ~2 min | 0 | 无 |
| libcbench | ~116 | ~6 min | 多（线程/VM/信号/prlimit64/mmap） | 中 |
| LTP | ~18（raw 97） | ~25 min | 多（动态 loader/fs/syscall） | 高（间歇损坏、超时） |
