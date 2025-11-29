#include "VoiceLLMService.h"

#include <functional>
#include <iostream>

#include "RagUtils.h"
#include "TextUtils.h"

VoiceLLMService::VoiceLLMService(const std::string& model)
    : server_("tcp://*:8899"), client_("tcp://localhost:7777"), llm_(model) {}

void VoiceLLMService::handleCallback(RKLLMResult* result, LLMCallState state) {
    if (state == RKLLM_RUN_NORMAL) {
        std::wstring ws = utf8_to_wstring(result->text);
        for (wchar_t c : ws) {
            buffer_ += c;
            if (is_split_punctuation(c)) {
                std::string send     = wstring_to_utf8(extract_after_think(buffer_));
                std::string response = client_.request(send);
                std::cout << "[tts -> llm] received: " << response << std::endl;
                buffer_.clear();
            }
        }
    } else if (state == RKLLM_RUN_FINISH) {
        if (!buffer_.empty()) {
            std::string send     = wstring_to_utf8(extract_after_think(buffer_)) + "END";
            std::string response = client_.request(send);
            std::cout << "[tts -> llm] received: " << response << std::endl;
            buffer_.clear();
        } else {
            std::string response = client_.request("END");
            std::cout << "[tts -> llm] received: " << response << std::endl;
        }
    }
}

void VoiceLLMService::runForever() {
    // 创建回调函数对象
    std::function<void(RKLLMResult*, LLMCallState)> callback =
        [this](RKLLMResult* result, LLMCallState state) { this->handleCallback(result, state); };

    while (true) {
        std::string text = server_.receive();
        std::cout << "[voice -> llm] received: " << text << std::endl;
        server_.send("LLM OK");

        auto [query, rag] = splitRagTag(text);

        if (!rag.empty())
            llm_.setChatTemplate(buildRagPrompt(rag));
        else
            llm_.setChatTemplate("");

        llm_.run(query, &callback);
    }
}
