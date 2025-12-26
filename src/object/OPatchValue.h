#ifndef O_CHECK_TOOL
#define O_CHECK_TOOL
#include "../engine/GameObject.h"
#include "../engine/GameConfig.h"
#include "../engine/DownloadTask.h"
#include <filesystem>
#include <memory>
class OPatchValue : public GameObject {
public:
  enum class PatchValueState { NotReady, Ready, Failed };
  OPatchValue();
  void Draw() override {}
  void Update(float dt) override {};
  PatchValueState GetState();
  bool Download();
  std::string GetValue(std::string key);

private:
  std::unique_ptr<DownloadTask> downloadTask;
  PatchValueState currentState = PatchValueState::NotReady;
  GameConfig config;
  const std::filesystem::path valueFile;
};
#endif