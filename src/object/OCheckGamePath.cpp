#include "OCheckGamePath.h"
#include "../engine/GameManager.h"
#include "../engine/GameUtil.h"
#include "../engine/LogManager.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
OCheckGamePath::OCheckGamePath(OPatchValue *pv) {
  currentMode = InstallMode::Invalid;
  patchValue = pv;
}

OCheckGamePath::InstallMode OCheckGamePath::GetInstallMode() {
  return currentMode;
}
std::string OCheckGamePath::AutoDetectPath() {
#ifdef _WIN32
  LogManager::Info(
      "[Auto-Detect] Starting game path detection on Windows system...");
  std::string steamPath = GameUtil::GetRegValue(
      HKEY_LOCAL_MACHINE, "SOFTWARE\\Valve\\Steam", "InstallPath");
  if (steamPath.empty()) {
    LogManager::Warning(
        "[Auto-Detect] Failed to locate Steam installation path in registry.");
    return {};
  }
  LogManager::Info("[Auto-Detect] Steam installation found at: " + steamPath);

  fs::path steamRoot(steamPath);
  std::vector<fs::path> libPathList;
  std::ifstream libFolderFile(steamRoot / "steamapps" / "libraryfolders.vdf");
  if (!libFolderFile) {
    LogManager::Warning("[Auto-Detect] Failed to read Steam library "
                        "configuration file (libraryfolders.vdf).");
    return {};
  }

  std::string line;
  while (std::getline(libFolderFile, line)) {
    auto pos = line.find("\"path\"");
    if (pos != std::string::npos) {
      auto first = line.find('"', pos + 6);
      auto second = line.find('"', first + 1);
      if (first != std::string::npos && second != std::string::npos) {
        std::string path = line.substr(first + 1, second - first - 1);
        for (size_t i = 0; i + 1 < path.size(); ++i) {
          if (path[i] == '\\' && path[i + 1] == '\\') {
            path.erase(i, 1);
          }
        }

        fs::path lib(path);
        if (lib == steamRoot) {
          libPathList.push_back(steamRoot / "steamapps");
        } else {
          libPathList.push_back(lib / "steamapps");
        }
      }
    }
  }

  if (libPathList.empty()) {
    LogManager::Warning(
        "[Auto-Detect] No Steam library folders found in configuration.");
    return {};
  }

  std::string appId =
      GameManager::Get()->Settings().Get<std::string>("deltaruneAppId");

  for (auto &libPath : libPathList) {
    LogManager::Info("[Auto-Detect] Checking Steam library: " +
                     libPath.string());

    fs::path manifestPath = libPath / ("appmanifest_" + appId + ".acf");
    std::ifstream manifestFile(manifestPath);

    if (!manifestFile)
      continue;

    fs::path gamePath = libPath / "common" / "DELTARUNE";

    if (!fs::exists(gamePath) || !fs::exists(gamePath / "data.win")) {
      LogManager::Warning("[Auto-Detect] Game manifest found but essential "
                          "game files are missing (data.win not found).");
      return {};
    }

    LogManager::Info("[Auto-Detect] Successfully detected DELTARUNE at: " +
                     gamePath.string());
    return gamePath.string();
  }

  LogManager::Warning("[Auto-Detect] DELTARUNE installation not found in any "
                      "Steam library folder.");
  return {};
#else
  LogManager::Warning("[Auto-Detect] Automatic game path detection is only "
                      "supported on Windows systems.");
  return {};
#endif
}

void OCheckGamePath::Update(float dt) {
  (void)dt;
  if (pending && patchValue->GetState() != OPatchValue::PatchValueState::NotReady) {
    HandleValidateResult(pendingPath);
  }
}

void OCheckGamePath::TriggerSuccess(const std::string &msg,
                                    const std::string &path) {
  gamePath = path;
  stateMessage = msg;
  if (OnSuccess) {
    OnSuccess(msg, path);
  }
}

