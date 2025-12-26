#include "OPatchValue.h"
#include "../engine/DownloadTask.h"
#include "../engine/GameManager.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace fs = std::filesystem;
OPatchValue::PatchValueState OPatchValue::GetState() { return currentState; }

std::string OPatchValue::GetValue(std::string key) {
  if (currentState != PatchValueState::Ready)
    return "";
  return config.Get<std::string>(key);
}

OPatchValue::OPatchValue()
    : config(""),
      valueFile(
          fs::path("patch") /
          ("value-" + GameManager::Get()->GetCurrentLanguage() + ".json")) {
  LogManager::Info("[PatchValue] Initializing patch value...");

  const std::string fileName =
      "patch/value-" + GameManager::Get()->GetCurrentLanguage() + ".json";

  LogManager::Info("[PatchValue] Looking for local file: " + fileName);

  if (std::filesystem::exists(fileName)) {
    config.SetFilePath(fileName);
    if (config.Load()) {
      LogManager::Info("[PatchValue] Loaded local patch value successfully.");
      currentState = PatchValueState::Ready;
      return;
    } else {
      LogManager::Warning("[PatchValue] File exists but failed to parse: " +
                          fileName);
    }
  } else {
    LogManager::Warning("[PatchValue] Local patch value file not found.");
  }

  currentState = PatchValueState::NotReady;
}

bool OPatchValue::Download() {
  if (currentState == PatchValueState::Ready) {
    LogManager::Info("[PatchValue] Already ready, skip download.");
    return true;
  }
  const std::string url =
      GameManager::Get()->Settings().Get<std::string>("patchRemoteUrl") +
      "/latest/" + valueFile.filename().string();

  LogManager::Info("[PatchValue] Downloading from: " + url);
  downloadTask = std::make_unique<DownloadTask>(
      url,

      [this, url](const std::string content) {
        std::filesystem::create_directories(valueFile.parent_path());

        std::ofstream outFile(valueFile, std::ios::binary);
        if (outFile) {
          outFile << content;
          outFile.close();

          LogManager::Info("[PatchValue] Downloaded and saved to: " +
                           valueFile.string());

          config.SetFilePath(valueFile.string());
          if (config.Load()) {
            LogManager::Info("[PatchValue] Patch value loaded successfully.");
            currentState = PatchValueState::Ready;
          } else {
            LogManager::Warning(
                "[PatchValue] Failed to parse downloaded file.");
            currentState = PatchValueState::Failed;
          }
        } else {
          LogManager::Warning("[PatchValue] Failed to write file: " +
                              valueFile.string());
          LogManager::Warning("[PatchValue] Patch validation will be skipped.");
          LogManager::Info("[PatchValue] You can manually download from: " +
                           url);
          LogManager::Info("[PatchValue] And place it at: " +
                           valueFile.string());

          currentState = PatchValueState::Failed;
        }
      },

      [this, url](const std::string errorMessage) {
        fs::path filePath =
            fs::path("patch") /
            ("value-" + GameManager::Get()->GetCurrentLanguage() + ".json");

        LogManager::Warning("[PatchValue] Download failed: " + errorMessage);
        LogManager::Warning("[PatchValue] Patch validation will be skipped.");
        LogManager::Info("[PatchValue] You can manually download from: " + url);
        LogManager::Info("[PatchValue] And place it at: " + filePath.string());
        currentState = PatchValueState::Failed;
      });
  if (!downloadTask) {
    LogManager::Warning("[PatchValue] Failed to create download task.");
    currentState = PatchValueState::Failed;
    return false;
  }
  return true;
}
