#!/bin/bash

# 日志分析脚本 - 直接分析已有的 qemu_all.log 文件
# 用法: ./analyze_log.sh [log_file]

# 如果任何命令失败，立即退出脚本
set -e

# --- 配置区 ---
LOG_FILE="${1:-qemu_all.log}"
TEMP_RESULT="temp_all_result.json"

# 测试配置
declare -A TESTS=(
    ["basic-glibc"]="judge/judge_basic.py"
    ["basic-musl"]="judge/judge_basic.py" 
    ["busybox-glibc"]="judge/judge_busybox.py"
    ["busybox-musl"]="judge/judge_busybox.py"
)

# --- 执行区 ---

echo "📊 开始分析日志文件: $LOG_FILE"
echo "📋 将分析以下测试:"
for test_name in "${!TESTS[@]}"; do
    echo "   • $test_name"
done
echo

# 检查日志文件是否存在
if [ ! -f "$LOG_FILE" ]; then
    echo "❌ 错误: 日志文件 '$LOG_FILE' 未找到！"
    echo "💡 使用方法: $0 [日志文件路径]"
    echo "💡 默认会查找 qemu_all.log"
    exit 1
fi

echo ">>> 步骤 1: 分析日志并进行多项评测..."

# 初始化总结果
echo "[]" > ${TEMP_RESULT}

