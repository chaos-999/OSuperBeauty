#!/usr/bin/env python3
"""
live_monitor.py - 用手机实时查看 OS 测评进度。

零依赖，只用 Python 3 标准库。

用法：
    python3 tools/live_monitor.py --log os_serial_out_la.txt
    python3 tools/live_monitor.py --log auto --port 8080

然后在手机浏览器打开：
    http://<电脑局域网IP>:8080

评测端需要把串口输出写到日志，例如：
    make qemu-la 2>&1 | tee os_serial_out_la.txt
"""

import argparse
import collections
import glob
import html
import json
import os
import re
import socket
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
GROUP_START_RE = re.compile(r"####\s*OS COMP TEST GROUP START\s+(.*?)\s*####")
GROUP_END_RE = re.compile(r"####\s*OS COMP TEST GROUP END\s+(.*?)\s*####")
TESTCASE_RE = re.compile(r"^testcase\s+(.*?)\s+(success|fail)\s*$", re.I)
LIB_START_RE = re.compile(r"^=+\s*START\s+(\S+)\s+(\S+)\s*=+\s*$")
LIB_END_RE = re.compile(r"^=+\s*END\s+(\S+)\s+(\S+)\s*=+\s*$")
LIB_FAIL_RE = re.compile(r"^FAIL\s+(\S+)\s*(?:\[status\s+([^\]]+)\])?", re.I)
PASS_RE = re.compile(r"^Pass!?\s*$", re.I)
FAIL_WORD_RE = re.compile(r"\b(fail|failed|error|panic|timeout|timed out)\b", re.I)


def strip_ansi(s: str) -> str:
    s = ANSI_RE.sub("", s)
    return s.replace("\r", "")


def local_ip() -> str:
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        try:
            return socket.gethostbyname(socket.gethostname())
        except Exception:
            return "127.0.0.1"


