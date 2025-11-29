cmake_minimum_required(VERSION 3.20)
project(LLMVoiceSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ZMQ 组件路径
set(ZMQ_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../zmq_comm)
set(RKLLM_RUNTIME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rknn-llm/rkllm-runtime/Linux/librkllm_api)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZMQ_COMPONENT_DIR}/include
    ${RKLLM_RUNTIME_DIR}/include
)

# 源文件
set(LLM_SOURCES
    src/main.cpp
    src/LLMWrapper.cpp
    src/VoiceLLMService.cpp
    src/TextUtils.cpp
    src/RagUtils.cpp
)

# 创建可执行文件
add_executable(llm_voice ${LLM_SOURCES})

# 链接库目录
link_directories(
    ${ZMQ_COMPONENT_DIR}/build
    ${RKLLM_RUNTIME_DIR}/aarch64
)

# 链接库
target_link_libraries(llm_voice
    ${RKLLM_RUNTIME_DIR}/aarch64/librkllmrt.so
    zmq_component
    zmq
    pthread
)
