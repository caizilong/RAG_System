#include "RagUtils.h"

std::pair<std::string, std::string> splitRagTag(const std::string& input)
{
    const std::string TAG = "<rag>";
    size_t pos = input.find(TAG);
    if (pos == std::string::npos)
        return {input, ""};
    return {input.substr(0, pos), input.substr(pos + TAG.size())};
}

std::string buildRagPrompt(const std::string& rag)
{
    std::string prompt =
        "你是一款智能座舱 AI 助手：\n"
        "1. 使用口语化表达\n"
        "回答必须基于以下内容：\n"
        "{rag_context}";

    size_t p = prompt.find("{rag_context}");
    return prompt.replace(p, 14, rag);
}
