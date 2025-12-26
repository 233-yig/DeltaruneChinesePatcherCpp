#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "GameConfig.h"
#include <string>

class GameScene;

class GameManager {
private:
  GameScene *currentScene = nullptr;

  GameManager();

public:
  ~GameManager();

  static GameManager *Get();

  bool Init();
  GameConfig &Settings() { return settings; }
  GameManager(const GameManager &) = delete;
  GameManager &operator=(const GameManager &) = delete;
  void ChangeLanguage(const std::string &langId);
  void StepLanguage(int step);
  std::string GetCurrentLanguage();

  void SetScene(GameScene *newScene);
  void Run();

private:
  void Update();
  void Draw();
  void RefreshSceneText();

  GameConfig settings;
};

#endif