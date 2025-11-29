#include "TextUtils.h"
#include <codecvt>
#include <locale>

static const std::set<wchar_t> SPLIT_CHARS = {
    L'：', L'，', L'。', L'\n', L'；', L'！', L'？'
};

std::wstring utf8_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(str);
}

bool is_split_punctuation(wchar_t c)
{
    return SPLIT_CHARS.count(c) > 0;
}

std::wstring extract_after_think(const std::wstring& input)
{
    const std::wstring punct = L" \t\n\r*#@$%^&，。：、；！？【】（）“”‘’";

    std::wstring output;
    for (wchar_t c : input) {
        if (punct.find(c) == std::wstring::npos)
            output += c;
    }
    return output;
}
