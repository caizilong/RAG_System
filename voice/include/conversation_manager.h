#ifndef CONVERSATION_MANAGER_H_
#define CONVERSATION_MANAGER_H_

#include <atomic>
#include <string>
#include <memory>
#include <vector>

#include "sherpa-onnx/csrc/online-recognizer.h"
#include "unicode_utils.h"
#include "zmq_client.h"

class ConversationManager {
public:
    ConversationManager(int argc, char* argv[]);
    ~ConversationManager() = default;

    void FeedAudio(const float* samples, int32_t n);
    void RunLoop();

    bool IsMicPaused() const { return pause_mic_; }
    bool ShouldStop() const { return stop_flag_; }

private:
    void ProcessEndpoint(const std::string& text);

private:
    std::atomic<bool> pause_mic_{false};
    std::atomic<bool> stop_flag_{false};

    float mic_sample_rate_ = 16000;

    // ASR model
    sherpa_onnx::OnlineRecognizer recognizer_;
    std::unique_ptr<sherpa_onnx::OnlineStream> stream_;

    ZmqClient llm_client_;
    ZmqClient tts_block_client_;
};

#endif