class Monitor:
    def __init__(self, log_pattern: str, tail_lines: int = 600):
        self.log_pattern = log_pattern
        self.tail_lines = tail_lines
        self.lock = threading.Lock()
        self.cond = threading.Condition(self.lock)
        self.lines = collections.deque(maxlen=tail_lines)
        self.groups = []
        self.group_map = {}
        self.current_group = None
        self.current_test = ""
        self.last_failure = ""
        self.total_lines = 0
        self.pass_count = 0
        self.fail_count = 0
        self.started_at = None
        self.updated_at = time.time()
        self.version = 0
        self.log_path = ""
        self.log_size = 0
        self.partial = ""
        self._position = 0
        self._inode = None
        self._paused = False
        self._stop = False

    # ---------- log file handling ----------

    def _resolve_log(self):
        if self.log_pattern != "auto":
            return self.log_pattern

        candidates = []
        for pat in ("os_serial_out_*.txt", "*.log"):
            candidates.extend(glob.glob(pat))
        candidates = [p for p in candidates if os.path.isfile(p)]
        if not candidates:
            return ""
        return max(candidates, key=lambda p: os.path.getmtime(p))

    def _reset_for(self, part: str):
        with self.cond:
            self.log_path = part
            self.lines.clear()
            self.groups = []
            self.group_map = {}
            self.current_group = None
            self.current_test = ""
            self.last_failure = ""
            self.total_lines = 0
            self.pass_count = 0
            self.fail_count = 0
            self.started_at = None
            self.partial = ""
            self._position = 0
            self._inode = None
            self.version += 1
            self.cond.notify_all()

    def _ensure_file(self):
        part = self._resolve_log()
        if not part:
            if self.log_path:
                self._reset_for("")
            return False

        try:
            st = os.stat(part)
        except OSError:
            self._reset_for("")
            return False

        if part != self.log_path:
            self._reset_for(part)

        if st.st_size < self._position or st.st_ino != self._inode:
            # 文件被截断/重建：保留当前页面内容过于混乱，直接重新开始。
            self._reset_for(part)
            self.log_path = part

        self._inode = st.st_ino
        self.log_size = st.st_size
        return True

    def _read_new(self):
        if not self._ensure_file():
            return
        try:
            with open(self.log_path, "r", encoding="utf-8", errors="replace") as f:
                f.seek(self._position)
                chunk = f.read()
                self._position = f.tell()
        except OSError:
            return
        if not chunk:
            return

        if self.started_at is None:
            self.started_at = time.time()

        text = self.partial + chunk
        parts = text.split("\n")
        self.partial = parts.pop() if parts else ""
        for raw in parts:
            self._process_line(raw)

    # ---------- line parsing ----------

    def _group(self, name: str) -> dict:
        if name not in self.group_map:
            g = {
                "name": name,
                "status": "waiting",
                "tests": 0,
                "pass": 0,
                "fail": 0,
                "current": "",
                "started": None,
                "ended": None,
            }
            self.group_map[name] = g
            self.groups.append(g)
        return self.group_map[name]

    def _touch(self):
        with self.cond:
            self.version += 1
            self.updated_at = time.time()
            self.cond.notify_all()

    def _process_line(self, raw: str):
        line = strip_ansi(raw).rstrip()
        if not line:
            return

        self.total_lines += 1
        self.lines.append(line)

        m = GROUP_START_RE.search(line)
        if m:
            g = self._group(m.group(1).strip())
            g["status"] = "running"
            g["started"] = g["started"] or time.time()
            self.current_group = g
            self.current_test = ""
            self._touch()
            return

        m = GROUP_END_RE.search(line)
        if m:
            g = self._group(m.group(1).strip())
            g["status"] = "done"
            g["ended"] = time.time()
            g["current"] = ""
            if self.current_group is g:
                self.current_group = None
            self.current_test = ""
            self._touch()
            return

        m = TESTCASE_RE.match(line)
        if m:
            desc, ok = m.group(1), m.group(2).lower() == "success"
            g = self.current_group or self._group("testcase")
            g["tests"] += 1
            if ok:
                g["pass"] += 1
                self.pass_count += 1
            else:
                g["fail"] += 1
                self.fail_count += 1
                self.last_failure = line
            self._touch()
            return

        m = LIB_START_RE.match(line)
        if m:
            binary, test = m.group(1), m.group(2)
            self.current_test = f"{binary} {test}"
            g = self.current_group or self._group("libctest")
            g["status"] = "running"
            g["current"] = self.current_test
            self._touch()
            return

        if PASS_RE.match(line):
            g = self.current_group or self._group("libctest")
            g["tests"] += 1
            g["pass"] += 1
            self.pass_count += 1
            self._touch()
            return

        m = LIB_FAIL_RE.match(line)
        if m:
            g = self.current_group or self._group("libctest")
            g["tests"] += 1
            g["fail"] += 1
            self.fail_count += 1
            self.last_failure = line
            self._touch()
            return

        m = LIB_END_RE.match(line)
        if m:
            self.current_test = ""
            if self.current_group:
                self.current_group["current"] = ""
            self._touch()
            return

        if FAIL_WORD_RE.search(line) and "0 fail" not in line.lower():
            self.last_failure = line
            self._touch()

    # ---------- API ----------

    def snapshot(self):
        with self.lock:
            done_groups = sum(1 for g in self.groups if g["status"] == "done")
            elapsed = int(time.time() - self.started_at) if self.started_at else 0
            return {
                "log": self.log_path,
                "log_size": self.log_size,
                "updated": self.updated_at,
                "elapsed": elapsed,
                "lines": list(self.lines),
                "groups": self.groups,
                "current_group": self.current_group["name"] if self.current_group else "",
                "current_test": self.current_test,
                "total_lines": self.total_lines,
                "pass": self.pass_count,
                "fail": self.fail_count,
                "last_failure": self.last_failure,
                "groups_done": done_groups,
                "groups_total": len(self.groups),
                "version": self.version,
            }

    def run(self):
        while not self._stop:
            try:
                self._read_new()
            except Exception as e:  # noqa: BLE001 - 监控不能因为单行解析失败退出
                with self.lock:
                    self.last_failure = f"monitor error: {e}"
                    self._touch()
            time.sleep(0.4)

    def stop(self):
        self._stop = True


