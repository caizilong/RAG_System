#!/bin/bash

# TTS Server 编译脚本
# 用于独立编译 tts_server，脱离 SummerTTS 原始构建系统

set -e  # 遇到错误立即退出

# 颜色输出
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}======================================${NC}"
echo -e "${GREEN}开始编译 TTS Server${NC}"
echo -e "${GREEN}======================================${NC}"

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo -e "${YELLOW}项目根目录: ${PROJECT_ROOT}${NC}"
echo -e "${YELLOW}TTS Server 目录: ${SCRIPT_DIR}${NC}"

# 检查必要的依赖目录
if [ ! -d "${PROJECT_ROOT}/SummerTTS" ]; then
    echo -e "${RED}错误: 找不到 SummerTTS 目录${NC}"
    exit 1
fi

if [ ! -d "${PROJECT_ROOT}/zmq_comm" ]; then
    echo -e "${RED}错误: 找不到 zmq_comm 目录${NC}"
    exit 1
fi

# 首先编译 zmq_comm
echo -e "${YELLOW}======================================${NC}"
echo -e "${YELLOW}步骤 1: 编译 zmq_comm 组件${NC}"
echo -e "${YELLOW}======================================${NC}"

cd "${PROJECT_ROOT}/zmq_comm"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make -j$(nproc)

if [ ! -f "libzmq_component_static.a" ]; then
    echo -e "${RED}错误: zmq_comm 编译失败${NC}"
    exit 1
fi
echo -e "${GREEN}zmq_comm 编译成功${NC}"

# 编译 tts_server
echo -e "${YELLOW}======================================${NC}"
echo -e "${YELLOW}步骤 2: 编译 tts_server${NC}"
echo -e "${YELLOW}======================================${NC}"

cd "${SCRIPT_DIR}"
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

cmake ..
make -j$(nproc)

if [ -f "tts_server" ]; then
    echo -e "${GREEN}======================================${NC}"
    echo -e "${GREEN}TTS Server 编译成功!${NC}"
    echo -e "${GREEN}======================================${NC}"
    echo -e "${YELLOW}可执行文件位置: ${SCRIPT_DIR}/build/tts_server${NC}"
    echo -e "${YELLOW}使用方法: ./tts_server <model_path>${NC}"
else
    echo -e "${RED}错误: tts_server 编译失败${NC}"
    exit 1
fi
