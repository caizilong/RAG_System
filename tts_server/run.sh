#!/bin/bash

# TTS Server 运行脚本
# 用于启动独立的 TTS 服务器

# 默认模型路径（请根据实际情况修改）
DEFAULT_MODEL_PATH="/home/pi/RAG_System/SummerTTS/models/your_model.bin"

# 检查参数
if [ $# -eq 0 ]; then
    if [ -f "$DEFAULT_MODEL_PATH" ]; then
        MODEL_PATH="$DEFAULT_MODEL_PATH"
    else
        echo "用法: $0 <model_path>"
        echo "请指定 TTS 模型文件路径"
        exit 1
    fi
else
    MODEL_PATH="$1"
fi

# 检查模型文件是否存在
if [ ! -f "$MODEL_PATH" ]; then
    echo "错误: 找不到模型文件: $MODEL_PATH"
    exit 1
fi

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "======================================" echo "启动 TTS Server"
echo "模型路径: $MODEL_PATH"
echo "======================================"

# 运行 tts_server
"${SCRIPT_DIR}/build/tts_server" "$MODEL_PATH"
