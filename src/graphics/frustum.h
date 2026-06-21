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
  [[nodiscard]] const std::array<glm::vec4, 6>& planes() const { return m_planes; }
private:
  std::array<glm::vec4, 6> m_planes;
};
}
