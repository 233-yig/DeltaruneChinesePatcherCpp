#include "OInstallPatch.h"
#include "../engine/GameManager.h"
#include "../engine/GameUtil.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include "raylib.h"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

OInstallPatch::OInstallPatch(OPatchValue *pv, OCheckGamePath *cgp)
    : patchValue(pv), checkGamePath(cgp),
      tempDir(std::filesystem::temp_directory_path()),
      patchFile(fs::path("patch") /
                ("patch-" + GameManager::Get()->GetCurrentLanguage() + ".7z")) {

  stateArrow = new BOText("->", {300, 480}, YELLOW);
  warningType = new BOText("", {300, 220}, YELLOW);
  warningGuidance = new BOText("", {300, 260}, WHITE);
  errorType = new BOText("", {300, 320}, RED);
  errorGuidance = new BOText("", {300, 360}, WHITE);
  installStepText = {
      new BOText("InstallStep.DownloadPatch", {350, 460}, WHITE),
      new BOText("InstallStep.ValidatePatch", {350, 495}, WHITE),
      new BOText("InstallStep.BackupGame", {350, 530}, WHITE),
      new BOText("InstallStep.ExtractPatch", {350, 565}, WHITE),
      new BOText("InstallStep.ApplyDelta", {350, 600}, WHITE),
      new BOText("InstallStep.CopyStaticFiles", {350, 635}, WHITE),
  };
}

OInstallPatch::~OInstallPatch() {
  installing = false;
  uninstalling = false;

  if (installThread.joinable())
    installThread.join();

  if (uninstallThread.joinable())
    uninstallThread.join();
}
void OInstallPatch::StartDownload() {
  if (flowState != PatchFlowState::Idle)
    return;
  flowState = PatchFlowState::Installing;
  currentStep = InstallStep::DownloadPatch;
  std::string patchName = patchFile.filename().string();
  std::string url =
      GameManager::Get()->Settings().Get<std::string>("patchRemoteUrl") +
      "/latest/" + patchName;
  LogManager::Info("[DownloadPatch] Downloading patch from: " + url);

  patchDownloadTask = std::make_unique<DownloadTask>(
      url,

      [this, url](const std::string &content) {
        std::ofstream outFile(patchFile, std::ios::binary);
        if (outFile) {
          outFile << content;
          outFile.close();

          LogManager::Info("[DownloadPatch] Downloaded and saved patch to: " +
                           patchFile.string());
          currentStep = InstallStep::ValidatePatch;
          if (!ValidatePatch(patchFile)) {
            verifyState = PatchVerifyState::Failed;
          } else {
            verifyState = PatchVerifyState::Passed;
          }
          flowState = PatchFlowState::Installed;
          return true;
        } else {
          LogManager::Error("[DownloadPatch] Failed to write patch file: " +
                            patchFile.string());
          LogManager::Info("[DownloadPatch] You can manually download from: " +
                           url);
          flowState = PatchFlowState::Failed;
          return false;
        }
      },
      [this, url](const std::string errorMessage) {
        // 下载失败回调
        LogManager::Error("[DownloadPatch] Download failed: " + errorMessage);
        LogManager::Info("[DownloadPatch] You can manually download from: " +
                         url);
        flowState = PatchFlowState::Failed;
        return false;
      });
}

