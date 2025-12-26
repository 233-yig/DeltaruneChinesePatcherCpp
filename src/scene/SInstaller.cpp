#include "SInstaller.h"
#include "../basicObject/BOButton.h"
#include "../basicObject/BOImage.h"
#include "../basicObject/BOText.h"
#include "../engine/GameManager.h"
#include "../engine/LangManager.h"
#include "../engine/LogManager.h"
#include "../object/OCheckGamePath.h"
#include "../object/OInstallPatch.h"
#include "../object/OPatchValue.h"
#include "../object/OReadme.h"
#include "tinyfiledialogs.h"

SInstaller::SInstaller() {
  int mainFontSize = LangManager::GetFont().baseSize;
  int smallFontSize = mainFontSize * 3 / 4;
  int largeFontSize = mainFontSize * 3 / 2;
  readmeText = new OReadme();
  patchValue = new OPatchValue();
  patchValue->Download();
  gamePathDetector = new OCheckGamePath(patchValue);

  installPatch = new OInstallPatch(patchValue, gamePathDetector);
  versionText = new BOText("About.Info", {280, 50});
  contactText = new BOText("Readme.Contact", {280, 600}, GREEN);
  languageText =
      new BOText("Meta.LanguageName", {120, 620}, WHITE, true, smallFontSize);
  currentGamePath = new BOText("", {290, 60}, WHITE, false, smallFontSize);
  gamePathState = new BOText("", {290, 100}, YELLOW, false, smallFontSize);
  browsePathButton = new BOButton("Patch.BrowseGamePath", {290, 150}, {200, 80}),
  downloadPatchButton =
      new BOButton("Patch.DownloadPatch", {290, 300}, {200, 80});
  installPatchButton =
      new BOButton("Patch.InstallPatch", {510, 300}, {200, 80});
  uninstallPatchButton =
      new BOButton("Patch.UninstallPatch", {730, 300}, {200, 80});

  background = {
      new BOImage("image/bg_static.png", {0, 0}, {960, 510}, Fade(WHITE, 0.8f)),
      new BOImage("image/bg_anim.png", {0, 510}, 5, 0.2f, {960, 210},
                  Fade(WHITE, 0.8f))};

  leftBar = {
      new BOImage("image/black.png", {0, 0}, {240, 720}, Fade(WHITE, 0.5f)),
      new BOImage("image/app_icon.png", {56, 40}, {128, 128}),
      new BOText("Meta.TitleShort", {120, 200}, YELLOW, true),
      new BOButton("Button.Readme", {40, 300}, {160, 80},
                   [this]() { SwitchPage(Page::README); }),
      new BOButton("Button.Patch", {40, 390}, {160, 80},
                   [this]() { SwitchPage(Page::PATCH); }),
      new BOButton("Button.About", {40, 480}, {160, 80},
                   [this]() { SwitchPage(Page::ABOUT); }),
      new BOButton("<", {10, 600}, {60, 60},
                   [this]() { GameManager::Get()->StepLanguage(-1); }),
      languageText,
      new BOButton(">", {170, 600}, {60, 60},
                   [this]() { GameManager::Get()->StepLanguage(1); }),
  };

  patchPage = {
      new BOImage("image/black.png", {270, 20}, {670, 400}, Fade(WHITE, 0.5f)),
      new BOImage("image/black.png", {270, 450}, {670, 250}, Fade(WHITE, 0.5f)),
      new BOText("GamePath.Label", {290, 25}),
      currentGamePath,
      gamePathState,
      browsePathButton,
      downloadPatchButton,
      installPatch,
      installPatchButton,
      uninstallPatchButton};

  readmePage = {
      new BOImage("image/black.png", {270, 20}, {670, 680}, Fade(WHITE, 0.5f)),
      new BOText("Readme.Title", {280, 40}, YELLOW, false, largeFontSize),
      new BOImage("image/black.png", {280, 150}, {680, 450}, Fade(WHITE, 0.5f)),
      readmeText, contactText};

  aboutPage = {
      new BOImage("image/black.png", {270, 20}, {670, 680}, Fade(WHITE, 0.5f)),
      versionText,
      new BOText("About.ContribTitle", {280, 280}, YELLOW),
      new BOText("About.ContribList", {280, 350}),
      new BOText("About.Github", {280, 600}, YELLOW),
  };
}

SInstaller::~SInstaller() {
  versionText = nullptr;
  readmeText = nullptr;
  contactText = nullptr;
  languageText = nullptr;
  currentGamePath = nullptr;
  gamePathState = nullptr;
  gamePathDetector = nullptr;
}

void SInstaller::Init() {
  // about page
  versionText->SetParam(
      "VERSION",
      GameManager::Get()->Settings().Get<std::string>("patcherVersion"));
  contactText->SetParam(
      "QQ_GROUP_CODE",
      GameManager::Get()->Settings().Get<std::string>("qqGroupCode"));

  // patch page
  currentGamePath->SetText("GamePath.Placeholder");
  gamePathDetector->OnSuccess = [this](const std::string &msg,
                                       const std::string &path) {
    gamePathState->SetText(msg);
    gamePathState->SetColor(GREEN);
    currentGamePath->SetText(path);
  };

  browsePathButton->SetCallback([this]() {
    char *result = tinyfd_selectFolderDialog(
        LangManager::GetText("MsgBox.SelectFolder").c_str(), ".");
    if (result) {
      currentGamePath->SetText(result);
      gamePathDetector->SetPath(result);
    }
  });
  gamePathDetector->OnError = [this](const std::string &msg) {
    gamePathState->SetText(msg);
    gamePathState->SetColor(YELLOW);
  };

  installPatchButton->SetCallback([this]() { installPatch->StartInstall(); });

  downloadPatchButton->SetCallback([this]() { installPatch->StartDownload(); });
  uninstallPatchButton->SetCallback(
      [this]() { installPatch->StartUninstall(); });
  AddObjects(background);
  AddObjects(leftBar);
  AddObjects(patchPage);
  AddObjects(readmePage);
  AddObjects(aboutPage);
  SwitchPage(Page::README);
  AddObject(gamePathDetector);
  gamePathDetector->AutoSetPath();

  LogManager::Info("[Main Scene] Deltarune CN Patcher successfully started.");
}

void SInstaller::Update(float dt) { GameScene::Update(dt); }
void SInstaller::SwitchPage(Page page) {
  for (auto *obj : patchPage)
    obj->SetActive(false);
  for (auto *obj : readmePage)
    obj->SetActive(false);
  for (auto *obj : aboutPage)
    obj->SetActive(false);
  switch (page) {
  case Page::PATCH:
    for (auto *obj : patchPage)
      obj->SetActive(true);
    break;
  case Page::README:
    for (auto *obj : readmePage)
      obj->SetActive(true);
    break;
  case Page::ABOUT:
    for (auto *obj : aboutPage)
      obj->SetActive(true);
    break;
  }
}
