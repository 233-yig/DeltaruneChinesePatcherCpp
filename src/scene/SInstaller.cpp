#include "SInstaller.h"
#include "../basicObject/BOButton.h"
#include "../basicObject/BOImage.h"
#include "../basicObject/BOScrollText.h"
#include "../basicObject/BOText.h"
#include "../engine/GameManager.h"
#include "../engine/LangManager.h"
#include "../engine/LogManager.h"
#include "../object/OCheckGamePath.h"
#include "../object/OPatchValue.h"

#include <nfd.hpp>

SInstaller::SInstaller() {
  int mainFontSize = LangManager::GetFont().baseSize;
  int smallFontSize = mainFontSize * 3 / 4;
  int largeFontSize = mainFontSize * 3 / 2;
  versionText = new BOText("About.Info", {280, 50});
  readmeText = new BOScrollText("", {280, 150, 630, 450}, WHITE, smallFontSize);
  contactText = new BOText("Readme.Contact", {280, 600}, GREEN);
  languageText =
      new BOText("Meta.LanguageName", {120, 620}, WHITE, true, smallFontSize);
  gamePathInput = new BOText("", {290, 60}, WHITE, false, smallFontSize);
  gamePathState = new BOText("", {290, 100}, YELLOW, false, smallFontSize);

  background = {
      new BOImage("image/bg_static.png", {0, 0}, {960, 510}, Fade(WHITE, 0.8f)),
      new BOImage("image/bg_anim.png", {0, 510}, 5, 0.2f, {960, 1050},
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
      gamePathInput,
      gamePathState,
      new BOButton("GamePath.Browse", {290, 150}, {240, 80},
                   [this]() {
                     NFD::UniquePath outPath;
                     nfdresult_t result = NFD::PickFolder(outPath);

                     if (result == NFD_OKAY) {
                       std::string path = outPath.get();

                       gamePathInput->SetText(path);
                       gamePathDetector->SetPath(path);
                     }
                   }),
  };

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

  pathValue = new OPatchValue();
  gamePathDetector = new OCheckGamePath(pathValue);
  gamePathDetector->OnSuccess = [this](const std::string &msg,
                                       const std::string &path) {
    gamePathState->SetText(msg);
    gamePathState->SetColor(GREEN);
    gamePathInput->SetText(path);
  };

  gamePathDetector->OnError = [this](const std::string &msg) {
    gamePathState->SetText(msg);
    gamePathState->SetColor(YELLOW);
  };
}

SInstaller::~SInstaller() {
  versionText = nullptr;
  readmeText = nullptr;
  contactText = nullptr;
  languageText = nullptr;
  gamePathInput = nullptr;
  gamePathState = nullptr;
  gamePathDetector = nullptr;
}

void SInstaller::Init() {
  std::string version =
      GameManager::Get()->Settings().Get<std::string>("patcherVersion");
  versionText->SetParam("VERSION", version);
  std::string qqGroup =
      GameManager::Get()->Settings().Get<std::string>("qqGroupCode");
  contactText->SetParam("QQ_GROUP_CODE", qqGroup);
  gamePathInput->SetText("GamePath.Placeholder");

  AddObjects(background);
  AddObjects(leftBar);
  AddObjects(patchPage);
  AddObjects(readmePage);
  AddObjects(aboutPage);
  SwitchPage(Page::README);
  AddObject(gamePathDetector);
  gamePathDetector->AutoSetPath();

  LogManager::Info(
      "[Main Scene] Deltarune CN Patcher successfully started, version: " +
      version);
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
