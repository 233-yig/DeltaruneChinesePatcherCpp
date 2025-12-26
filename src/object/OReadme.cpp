#include "OReadme.h"
#include "../engine/LangManager.h"

#include "../engine/GameManager.h"
#include "../engine/LogManager.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

OReadme::OReadme() {
  std::string filename = FindReadmeFile();
  std::string content = LoadReadmeContent(filename);

  if (content == "") content = "Readme.NotFound";

  readmeText = new BOScrollText(content, {280, 150, 630, 450}, WHITE,
                                LangManager::GetFont().baseSize * 3 / 4);
}

OReadme::~OReadme() { delete readmeText; }

void OReadme::Update(float dt) {
  if (readmeText)
    readmeText->Update(dt);
}

void OReadme::Draw() {
  if (readmeText)
    readmeText->Draw();
}

void OReadme::RefreshText() {
  readmeText->RefreshText();
}
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
  if (filename.empty())
    return "";

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
