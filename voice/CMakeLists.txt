cmake_minimum_required(VERSION 3.12)
project(VoiceRecognitionSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sherpa-onnx 路径
set(SHERPA_ONNX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../sherpa-onnx)
set(SHERPA_ONNX_BUILD_DIR ${SHERPA_ONNX_DIR}/build)

# ZMQ 组件路径
set(ZMQ_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../zmq_comm)

# 包含目录 - 使用 sherpa-onnx 构建目录下的 _deps 来获取所有依赖
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SHERPA_ONNX_DIR}
    ${SHERPA_ONNX_BUILD_DIR}/_deps/onnxruntime-src/include
    ${SHERPA_ONNX_BUILD_DIR}/_deps/kaldi_decoder-src
    ${SHERPA_ONNX_BUILD_DIR}/_deps/kaldifst-src
    ${SHERPA_ONNX_BUILD_DIR}/_deps/kaldi_native_fbank-src
    ${SHERPA_ONNX_BUILD_DIR}/_deps/openfst-src/src/include
    ${ZMQ_COMPONENT_DIR}/include
)

# 源文件
set(VOICE_SOURCES
    src/main.cc
    src/conversation_manager.cc
    src/recorder.cc
    src/unicode_utils.cc
    ${SHERPA_ONNX_DIR}/sherpa-onnx/csrc/microphone.cc
)

# 创建可执行文件
add_executable(voice_recognition ${VOICE_SOURCES})

# 链接库目录
link_directories(
    ${SHERPA_ONNX_BUILD_DIR}/lib
    ${ZMQ_COMPONENT_DIR}/build
)

# 查找 PortAudio
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# 链接库
target_link_libraries(voice_recognition
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-cxx-api.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libkaldi-decoder-core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-fst.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-fstfar.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-kaldifst-core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libkaldi-native-fbank-core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libsherpa-onnx-portaudio_static.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libcppinyin_core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libssentencepiece_core.a
    ${SHERPA_ONNX_BUILD_DIR}/lib/libkissfft-float.a
    ${SHERPA_ONNX_BUILD_DIR}/_deps/onnxruntime-src/lib/libonnxruntime.a
    zmq_component
    zmq
    jack
    asound
    pthread
    dl
)

# 运行时库路径
set_target_properties(voice_recognition PROPERTIES
    INSTALL_RPATH "${SHERPA_ONNX_BUILD_DIR}/lib:${ZMQ_COMPONENT_DIR}/build"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# 安装
install(TARGETS voice_recognition DESTINATION bin)