# 评测每个测试
for test_name in "${!TESTS[@]}"; do
    judge_script="${TESTS[$test_name]}"
    
    echo
    echo "🔍 正在评测: $test_name"
    echo "─────────────────────────────────────────"
    
    # 检查评测脚本是否存在
    if [ ! -f "$judge_script" ]; then
        echo "⚠️  警告: 评测脚本 '$judge_script' 未找到，跳过 $test_name"
        continue
    fi
    
    # 定义标记
    START_MARKER="#### OS COMP TEST GROUP START ${test_name}"
    END_MARKER="#### OS COMP TEST GROUP END ${test_name}"
    
    # 提取对应测试的日志内容
    test_log=$(mktemp)
    awk "/${START_MARKER}/{p=1;next} /${END_MARKER}/{p=0} p" ${LOG_FILE} > ${test_log}
    
    # 检查是否找到测试内容
    if [ ! -s "$test_log" ]; then
        echo "⚠️  警告: 在日志中未找到 $test_name 的测试内容"
        rm -f "$test_log"
        continue
    fi
    
    # 显示找到的测试行数
    line_count=$(wc -l < "$test_log")
    echo "📄 找到 $line_count 行测试日志"
    
    # 运行评测脚本
    test_result=$(mktemp)
    if python3 ${judge_script} < ${test_log} > ${test_result} 2>/dev/null; then
        # 读取评测结果
        if [ -s "$test_result" ]; then
            # 获取JSON结果（通常是最后一行）
            json_result=$(tail -n 1 "$test_result" | grep '^\[.*\]$' || echo "[]")
            if [ "$json_result" != "[]" ]; then
                # 为每个结果添加测试类型标识
                # 将 JSON 结果写入临时文件以避免 shell 转义问题
                temp_json=$(mktemp)
                echo "$json_result" > "$temp_json"
                
                enhanced_result=$(python3 -c "
import json
import sys
try:
    with open('$temp_json', 'r') as f:
        data = json.load(f)
    for item in data:
        item['test_type'] = '$test_name'
    print(json.dumps(data))
except Exception as e:
    print('[]')
    print(f'JSON parse error: {e}', file=sys.stderr)
")
                rm -f "$temp_json"
                echo "✅ $test_name 评测完成"
                
                # 合并到总结果中
                temp_enhanced=$(mktemp)
                echo "$enhanced_result" > "$temp_enhanced"
                
                python3 -c "
import json
import sys
try:
    with open('${TEMP_RESULT}', 'r') as f:
        all_results = json.load(f)
    with open('$temp_enhanced', 'r') as f:
        new_results = json.load(f)
    all_results.extend(new_results)
    with open('${TEMP_RESULT}', 'w') as f:
        json.dump(all_results, f)
except Exception as e:
    print(f'Error merging results: {e}', file=sys.stderr)
"
                rm -f "$temp_enhanced"
            else
                echo "⚠️  $test_name: 未找到有效的评测结果"
            fi
        else
            echo "⚠️  $test_name: 评测脚本无输出"
        fi
    else
        echo "❌ $test_name: 评测脚本执行失败"
    fi
    
    # 清理临时文件
    rm -f "$test_log" "$test_result"
done

echo
echo ">>> 步骤 2: 生成综合评测报告..."

# 生成最终报告
python3 << 'EOF'
import json
import sys
from collections import defaultdict

try:
    with open('temp_all_result.json', 'r') as f:
        content = f.read().strip()
        
    if not content or content == "[]":
        print("⚠️  警告: 没有找到任何评测结果")
        sys.exit(0)
        
    results = json.loads(content)
    
    if not results:
        print("⚠️  警告: 评测结果为空")
        sys.exit(0)
    
    # 按测试类型分组
    grouped_results = defaultdict(list)
    for result in results:
        test_type = result.get('test_type', 'unknown')
        grouped_results[test_type].append(result)
    
    print("\n" + "="*100)
    print("🎯 综合系统评测报告")
    print("="*100)
    
    # 总体统计
    total_tests = len(results)
    total_all = sum(r['all'] for r in results)
    total_pass = sum(r['pass'] for r in results)
    total_score = sum(r['score'] for r in results)
    
    print(f"\n📊 总体概览:")
    print(f"   • 测试类型数: {len(grouped_results)}")
    print(f"   • 测试用例数: {total_tests}")
    print(f"   • 总分: {total_all}")
    print(f"   • 得分: {total_score}")
    print(f"   • 得分率: {total_score/total_all*100:.1f}%" if total_all > 0 else "   • 得分率: 0%")
    
    # 按测试类型显示详细结果
    test_type_order = ['basic-glibc', 'basic-musl', 'busybox-glibc', 'busybox-musl']
    
    for test_type in test_type_order:
        if test_type not in grouped_results:
            continue
            
        type_results = grouped_results[test_type]
        type_all = sum(r['all'] for r in type_results)
        type_pass = sum(r['pass'] for r in type_results)
        type_score = sum(r['score'] for r in type_results)
        
        print(f"\n" + "─"*100)
        print(f"📋 {test_type.upper()} 测试结果")
        print("─"*100)
        
        print(f"{'测试名称':<25} {'总分':<6} {'通过':<6} {'得分':<6} {'状态':<10}")
        print("─" * 70)
        
        # 按得分排序
        type_results_sorted = sorted(type_results, key=lambda x: (-x['score'], -x['pass'], x['name']))
        
        for result in type_results_sorted:
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
            
            print(f"{name:<25} {all_points:<6} {pass_points:<6} {score:<6} {status:<10}")
        
        # 子项统计
        pass_rate = (type_pass / type_all * 100) if type_all > 0 else 0
        score_rate = (type_score / type_all * 100) if type_all > 0 else 0
        
        print("─" * 70)
        print(f"{'小计':<25} {type_all:<6} {type_pass:<6} {type_score:<6}")
        print(f"📈 {test_type} 统计: 得分率 {score_rate:.1f}%, 通过率 {pass_rate:.1f}%")
    
    print("\n" + "="*100)
    print("🏆 最终统计")
    print("="*100)
    
    final_pass_rate = (total_pass / total_all * 100) if total_all > 0 else 0
    final_score_rate = (total_score / total_all * 100) if total_all > 0 else 0
    
    print(f"   📊 测试类型: {len(grouped_results)} 种")
    print(f"   📝 测试用例: {total_tests} 个")  
    print(f"   💯 总分: {total_all} 分")
    print(f"   🎯 得分: {total_score} 分")
    print(f"   📈 得分率: {final_score_rate:.1f}%")
    print(f"   ✅ 通过率: {final_pass_rate:.1f}%")
    
    # 评级
    if final_score_rate >= 90:
        grade = "🥇 优秀"
    elif final_score_rate >= 75:
        grade = "🥈 良好"  
    elif final_score_rate >= 60:
        grade = "🥉 及格"
    else:
        grade = "📉 需要改进"
        
    print(f"   🏅 评级: {grade}")
    print("="*100)

except FileNotFoundError:
    print("⚠️  警告: 找不到评测结果文件")
except json.JSONDecodeError as e:
    print(f"⚠️  警告: JSON解析失败: {e}")
except Exception as e:
    print(f"⚠️  错误: {e}")
EOF

# 清理临时文件
rm -f ${TEMP_RESULT}

echo ""
echo ">>> 日志分析完成！"
echo "📄 分析的日志文件: ${LOG_FILE}"
echo "💡 提示: 你可以使用 './judge.sh basic' 或 './judge.sh busybox' 来单独评测特定类型。"