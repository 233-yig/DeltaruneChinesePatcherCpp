#ifndef O_INSTALL_PATCH
#define O_INSTALL_PATCH
#include "../engine/GameObject.h"
#include "OCheckGamePath.h"
#include "OPatchValue.h"
#include <filesystem>

namespace fs = std::filesystem;
class OInstallPatch : public GameObject {
public:
  OInstallPatch(OPatchValue *, OCheckGamePath *);
  enum class InstallStep {
    NotStarted,
    DownloadPatch,
    BackupGame,
    ExtractPatch,
    ApplyDelta,
    CopyStaticFiles,
    Finished
  };
  bool Install();
  InstallStep GetCurrentStep();

private:
  bool BackupGame(fs::path);
  bool DownloadPatch();
  bool ExtractPatch(fs::path, fs::path);
  bool ApplyDelta(fs::path);
  bool CopyStaticFiles(fs::path);
  bool ValidatePatch(fs::path);
  bool RunExternalTool(const std::string &cmd, const std::string &args);

  void Abort(const std::string &reason);

private:
  OPatchValue *patchValue;
  OCheckGamePath *checkGamePath;
  const fs::path tempDir;
  const fs::path patchFile;
  InstallStep currentStep = InstallStep::NotStarted;
  std::unique_ptr<DownloadTask> patchDownloadTask = nullptr;
};

#endif