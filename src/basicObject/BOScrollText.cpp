#include "BOScrollText.h"
#include "../engine/LangManager.h"
BOScrollText::BOScrollText(const std::string &key, Rectangle area, Color c,
                         int siz)
    : BOText(key, {area.x, area.y}, c, false, siz), viewRect(area) {
  CalculateHeight();
}

void BOScrollText::RefreshText() {
  BOText::RefreshText();
  CalculateHeight();
}

void BOScrollText::CalculateHeight() {
  const Font &font = LangManager::GetFont();
  Vector2 measure =
      MeasureTextEx(font, displayText.c_str(), (float)fontSize, 1.0f);
  totalContentHeight = measure.y;
}

void BOScrollText::Update(float dt) {
  Vector2 mouse = GetMousePosition();
  if (!CheckCollisionPointRec(mouse, viewRect))
    return;

  Vector2 wheelMove = GetMouseWheelMoveV();
  if (wheelMove.y != 0.0f) {
    SetScrollOffset(scrollOffset + wheelMove.y * 20.0f);
  }
}

void BOScrollText::Draw() {
  BeginScissorMode((int)viewRect.x, (int)viewRect.y, (int)viewRect.width,
                   (int)viewRect.height);

  Vector2 savedDrawPos = drawPos;
  drawPos.y += scrollOffset;

  BOText::Draw();

  drawPos = savedDrawPos;
  EndScissorMode();
}

void BOScrollText::SetScrollOffset(float offset) {
  scrollOffset = offset;
  if (scrollOffset > 0.0f) {
    scrollOffset = 0.0f;
  }
  if (totalContentHeight > viewRect.height) {
    float maxScroll = -(totalContentHeight - viewRect.height);
    if (scrollOffset < maxScroll) {
      scrollOffset = maxScroll;
    }
  } else {
    scrollOffset = 0.0f;
  }
}
