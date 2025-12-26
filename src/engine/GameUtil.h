#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#include <string>
#include "../extra/WindowsIncludeFix.h"

namespace GameUtil {
std::wstring Utf8ToWide(const std::string &s);
std::string WideToUtf8(const std::wstring &ws);
std::string CalcFileSha256(const std::string &path);
#ifdef _WIN32
std::string GetRegValue(HKEY hKey, const std::string &subKey,
                        const std::string &value);
#endif
}
#endif