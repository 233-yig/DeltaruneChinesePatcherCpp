#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <raylib.h>

class GameObject {
protected:
  bool active = true;

public:
  virtual ~GameObject() {}
  virtual void Update(float deltaTime) = 0;
  virtual void Draw() = 0;
  virtual void RefreshText() {}

  void SetActive(bool state) { active = state; }
  bool IsActive() const { return active; }
};
#endif