cmake_minimum_required(VERSION 3.5)

project(tts_server)

# 设置 C++ 标准和编译选项
set(CMAKE_CXX_FLAGS " -O3 -fopenmp -std=c++14 ")
set(CMAKE_C_FLAGS " -O3 -fopenmp -std=c++11 ")

# SummerTTS 路径
set(SUMMERTTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../SummerTTS)

# ZMQ 通信组件路径
set(ZMQ_COMM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../zmq_comm)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SUMMERTTS_DIR}/include
    ${SUMMERTTS_DIR}/src/header
    ${SUMMERTTS_DIR}/eigen-3.4.0
    ${SUMMERTTS_DIR}/src/tn/header
    ${ZMQ_COMM_DIR}/include
)

# tts_server 源文件
file(GLOB TTS_SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# SummerTTS 核心源文件
set(SUMMERTTS_SOURCES
    # Text Normalization
    ${SUMMERTTS_DIR}/src/tn/glog/src/demangle.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/logging.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/raw_logging.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/symbolize.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/utilities.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/vlog_is_on.cc
    ${SUMMERTTS_DIR}/src/tn/glog/src/signalhandler.cc
    ${SUMMERTTS_DIR}/src/tn/gflags/src/gflags.cc
    ${SUMMERTTS_DIR}/src/tn/gflags/src/gflags_reporting.cc
    ${SUMMERTTS_DIR}/src/tn/gflags/src/gflags_completions.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/compat.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/flags.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/fst.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/fst-types.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/mapped-file.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/properties.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/symbol-table.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/symbol-table-ops.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/util.cc
    ${SUMMERTTS_DIR}/src/tn/openfst/src/lib/weight.cc
    ${SUMMERTTS_DIR}/src/tn/processor.cc
    ${SUMMERTTS_DIR}/src/tn/token_parser.cc
    ${SUMMERTTS_DIR}/src/tn/utf8_string.cc
    
    # English IPA
    ${SUMMERTTS_DIR}/src/engipa/EnglishText2Id.cpp
    ${SUMMERTTS_DIR}/src/engipa/InitIPASymbols.cpp
    ${SUMMERTTS_DIR}/src/engipa/alphabet.cpp
    ${SUMMERTTS_DIR}/src/engipa/ipa.cpp
    
    # Chinese Pinyin
    ${SUMMERTTS_DIR}/src/hz2py/hanzi2phoneid.cpp
    ${SUMMERTTS_DIR}/src/hz2py/Hanz2Piny.cpp
    ${SUMMERTTS_DIR}/src/hz2py/pinyinmap.cpp
    
    # Neural Network Operations
    ${SUMMERTTS_DIR}/src/nn_op/nn_conv1d.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_softmax.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_layer_norm.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_relu.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_gelu.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_tanh.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_flip.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_cumsum.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_softplus.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_clamp_min.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_sigmoid.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_conv1d_transposed.cpp
    ${SUMMERTTS_DIR}/src/nn_op/nn_leaky_relu.cpp
    
    # Platform
    ${SUMMERTTS_DIR}/src/platform/tts_file_io.cpp
    ${SUMMERTTS_DIR}/src/platform/tts_logger.cpp
    
    # Utils
    ${SUMMERTTS_DIR}/src/utils/utils.cpp
    
    # Modules
    ${SUMMERTTS_DIR}/src/modules/iStft.cpp
    ${SUMMERTTS_DIR}/src/modules/hann.cpp
    ${SUMMERTTS_DIR}/src/modules/attention_encoder.cpp
    ${SUMMERTTS_DIR}/src/modules/multi_head_attention.cpp
    ${SUMMERTTS_DIR}/src/modules/ffn.cpp
    ${SUMMERTTS_DIR}/src/modules/ConvFlow.cpp
    ${SUMMERTTS_DIR}/src/modules/DDSConv.cpp
    ${SUMMERTTS_DIR}/src/modules/ElementwiseAffine.cpp
    ${SUMMERTTS_DIR}/src/modules/random_gen.cpp
    ${SUMMERTTS_DIR}/src/modules/ResidualCouplingLayer.cpp
    ${SUMMERTTS_DIR}/src/modules/ResBlock1.cpp
    ${SUMMERTTS_DIR}/src/modules/WN.cpp
    ${SUMMERTTS_DIR}/src/modules/pqmf.cpp
    
    # Models
    ${SUMMERTTS_DIR}/src/models/TextEncoder.cpp
    ${SUMMERTTS_DIR}/src/models/StochasticDurationPredictor.cpp
    ${SUMMERTTS_DIR}/src/models/FixDurationPredictor.cpp
    ${SUMMERTTS_DIR}/src/models/DurationPredictor_base.cpp
    ${SUMMERTTS_DIR}/src/models/ResidualCouplingBlock.cpp
    ${SUMMERTTS_DIR}/src/models/Generator_base.cpp
    ${SUMMERTTS_DIR}/src/models/Generator_hifigan.cpp
    ${SUMMERTTS_DIR}/src/models/Generator_MS.cpp
    ${SUMMERTTS_DIR}/src/models/Generator_Istft.cpp
    ${SUMMERTTS_DIR}/src/models/Generator_MBB.cpp
    ${SUMMERTTS_DIR}/src/models/SynthesizerTrn.cpp
)

# 创建可执行文件
add_executable(tts_server
    ${TTS_SERVER_SOURCES}
    ${SUMMERTTS_SOURCES}
)

# 链接库
target_link_libraries(tts_server
    ${ZMQ_COMM_DIR}/build/libzmq_component_static.a
    zmq
    portaudio
    asound
    pthread
)
