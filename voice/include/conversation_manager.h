#ifndef CONVERSATION_MANAGER_H_
#define CONVERSATION_MANAGER_H_

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "ZmqClient.h"
#include "sherpa-onnx/csrc/online-recognizer.h"
#include "unicode_utils.h"

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
    static sherpa_onnx::OnlineRecognizerConfig ParseConfig(int argc, char* argv[]);

   private:
    std::atomic<bool> pause_mic_{false};
    std::atomic<bool> stop_flag_{false};

    float mic_sample_rate_ = 16000;

    // ASR model
    sherpa_onnx::OnlineRecognizer recognizer_;
    std::unique_ptr<sherpa_onnx::OnlineStream> stream_;

    zmq_component::ZmqClient llm_client_;
    zmq_component::ZmqClient tts_block_client_;
};

#endif
