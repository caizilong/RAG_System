#include "VoiceLLMService.h"
#include "TextUtils.h"
#include "RagUtils.h"
#include <iostream>

VoiceLLMService::VoiceLLMService(const std::string& model)
    : server_("tcp://*:8899"),
      client_("tcp://localhost:7777"),
      llm_(model)
{}

void VoiceLLMService::handleCallback(RKLLMResult *result, LLMCallState state)
{
    if (state == RKLLM_RUN_NORMAL) {
        std::wstring ws = utf8_to_wstring(result->text);
        for (wchar_t c : ws) {
            buffer_ += c;
            if (is_split_punctuation(c)) {
                std::string send = wstring_to_utf8(extract_after_think(buffer_));
                client_.request(send);
                buffer_.clear();
            }
        }
    } else if (state == RKLLM_RUN_FINISH) {
        client_.request("END");
        buffer_.clear();
    }
}

void VoiceLLMService::runForever()
{
    while (true) {
        std::string text = server_.receive();
        server_.send("LLM OK");

        auto [query, rag] = splitRagTag(text);

        if (!rag.empty())
            llm_.setChatTemplate(buildRagPrompt(rag));
        else
            llm_.setChatTemplate("");

        llm_.run(query);
    }
}
