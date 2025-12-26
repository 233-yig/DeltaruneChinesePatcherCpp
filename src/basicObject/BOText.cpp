#include "BOText.h"
#include "../engine/LangManager.h"

BOText::BOText(const std::string &key, Vector2 p, Color c, bool center, int siz,
             float a)
    : langKey(key), pos(p), color(c), alpha(a), centered(center) {

  if (siz == DEFAULT_FONT_SIZE) {
    fontSize = LangManager::GetFont().baseSize;
  } else {
    fontSize = siz;
  }

  RefreshText();
}

std::string BOText::ProcessPlaceholders(const std::string &text) const {
  std::string result = text;
  for (const auto &[k, v] : params) {
    std::string placeholder = "{" + k + "}";
    size_t pos = 0;
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
      result.replace(pos, placeholder.length(), v);
      pos += v.length();
    }
  }
  return result;
}

void BOText::SetParam(const std::string &key, const std::string &value) {
  params[key] = value;
  RefreshText();
}

void BOText::SetParam(const std::string &key, float value) {
  SetParam(key, std::to_string(value));
}

void BOText::SetParam(const std::string &key, int value) {
  SetParam(key, std::to_string(value));
}

void BOText::SetText(const std::string &newKey) {
  if (langKey != newKey) {
    langKey = newKey;
    RefreshText();
  }
}

void BOText::SetPosition(Vector2 p) {
  pos = p;
  RefreshText();
}

void BOText::SetColor(Color c) { color = c; }

void BOText::SetAlpha(float a) { alpha = a; }

Vector2 BOText::GetSize() const {
  return MeasureTextEx(LangManager::GetFont(), displayText.c_str(),
                       (float)fontSize, 1.0f);
}

void BOText::RefreshText() {
  std::string rawText = LangManager::GetText(langKey);
  displayText = ProcessPlaceholders(rawText);

  Vector2 textSize = MeasureTextEx(LangManager::GetFont(), displayText.c_str(),
                                   (float)fontSize, 1.0f);

  drawPos.x = pos.x - (centered ? textSize.x / 2.0f : 0.0f);
  drawPos.y = pos.y;
}

void BOText::Update(float dt) {}

void BOText::Draw() {
  DrawTextEx(LangManager::GetFont(), displayText.c_str(), drawPos,
             (float)fontSize, 1.0f, Fade(color, alpha));
}
