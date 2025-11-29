// recorder.cc

#include "recorder.h"
#include <iostream>
#include <cstdlib>

#include "unicode_utils.h"

Recorder::Recorder(ConversationManager& conv_manager)
    : conv_(conv_manager), stop_(false) {

    // 初始化 PortAudio 由 Microphone 类完成
    // 此处仅打开流

    PaDeviceIndex device_index = Pa_GetDefaultInputDevice();
    if (device_index == paNoDevice) {
        fprintf(stderr, "No default input device.\n");
        exit(EXIT_FAILURE);
    }

    // 允许用户通过环境变量指定
    if (const char* p = getenv("SHERPA_ONNX_MIC_DEVICE")) {
        device_index = atoi(p);
    }

    // 显示设备
    int num_devices = Pa_GetDeviceCount();
    for (int i = 0; i < num_devices; ++i) {
        const auto* info = Pa_GetDeviceInfo(i);
        fprintf(stderr, " %s [%d] %s\n",
                (i == device_index) ? "*" : " ",
                i,
                info->name);
    }

    const PaDeviceInfo* info = Pa_GetDeviceInfo(device_index);
    mic_sample_rate_ = 16000;

    if (const char* p = getenv("SHERPA_ONNX_MIC_SAMPLE_RATE")) {
        mic_sample_rate_ = atof(p);
    }

    PaStreamParameters param;
    param.device = device_index;
    param.channelCount = 1;
    param.sampleFormat = paFloat32;
    param.suggestedLatency = info->defaultLowInputLatency;
    param.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream_,
        &param,
        nullptr,
        mic_sample_rate_,
        0,
        paClipOff,
        RecordCallback,
        &conv_
    );

    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
}

Recorder::~Recorder() {
    if (stream_) {
        Pa_CloseStream(stream_);
    }
}

int32_t Recorder::RecordCallback(
    const void* input_buffer,
    void*,
    unsigned long frames,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void* user_data) 
{
    auto* conv = reinterpret_cast<ConversationManager*>(user_data);

    if (!conv->IsMicPaused()) {
        conv->FeedAudio(reinterpret_cast<const float*>(input_buffer), frames);
    }

    return conv->ShouldStop() ? paComplete : paContinue;
}

void Recorder::Run() {
    Pa_StartStream(stream_);

    conv_.RunLoop();

    stop_ = true;
}
