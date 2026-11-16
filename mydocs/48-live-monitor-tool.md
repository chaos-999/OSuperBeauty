# 手机实时查看测评进度：live_monitor.py

> 一个零依赖的串口日志看板：电脑上运行 QEMU，手机通过浏览器实时查看
> 测试组、用例通过/失败、当前用例和完整串口日志。

---

## 1. 它解决什么问题

传统方式：

```bash
make qemu-la
```

只能盯着电脑终端。跑 `basic`、`busybox`、`libctest` 时经常要等很久，
人离开电脑就不知道跑到哪了。

`tools/live_monitor.py` 把串口日志解析成网页：

- 测试组进度；
- 当前正在跑的测试；
- 通过/失败数量；
- 最近一条失败；
- 完整串口日志（自动去 ANSI、失败高亮）；
- 手机端自适应布局。

---

## 2. 启动方式

### 2.1 终端 A：运行 QEMU，并把输出写入日志

```bash
make qemu-la-log
# 或
make qemu-rv-log
```

等价于：

```bash
make qemu-la 2>&1 | tee os_serial_out_la.txt
```

### 2.2 终端 B：启动看板

```bash
make monitor
# 或固定日志
make monitor-la
make monitor-rv
```

启动后会打印：

```text
==========================================================
 OS 测评实时看板
 电脑:   http://127.0.0.1:8080
 手机:   http://192.168.1.23:8080
 日志:   auto
==========================================================
```

手机和电脑连同一个 Wi-Fi，浏览器打开 `http://192.168.1.23:8080` 即可。

也可以直接运行：

```bash
python3 tools/live_monitor.py --log os_serial_out_la.txt --port 8080
```

---

## 3. 看板功能

### 3.1 顶部状态

```text
● OS 测评 · os_serial_out_la.txt
  已运行 12m 3s · 日志 48213 B · 更新 18:42:11
```

### 3.2 统计卡片

- 测试组完成数 / 总数；
- 用例总数；
- 通过数；
- 失败数。

### 3.3 测试组

每个测试组显示：

- 组名；
- 运行中 / 完成 / 等待；
- 用例数；
- 当前正在跑的测试；
- 通过率进度条。

### 3.4 最近失败

显示最近一条包含 `fail` / `FAIL` / `error` / `panic` / `timeout`
的日志，方便手机上直接看到失败原因。

### 3.5 串口日志

- 自动去掉 ANSI 转义；
- 失败行红色高亮；
- `Pass!` / `success` 绿色；
- 测试组起止行蓝色；
- 默认自动滚动到底部；
- 可以“暂停滚动”翻看历史；
- “回到最新”一键跳回底部。

---

## 4. 解析规则

工具兼容当前测试脚本的几种输出格式：

```text
#### OS COMP TEST GROUP START basic-musl ####
testcase busybox echo success
testcase busybox false fail
#### OS COMP TEST GROUP END basic-musl ####
```

以及 libctest：

```text
========== START entry-static.exe argv ==========
Pass!
========== END entry-static.exe argv ==========

========== START entry-dynamic.exe dlopen ==========
FAIL dlopen [status 255]
========== END entry-dynamic.exe dlopen ==========
```

规则：

- `GROUP START/END` → 测试组状态；
- `testcase ... success/fail` → 用例计数；
- `START <bin> <test>` → 当前用例；
- `Pass!` → 通过 +1；
- `FAIL ... [status N]` → 失败 +1；
- 其他 `fail/error/panic/timeout` → 记录到“最近失败”。

---

## 5. 日志自动选择

默认：

```bash
python3 tools/live_monitor.py --log auto
```

会每秒扫描：

```text
os_serial_out_*.txt
*.log
```

选择修改时间最新的文件。适合：

- 先跑 LA，再跑 RV；
- QEMU 重启后日志被重建；
- 不想手动改参数。

也可以固定：

```bash
python3 tools/live_monitor.py --log os_serial_out_la.txt
```

---

## 6. 实现说明

文件：`tools/live_monitor.py`

- 纯 Python 3 标准库，不需要 `pip install`；
- HTTP + SSE（Server-Sent Events）推送；
- 单线程后台 tail 日志，多手机同时看；
- 检测日志截断/重建并自动重置；
- 提供 `/health` 和 `/download`：
  - `http://<ip>:8080/health` → `{"ok": true}`
  - `http://<ip>:8080/download` → 下载当前日志。

启动流程：

```text
QEMU stdout
  → tee os_serial_out_la.txt
  → live_monitor.py tail
  → SSE /events
  → 手机浏览器 EventSource
```

---

## 7. 安全提醒

默认监听 `0.0.0.0:8080`，局域网内任何设备都能访问，且没有登录认证。
只建议在可信 Wi-Fi 下使用。

如果需要只允许本机访问：

```bash
python3 tools/live_monitor.py --host 127.0.0.1
```

如果手机访问不了：

1. 确认手机和电脑在同一网段；
2. 检查电脑防火墙是否放行 8080；
3. 在电脑上先打开 `http://127.0.0.1:8080` 确认服务正常；
4. 用启动时打印的局域网 IP，不要用 `127.0.0.1`。

---

## 8. 一句话总结

`make qemu-la-log` 负责把串口落盘，`make monitor` 负责把日志变成手机网页；
两者配合即可在手机上实时查看测评进度，不需要改内核，也不需要额外依赖。
