#include "OCheckTool.h"
#include "../engine/GameManager.h"
#include "../engine/GameUtil.h"
#include <filesystem>

OCheckTool::OCheckTool() : toolList({}) {
  if (GameManager::Get()->Settings().Has("toolSha256")) {
    auto tools =
        GameManager::Get()->Settings().Get<nlohmann::json>("toolSha256");
    if (tools.is_object()) {
      toolList = tools;
      LogManager::Info("[ToolCheck] Loaded tool SHA256 config.");
    } else {
      LogManager::Warning(
          "[ToolCheck] toolSha256 exists but is not a JSON object. "
          "Skip tool verification.");
      skipCheck = true;
    }
  } else {
    LogManager::Warning(
        "[ToolCheck] toolSha256 missing in config. Skip tool verification.");
    skipCheck = true;
  }
}

bool OCheckTool::ToolListIsMissing() { return skipCheck; }
bool OCheckTool::CheckToolExists() {
  if (skipCheck)
    return true;
  for (auto &[key, value] : toolList.items()) {
    std::string path = key;
    if (!std::filesystem::exists(path)) {
      LogManager::Error("[ToolCheck] Tool is missing: " + path);
      LogManager::Info("[ToolCheck] Please re-download the patcher package.");
      return false;
    }
  }
  return true;
}

bool OCheckTool::CheckToolSha256() {
  if (skipCheck) {
    LogManager::Info("[ToolCheck] Skipped SHA256 verification.");
    return true;
  }

  for (auto &[key, value] : toolList.items()) {
    const std::string path = key;
    const std::string expectedHash = value.get<std::string>();

    const std::string actualHash = GameUtil::CalcFileSha256(path);

    if (actualHash != expectedHash) {
      LogManager::Warning("[ToolCheck] SHA256 mismatch for tool: " + path);
      LogManager::Warning("[ToolCheck] Expected: " + expectedHash);
      LogManager::Warning("[ToolCheck] Actual:   " + actualHash);
      LogManager::Warning(
          "[ToolCheck] Tool file may be corrupted or tampered with.");
      LogManager::Info("[ToolCheck] Please re-download the patcher package.");
      return false;
    }
  }

  LogManager::Info("[ToolCheck] All tools passed SHA256 verification.");
  return true;
}
