#include "BOImage.h"
#include "../engine/LogManager.h"
BOImage::BOImage(std::string path, Vector2 pos, Vector2 siz, Color color)
    : position(pos), tint(color), frameCount(1) {

  texture = LoadTexture(path.c_str());

  if (texture.id == 0) {
    LogManager::Error("[Image] Failed to load texture: " + path);
    size = {0, 0};
    return;
  }
  size = {siz.x < 0 ? (float)texture.width : siz.x,
          siz.y < 0 ? (float)texture.height : siz.y};
}
BOImage::BOImage(std::string path, Vector2 pos, int frames, float duration,
               Vector2 siz, Color color)
    : position(pos), tint(color), frameCount(frames), frameDuration(duration),
      currentFrame(0), frameTimer(0.0f) {

  texture = LoadTexture(path.c_str());

  if (texture.id == 0) {
    LogManager::Error("[Image] Failed to load texture: " + path);
    frameHeight = 0;
    size = {0, 0};
    return;
  }

  frameHeight = (float)texture.height / frameCount;

  size = {siz.x < 0 ? (float)texture.width : siz.x,
          siz.y < 0 ? frameHeight : siz.y};
}

BOImage::~BOImage() { UnloadTexture(texture); }

void BOImage::Update(float deltaTime) {

  if (frameCount <= 1)
    return;

  frameTimer += deltaTime;
  if (frameTimer >= frameDuration) {
    frameTimer -= frameDuration;
    currentFrame = (currentFrame + 1) % frameCount;
  }
}

void BOImage::Draw() {
  if (texture.id == 0)
    return;

  float srcY = frameCount > 1 ? currentFrame * frameHeight : 0;
  float srcHeight = frameCount > 1 ? frameHeight : (float)texture.height;

  Rectangle source = {0, srcY, (float)texture.width, srcHeight};
  Rectangle dest = {position.x, position.y, size.x, size.y};
  DrawTexturePro(texture, source, dest, {0, 0}, 0, tint);
}

Vector2 BOImage::GetSize() const { return size; }

void BOImage::SetPosition(Vector2 pos) { position = pos; }
