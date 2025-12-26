#ifndef O_INSTALL_PATCH
#define O_INSTALL_PATCH
#include "../basicOBject/BOText.h"
#include "../engine/GameObject.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include <filesystem>


namespace fs = std::filesystem;
class OInstallPatch : public GameObject {
public:
  OInstallPatch(OPatchValue *, OCheckGamePath *);
  enum class InstallStep {
    DownloadPatch,
    BackupGame,
    ExtractPatch,
    ApplyDelta,
    CopyStaticFiles
  };
  enum class InstallResult {
    NotStarted,
    Installing,
    Successful,
    Failed
  };
  bool Install();
  InstallStep GetCurrentStep();
  void Draw() override;
  void Update(float dt) override {};

private:
  bool BackupGame(fs::path);
  bool DownloadPatch();
  bool ExtractPatch(fs::path);
  bool ApplyDelta(fs::path);
  bool CopyStaticFiles(fs::path);
  bool ValidatePatch(fs::path);
  bool RunExternalTool(const std::string &cmd, const std::string &args);

  std::vector<BOText *> installStepText;
  BOText* stateArrow;
  void Abort(const std::string &reason);

private:
  OPatchValue *patchValue;
  OCheckGamePath *checkGamePath;
  const fs::path tempDir;
  const fs::path patchFile;
  InstallResult currentResult = InstallResult::NotStarted;
  InstallStep currentStep = InstallStep::DownloadPatch;
  std::unique_ptr<DownloadTask> patchDownloadTask = nullptr;
  
};

#endif