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

class SInstaller : public GameScene {
public:
  SInstaller();
  ~SInstaller() override;
  void Init() override;
  void Update(float dt) override;

private:
  enum class Page { README, PATCH, ABOUT };
  BOText *versionText{nullptr};
  BOScrollText *readmeText{nullptr};
  BOText *contactText{nullptr};
  BOText *languageText{nullptr};
  BOText *gamePathInput{nullptr};
  BOText *gamePathState{nullptr};
  std::vector<GameObject *> background;
  std::vector<GameObject *> leftBar;
  std::vector<GameObject *> patchPage;
  std::vector<GameObject *> readmePage;
  std::vector<GameObject *> aboutPage;

  void SwitchPage(Page page);

  OCheckGamePath *gamePathDetector;
  OPatchValue *pathValue;
};
#endif