void OInstallPatch::StartInstall() {
  if (installing || uninstalling)
    return;

  if (installThread.joinable()) {
    installThread.join();
  }
  installing = true;
  flowState = PatchFlowState::Installing;

  installThread = std::thread(&OInstallPatch::InstallWorker, this);
}
void OInstallPatch::StartUninstall() {
  if (installing || uninstalling)
    return;

  if (uninstallThread.joinable()) {
    uninstallThread.join();
  }
  uninstalling = true;
  flowState = PatchFlowState::Installing;

  uninstallThread = std::thread(&OInstallPatch::UninstallWorker, this);
}
void OInstallPatch::Draw() {
  if (hasError) {
    errorType->SetText("Error." + errorKey);
    errorGuidance->SetText("ErrorGuidance." + errorKey);
    errorType->Draw();
    errorGuidance->Draw();
  }

  warningGuidance->Draw();
  warningType->Draw();
  for (size_t i = 0; i < installStepText.size(); ++i) {
    Color c = WHITE;
    InstallStep step = currentStep.load();
    PatchFlowState state = flowState.load();

    if (i < static_cast<size_t>(step))
      c = GREEN;
    else if (i == static_cast<size_t>(step)) {
      switch (state) {
      case PatchFlowState::Installing:
        c = YELLOW;
        break;
      case PatchFlowState::Failed:
        c = RED;
        break;
      case PatchFlowState::Installed:
        c = GREEN;
        break;
      default:
        break;
      }
      stateArrow->SetPosition({300, installStepText[i]->GetPosition().y});
      stateArrow->Draw();
    }

    installStepText[i]->SetColor(c);
    installStepText[i]->Draw();
  }
}
bool OInstallPatch::ValidatePatch(const fs::path &patchFile) {
  if (patchValue->GetState() != OPatchValue::PatchValueState::Ready) {
    Warning("[DownloadPatch] Can't verify patch, patch may be broken, "
            "continue at your risk.",
            "PatchValidationFailed");
    return false;
  }
  std::string expectedHash = patchValue->GetValue("patchSha256sum");
  bool checkPassed =
      GameUtil::CalcFileSha256(patchFile.string()) == expectedHash;
  if (checkPassed) {
    LogManager::Info("[DownloadPatch] Patch is valid: Version " +
                     patchValue->GetValue("patchVersion"));
    LogManager::Info("[DownloadPatch] Patch build time: " +
                     patchValue->GetValue("patchBuildTime"));
  } else {
    Warning("[DownloadPatch] Patch check failed! It's likely "
            "that patch is broken, and your installation may fail.",
            "PatchValidationFailed");
  }
  return checkPassed;
}

void OInstallPatch::Abort(const std::string &reason,
                          const std::string &uiLangKey) {
  LogManager::Error("[Install] " + reason);
  {
    std::lock_guard lock(uiMsgMutex);
    errorKey = uiLangKey;
    hasError = true;
  }

  flowState = PatchFlowState::Failed;
  installing = false;
  uninstalling = false;
}

void OInstallPatch::Warning(const std::string &reason,
                            const std::string &uiLangKey) {
  LogManager::Warning("[Install] " + reason);
  std::lock_guard lock(uiMsgMutex);
  warningKey = uiLangKey;
  hasWarning = true;
}

void OInstallPatch::InstallWorker() {
  hasError = false;
  hasWarning = false;
  errorKey.clear();
  warningKey.clear();
  currentStep = InstallStep::ValidatePatch;
  if (!fs::exists(patchFile)) {
    Abort("Patch not found, please download first", "PatchNotFound");
    return;
  }
  if (!ValidatePatch(patchFile)) {
    verifyState = PatchVerifyState::Failed;
  } else {
    verifyState = PatchVerifyState::Passed;
  }

  currentStep = InstallStep::BackupGame;
  fs::path gamePath = checkGamePath->GetPath();
  if (BackupGame(gamePath)) {
    Warning("[Install] Backup failed, continuing without backup",
            "BackupFailed");
  }

  currentStep = InstallStep::ExtractPatch;
  if (!ExtractPatch(patchFile))
    return Abort("Extract failed", "PatchExtractFailed");

  currentStep = InstallStep::ApplyDelta;
  if (!ApplyDelta(gamePath))
    return Abort("Delta failed", "DeltaApplyFailed");

  currentStep = InstallStep::CopyStaticFiles;
  if (!CopyStaticFiles(gamePath))
    return Abort("Copy failed", "StaticFileCopyFailed");

  currentStep = InstallStep::Finished;
  flowState = PatchFlowState::Installed;
  installing = false;
}

void OInstallPatch::UninstallWorker() {
  hasError = false;
  hasWarning = false;
  errorKey.clear();
  warningKey.clear();
  fs::path gamePath = checkGamePath->GetPath();
  fs::path backupPath = gamePath / "backup";
  if (!fs::exists(backupPath))
    return Abort("Uninstall failed: backup folder doesn't exist.",
                 "BackupFolderMissing");
  if (!BackupGame(gamePath, true))
    return Abort("Uninstall failed: backup is broken.", "BackupRestoreFailed");
  currentStep = InstallStep::DownloadPatch;
  flowState = PatchFlowState::Idle;
  uninstalling = false;
}

