#!/bin/bash
# judge-offline.sh - 从指定日志文件进行离线评测
# 用法:
#   ./judge-offline.sh <log_file> [test_group1 test_group2 ...]
# 例:
#   ./judge-offline.sh qemu.log
#   ./judge-offline.sh board.log git-glibc
#   ./judge-offline.sh board.log git-musl git-glibc

set -e

if [ $# -lt 1 ]; then
  echo "用法: $0 <log_file> [test_group ...]"
  exit 1
fi

LOG_FILE="$1"
shift

if [ ! -f "${LOG_FILE}" ]; then
  echo "错误: 日志文件不存在: ${LOG_FILE}"
  exit 1
fi

# 默认评测组，与 judge-site.sh 保持一致
if [ $# -gt 0 ]; then
  TEST_GROUPS=("$@")
else
  TEST_GROUPS=("git-glibc" "git-musl")
fi

TEMP_DIR=".judge_tmp_offline"
mkdir -p "${TEMP_DIR}"

for TEST_NAME in "${TEST_GROUPS[@]}"; do
  JUDGE_SCRIPT="judge/judge_${TEST_NAME}.py"
  TEMP_RESULT="${TEMP_DIR}/temp_judge_result_${TEST_NAME}.json"

  START_MARKER_REGEX="#### OS COMP TEST GROUP START ${TEST_NAME}"
  END_MARKER_REGEX="#### OS COMP TEST GROUP END ${TEST_NAME}"

  if [ ! -f "${JUDGE_SCRIPT}" ]; then
    echo "⚠️  警告: 评测脚本 '${JUDGE_SCRIPT}' 未找到，跳过 ${TEST_NAME}！"
    continue
  fi

  # 提取标记之间的内容并评测（若日志中无该组标记则结果为空）
  awk "/${START_MARKER_REGEX}/{p=1;next} /${END_MARKER_REGEX}/{p=0} p" "${LOG_FILE}" | python3 "${JUDGE_SCRIPT}" > "${TEMP_RESULT}" || true

  # 结果格式化（与 judge-site.sh 相同的 Python 片段）
  CURRENT_TEST_GROUP="${TEST_NAME}" python3 << 'EOF'
import json, os, sys
tmp_dir = '.judge_tmp_offline'
group_name = os.environ.get('CURRENT_TEST_GROUP', '')
path = os.path.join(tmp_dir, f'temp_judge_result_{group_name}.json') if group_name else ''
if not path or not os.path.exists(path):
    print("⚠️  警告: 没有找到评测结果数据文件")
    sys.exit(0)

content = open(path, 'r').read().strip()
if not content:
    print("⚠️  警告: 没有找到评测结果数据")
    sys.exit(0)

lines = content.split('\n')
json_line = None
for line in reversed(lines):
    line = line.strip()
    if line.startswith('[') and line.endswith(']'):
        json_line = line
        break

if not json_line:
    print("⚠️  警告: 无法找到有效的JSON评测结果")
    print("原始输出:")
    print(content)
    sys.exit(0)

results = json.loads(json_line)
if not results:
    print("⚠️  警告: 评测结果为空")
    sys.exit(0)

def row_total(r): return r.get('score', 0)
def row_pass(r): return r.get('score', 0)

total_tests = len(results)
total_all = sum(row_total(r) for r in results)
total_pass = sum(row_pass(r) for r in results)
total_score = sum(r.get('score', 0) for r in results)

print("\n" + "="*80)
print(f"📊 {os.environ.get('CURRENT_TEST_GROUP', '评测')} 评测结果")
print("="*80)
print(f"{'测试名称':<20} {'总分':<6} {'通过':<6} {'得分':<6} {'状态':<10}")
print("-" * 80)

results_sorted = sorted(results, key=lambda x: (-x.get('score', 0), -x.get('pass', 0), x.get('name', '')))
for r in results_sorted:
    name = r.get('name', '')
    score = r.get('score', 0)
    all_points = score
    pass_points = score
    status = "✅ 完美" if score >= all_points else ("🟡 部分" if score > 0 else "❌ 失败")
    print(f"{name:<20} {all_points:<6} {pass_points:<6} {score:<6} {status:<10}")

print("-" * 80)
score_rate = (total_score / total_all * 100) if total_all > 0 else 0
pass_rate  = (total_pass  / total_all * 100) if total_all > 0 else 0
print(f"{'总计':<20} {total_all:<6} {total_pass:<6} {total_score:<6}")
print()
print("📈 测试统计:")
print(f"   • 测试总数: {total_tests}")
print(f"   • 总分: {total_all} 分")
print(f"   • 获得分数: {total_score} 分")
print(f"   • 得分率: {score_rate:.1f}%")
print(f"   • 通过率: {pass_rate:.1f}%")
print("="*80)
EOF

done

echo ""
echo ">>> 离线评测完成！已从 '${LOG_FILE}' 读取并评测。"


