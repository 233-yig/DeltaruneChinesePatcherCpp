#ifndef O_SCROLL_TEXT_H
#define O_SCROLL_TEXT_H

#include "BOText.h"

class BOScrollText : public BOText {
private:
  Rectangle viewRect{};
  float scrollOffset{0.0f};
  float totalContentHeight{0.0f};

  void CalculateHeight();

public:
  BOScrollText(const std::string &key, Rectangle area, Color c = WHITE,
              int siz = DEFAULT_FONT_SIZE);

  void RefreshText() override;
  void Update(float deltaTime) override;
  void Draw() override;
  void SetScrollOffset(float offset);
  float GetScrollOffset() const { return scrollOffset; }
  float GetContentHeight() const { return totalContentHeight; }
};

#endif
