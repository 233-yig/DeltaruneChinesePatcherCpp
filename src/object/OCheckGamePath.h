#ifndef O_GAME_PATH_DETECT_H
#define O_GAME_PATH_DETECT_H

#include "../engine/GameObject.h"
#include "OPatchValue.h"
#include <functional>
#include <string>

class OCheckGamePath : public GameObject {
public:
  OCheckGamePath(OPatchValue *);
  enum class PathState {
    NeedPatchValue,
    InvalidNotFound,
    InvalidShaMismatch,
    ValidFresh,
    ValidUpdate
  };
  enum class InstallMode { Fresh, Update, Invalid };
  void Start();
  void AutoSetPath();
  void SetPath(const std::string &path);
  InstallMode GetInstallMode();
  const std::string &GetPath() const { return gamePath; }
  const std::string &GetError() const { return stateMessage; }
  std::function<void(const std::string &, const std::string &)> OnSuccess;
  std::function<void(const std::string &)> OnError;
  void Update(float dt) override;
  void Draw() override {}
  void RefreshText() override {}

private:
  void TriggerSuccess(const std::string &msg, const std::string &path);
  void TriggerError(const std::string &msg);
  PathState ValidatePath(const std::string &path);
  std::string AutoDetectPath();
  void HandleValidateResult(const std::string &path);

  std::string PathStateToMsg(PathState st) {
    switch (st) {
    case PathState::NeedPatchValue:
      return "GamePath.NeedPatchValue";
    case PathState::ValidFresh:
      return hasACF ? "GamePath.ValidFresh" : "GamePath.ValidFreshNoACF";
    case PathState::ValidUpdate:
      return hasACF ? "GamePath.ValidUpdate" : "GamePath.ValidUpdateNoACF";
    case PathState::InvalidNotFound:
      return "GamePath.InvalidNotFound";
    case PathState::InvalidShaMismatch:
      return "GamePath.InvalidShaMismatch";
    default:
      return "GamePath.Invalid";
    }
  }

  std::string gamePath;
  std::string pendingPath;
  bool pending = false;
  bool pendingAuto = false;
  std::string stateMessage;
  InstallMode currentMode;
  OPatchValue *patchValue = nullptr;
  bool hasACF = false;
};

#endif
