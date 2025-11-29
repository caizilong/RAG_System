#pragma once
#include <string>

#include "LLMWrapper.h"
#include "ZmqClient.h"
#include "ZmqServer.h"

class VoiceLLMService {
   public:
    VoiceLLMService(const std::string& model_path);
    void runForever();

   private:
    void handleCallback(RKLLMResult* result, LLMCallState state);

   private:
    zmq_component::ZmqServer server_;
    zmq_component::ZmqClient client_;
    LLMWrapper llm_;

    std::wstring buffer_;
};
