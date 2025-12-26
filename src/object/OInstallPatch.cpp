#include "OInstallPatch.h"
#include "../engine/GameManager.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include <filesystem>
#include <vector>

OInstallPatch::OInstallPatch(OPatchValue *pv, OCheckGamePath *cgp)
    : patchValue(pv), checkGamePath(cgp) {}
OInstallPatch::InstallStep OInstallPatch::GetCurrentStep() {
  return currentStep;
}

bool OInstallPatch::Install() {
  currentStep = InstallStep::NotStarted;
  fs::path path = checkGamePath->GetPath();
  OCheckGamePath::InstallMode mode = checkGamePath->GetInstallMode();
  LogManager::Info("[Install] Start installing patch...");
  LogManager::Info("[Install] Game path: " + path.string());

  if (mode == OCheckGamePath::InstallMode::Fresh) {
    currentStep = InstallStep::BackupGame;
    BackupGame(path);
  }

  currentStep = InstallStep::ExtractPatch;
  if (!ExtractPatch(path))
    return false;
  currentStep = InstallStep::ApplyDelta;
  if (!ApplyDelta(path))
    return false;
  currentStep = InstallStep::CopyStaticFiles;
  if (!CopyStaticFiles(path))
    return false;

  currentStep = InstallStep::Finished;
  LogManager::Info("[Install] Patch installed successfully!");
  return true;
}

bool OInstallPatch::BackupGame(fs::path gamePath) {
  bool hasError = false;
  LogManager::Info("[Install] Backing up game files...");

  std::vector<std::string> fileList =
      GameManager::Get()->Settings().Get<std::vector<std::string>>(
          "backupFiles");
  fs::path backupPath = gamePath / "backup";
  fs::create_directories(backupPath);

  try {
    for (auto &file : fileList) {
      fs::create_directories((backupPath / file).parent_path());
      fs::copy_file(gamePath / file, backupPath / file);
    }
  } catch (const std::exception &e) {
    hasError = true;
    LogManager::Warning(std::string("Backup failed: ") + e.what());
  }

  LogManager::Info("[Install] Backup finished.");
  return hasError;
}

bool OInstallPatch::ExtractPatch(fs::path gamePath) {
  LogManager::Info("[Install] Extracting patch to temp directory...");
  fs::remove_all(tempDir);
  fs::create_directories(tempDir);

  fs::path patchArchive = fs::path("patch") / "patch.7z";
  fs::path sevenZip;
#ifdef _WIN32
  sevenZip = fs::path("external/win/7z.exe");
#else
  sevenZip = fs::path("external/linux/7z");
#endif

  if (!fs::exists(patchArchive)) {
    LogManager::Error("[Install] Patch archive not found: " +
                      patchArchive.string());
    return false;
  }

  std::string cmd = sevenZip.string() + " x \"" + patchArchive.string() +
                    "\" -o\"" + tempDir.string() + "\" -y";

  int ret = std::system(cmd.c_str());
  if (ret != 0) {
    LogManager::Error("[Install] Failed to extract patch, code=" +
                      std::to_string(ret));
    return false;
  }

  LogManager::Info("[Install] Patch extracted to: " + tempDir.string());
  return true;
}

bool OInstallPatch::ApplyDelta(fs::path gamePath) {
  LogManager::Info("[Install] Applying delta patches...");
  fs::path xDelta3;
#ifdef _WIN32
  xDelta3 = fs::path("external/win/xdelta3.exe");
#else
  xDelta3 = fs::path("external/linux/xdelta3");
#endif

  auto deltaList =
      GameManager::Get()->Settings().Get<nlohmann::json>("patchFileDelta");

  try {
    for (auto &[key, value] : deltaList.items()) {
      fs::path deltaPath = tempDir / key;
      fs::path oldFile = gamePath / "backup" / value;
      fs::path newFile = gamePath / value;

      if (!fs::exists(deltaPath) || !fs::exists(oldFile)) {
        LogManager::Error("[Install] Missing delta input: " + key);
        return false;
      }

      std::string cmd = xDelta3.string() + " -d -s \"" + oldFile.string() +
                        "\" \"" + deltaPath.string() + "\" \"" +
                        newFile.string() + "\"";

      int ret = std::system(cmd.c_str());
      if (ret != 0) {
        LogManager::Error("[Install] xdelta failed for: " + key +
                          ", code=" + std::to_string(ret));
        return false;
      }
    }
  } catch (const std::exception &e) {
    LogManager::Error("[Install] Delta apply exception: " +
                      std::string(e.what()));
    return false;
  }

  LogManager::Info("[Install] Delta patches applied.");
  return true;
}

bool OInstallPatch::CopyStaticFiles(fs::path gamePath) {
  LogManager::Info("[Install] Copying patched files to game directory...");
  auto staticFileList =
      GameManager::Get()->Settings().Get<std::vector<std::string>>(
          "patchFileStatic");

  try {
    for (const auto &staticFile : staticFileList) {
      fs::path src = tempDir / staticFile;
      fs::path dst = gamePath / staticFile;

      if (!fs::exists(src)) {
        LogManager::Error("[Install] Source file missing: " + src.string());
        return false;
      }

      fs::create_directories(dst.parent_path());
      fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
    }
  } catch (const std::exception &e) {
    LogManager::Error("[Install] Copy failed: " + std::string(e.what()));
    return false;
  }

  LogManager::Info("[Install] Files copied successfully.");
  return true;
}
