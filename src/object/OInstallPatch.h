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
  bool ExtractPatch(fs::path);
  bool ApplyDelta(fs::path);
  bool CopyStaticFiles(fs::path);

  bool RunExternalTool(const std::string &cmd, const std::string &args);

  void Abort(const std::string &reason);

private:
  OPatchValue *patchValue;
  OCheckGamePath *checkGamePath;
  const fs::path tempDir = fs::path("temp");
  InstallStep currentStep = InstallStep::NotStarted;
};

#endif