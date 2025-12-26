#ifndef O_IMAGE_H
#define O_IMAGE_H

#include "../engine/GameObject.h"
#include <raylib.h>
#include <string>

class BOImage : public GameObject {
private:
  Texture2D texture{};
  Vector2 position{};
  Vector2 size{};
  Color tint{WHITE};
  int frameCount{1};
  float frameHeight{0};
  int currentFrame{0};
  float frameTimer{0};
  float frameDuration{0};

public:
  static constexpr float AUTO = -1.0f;
  BOImage(std::string path, Vector2 pos, Vector2 siz = {AUTO, AUTO},
         Color color = WHITE);
  BOImage(std::string path, Vector2 pos, int frames, float duration,
         Vector2 siz = {AUTO, AUTO}, Color color = WHITE);

  ~BOImage() override;
  BOImage(const BOImage &) = delete;
  BOImage &operator=(const BOImage &) = delete;

  void Update(float deltaTime) override;
  void Draw() override;

  Vector2 GetSize() const;
  void SetPosition(Vector2 pos);
  bool IsAnimated() const { return frameCount > 1; }
};

#endif
