#include "LLMWrapper.h"

#include <cstring>
#include <functional>
#include <iostream>

static int GlobalCallback(RKLLMResult* result, void* userdata, LLMCallState state) {
    // 将回调内容转发给userdata
    if (userdata) {
        auto func = reinterpret_cast<std::function<void(RKLLMResult*, LLMCallState)>*>(userdata);
        (*func)(result, state);
    }
    return 0;
}

LLMWrapper::LLMWrapper(const std::string& model_path) {
    RKLLMParam param         = rkllm_createDefaultParam();
    param.model_path         = model_path.c_str();
    param.max_new_tokens     = 100;
    param.max_context_len    = 256;
    param.skip_special_token = true;

    int ret = rkllm_init(&handle_, &param, GlobalCallback);
    if (ret != 0) {
        std::cerr << "rkllm init failed\n";
        handle_ = nullptr;
    }
}

LLMWrapper::~LLMWrapper() {
    if (handle_) {
        rkllm_destroy(handle_);
    }
}

void LLMWrapper::setChatTemplate(const std::string& system_prompt) {
    rkllm_set_chat_template(handle_, system_prompt.c_str(), "<｜User｜>",
                            "<｜Assistant｜><think>\n</think>");
}

void LLMWrapper::run(const std::string& user_input, void* userdata) {
    RKLLMInput input;
    memset(&input, 0, sizeof(input));
    input.input_type   = RKLLM_INPUT_PROMPT;
    input.prompt_input = (char*)user_input.c_str();

    RKLLMInferParam infer;
    memset(&infer, 0, sizeof(infer));
    infer.mode         = RKLLM_INFER_GENERATE;
    infer.keep_history = 0;

    rkllm_run(handle_, &input, &infer, userdata);
}
