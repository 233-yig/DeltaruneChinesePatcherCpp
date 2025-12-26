#include "GameManager.h"
#include "GameConfig.h"
#include "GameObject.h"
#include "GameScene.h"
#include "GameValues.h"
#include "LangManager.h"
#include "LogManager.h"
#include <raylib.h>
#include <string>
#include <tinyfiledialogs.h>

GameManager::GameManager()
    : settings("settings.json", Global::defaultSettings) {}

bool GameManager::Init() {
  LogManager::Info("[Init] Initializing game manager...");

  if (!settings.Load()) {
    LogManager::Critical(
        "[Init] Failed to load global settings: settings.json");
    return false;
  }
  LogManager::Info("[Init] Settings loaded successfully.");
  SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
  int width = settings.Get<int>("baseWidth");
  int height = settings.Get<int>("baseHeight");

  InitWindow(width, height, "Window");
  if (!IsWindowReady()) {
    LogManager::Critical("[Init] Failed to initialize window.");
    return false;
  }
  LogManager::Info("[Init] Window initialized: " + std::to_string(width) + "x" +
                   std::to_string(height));

  Image icon = LoadImage("image/app_icon.png");
  if (icon.data) {
    SetWindowIcon(icon);
    UnloadImage(icon);
    LogManager::Info("[Init] Window icon loaded.");
  } else {
    LogManager::Warning(
        "[Init] Failed to load window icon: image/app_icon.png");
  }

  SetTargetFPS(60);
  if (!LangManager::Init(settings)) {
    LogManager::Critical("[Init] Language system initialization failed.");
    return false;
  }
  SetWindowTitle(LangManager::GetText("Meta.Title").c_str());
  LogManager::Info("[Init] Language system initialized.");

  LogManager::Info("[Init] Initialization completed.");
  return true;
}

GameManager::~GameManager() {
  if (currentScene) {
    delete currentScene;
    currentScene = nullptr;
  }
  CloseWindow();
}

GameManager *GameManager::Get() {
  static GameManager instance;
  return &instance;
}

void GameManager::ChangeLanguage(const std::string &langId) {
  LangManager::SetLanguage(langId, settings);
  SetWindowTitle(LangManager::GetText("Meta.Title").c_str());
  RefreshSceneText();
}

void GameManager::StepLanguage(int step) {
  LangManager::StepLanguage(step, settings);
  SetWindowTitle(LangManager::GetText("Meta.Title").c_str());
  RefreshSceneText();
}

std::string GameManager::GetCurrentLanguage() {
  return LangManager::GetCurrentLanguage(settings);
}

void GameManager::RefreshSceneText() {
  if (currentScene) {
    for (auto obj : currentScene->GetObjects()) {
      obj->RefreshText();
    }
  }
}

void GameManager::SetScene(GameScene *newScene) {
  if (currentScene) {
    delete currentScene;
  }
  currentScene = newScene;
  if (currentScene) {
    currentScene->Init();
  }
}

void GameManager::Run() {
  while (!WindowShouldClose()) {
    Update();
    Draw();
  }
}

void GameManager::Update() {
  float deltaTime = GetFrameTime();
  if (currentScene) {
    currentScene->Update(deltaTime);
  }
}

void GameManager::Draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  if (currentScene) {
    currentScene->Draw();
  }

  EndDrawing();
}