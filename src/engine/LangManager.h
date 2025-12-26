#ifndef LANG_MANAGER_H
#define LANG_MANAGER_H

#include "GameConfig.h"
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class LangManager {
public:
  explicit LangManager(GameConfig &cfg);
  ~LangManager();

  static bool Init(GameConfig &settings);
  static void Cleanup();
  static void SetLanguage(const std::string &langId, GameConfig &settings);
  static std::string GetCurrentLanguage(GameConfig &settings);
  static void StepLanguage(int step, GameConfig &settings);
  static std::string GetText(const std::string &key);
  static Font &GetFont();
  static const std::vector<std::string> &GetAvailableLanguages();

private:
  static bool LoadFont(GameConfig &settings);
  static bool LoadLanguageList();
  static bool LoadLanguageData(const std::string &langId);

  static Font mainFont;
  static nlohmann::json currentLangData;
  static std::vector<std::string> langList;
};

#endif