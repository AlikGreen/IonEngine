#pragma once
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#include <entis/entis.h>

#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"

namespace ion
{
class Transform
{
public:
    [[nodiscard]] glm::mat4 getLocalMatrix() const;
    void setLocalMatrix(const glm::mat4 &transform);

    [[nodiscard]] glm::vec3 eulerAngles() const { return glm::eulerAngles(m_rotation); }
    void eulerAngles(const glm::vec3 vec) { m_rotation = glm::quat(vec); m_dirty = true; }

    [[nodiscard]] glm::vec3 position() const { return m_position; }
    [[nodiscard]] glm::quat rotation() const { return m_rotation; }
    [[nodiscard]] glm::vec3 scale() const { return m_scale; }

    void position(const glm::vec3 position) { m_position = position; m_dirty = true; }
    void rotation(const glm::quat rotation) { m_rotation = rotation; m_dirty = true; }
    void scale(const glm::vec3 scale) { m_scale = scale; m_dirty = true; }

    void translate(const glm::vec3 translation) { m_position += translation; m_dirty = true; }

    [[nodiscard]] glm::vec3 forward() const { return xyz((getLocalMatrix() * glm::vec4(0, 0, 1, 0))); }
    [[nodiscard]] glm::vec3 backward() const { return -forward(); }
    [[nodiscard]] glm::vec3 up() const { return xyz((getLocalMatrix() * glm::vec4(0, 1, 0, 0))); }
    [[nodiscard]] glm::vec3 down() const { return -up(); }
    [[nodiscard]] glm::vec3 right() const { return xyz((getLocalMatrix() * glm::vec4(1, 0, 0, 0))); }
    [[nodiscard]] glm::vec3 left() const { return -right(); }

    [[nodiscard]] bool dirty() const { return m_dirty; }
    void dirty(const bool dirty) { m_dirty = dirty; }

    static glm::mat4 getWorldMatrix(entis::Entity entity, const glm::mat4& parentMatrix = glm::mat4(1.0f));
    static glm::mat4 setWorldMatrix(entis::Entity entity, const glm::mat4& matrix);
private:
    friend class TransformSerializer;

    glm::vec3 m_position{};
    glm::quat m_rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 m_scale{1.0f};
    bool m_dirty = true;
};
}
