// unicode_utils.cc
#include "unicode_utils.h"
#include <cwctype>
#include <clocale>

std::string tolowerUnicode(const std::string &input_str) {

  std::setlocale(LC_ALL, "");

  std::wstring input_wstr(input_str.size() + 1, '\0');
  std::mbstowcs(&input_wstr[0], input_str.c_str(), input_str.size());

  std::wstring lowercase_wstr;
  for (wchar_t wc : input_wstr) {
    lowercase_wstr.push_back(std::towlower(wc));
  }

  std::string lowercase_str(input_str.size() + 1, '\0');
  std::wcstombs(&lowercase_str[0], lowercase_wstr.c_str(),
                lowercase_wstr.size());

  return lowercase_str;
}
