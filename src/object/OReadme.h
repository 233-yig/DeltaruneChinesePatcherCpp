#ifndef O_README
#define O_README
#include "../engine/GameObject.h"
#include <string>

class OReadme : GameObject {
public:
  OReadme() = default;
  std::string FindReadmeFile() const;
  std::string LoadReadmeContent(const std::string &filename) const;
  std::string GetReadmeText() const;
};
#endif