bool OInstallPatch::BackupGame(const fs::path &gamePath, bool uninstall) {
  LogManager::Info(uninstall ? "[Uninstall] Restoring game files..."
                             : "[Install] Backing up game files...");

  const auto fileList =
      GameManager::Get()->Settings().Get<std::vector<std::string>>(
          "backupFiles");

  fs::path src = uninstall ? (gamePath / "backup") : gamePath;
  fs::path dst = uninstall ? gamePath : (gamePath / "backup");

  if (!uninstall) {
    fs::remove_all(dst);
    fs::create_directories(dst);
  }

  bool success = true;

  for (const auto &file : fileList) {
    try {
      fs::path srcFile = src / file;
      fs::path dstFile = dst / file;

      if (!fs::exists(srcFile)) {
        LogManager::Warning("[Backup] Missing file: " + srcFile.string());
        success = false;
        continue;
      }

      fs::create_directories(dstFile.parent_path());
      fs::copy_file(srcFile, dstFile, fs::copy_options::overwrite_existing);
    } catch (const std::exception &e) {
      success = false;
      LogManager::Warning("[Backup] Failed for " + file + ": " + e.what());
    }
  }

  if (uninstall && success) {
    fs::remove_all(src);
    LogManager::Info("[Uninstall] Backup removed.");
  }

  LogManager::Info(success ? "[Backup] Finished successfully."
                           : "[Backup] Finished with warnings.");

  return success;
}

bool OInstallPatch::ExtractPatch(const fs::path &patchFile) {
  LogManager::Info("[Install] Extracting patch to temp directory...");
  fs::create_directories(tempDir);
  fs::path sevenZip;
#ifdef _WIN32
  sevenZip = fs::current_path() / "external/win/7z.exe";
#else
  sevenZip = fs::current_path() / "external/linux/7z";
#endif

  fs::path errLog = tempDir / "err-7z.log";
  if (!fs::exists(patchFile)) {
    LogManager::Error("[Install] Patch archive not found: " +
                      patchFile.string());
    return false;
  }

  std::string cmd = GameUtil::ConvertPath(sevenZip) + " x \"" +
                    GameUtil::ConvertPath(patchFile) + "\" -o" +
                    GameUtil::ConvertPath(tempDir) + " -aoa 2> \"" +
                    GameUtil::ConvertPath(errLog) + "\"";

  LogManager::Info("7z execute command: " + cmd);
  int ret = std::system(cmd.c_str());
  if (ret != 0) {
    std::string err;
    if (fs::exists(errLog)) {
      std::ifstream in(errLog);
      err.assign(std::istreambuf_iterator<char>(in),
                 std::istreambuf_iterator<char>());
    }

    LogManager::Error("[Install] 7z failed, code=" + std::to_string(ret));
    if (!err.empty()) {
      LogManager::Error("[7z stderr]\n" + err);
    }
    return false;
  }

  LogManager::Info("[Install] Patch extracted to: " + tempDir.string());
  return true;
}

bool OInstallPatch::ApplyDelta(const fs::path &gamePath) {
  LogManager::Info("[Install] Applying delta patches...");
  fs::path xDelta3;
#ifdef _WIN32
  xDelta3 = fs::current_path() / "external/win/xdelta3.exe";
#else
  xDelta3 = fs::current_path() / "external/linux/xdelta3";
#endif

  auto deltaList =
      GameManager::Get()->Settings().Get<nlohmann::json>("patchFileXDelta");
  fs::path errLog = tempDir / "err-xdelta.log";

  try {
    for (auto &[key, value] : deltaList.items()) {
      fs::path oldFile = gamePath / "backup" / key;
      fs::path newFile = gamePath / key;
      fs::path deltaPath = tempDir / value;

      if (!fs::exists(deltaPath) || !fs::exists(oldFile)) {
        LogManager::Error("[Install] Missing delta input: " + key);
        return false;
      }
      if (fs::exists(newFile))
        fs::remove(newFile);
      std::string cmd = GameUtil::ConvertPath(xDelta3) + " -d -s \"" +
                        GameUtil::ConvertPath(oldFile) + "\" \"" +
                        GameUtil::ConvertPath(deltaPath) + "\" \"" +
                        GameUtil::ConvertPath(newFile) + "\" 2> \"" +
                        GameUtil::ConvertPath(errLog) + "\"";

      int ret = std::system(cmd.c_str());
      if (ret != 0) {
        std::string err;
        if (fs::exists(errLog)) {
          std::ifstream in(errLog);
          err.assign(std::istreambuf_iterator<char>(in),
                     std::istreambuf_iterator<char>());
        }

        LogManager::Error("[Install] xdelta failed for: " + std::string(value) +
                          ", code=" + std::to_string(ret));
        if (!err.empty()) {
          LogManager::Error("[xdelta stderr]\n" + err);
        }
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

bool OInstallPatch::CopyStaticFiles(const fs::path &gamePath) {
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
