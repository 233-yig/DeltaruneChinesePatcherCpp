#include "OReadme.h"

#include "../engine/GameManager.h"
#include "../engine/LogManager.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

std::string OReadme::FindReadmeFile() const {
  std::string lang = GameManager::Get()->GetCurrentLanguage();

  std::vector<std::string> candidates = {"readme-" + lang + ".txt",
                                         "readme.txt"};

  for (const auto &filename : candidates) {
    if (fs::exists(filename)) {
      LogManager::Info("[Readme] Found readme file: " + filename);
      return filename;
    }
  }

  LogManager::Warning("[Readme] No readme file found.");
  return "";
}

std::string OReadme::LoadReadmeContent(const std::string &filename) const {
  if (filename.empty()) {
    return "";
  }

  try {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      LogManager::Warning("[Readme] Failed to open: " + filename);
      return "";
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  } catch (const std::exception &e) {
    LogManager::Warning("[Readme] Failed to load readme: " +
                        std::string(e.what()));
    return "";
  }
}

std::string OReadme::GetReadmeText() const {
  std::string file = FindReadmeFile();
  return LoadReadmeContent(file);
}
