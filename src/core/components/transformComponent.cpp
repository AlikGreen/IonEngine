#include "transformComponent.h"

#include "parentComponent.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ion
{
    glm::mat4 Transform::getLocalMatrix() const
    {
        return glm::translate(glm::mat4(1.0f), position)
         * glm::mat4_cast(rotation)
         * glm::scale(glm::mat4(1.0f), scale);
    }

    void Transform::setLocalMatrix(const glm::mat4 &transform)
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotQuat;
        glm::decompose(transform, scale, rotQuat, position, skew, perspective);

        rotation = glm::eulerAngles(rotQuat);
    }

    glm::vec3 Transform::eulerAngles() const
    {
        return glm::eulerAngles(rotation);
    }

    void Transform::eulerAngles(const glm::vec3 vec)
    {
        rotation = glm::quat(vec);
    }

    glm::vec3 Transform::forward() const
    {
        return xyz((getLocalMatrix() * glm::vec4(0, 0, 1, 0)));
    }

    glm::vec3 Transform::backward() const
    {
        return -forward();
    }

    glm::vec3 Transform::up() const
    {
        return xyz((getLocalMatrix() * glm::vec4(0, 1, 0, 0)));
    }

    glm::vec3 Transform::down() const
    {
        return -up();
    }

    glm::vec3 Transform::right() const
    {
        return xyz((getLocalMatrix() * glm::vec4(1, 0, 0, 0)));
    }

    glm::vec3 Transform::left() const
    {
        return -right();
    }


    glm::mat4 Transform::getWorldMatrix(entis::Entity entity, const glm::mat4& parentMatrix)
    {
        const auto parent = entity.get<Parent>();

        const glm::mat4 localMatrix = entity.get<Transform>().getLocalMatrix();

        if(parent.hasParent())
        {
            const glm::mat4 parentWorldMatrix = getWorldMatrix(parent.getParent(), parentMatrix);
            return parentWorldMatrix * localMatrix;
        }

        return parentMatrix * localMatrix;
    }

    glm::mat4 Transform::setWorldMatrix(entis::Entity entity, const glm::mat4& matrix)
    {
        const auto& parent = entity.get<Parent>();

        glm::mat4 localMatrix = matrix;

        if (parent.hasParent())
        {
            const glm::mat4 parentWorld = getWorldMatrix(parent.getParent(), glm::mat4(1.0f));
            localMatrix = glm::inverse(parentWorld) * matrix;
        }

        auto& transform = entity.get<Transform>();

        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(localMatrix, transform.scale, transform.rotation, transform.position, skew, perspective);

        return localMatrix;
    }
}
