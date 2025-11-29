#!/bin/bash

# 测试 voice_recognition 程序
# 此脚本需要传入 sherpa-onnx 的模型参数

echo "==================== Voice Recognition Test ===================="
echo "此程序需要以下参数:"
echo "  --encoder=<path_to_encoder>"
echo "  --decoder=<path_to_decoder>"
echo "  --joiner=<path_to_joiner>"
echo "  --tokens=<path_to_tokens>"
echo ""
echo "示例:"
echo "  ./voice/build/voice_recognition \\"
echo "    --encoder=/path/to/encoder.onnx \\"
echo "    --decoder=/path/to/decoder.onnx \\"
echo "    --joiner=/path/to/joiner.onnx \\"
echo "    --tokens=/path/to/tokens.txt"
echo ""
echo "请确保:"
echo "  1. ZMQ LLM 服务器运行在 tcp://localhost:5555"
echo "  2. ZMQ TTS 服务器运行在 tcp://localhost:6677"
echo "  3. 有可用的音频输入设备"
echo ""
echo "================================================================"

# 检查可执行文件是否存在
if [ ! -f "./voice/build/voice_recognition" ]; then
    echo "错误: voice_recognition 可执行文件不存在"
    echo "请先运行: cd voice/build && cmake .. && make"
    exit 1
fi

echo ""
echo "可执行文件检查通过: ./voice/build/voice_recognition"
echo "文件大小: $(ls -lh ./voice/build/voice_recognition | awk '{print $5}')"
echo ""
