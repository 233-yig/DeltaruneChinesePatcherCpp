#ifndef O_BUTTON_H
#define O_BUTTON_H

#include "../engine/GameObject.h"
#include <functional>
#include <raylib.h>
#include <string>

class BOButton : public GameObject {
public:
  enum class State {
    DISABLED,
    NORMAL,
    HOVERED,
    PRESSED,
  };
  BOButton(const std::string &langKey, Vector2 position, Vector2 size,
          std::function<void()> callback = nullptr, bool disabled = false);

  ~BOButton() override;
  BOButton(const BOButton &) = delete;
  BOButton &operator=(const BOButton &) = delete;
  void RefreshText() override;
  void Update(float deltaTime) override;
  void Draw() override;
  void SetEnabled(bool enabled);
  bool IsEnabled() const { return state != State::DISABLED; }
  State GetState() const { return state; }
  void SetPosition(Vector2 newPos);
  void SetSize(Vector2 newSize);
  void SetCallback(std::function<void()> callback);
  void SetText(const std::string &key);
  Vector2 GetPosition() const { return pos; }
  Vector2 GetSize() const { return size; }
  Rectangle GetBounds() const { return {pos.x, pos.y, size.x, size.y}; }

private:
  static void DrawNineSlice(Texture2D tex, Rectangle rect, float margin,
                            Color tint);
  Vector2 pos{};
  Vector2 size{};
  std::string langKey;
  std::string displayText;
  Vector2 cachedTextPos{};

  Texture2D textures[4]{};
  State state{State::NORMAL};
  std::function<void()> onClick;
  static constexpr int NINE_SLICE_MARGIN = 26;
  static constexpr const char *TEXTURE_PATHS[4] = {
      "image/btn_disabled.png", "image/btn_normal.png",
      "image/btn_selected.png", "image/btn_pressed.png"};
  static constexpr Color TEXT_COLORS[4] = {GRAY, WHITE, WHITE, YELLOW};
};

#endif
