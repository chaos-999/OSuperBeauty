#!/bin/bash

# 如果任何命令失败，立即退出脚本
set -e

# --- 配置区 ---
LOG_FILE="qemu.log"
# 支持多个评测组（按需扩展）。若传参，则用传参列表；否则默认同时测 git-glibc 和 git-musl
if [ $# -gt 0 ]; then
  TEST_GROUPS=("$@")
else
  TEST_GROUPS=("git-glibc" "git-musl")
fi

TEMP_DIR=".judge_tmp"
mkdir -p "${TEMP_DIR}"

# --- 执行区 ---

echo ">>> 步骤 1: 清理项目..."
make clean

echo ">>> 步骤 2: 构建并运行内核 (输出将被重定向到 ${LOG_FILE})..."
# 即使 QEMU 非0退出也继续执行评分
make qemu-la 2>&1 | tee ${LOG_FILE} || true

echo ">>> QEMU 执行完毕。"
echo ">>> 步骤 3: 分析日志并调用评测脚本..."

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
  awk "/${START_MARKER_REGEX}/{p=1;next} /${END_MARKER_REGEX}/{p=0} p" ${LOG_FILE} | python3 ${JUDGE_SCRIPT} > ${TEMP_RESULT} || true

  echo ">>> 步骤 4: 格式化评测结果 (${TEST_NAME})..."

  CURRENT_TEST_GROUP="${TEST_NAME}" python3 << 'EOF'
import json
import sys

try:
    import os
    import glob
    # 自动发现最新的组结果文件
    tmp_dir = '.judge_tmp'
    files = sorted(glob.glob(os.path.join(tmp_dir, 'temp_judge_result_*.json')))
    if not files:
        print("⚠️  警告: 没有找到评测结果数据文件")
        sys.exit(0)

    # 仅格式化刚刚这轮组对应的结果文件（通过环境变量传入组名）
    group_name = os.environ.get('CURRENT_TEST_GROUP', '')
    path = os.path.join(tmp_dir, f'temp_judge_result_{group_name}.json') if group_name else files[-1]

    with open(path, 'r') as f:
        content = f.read().strip()

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

    total_tests = len(results)
    # 兼容不同评测脚本字段：以 score 作为真实分值统计，避免量纲不一致
    def row_total(r):
        return r.get('score', 0)
    def row_pass(r):
        # 若脚本未给出按分值的通过量，等同计为得分本身
        return r.get('score', 0)

    total_all = sum(row_total(r) for r in results)
    total_pass = sum(row_pass(r) for r in results)
    total_score = sum(r.get('score', 0) for r in results)

    print("\n" + "="*80)
    print(f"📊 {os.environ.get('CURRENT_TEST_GROUP', '评测')} 评测结果")
    print("="*80)

    print(f"{'测试名称':<20} {'总分':<6} {'通过':<6} {'得分':<6} {'状态':<10}")
    print("-" * 80)

    results_sorted = sorted(results, key=lambda x: (-x.get('score', 0), -x.get('pass', 0), x.get('name', '')))

    for result in results_sorted:
        name = result.get('name', '')
        score = result.get('score', 0)
        # 以 score 作为显示用总分与通过分，保持量纲一致
        all_points = score
        pass_points = score
        status = "✅ 完美" if score >= all_points else ("🟡 部分" if score > 0 else "❌ 失败")
        print(f"{name:<20} {all_points:<6} {pass_points:<6} {score:<6} {status:<10}")

    print("-" * 80)
    pass_rate = (total_pass / total_all * 100) if total_all > 0 else 0
    score_rate = (total_score / total_all * 100) if total_all > 0 else 0

    print(f"{'总计':<20} {total_all:<6} {total_pass:<6} {total_score:<6}")
    print()
    print(f"📈 测试统计:")
    print(f"   • 测试总数: {total_tests}")
    print(f"   • 总分: {total_all} 分")
    print(f"   • 获得分数: {total_score} 分")
    print(f"   • 得分率: {score_rate:.1f}%")
    print(f"   • 通过率: {pass_rate:.1f}%")
    print("="*80)

except FileNotFoundError:
    print("⚠️  警告: 找不到评测结果文件")
except json.JSONDecodeError as e:
    print(f"⚠️  警告: JSON解析失败: {e}")
    print("原始内容:")
    try:
        with open(path, 'r') as f:
            print(f.read())
    except:
        print("无法读取文件内容")
except Exception as e:
    print(f"⚠️  错误: {e}")
EOF
done

# 保留临时结果文件，避免覆盖，可用于后续对比

echo ""
echo ">>> 评测完成！详细的QEMU运行日志保存在 ${LOG_FILE} 文件中，可供调试查看。"


