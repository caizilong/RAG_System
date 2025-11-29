cmake_minimum_required(VERSION 3.20)
project(LLMVoiceSystem)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/llm
    ${PROJECT_SOURCE_DIR}/service
    ${PROJECT_SOURCE_DIR}/utils
    ${PROJECT_SOURCE_DIR}/zmq
)

add_executable(llm_voice
    main.cpp

    llm/LLMWrapper.cpp
    service/VoiceLLMService.cpp
    utils/TextUtils.cpp
    utils/RagUtils.cpp
    zmq/ZmqClient.cpp
    zmq/ZmqServer.cpp
)

# 链接 rkllm 库（假设在 /usr/lib）
target_link_libraries(llm_voice rkllm zmq)
