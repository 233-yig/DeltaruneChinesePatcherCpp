#include "GameUtil.h"
#include <picosha2.h>
#include <string>

std::wstring GameUtil::Utf8ToWide(const std::string &s) {
  if (s.empty())
    return {};
  int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  std::wstring ws(len - 1, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws.data(), len);
  return ws;
}

std::string GameUtil::WideToUtf8(const std::wstring &ws) {
  if (ws.empty())
    return {};
  int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr,
                                nullptr);
  std::string s(len - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), len, nullptr,
                      nullptr);
  return s;
}

std::string GameUtil::CalcFileSha256(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return "";
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  std::string data = ss.str();

  std::string outHex = picosha2::hash256_hex_string(data);
  return outHex;
}

#ifdef _WIN32
std::string GameUtil::GetRegValue(HKEY hKey, const std::string &subKey,
                                  const std::string &value) {

  std::wstring wSubKey = Utf8ToWide(subKey);
  std::wstring wValue = Utf8ToWide(value);

  DWORD size = 0, type = 0;
  if (RegGetValueW(hKey, wSubKey.c_str(), wValue.c_str(),
                   RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY, &type, nullptr,
                   &size) != ERROR_SUCCESS) {
    return {};
  }

  std::wstring wPath(size / sizeof(wchar_t), L'\0');

  if (RegGetValueW(hKey, wSubKey.c_str(), wValue.c_str(),
                   RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY, &type, wPath.data(),
                   &size) != ERROR_SUCCESS) {
    return {};
  }

  if (!wPath.empty() && wPath.back() == L'\0')
    wPath.pop_back();

  return WideToUtf8(wPath);
}
#endif