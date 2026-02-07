#pragma once
#include "assets/mesh.h"
#include "glm/glm.hpp"

namespace ion
{
class Frustum
{
public:
  Frustum() = default;

  void update(const glm::mat4 &projection, const glm::mat4 &view);

  bool intersects(const AABB &aabb) const;
private:
  glm::vec4 left, right, bottom, top, nearP, farP;
};
}
