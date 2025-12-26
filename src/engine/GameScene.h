#ifndef GAME_SCENE_H
#define GAME_SCENE_H
#include <vector>

class GameObject;

class GameScene {
protected:
  std::vector<GameObject *> objects;
  std::vector<GameObject *> objectsToAdd;
  std::vector<GameObject *> objectsToRemove;

public:
  virtual ~GameScene();

  virtual void Init() = 0;
  virtual void Update(float deltaTime);
  virtual void Draw();

  void AddObject(GameObject *obj);
  void AddObjects(const std::vector<GameObject *> &src);
  void RemoveObject(GameObject *obj);
  void ClearObjects();
  
  const std::vector<GameObject *> &GetObjects() const;
protected:
  void FlushObjects();
};

#endif
