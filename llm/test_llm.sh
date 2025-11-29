#!/bin/bash

# LLM Voice System 测试脚本

echo "==================== LLM Voice System Test ===================="
echo ""

# 检查可执行文件
if [ ! -f "./llm/build/llm_voice" ]; then
    echo "❌ 错误: llm_voice 可执行文件不存在"
    echo "请先运行编译:"
    echo "  cd llm/build"
    echo "  cmake .."
    echo "  make"
    exit 1
fi

echo "✅ 可执行文件检查通过"
echo "   路径: ./llm/build/llm_voice"
echo "   大小: $(ls -lh ./llm/build/llm_voice | awk '{print $5}')"
echo ""

# 检查依赖库
echo "检查依赖库..."
if ldd ./llm/build/llm_voice | grep -q "not found"; then
    echo "⚠️  警告: 存在缺失的依赖库"
    ldd ./llm/build/llm_voice | grep "not found"
else
    echo "✅ 所有依赖库都已找到"
fi
echo ""

# 显示使用说明
echo "==================== 使用说明 ===================="
echo ""
echo "运行命令:"
echo "  ./llm/build/llm_voice <model_path>"
echo ""
echo "参数说明:"
echo "  model_path: RKLLM 模型文件路径 (如: models/llm/qwen.rkllm)"
echo ""
echo "前置条件:"
echo "  1. 确保 ZMQ 服务正常运行"
echo "     - 服务端口: tcp://*:8899 (接收 ASR 数据)"
echo "     - 客户端口: tcp://localhost:7777 (发送到 TTS)"
echo "  2. 确保模型文件存在且可访问"
echo "  3. 确保 librkllmrt.so 在系统路径中"
echo ""
echo "示例:"
echo "  ./llm/build/llm_voice /path/to/model.rkllm"
echo ""
echo "功能特性:"
echo "  ✅ LLM 推理引擎"
echo "  ✅ RAG 知识检索支持 (使用 <rag> 标签)"
echo "  ✅ 流式文本生成"
echo "  ✅ 标点符号分割输出"
echo "  ✅ Unicode 中文处理"
echo "  ✅ ZMQ 通信"
echo ""
echo "================================================================"
