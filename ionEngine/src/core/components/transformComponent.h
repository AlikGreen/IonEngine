#pragma once
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#include <entis/entis.h>

namespace ion
{
class Transform
{
public:
    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& pos);

    const glm::vec3& getRotation() const;
    void setRotation(const glm::vec3& rot);
    void rotate(const glm::vec3& rot);

    const glm::vec3& getScale() const;
    void setScale(const glm::vec3& scl);

    const glm::mat4& getLocalMatrix() const;
    void setLocalMatrix(const glm::mat4& transform);

    glm::vec3 forward() const;
    glm::vec3 backward() const;
    glm::vec3 up() const;
    glm::vec3 down() const;
    glm::vec3 right() const;
    glm::vec3 left() const;

    void translate(glm::vec3 translation);

    static glm::mat4 getWorldMatrix(entis::Entity entity, const glm::mat4& parentMatrix = glm::mat4(1.0f));
private:
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f};

    mutable bool dirty = true;
    mutable glm::mat4 cachedLocalMatrix{1.0f};
    mutable glm::mat4 cachedLocalRotationMatrix{1.0f};

    void updateMatrix() const;
};
}
