#include "conversation_manager.h"
#include <iostream>
#include <thread>

#include "sherpa-onnx/csrc/display.h"

ConversationManager::ConversationManager(int argc, char* argv[])
    : llm_client_("tcp://localhost:5555"),
      tts_block_client_("tcp://localhost:6677") {

    // ------------------------------
    // sherpa-onnx 配置
    // ------------------------------
    const char* usage = "speech recognition with microphone...\n";
    sherpa_onnx::ParseOptions po(usage);
    sherpa_onnx::OnlineRecognizerConfig config;

    config.Register(&po);
    po.Read(argc, argv);
    if (po.NumArgs() != 0) {
        po.PrintUsage();
        exit(EXIT_FAILURE);
    }

    recognizer_ = sherpa_onnx::OnlineRecognizer(config);
    stream_.reset(recognizer_.CreateStream());
}

void ConversationManager::FeedAudio(const float* samples, int32_t n) {
    stream_->AcceptWaveform(mic_sample_rate_, samples, n);
}

void ConversationManager::RunLoop() 
{
    sherpa_onnx::Display display(30);
    std::string last_text;
    int segment_index = 0;

    while (!stop_flag_) {

        while (recognizer_.IsReady(stream_.get())) {
            recognizer_.DecodeStream(stream_.get());
        }

        auto text = recognizer_.GetResult(stream_.get()).text;
        bool endpoint = recognizer_.IsEndpoint(stream_.get());

        if (!text.empty() && text != last_text) {
            last_text = text;
            display.Print(segment_index, tolowerUnicode(text));
        }

        if (endpoint) {
            ProcessEndpoint(text);
            recognizer_.Reset(stream_.get());
            segment_index++;
        }

        Pa_Sleep(20);
    }
}

void ConversationManager::ProcessEndpoint(const std::string& text)
{
    if (text.empty()) return;

    // ------- 发送给 LLM ---------
    std::string reply = llm_client_.request(text);
    std::cout << "[llm] " << reply << std::endl;

    // ------- 暂停麦克风，避免回声 ------
    pause_mic_ = true;

    // ------- 通知 TTS 播放 -------
    std::string resp = tts_block_client_.request("block");
    std::cout << "[tts] done: " << resp << std::endl;

    pause_mic_ = false;
}
