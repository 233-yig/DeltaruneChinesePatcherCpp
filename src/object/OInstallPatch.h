#ifndef O_INSTALL_PATCH
#define O_INSTALL_PATCH

#include "../basicOBject/BOText.h"
#include "../engine/GameObject.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include <atomic>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class OInstallPatch : public GameObject {
public:
  OInstallPatch(OPatchValue *, OCheckGamePath *);
  ~OInstallPatch();

  enum class InstallStep {
    DownloadPatch,
    ValidatePatch,
    BackupGame,
    ExtractPatch,
    ApplyDelta,
    CopyStaticFiles,
    Finished
  };

  enum class PatchFlowState { Idle, Installing, Installed, Failed };

  // UI 调用
  void StartDownload();
  void StartInstall();
  void StartUninstall();

  // GameObject
  void Update(float dt) override {}
  void Draw() override;

  enum class PatchVerifyState { Unknown, Passed, Failed };
  std::atomic<PatchVerifyState> verifyState{PatchVerifyState::Unknown};

private:
  // === 后台线程入口 ===
  void InstallWorker();
  void UninstallWorker();

  // === 实际步骤 ===
  bool BackupGame(const fs::path &, bool uninstall = false);
  bool ExtractPatch(const fs::path &);
  bool ApplyDelta(const fs::path &);
  bool CopyStaticFiles(const fs::path &);
  bool ValidatePatch(const fs::path &);

  void Abort(const std::string &reason);

private:
  OPatchValue *patchValue;
  OCheckGamePath *checkGamePath;

  const fs::path tempDir;
  const fs::path patchFile;

  // === UI 状态 ===
  std::atomic<PatchFlowState> flowState{PatchFlowState::Idle};
  std::atomic<InstallStep> currentStep{InstallStep::DownloadPatch};

  // === 下载 ===
  std::unique_ptr<DownloadTask> patchDownloadTask;

  // === 安装线程 ===
  std::thread installThread;
  std::thread uninstallThread;
  std::atomic_bool installing{false};
  std::atomic_bool uninstalling{false};

  // === UI ===
  std::vector<BOText *> installStepText;
  BOText *stateArrow;
};

#endif
