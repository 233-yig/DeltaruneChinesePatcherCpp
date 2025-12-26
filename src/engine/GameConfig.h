#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "LogManager.h"
#include <nlohmann/json.hpp>
#include <string>

class GameConfig {
public:
  GameConfig(const std::string &path, nlohmann::json defaultValues = {});

  bool Load();
  bool Save() const;

  void Set(const std::string &key, const nlohmann::json &value);

  template <typename T> T Get(const std::string &key) const {
    nlohmann::json result = GetJson(key);
    if (result == nullptr) {
      LogManager::Warning("Can't find key " + key + " in " + filePath);
      return T{};
    }
    return result.get<T>();
  }

  bool Has(const std::string &key) const;
  void SetFilePath(const std::string &path);

private:
  std::string filePath;
  nlohmann::json values;
  nlohmann::json defaults;

  void FillDefaults();
  nlohmann::json GetJson(const std::string &key) const;
};

#endif
