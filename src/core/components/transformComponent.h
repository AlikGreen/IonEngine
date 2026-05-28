#pragma once
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#include <entis/entis.h>

#include "glm/detail/type_quat.hpp"

namespace ion
{
class Transform
{
public:
    [[nodiscard]] glm::mat4 getLocalMatrix() const;
    void setLocalMatrix(const glm::mat4 &transform);

    glm::vec3 eulerAngles() const;
    void eulerAngles(glm::vec3 vec);

    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 backward() const;
    [[nodiscard]] glm::vec3 up() const;
    [[nodiscard]] glm::vec3 down() const;
    [[nodiscard]] glm::vec3 right() const;
    [[nodiscard]] glm::vec3 left() const;

    static glm::mat4 getWorldMatrix(entis::Entity entity, const glm::mat4& parentMatrix = glm::mat4(1.0f));
    static glm::mat4 setWorldMatrix(entis::Entity entity, const glm::mat4& matrix);

    glm::vec3 position{};
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale{1.0f};
};
}