HTML = r"""<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>OS 测评实时进度</title>
<style>
:root{
  --bg:#0b1020;--panel:#151b2e;--panel2:#1d2540;--text:#e8edff;--muted:#8b96b8;
  --green:#38d39f;--red:#ff6b81;--yellow:#ffd166;--blue:#6ea8fe;--line:#2a3352;
}
*{box-sizing:border-box}
body{margin:0;background:linear-gradient(180deg,#0b1020,#0d1428 40%,#0b1020);
  color:var(--text);font:15px/1.55 -apple-system,BlinkMacSystemFont,"Segoe UI",
  "PingFang SC","Microsoft YaHei",sans-serif;min-height:100vh}
header{position:sticky;top:0;z-index:10;background:rgba(11,16,32,.92);
  backdrop-filter:blur(10px);border-bottom:1px solid var(--line);padding:10px 14px}
h1{font-size:17px;margin:0 0 6px;display:flex;align-items:center;gap:8px}
.dot{width:9px;height:9px;border-radius:50%;background:var(--muted);flex:none}
.dot.on{background:var(--green);box-shadow:0 0 10px var(--green)}
.dot.off{background:var(--red)}
.sub{color:var(--muted);font-size:12px;word-break:break-all}
.wrap{padding:12px;max-width:1100px;margin:0 auto}
.cards{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-bottom:12px}
.card{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:9px 10px}
.card b{display:block;font-size:20px;line-height:1.2}
.card span{color:var(--muted);font-size:11px}
.card.pass b{color:var(--green)}.card.fail b{color:var(--red)}
.card.groups b{color:var(--blue)}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:14px;
  padding:12px;margin-bottom:12px}
.panel h2{font-size:14px;margin:0 0 10px;color:var(--muted);font-weight:600;
  display:flex;justify-content:space-between;align-items:center;gap:8px}
.badge{font-size:11px;padding:2px 8px;border-radius:999px;background:var(--panel2);
  color:var(--muted);font-weight:500}
.badge.run{color:var(--blue);background:rgba(110,168,254,.12)}
.badge.done{color:var(--green);background:rgba(56,211,159,.12)}
.badge.fail{color:var(--red);background:rgba(255,107,129,.12)}
.group{border-top:1px solid var(--line);padding:9px 0}
.group:first-of-type{border-top:0;padding-top:0}
.gline{display:flex;justify-content:space-between;gap:8px;align-items:baseline}
.gname{font-weight:600;font-size:14px;word-break:break-all}
.gmeta{color:var(--muted);font-size:12px;white-space:nowrap}
.gcur{color:var(--blue);font-size:12px;margin-top:3px;word-break:break-all}
.bar{height:4px;background:#10172a;border-radius:99px;overflow:hidden;margin-top:7px}
.bar i{display:block;height:100%;background:linear-gradient(90deg,var(--blue),var(--green));
  width:0;transition:width .3s}
.failbox{border-color:rgba(255,107,129,.45);background:rgba(255,107,129,.07)}
.failbox .txt{color:#ffc0c8;font-size:13px;word-break:break-all}
.toolbar{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
button{background:var(--panel2);color:var(--text);border:1px solid var(--line);
  border-radius:9px;padding:6px 10px;font-size:13px}
button:active{transform:scale(.98)}
#log{height:55vh;overflow:auto;background:#070b16;border:1px solid var(--line);
  border-radius:12px;padding:8px;font:12px/1.45 ui-monospace,SFMono-Regular,Menlo,
  Consolas,monospace;white-space:pre-wrap;word-break:break-all;overscroll-behavior:contain}
#log div{padding:1px 3px;border-radius:3px}
.l-fail{color:#ff9aa8;background:rgba(255,107,129,.08)}
.l-pass{color:#8ff0cb}
.l-group{color:#8fb7ff}
.l-dim{color:#6f7a99}
.empty{color:var(--muted);text-align:center;padding:22px 0}
footer{color:#5f6a88;font-size:11px;text-align:center;padding:10px 0 22px}
@media(max-width:560px){.cards{grid-template-columns:repeat(2,1fr)}.card b{font-size:18px}}
</style>
</head>
<body>
<header>
  <h1><i id="dot" class="dot"></i><span id="title">等待日志…</span></h1>
  <div class="sub" id="sub">请在电脑上运行评测，并把串口输出 tee 到日志文件</div>
</header>
<div class="wrap">
  <div class="cards">
    <div class="card groups"><b id="groups">0/0</b><span>测试组</span></div>
    <div class="card"><b id="tests">0</b><span>用例</span></div>
    <div class="card pass"><b id="pass">0</b><span>通过</span></div>
    <div class="card fail"><b id="fail">0</b><span>失败</span></div>
  </div>

  <div class="panel failbox" id="failpanel" style="display:none">
    <h2>最近失败 <span class="badge fail" id="failcount">0</span></h2>
    <div class="txt" id="lastfail"></div>
  </div>

  <div class="panel">
    <h2>测试组进度 <span class="badge" id="elapsed">0s</span></h2>
    <div id="groupsbox"><div class="empty">等待测试组开始…</div></div>
  </div>

  <div class="panel">
    <h2>
      <span>串口日志</span>
      <span class="toolbar">
        <button id="pause">暂停滚动</button>
        <button id="top">回到最新</button>
      </span>
    </h2>
    <div id="log"></div>
  </div>
  <footer>手机与电脑需在同一局域网 · 页面自动刷新</footer>
</div>
<script>
let paused=false, es=null, lastVersion=-1, retry=0;
const $=id=>document.getElementById(id);
const esc=s=>String(s);

function fmtTime(s){
  s=Math.max(0, s|0);
  const h=(s/3600)|0, m=((s%3600)/60)|0, x=s%60;
  return (h? h+"h ":"")+(m? m+"m ":"")+x+"s";
}
function lineClass(line){
  const l=line.toLowerCase();
  if(/fail|failed|error|panic|timeout|timed out/.test(l)) return "l-fail";
  if(/pass!|success/.test(l)) return "l-pass";
  if(/####.*group/.test(l)) return "l-group";
  if(/^=+ (start|end)|^sc\\[|unknown sys call/.test(l)) return "l-dim";
  return "";
}
function render(d){
  $("dot").className="dot "+(es?"on":"off");
  $("title").textContent=d.log?("OS 测评 · "+d.log):"等待日志…";
  $("sub").textContent=d.log
    ? ("已运行 "+fmtTime(d.elapsed)+" · 日志 "+d.log_size+" B · 更新 "+new Date(d.updated*1000).toLocaleTimeString())
    : "请在电脑上运行评测，并把串口输出 tee 到日志文件";
  $("groups").textContent=d.groups_done+"/"+d.groups_total;
  $("tests").textContent=d.groups.reduce((a,g)=>a+g.tests,0);
  $("pass").textContent=d.pass;
  $("fail").textContent=d.fail;
  $("elapsed").textContent=fmtTime(d.elapsed);

  const fp=$("failpanel");
  if(d.last_failure){fp.style.display="";$("lastfail").textContent=d.last_failure;$("failcount").textContent=d.fail;}
  else fp.style.display="none";

  const gb=$("groupsbox");
  if(!d.groups.length){
    gb.innerHTML='<div class="empty">等待测试组开始…</div>';
  }else{
    gb.innerHTML="";
    for(const g of d.groups){
      const total=g.pass+g.fail;
      const rate=total?Math.round(g.pass*100/total):0;
      const cls=g.fail?"fail":(g.status==="done"?"done":"run");
      const row=document.createElement("div");
      row.className="group";
      row.innerHTML=
        '<div class="gline"><div class="gname"></div>'+
        '<div class="gmeta"><span class="badge '+cls+'">'+
        (g.status==="done"?"完成":g.status==="running"?"运行中":"等待")+
        '</span> '+(g.pass+g.fail)+' 用例</div></div>'+
        (g.current?'<div class="gcur"></div>':'')+
        '<div class="bar"><i style="width:'+rate+'%"></i></div>';
      row.querySelector(".gname").textContent=g.name;
      if(g.current)row.querySelector(".gcur").textContent="→ "+g.current;
      row.querySelector(".gmeta").title="通过 "+g.pass+" / 失败 "+g.fail;
      gb.appendChild(row);
    }
  }

  const log=$("log");
  const nearBottom=log.scrollHeight-log.scrollTop-log.clientHeight<80;
  log.innerHTML="";
  const frag=document.createDocumentFragment();
  for(const line of d.lines){
    const el=document.createElement("div");
    el.className=lineClass(line);
    el.textContent=line;
    frag.appendChild(el);
  }
  log.appendChild(frag);
  if(!paused && nearBottom) log.scrollTop=log.scrollHeight;
}
function connect(){
  if(es)es.close();
  es=new EventSource("/events");
  es.onopen=()=>{retry=0;$("dot").className="dot on";};
  es.onerror=()=>{
    $("dot").className="dot off";
    retry=Math.min(retry+1,6);
    setTimeout(connect, 500*retry);
  };
  es.onmessage=e=>{
    try{
      const d=JSON.parse(e.data);
      if(d.version===lastVersion)return;
      lastVersion=d.version;
      render(d);
    }catch(err){console.error(err);}
  };
}
$("pause").onclick=()=>{
  paused=!paused;
  $("pause").textContent=paused?"继续滚动":"暂停滚动";
};
$("top").onclick=()=>{const l=$("log");l.scrollTop=l.scrollHeight;};
connect();
setInterval(()=>{if(!es||es.readyState===2)connect();},5000);
</script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    monitor = None

    def log_message(self, fmt, *args):
        pass

    def _send(self, code, body, ctype="text/plain; charset=utf-8", extra=None):
        data = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        if extra:
            for k, v in extra.items():
                self.send_header(k, v)
        self.end_headers()
        try:
            self.wfile.write(data)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/":
            self._send(200, HTML, "text/html; charset=utf-8")
        elif path == "/health":
            self._send(200, json.dumps({"ok": True}), "application/json")
        elif path == "/download":
            log = self.monitor.log_path
            if not log or not os.path.exists(log):
                self._send(404, "no log")
                return
            try:
                with open(log, "rb") as f:
                    data = f.read()
                self._send(200, data, "text/plain; charset=utf-8",
                           {"Content-Disposition": 'attachment; filename="serial.log"'})
            except OSError:
                self._send(500, "read log failed")
        elif path == "/events":
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream; charset=utf-8")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Connection", "keep-alive")
            self.send_header("X-Accel-Buffering", "no")
            self.end_headers()
            try:
                self.wfile.write(b"retry: 1000\n\n")
                last_ver = -1
                last_send = 0
                while True:
                    with self.monitor.cond:
                        if self.monitor.version == last_ver:
                            self.monitor.cond.wait(timeout=1.0)
                        ver = self.monitor.version
                    snap = self.monitor.snapshot()
                    now = time.time()
                    if ver != last_ver or now - last_send > 2:
                        payload = json.dumps(snap, ensure_ascii=False, separators=(",", ":"))
                        self.wfile.write(f"data: {payload}\n\n".encode("utf-8"))
                        self.wfile.flush()
                        last_ver = ver
                        last_send = now
            except (BrokenPipeError, ConnectionResetError, OSError):
                return
        else:
            self._send(404, "not found")


def main():
    ap = argparse.ArgumentParser(description="OS 测评串口日志实时看板")
    ap.add_argument("--log", default="auto",
                    help="日志路径，或 auto 自动找最新的 os_serial_out_*.txt/*.log")
    ap.add_argument("--host", default="0.0.0.0", help="监听地址，默认 0.0.0.0")
    ap.add_argument("--port", type=int, default=8080, help="监听端口，默认 8080")
    args = ap.parse_args()

    monitor = Monitor(args.log)
    threading.Thread(target=monitor.run, daemon=True).start()
    Handler.monitor = monitor

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    ip = local_ip()
    print("=" * 58)
    print(" OS 测评实时看板")
    print(f" 电脑:   http://127.0.0.1:{args.port}")
    print(f" 手机:   http://{ip}:{args.port}")
    print(f" 日志:   {args.log}")
    print(" 提示:   QEMU 输出请用 tee 写入日志，例如")
    print("         make qemu-la 2>&1 | tee os_serial_out_la.txt")
    print("=" * 58)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        monitor.stop()
        server.server_close()


if __name__ == "__main__":
    main()
