#include "OInstallPatch.h"
#include "../engine/GameManager.h"
#include "../engine/GameUtil.h"
#include "../engine/LangManager.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include "raylib.h"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

OInstallPatch::OInstallPatch(OPatchValue *pv, OCheckGamePath *cgp)
    : patchValue(pv), checkGamePath(cgp), tempDir(fs::path("temp")),
      patchFile(fs::path("patch") /
                ("patch-" + GameManager::Get()->GetCurrentLanguage() + ".7z")) {
  stateArrow = new BOText("->", {300, 480}, YELLOW);
  installStepText = {
      new BOText("InstallStep.DownloadPatch", {350, 495}, WHITE),
      new BOText("InstallStep.BackupGame", {350, 530}, WHITE),
      new BOText("InstallStep.ExtractPatch", {350, 565}, WHITE),
      new BOText("InstallStep.ApplyDelta", {350, 600}, WHITE),
      new BOText("InstallStep.CopyStaticFiles", {350, 635}, WHITE),
  };
}
OInstallPatch::InstallStep OInstallPatch::GetCurrentStep() {
  return currentStep;
}

void OInstallPatch::Draw() {
  for (int i = 0; i < (int)currentStep; i++) {
    installStepText[i]->SetColor(GREEN);
    installStepText[i]->Draw();
  }
  std::vector<std::pair<InstallResult, Color>> stateColor = {
      {InstallResult::NotStarted, WHITE},
      {InstallResult::Installing, YELLOW},
      {InstallResult::Failed, RED},
      {InstallResult::Successful, GREEN},
  };
  for (auto &[result, color] : stateColor) {
    if (currentResult == result)
      installStepText[(int)currentStep]->SetColor(color);
    break;
  }
  stateArrow->SetPosition(
      {300, installStepText[(int)currentStep]->GetPosition().y});
  installStepText[(int)currentStep]->Draw();
  stateArrow->Draw();
  for (int i = (int)currentStep + 1; i < installStepText.size(); i++) {
    installStepText[i]->SetColor(WHITE);
    installStepText[i]->Draw();
  }
}
bool OInstallPatch::ValidatePatch(fs::path patchFile) {
  if (patchValue->GetState() != OPatchValue::PatchValueState::Ready) {
    LogManager::Warning(
        "[DownloadPatch] Can't verify patch, patch may be broken, "
        "continue at your risk.");
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
    LogManager::Warning("[DownloadPatch] Patch check failed! It's likely "
                        "that patch is broken.");
    LogManager::Warning("[DownloadPatch] Your installation may fail.");
  }
  return true;
}

bool OInstallPatch::DownloadPatch() {
  std::string patchName = patchFile.filename().string();
  const std::string url =
      GameManager::Get()->Settings().Get<std::string>("patchRemoteUrl") +
      "/latest/" + patchName;
  LogManager::Info("[DownloadPatch] Downloading patch from: " + url);

  patchDownloadTask = std::make_unique<DownloadTask>(
      url,
      [this, url](const std::string content) {
        std::filesystem::create_directories(patchFile.parent_path());

        std::ofstream outFile(patchFile, std::ios::binary);
        if (outFile) {
          outFile << content;
          outFile.close();

          LogManager::Info("[DownloadPatch] Downloaded and saved patch to: " +
                           patchFile.string());
          return true;
        } else {
          LogManager::Error("[DownloadPatch] Failed to write patch file: " +
                            patchFile.string());
          LogManager::Info("[DownloadPatch] You can manually download from: " +
                           url);
          return false;
        }
      },
      [this, url](const std::string errorMessage) {
        // 下载失败回调
        LogManager::Error("[DownloadPatch] Download failed: " + errorMessage);
        LogManager::Info("[DownloadPatch] You can manually download from: " +
                         url);
        return false;
      });
  return false;
}

bool OInstallPatch::Install() {
  currentResult = InstallResult::Installing;
  fs::path path = checkGamePath->GetPath();
  OCheckGamePath::InstallMode mode = checkGamePath->GetInstallMode();
  LogManager::Info("[Install] Start installing patch...");
  LogManager::Info("[Install] Game path: " + path.string());

  if (!fs::exists(patchFile)) {
    currentStep = InstallStep::DownloadPatch;
    if (!DownloadPatch()) {
      currentResult = InstallResult::Failed;
      return false;
    }
  }

  if (!ValidatePatch(patchFile)) {
    int choice = tinyfd_messageBox(
        "Error", LangManager::GetText("Install.ShaMismatchWarning").c_str(),
        "yesno", "warning", 1);
    if (choice == 1) {
      currentResult = InstallResult::Failed;
      return false;
    }
  }

  if (mode == OCheckGamePath::InstallMode::Fresh) {
    currentStep = InstallStep::BackupGame;
    BackupGame(path);
  }

  currentStep = InstallStep::ExtractPatch;
  if (!ExtractPatch(patchFile)) {
    currentResult = InstallResult::Failed;
    return false;
  }
  currentStep = InstallStep::ApplyDelta;
  if (!ApplyDelta(path)) {
    currentResult = InstallResult::Failed;
    return false;
  }
  currentStep = InstallStep::CopyStaticFiles;
  if (!CopyStaticFiles(path)) {
    currentResult = InstallResult::Failed;
    return false;
  }

  currentResult = InstallResult::Successful;
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

bool OInstallPatch::ExtractPatch(fs::path patchFile) {
  LogManager::Info("[Install] Extracting patch to temp directory...");
  fs::remove_all(tempDir);
  fs::create_directories(tempDir);
  fs::path sevenZip;
#ifdef _WIN32
  sevenZip = fs::path("external/win/7z.exe");
#else
  sevenZip = fs::path("external/linux/7z");
#endif

  if (!fs::exists(patchFile)) {
    LogManager::Error("[Install] Patch archive not found: " +
                      patchFile.string());
    return false;
  }

  std::string cmd = "\"" + sevenZip.string() + "\" x \"" + patchFile.string() +
                    "\" -o\"" + tempDir.string() + "\" -y";
  LogManager::Info(cmd);

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
