#include "GameConfig.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

GameConfig::GameConfig(const std::string &path, nlohmann::json defaultValues)
    : filePath(path), defaults(std::move(defaultValues)) {}

bool GameConfig::Load() {
  if (!fs::exists(filePath)) {
    values = defaults;
    return Save();
  }

  std::ifstream i(filePath);
  if (!i.is_open())
    return false;

  i >> values;
  FillDefaults();
  return true;
}

bool GameConfig::Save() const {
  std::ofstream o(filePath);
  if (!o.is_open())
    return false;
  o << values.dump(4);
  return true;
}

void GameConfig::FillDefaults() {
  bool needSave = false;
  for (auto &[key, val] : defaults.items()) {
    if (!values.contains(key)) {
      values[key] = val;
      needSave = true;
    }
  }
  if (needSave)
    Save();
}

void GameConfig::Set(const std::string &key, const nlohmann::json &value) {
  values[key] = value;
  Save();
}

nlohmann::json GameConfig::GetJson(const std::string &key) const {
  if (values.contains(key))
    return values.at(key);

  if (defaults.contains(key))
    return defaults.at(key);

  return nullptr;
}

bool GameConfig::Has(const std::string &key) const {
  return values.contains(key) || defaults.contains(key);
}

void GameConfig::SetFilePath(const std::string &path) {
  filePath = path;
}