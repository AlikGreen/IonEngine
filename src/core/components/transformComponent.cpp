#include "transformComponent.h"

#include "parentComponent.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ion
{
    glm::mat4 Transform::getLocalMatrix() const
    {
        const glm::mat4 localTranslationMatrix = glm::translate(glm::mat4(1.0f), position);

        const glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
        const glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
        const glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1));
        const glm::mat4 localRotationMatrix = Rz * Ry * Rx;

        const glm::mat4 localScaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        return localTranslationMatrix * localRotationMatrix * localScaleMatrix;
    }

    void Transform::setLocalMatrix(const glm::mat4 &transform)
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotQuat;
        glm::decompose(transform, scale, rotQuat, position, skew, perspective);

        rotation = glm::eulerAngles(rotQuat);
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


    // TODO speed up and remove recursion

    glm::mat4 Transform::getWorldMatrix(entis::Entity entity, const glm::mat4& parentMatrix)
    {
        const auto parent = entity.get<Parent>();

        // Get this entity's local transform matrix
        const glm::mat4 localMatrix = entity.get<Transform>().getLocalMatrix();

        if(parent.hasParent())
        {
            // Recursively get the parent's world matrix and multiply with local
            const glm::mat4 parentWorldMatrix = getWorldMatrix(parent.getParent(), parentMatrix);
            return parentWorldMatrix * localMatrix;
        }

        // No parent, so world matrix is just parent matrix * local matrix
        return parentMatrix * localMatrix;
    }
}
