// recorder.h

#ifndef RECORDER_H_
#define RECORDER_H_

#include <atomic>
#include <memory>

#include "portaudio.h"
#include "sherpa-onnx/csrc/online-recognizer.h"
#include "conversation_manager.h"

class Recorder {
public:
    explicit Recorder(ConversationManager& conv_manager);
    ~Recorder();

    void Run();

private:
    static int32_t RecordCallback(
        const void* input_buffer,
        void* output_buffer,
        unsigned long frames_per_buffer,
        const PaStreamCallbackTimeInfo* time_info,
        PaStreamCallbackFlags status_flags,
        void* user_data);

private:
    ConversationManager& conv_;
    PaStream* stream_ = nullptr;

    float mic_sample_rate_ = 16000;
    std::atomic<bool> stop_;
};

#endif  // RECORDER_H_
