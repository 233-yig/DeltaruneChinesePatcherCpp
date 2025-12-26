#include "GameScene.h"
#include "../engine/GameObject.h"

#include <algorithm>

GameScene::~GameScene() {
  ClearObjects();
}

void GameScene::Update(float deltaTime) {
  FlushObjects();
  for (auto& obj : objects) {
    if (obj && obj->IsActive()) {
      obj->Update(deltaTime);
    }
  }
}

void GameScene::Draw() {
  for (auto& obj : objects) {
    if (obj && obj->IsActive()) {
      obj->Draw();
    }
  }
}

void GameScene::AddObject(GameObject *obj) {
  if (obj) {
    objectsToAdd.push_back(obj);
  }
}

void GameScene::AddObjects(const std::vector<GameObject *> &src) {
  objectsToAdd.insert(objectsToAdd.end(), src.begin(), src.end());
}

void GameScene::RemoveObject(GameObject *obj) {
  if (obj) {
    objectsToRemove.push_back(obj);
  }
}

void GameScene::ClearObjects() {

  for (auto obj : objects) {
    delete obj;
  }
  objects.clear();

  objectsToAdd.clear();
  objectsToRemove.clear();
}

void GameScene::FlushObjects() {
  for (auto obj : objectsToRemove) {
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
      delete *it;
      objects.erase(it);
    }
  }
  objectsToRemove.clear();
  objects.insert(objects.end(), objectsToAdd.begin(), objectsToAdd.end());
  objectsToAdd.clear();
}

const std::vector<GameObject *> &GameScene::GetObjects() const {
  return objects;
}
