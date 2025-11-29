# Voice Recognition Module

这是一个基于 sherpa-onnx 的语音识别模块,完全独立于 sherpa-onnx 源码目录,可以作为独立模块使用。

## 目录结构

```
voice/
├── include/           # 头文件
│   ├── conversation_manager.h   # 对话管理器
│   ├── recorder.h              # 录音器
│   └── unicode_utils.h         # Unicode工具
├── src/              # 源文件
│   ├── main.cc                # 主程序入口
│   ├── conversation_manager.cc # 对话管理实现
│   ├── recorder.cc            # 录音器实现
│   └── unicode_utils.cc       # Unicode工具实现
├── build/            # 构建目录
├── CMakeLists.txt    # CMake 配置
└── test_voice.sh     # 测试脚本
```

## 功能说明

### 1. 主要组件

#### ConversationManager (对话管理器)
- **功能**: 管理语音识别流程和对话逻辑
- **核心方法**:
  - `FeedAudio()`: 将音频数据送入识别引擎
  - `RunLoop()`: 运行识别循环
  - `ProcessEndpoint()`: 处理识别结果端点

#### Recorder (录音器)
- **功能**: 管理音频输入设备
- **核心方法**:
  - `Run()`: 启动录音流
  - `RecordCallback()`: 音频数据回调函数

### 2. 代码运行流程

```
main() 主程序启动
  ↓
初始化 Microphone (PortAudio)
  ↓
创建 ConversationManager
  │
  ├→ 解析命令行参数 (ParseConfig)
  ├→ 创建 OnlineRecognizer
  ├→ 创建音频流 (CreateStream)
  ├→ 连接 LLM 客户端 (tcp://localhost:5555)
  └→ 连接 TTS 客户端 (tcp://localhost:6677)
  ↓
创建 Recorder 并启动录音
  │
  ├→ 选择音频设备
  ├→ 打开 PortAudio 流
  └→ 设置回调 RecordCallback
  ↓
进入主循环 RunLoop()
  │
  └→ 循环处理:
      ├→ 检查是否有足够的音频帧 (IsReady)
      ├→ 解码音频流 (DecodeStream)
      ├→ 获取识别结果 (GetResult)
      ├→ 检测是否到达端点 (IsEndpoint)
      └→ 如果到达端点:
          ├→ 发送文本到 LLM (llm_client.request)
          ├→ 暂停麦克风 (pause_mic_ = true)
          ├→ 通知 TTS 播放 (tts_block_client.request)
          └→ 恢复麦克风 (pause_mic_ = false)
```

### 3. 关键技术点

#### 音频回调机制
```cpp
int32_t RecordCallback(const void* input_buffer, ...) {
    // 检查麦克风是否暂停
    if (!conv->IsMicPaused()) {
        // 将音频数据送入识别引擎
        conv->FeedAudio(samples, frames);
    }
    return conv->ShouldStop() ? paComplete : paContinue;
}
```

#### 端点检测处理
```cpp
void ProcessEndpoint(const std::string& text) {
    // 1. 发送到LLM获取回复
    std::string reply = llm_client_.request(text);
    
    // 2. 暂停麦克风避免回声
    pause_mic_ = true;
    
    // 3. 通知TTS播放
    tts_block_client_.request("block");
    
    // 4. 恢复麦克风
    pause_mic_ = false;
}
```

## 编译构建

### 依赖项
- CMake >= 3.12
- C++17 编译器
- sherpa-onnx (已编译)
- zmq_component (ZMQ通信组件)
- PortAudio
- ALSA
- JACK (音频服务器)

### 构建步骤

```bash
cd voice
mkdir -p build
cd build
cmake ..
make -j4
```

### 编译输出
- 可执行文件: `voice/build/voice_recognition`
- 文件大小: ~23MB

## 运行方法

### 1. 准备工作
确保以下服务已启动:
- LLM 服务器 (tcp://localhost:5555)
- TTS 服务器 (tcp://localhost:6677)

### 2. 运行命令
```bash
./voice/build/voice_recognition \
  --encoder=/path/to/encoder.onnx \
  --decoder=/path/to/decoder.onnx \
  --joiner=/path/to/joiner.onnx \
  --tokens=/path/to/tokens.txt
```

### 3. 参数说明
- `--encoder`: 编码器模型路径
- `--decoder`: 解码器模型路径
- `--joiner`: 连接器模型路径
- `--tokens`: 词表文件路径

## 与原始 sherpa-onnx/voice 的区别

### 相同点
- 功能完全一致
- 使用相同的 sherpa-onnx ASR 引擎
- 使用相同的 ZMQ 通信机制

### 不同点
- **独立性**: 不修改 sherpa-onnx 源码目录
- **模块化**: 作为独立模块可以单独编译
- **清晰度**: 代码结构更清晰,职责分离明确

## 代码改进点

1. **初始化方式优化**: 使用静态辅助函数 `ParseConfig()` 在初始化列表中构造 `OnlineRecognizer`
2. **命名空间**: 正确使用 `zmq_component::ZmqClient`
3. **包含路径**: 添加了所有必要的依赖库包含路径
4. **链接配置**: 完整的静态库链接配置

## 注意事项

1. 确保 sherpa-onnx 已经完整编译
2. 确保 zmq_component 库已经编译
3. 音频设备需要正常工作
4. LLM 和 TTS 服务需要先启动

## 测试

运行测试脚本:
```bash
./voice/test_voice.sh
```

这将检查可执行文件是否存在并显示使用说明。
