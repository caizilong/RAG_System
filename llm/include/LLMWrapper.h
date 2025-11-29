#pragma once
#include <string>
#include "rkllm.h"

class LLMWrapper {
public:
    LLMWrapper(const std::string& model_path);
    ~LLMWrapper();

    void setChatTemplate(const std::string& system_prompt);
    void run(const std::string& user_input);

private:
    LLMHandle handle_;
};
