#!/bin/bash
set -e

echo "=============== RAG System 完整编译脚本 ==============="

# 1. 编译 sherpa-onnx
echo ""
echo "[1/5] 编译 sherpa-onnx..."
cd /home/pi/RAG_System/sherpa-onnx
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake .. && make -j4
echo "✅ sherpa-onnx 编译完成"

# 2. 编译 zmq_comm
echo ""
echo "[2/5] 编译 zmq_comm..."
cd /home/pi/RAG_System/zmq_comm
mkdir -p build && cd build
cmake .. && make -j4
echo "✅ zmq_comm 编译完成"

# 3. 编译 voice
echo ""
echo "[3/5] 编译 voice..."
cd /home/pi/RAG_System/voice
mkdir -p build && cd build
cmake .. && make -j4
echo "✅ voice 编译完成"

# 4. 编译 llm
echo ""
echo "[4/5] 编译 llm..."
cd /home/pi/RAG_System/llm
mkdir -p build && cd build
cmake .. && make -j4
echo "✅ llm 编译完成"

# 5. 编译 tts_server
echo ""
echo "[5/5] 编译 tts_server..."
cd /home/pi/RAG_System/tts_server
./build.sh
echo "✅ tts_server 编译完成"

echo ""
echo "=============== 所有模块编译完成! ==============="
echo ""
echo "可执行文件位置:"
echo "  - voice:      /home/pi/RAG_System/voice/build/voice_recognition"
echo "  - llm:        /home/pi/RAG_System/llm/build/llm_voice"
echo "  - tts_server: /home/pi/RAG_System/tts_server/build/tts_server"