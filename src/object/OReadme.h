#ifndef O_README
#define O_README

#include "../engine/GameObject.h"
#include "../basicObject/BOScrollText.h"
#include <string>

class OReadme : public GameObject {
public:
  OReadme();
  ~OReadme();

  void Update(float dt) override;
  void Draw() override;

private:
  std::string FindReadmeFile() const;
  std::string LoadReadmeContent(const std::string &filename) const;
  void RefreshText() override;

private:
  BOScrollText* readmeText = nullptr;
};

#endif