void OCheckGamePath::TriggerError(const std::string &msg) {
  stateMessage = msg;
  if (OnError) {
    OnError(msg);
  }
}

void OCheckGamePath::AutoSetPath() {
  pendingAuto = true;
  pendingPath = AutoDetectPath();

  if (pendingPath.empty()) {
    TriggerError("GamePath.NotDetected");
    return;
  }

  HandleValidateResult(pendingPath);
}

void OCheckGamePath::SetPath(const std::string &path) {
  pendingAuto = false;
  pendingPath = path;
  HandleValidateResult(path);
}
void OCheckGamePath::HandleValidateResult(const std::string &path) {
  PathState st = ValidatePath(path);

  if (st == PathState::InvalidPatchValueNotReady) {
    pending = true;
    return;
  }

  pending = false;

  if (st == PathState::Valid || st == PathState::ValidButNoACF) {
    if (pendingAuto)
      TriggerSuccess("GamePath.Detected", path);
    else
      TriggerSuccess(PathStateToMsg(st), path);
  } else {
    if (pendingAuto)
      TriggerError("GamePath.NotDetected");
    else
      TriggerError(PathStateToMsg(st));
  }
}

OCheckGamePath::PathState
OCheckGamePath::ValidatePath(const std::string &path) {
  fs::path p(path);

  LogManager::Info("[GamePath] Validating path: " + path);

  if (!fs::exists(p)) {
    LogManager::Warning("[GamePath] Path not exists: " + path);
    return PathState::InvalidNotFound;
  }

  fs::path exePath = p / "DELTARUNE.exe";
  if (!fs::exists(exePath)) {
    LogManager::Warning("[GamePath] DELTARUNE.exe not found in: " + path);
    return PathState::InvalidNotFound;
  }

  auto patchState = patchValue->GetState();
  std::string appId =
      GameManager::Get()->Settings().Get<std::string>("deltaruneAppId");

  fs::path acfPath =
      p.parent_path().parent_path() / ("appmanifest_" + appId + ".acf");

  struct Candidate {
    fs::path file;
    InstallMode mode;
  };

  const Candidate candidates[] = {
      {p / "backup" / "data.win", InstallMode::Update},
      {p / "data.win", InstallMode::Fresh},
  };

  bool skipHash = (patchState == OPatchValue::PatchValueState::Failed);
  std::string expectedHash;

  if (!skipHash) {
    expectedHash = patchValue->GetValue("deltaruneSha256sum");
    LogManager::Info("[GamePath] Expected data.win sha256: " + expectedHash);
  } else {
    LogManager::Warning(
        "[GamePath] Patch value failed, sha256 check will be skipped.");
  }

  bool foundDataWin = false;
  for (const auto &c : candidates) {
    if (!fs::exists(c.file)) {
      LogManager::Info("[GamePath] Candidate not found: " + c.file.string());
      continue;
    }
    LogManager::Info("[GamePath] Found candidate: " + c.file.string());
    foundDataWin = true;

    if (!skipHash) {
      LogManager::Info("[GamePath] Calculating sha256...");
      std::string actual = GameUtil::CalcFileSha256(c.file.string());
      if (actual != expectedHash) {
        LogManager::Warning("[GamePath] Sha256 mismatch: " + actual);
        continue;
      }
      LogManager::Info("[GamePath] Sha256 matched.");
    }

    currentMode = c.mode;
    LogManager::Info(std::string("[GamePath] Install mode: ") +
                     (c.mode == InstallMode::Fresh ? "Fresh" : "Update"));

    if (fs::exists(acfPath)) {
      LogManager::Info("[GamePath] Steam ACF found: " + acfPath.string());
      return PathState::Valid;
    } else {
      LogManager::Warning("[GamePath] Steam ACF not found: " +
                          acfPath.string());
      return PathState::ValidButNoACF;
    }
  }

  LogManager::Warning("[GamePath] No valid data.win found");
  return foundDataWin ? PathState::InvalidShaMismatch
                      : PathState::InvalidNotFound;
}
