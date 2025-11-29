#pragma once
#include <string>
#include <utility>

std::pair<std::string, std::string> splitRagTag(const std::string& input);
std::string buildRagPrompt(const std::string& rag);
