#include "BOButton.h"
#include "../engine/LangManager.h"
#include "../engine/LogManager.h"
constexpr const char *BOButton::TEXTURE_PATHS[4];
constexpr Color BOButton::TEXT_COLORS[4];

BOButton::BOButton(const std::string &langKey, Vector2 position, Vector2 size,
                 std::function<void()> callback, bool disabled)
    : pos(position), size(size), langKey(langKey), onClick(callback),
      state(disabled ? State::DISABLED : State::NORMAL) {
  for (int i = 0; i < 4; ++i) {
    textures[i] = LoadTexture(TEXTURE_PATHS[i]);
    if (textures[i].id == 0) {
      LogManager::Warning(std::string("[Button] Failed to load button texture: ") + TEXTURE_PATHS[i]);
    }
  }

  RefreshText();
}

BOButton::~BOButton() {
  for (auto &tex : textures) {
    UnloadTexture(tex);
  }
}

void BOButton::RefreshText() {
  displayText = LangManager::GetText(langKey);

  const Font &font = LangManager::GetFont();
  float fontSize = (float)font.baseSize;

  Vector2 textSize = MeasureTextEx(font, displayText.c_str(), fontSize, 1.0f);
  cachedTextPos.x = pos.x + (size.x - textSize.x) * 0.5f;
  cachedTextPos.y = pos.y + (size.y - textSize.y) * 0.5f;
}

void BOButton::SetEnabled(bool enabled) {
  if (enabled) {
    if (state == State::DISABLED) {
      state = State::NORMAL;
    }
  } else {
    state = State::DISABLED;
  }
}

void BOButton::SetPosition(Vector2 newPos) {
  pos = newPos;
  RefreshText();
}

void BOButton::SetSize(Vector2 newSize) {
  size = newSize;
  RefreshText();
}

void BOButton::SetCallback(std::function<void()> callback) {
  onClick = callback;
}

void BOButton::SetText(const std::string &key) {
  if (langKey != key) {
    langKey = key;
    RefreshText();
  }
}

void BOButton::Update(float dt) {
  if (state == State::DISABLED) {
    return;
  }

  Rectangle bounds = GetBounds();
  Vector2 mousePos = GetMousePosition();
  bool isHovered = CheckCollisionPointRec(mousePos, bounds);
  if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    state = State::PRESSED;
    return;
  }
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    if (state == State::PRESSED) {
      if (isHovered && onClick) {
        onClick();
      }
      state = isHovered ? State::HOVERED : State::NORMAL;
    }
    return;
  }
  if (state != State::PRESSED) {
    state = isHovered ? State::HOVERED : State::NORMAL;
  }
}

void BOButton::Draw() {

  Rectangle drawRect = GetBounds();
  int stateIndex = static_cast<int>(state);

  if (textures[stateIndex].id != 0) {
    DrawNineSlice(textures[stateIndex], drawRect, NINE_SLICE_MARGIN, WHITE);
  }
  const Font &font = LangManager::GetFont();
  float fontSize = (float)font.baseSize;

  DrawTextEx(font, displayText.c_str(), cachedTextPos, fontSize, 1.0f,
             TEXT_COLORS[stateIndex]);
}

void BOButton::DrawNineSlice(Texture2D tex, Rectangle rect, float margin,
                            Color tint) {
  float w = (float)tex.width;
  float h = (float)tex.height;
  Rectangle src[9] = {

      {0, 0, margin, margin},
      {margin, 0, w - 2 * margin, margin},
      {w - margin, 0, margin, margin},

      {0, margin, margin, h - 2 * margin},
      {margin, margin, w - 2 * margin, h - 2 * margin},
      {w - margin, margin, margin, h - 2 * margin},

      {0, h - margin, margin, margin},
      {margin, h - margin, w - 2 * margin, margin},
      {w - margin, h - margin, margin, margin}};
  Rectangle dst[9] = {

      {rect.x, rect.y, margin, margin},
      {rect.x + margin, rect.y, rect.width - 2 * margin, margin},
      {rect.x + rect.width - margin, rect.y, margin, margin},

      {rect.x, rect.y + margin, margin, rect.height - 2 * margin},
      {rect.x + margin, rect.y + margin, rect.width - 2 * margin,
       rect.height - 2 * margin},
      {rect.x + rect.width - margin, rect.y + margin, margin,
       rect.height - 2 * margin},

      {rect.x, rect.y + rect.height - margin, margin, margin},
      {rect.x + margin, rect.y + rect.height - margin, rect.width - 2 * margin,
       margin},
      {rect.x + rect.width - margin, rect.y + rect.height - margin, margin,
       margin}};
  for (int i = 0; i < 9; i++) {
    DrawTexturePro(tex, src[i], dst[i], {0, 0}, 0, tint);
  }
}