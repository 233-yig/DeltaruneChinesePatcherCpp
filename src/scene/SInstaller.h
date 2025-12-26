#ifndef S_INSTALLER_H
#define S_INSTALLER_H
#include "../engine/GameScene.h"
#include <vector>

class BOText;
class BOScrollText;
class BOTextBox;
class BOButton;
class GameObject;
class OCheckGamePath;
class OPatchValue;
class OInstallPatch;
class OReadme;

class SInstaller : public GameScene {
public:
  SInstaller();
  ~SInstaller() override;
  void Init() override;
  void Update(float dt) override;

private:
  enum class Page { README, PATCH, ABOUT };
  BOText *versionText{nullptr};
  BOText *contactText{nullptr};
  BOText *languageText{nullptr};
  BOText *currentGamePath{nullptr};
  BOText *gamePathState{nullptr};
  BOText *installResult{nullptr};
  BOButton *browsePathButton{nullptr};
  std::vector<GameObject *> background;
  std::vector<GameObject *> leftBar;
  std::vector<GameObject *> patchPage;
  std::vector<GameObject *> readmePage;
  std::vector<GameObject *> aboutPage;

  BOButton *installPatchButton{nullptr};
  BOButton *downloadPatchButton{nullptr};
  BOButton *uninstallPatchButton{nullptr};

  void SwitchPage(Page page);

  OCheckGamePath *gamePathDetector;
  OPatchValue *patchValue;
  OInstallPatch *installPatch;
  OReadme* readmeText;
};
#endif