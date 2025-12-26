#include "engine/GameManager.h"
#include "scene/SInstaller.h"

int main(int argc, char **argv) {
  GameManager *manager = GameManager::Get();
  manager->Init();
  manager->SetScene(new SInstaller());
  manager->Run();
  return 0;
}