#pragma once

#include "mesh.h"
#include "glm/glm.hpp"

namespace ion
{
class Frustum
{
public:
  Frustum() = default;

  void update(const glm::mat4 &projection, const glm::mat4 &view);

  [[nodiscard]] bool intersects(const AABB &aabb) const;
private:
  glm::vec4 left, right, bottom, top, nearP, farP;
};
}
