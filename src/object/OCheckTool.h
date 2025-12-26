#ifndef O_CHECK_TOOL
#define O_CHECK_TOOL
#include "../engine/GameObject.h"
#include <nlohmann/json.hpp>
class OCheckTool : GameObject {
public:
  void Draw() override {}
  void Update(float dt) override {};
  bool CheckToolSha256();
  bool CheckToolExists();
  bool ToolListIsMissing();
  OCheckTool();

private:
  nlohmann::json toolList;
  bool skipCheck = false;
};
#endif