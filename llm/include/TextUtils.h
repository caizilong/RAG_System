#pragma once
#include <string>
#include <set>

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);
std::wstring extract_after_think(const std::wstring& text);
bool is_split_punctuation(wchar_t c);
