# TTS Server - 独立语音合成服务器

这是一个从 SummerTTS 中分离出来的独立 TTS 服务器组件。

## 项目结构

```
tts_server/
├── include/           # 头文件
│   ├── TTSModel.h    # TTS 模型封装
│   ├── AudioPlayer.h # 音频播放器
│   ├── MessageQueue.h# 消息队列
│   ├── TextProcessor.h # 文本处理
│   └── Utils.h       # 工具函数
├── src/              # 源文件
│   ├── main.cpp      # 主程序
│   ├── TTSModel.cpp
│   ├── AudioPlayer.cpp
│   ├── MessageQueue.cpp
│   ├── TextProcessor.cpp
│   └── Utils.cpp
├── build/            # 编译输出目录
├── CMakeLists.txt    # CMake 配置文件
├── build.sh          # 编译脚本
├── run.sh            # 运行脚本
└── README.md         # 本文件
```

## 依赖项

本项目依赖以下组件和库：

### 内部依赖
- **SummerTTS**: 核心 TTS 模型实现（位于 `../SummerTTS`）
- **zmq_comm**: ZMQ 通信组件（位于 `../zmq_comm`）

### 外部依赖
- libzmq: ZeroMQ 消息队列库
- portaudio: 跨平台音频 I/O 库
- alsa: Linux 音频库
- pthread: POSIX 线程库
- OpenMP: 并行计算支持

## 编译说明

### 1. 编译步骤

使用提供的编译脚本：

```bash
cd /home/pi/RAG_System/tts_server
./build.sh
```

编译脚本会自动完成以下步骤：
1. 检查必要的依赖目录（SummerTTS, zmq_comm）
2. 编译 zmq_comm 静态库
3. 编译 tts_server 可执行文件

### 2. 手动编译（可选）

如果需要手动编译：

```bash
# 1. 编译 zmq_comm
cd /home/pi/RAG_System/zmq_comm
mkdir -p build && cd build
cmake ..
make

# 2. 编译 tts_server
cd /home/pi/RAG_System/tts_server
mkdir -p build && cd build
cmake ..
make -j4
```

### 3. 编译输出

编译成功后，可执行文件位于：`build/tts_server`

## 运行说明

### 基本用法

```bash
./build/tts_server <model_path>
```

参数说明：
- `<model_path>`: TTS 模型文件路径

### 使用运行脚本

编辑 `run.sh` 中的 `DEFAULT_MODEL_PATH` 变量，然后运行：

```bash
./run.sh
```

或指定模型路径：

```bash
./run.sh /path/to/your/model.bin
```

## 通信协议

TTS Server 使用 ZeroMQ 进行通信，监听两个端口：

1. **端口 7777**: 接收文本消息（来自 LLM）
   - 接收格式：UTF-8 文本字符串
   - 返回格式："Echo: received"

2. **端口 6677**: 状态通信（与 voice 模块）
   - 发送播放完成消息："[tts -> voice]play end success"

## 工作流程

1. **文本接收**: 从端口 7777 接收文本
2. **文本过滤**: 过滤掉 `<think>` 标签内容
3. **TTS 推理**: 调用 SummerTTS 进行语音合成
4. **音频播放**: 通过 ALSA 播放合成的音频
5. **状态通知**: 播放完成后通知 voice 模块

## 修改说明

### 从 SummerTTS 分离的改动

1. **CMakeLists.txt**: 创建独立的 CMake 配置
   - 引用 SummerTTS 源文件而不是复制
   - 配置独立的头文件包含路径
   - 使用静态链接 zmq_comm 库

2. **头文件修复**: 添加了缺失的 `#include <cstdint>` 和 `#include <string>`
   - `tts_server/include/Utils.h`
   - `tts_server/include/MessageQueue.h`
   - `SummerTTS/src/header/Hanz2Piny.h`
   - `SummerTTS/src/header/pinyinmap.h`
   - `SummerTTS/src/header/hanzi2phoneid.h`

3. **C++ 标准**: 从 C++11 升级到 C++14（支持 `std::make_unique`）

4. **构建脚本**: 创建独立的 `build.sh` 和 `run.sh`

## 注意事项

1. **模型文件**: 确保 TTS 模型文件存在且路径正确
2. **音频设备**: 确保系统有可用的 ALSA 音频设备
3. **端口占用**: 确保端口 7777 和 6677 未被占用
4. **依赖库**: 确保所有依赖库已安装

## 故障排除

### 编译错误

1. 如果提示找不到 SummerTTS:
   ```
   错误: 找不到 SummerTTS 目录
   ```
   解决: 确保 SummerTTS 位于 `/home/pi/RAG_System/SummerTTS`

2. 如果提示找不到 zmq_comm:
   ```
   错误: 找不到 zmq_comm 目录
   ```
   解决: 确保 zmq_comm 位于 `/home/pi/RAG_System/zmq_comm`

### 运行错误

1. 如果提示"Cannot open PCM device":
   - 检查 ALSA 配置
   - 检查音频设备是否可用

2. 如果提示 ZMQ 错误:
   - 检查端口是否被占用
   - 检查 libzmq 是否正确安装

## 目录依赖关系

```
RAG_System/
├── SummerTTS/          # TTS 核心实现（必需）
├── zmq_comm/           # ZMQ 通信组件（必需）
└── tts_server/         # 本项目
```

## 开发说明

本项目遵循以下设计原则：
- **解耦设计**: 通过引用而非复制的方式使用 SummerTTS
- **独立构建**: 完全独立的 CMake 配置和编译流程
- **最小修改**: 仅修改必要的头文件以保证编译通过
