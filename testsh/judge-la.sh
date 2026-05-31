#!/bin/bash

# 如果任何命令失败，立即退出脚本
set -e

# --- 配置区 ---
# 日志文件名
LOG_FILE="qemu.log"
# 你要运行的测试组名称 (对应 #### START/END basic ####)
TEST_NAME="basic"
# 你的评测脚本路径
JUDGE_SCRIPT="judge/judge_${TEST_NAME}.py" # 假设评测脚本在 judge/ 目录下
# 临时结果文件
TEMP_RESULT="temp_judge_result.json"

# --- 执行区 ---

echo ">>> 步骤 1: 清理项目..."
make clean

echo ">>> 步骤 2: 构建并运行内核 (输出将被重定向到 ${LOG_FILE})..."
# 运行 make qemu，并将标准输出和标准错误都重定向到 LOG_FILE
# `|| true` 的作用是：即使QEMU因为OS panic等原因非正常退出（返回非0值），
# 脚本也不会因为 `set -e` 而中止，而是会继续执行评分步骤。
make qemu-la > ${LOG_FILE} 2>&1 || true

echo ">>> QEMU 执行完毕。"
echo ">>> 步骤 3: 分析日志并调用评测脚本..."

# 定义评测标记
START_MARKER="#### OS COMP TEST GROUP START ${TEST_NAME} ####"
END_MARKER="#### OS COMP TEST GROUP END ${TEST_NAME} ####"

# 检查评测脚本是否存在
if [ ! -f "$JUDGE_SCRIPT" ]; then
    echo "错误: 评测脚本 '$JUDGE_SCRIPT' 未找到！"
    exit 1
fi

# 使用 awk 提取标记之间的内容，然后通过管道传给 Python 脚本
# 如果找不到标记，awk 不会输出任何内容，python脚本会收到EOF，这正是我们想要的行为。
START_MARKER_REGEX="#### OS COMP TEST GROUP START ${TEST_NAME}"
END_MARKER_REGEX="#### OS COMP TEST GROUP END ${TEST_NAME}"
awk "/${START_MARKER_REGEX}/{p=1;next} /${END_MARKER_REGEX}/{p=0} p" ${LOG_FILE} | python3 ${JUDGE_SCRIPT} > ${TEMP_RESULT}

echo ">>> 步骤 4: 格式化评测结果..."

# 使用Python脚本来格式化JSON结果为人类友好的表格
python3 << 'EOF'
import json
import sys

# 尝试读取结果文件
try:
    with open('temp_judge_result.json', 'r') as f:
        content = f.read().strip()
        
    if not content:
        print("⚠️  警告: 没有找到评测结果数据")
        sys.exit(0)
        
    # 分割内容，最后一行应该是JSON数组
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
    
    # 解析JSON
    results = json.loads(json_line)
    
    if not results:
        print("⚠️  警告: 评测结果为空")
        sys.exit(0)
    
    # 计算总统计
    total_tests = len(results)
    total_all = sum(r['all'] for r in results)
    total_pass = sum(r['pass'] for r in results)
    total_score = sum(r['score'] for r in results)
    
    print("\n" + "="*80)
    print("📊 系统调用测试评测结果")
    print("="*80)
    
    # 打印表头
    print(f"{'测试名称':<20} {'总分':<6} {'通过':<6} {'得分':<6} {'状态':<10}")
    print("-" * 80)
    
    # 按得分排序，通过的测试排在前面
    results_sorted = sorted(results, key=lambda x: (-x['score'], -x['pass'], x['name']))
    
    for result in results_sorted:
        name = result['name'].replace('test_', '')
        all_points = result['all']
        pass_points = result['pass']
        score = result['score']
        
        # 状态指示
        if score == all_points:
            status = "✅ 完美"
        elif score > 0:
            status = "🟡 部分"
        else:
            status = "❌ 失败"
        
        print(f"{name:<20} {all_points:<6} {pass_points:<6} {score:<6} {status:<10}")
    
    print("-" * 80)
    
    # 总结
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
        with open('temp_judge_result.json', 'r') as f:
            print(f.read())
    except:
        print("无法读取文件内容")
except Exception as e:
    print(f"⚠️  错误: {e}")
EOF

# 清理临时文件
rm -f ${TEMP_RESULT}

echo ""
echo ">>> 评测完成！详细的QEMU运行日志保存在 ${LOG_FILE} 文件中，可供调试查看。"