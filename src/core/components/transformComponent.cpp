#include "transformComponent.h"

#include "parentComponent.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ion
{
    glm::mat4 Transform::getLocalMatrix() const
    {
        return glm::translate(glm::mat4(1.0f), m_position)
         * glm::mat4_cast(m_rotation)
         * glm::scale(glm::mat4(1.0f), m_scale);
    }

    void Transform::setLocalMatrix(const glm::mat4 &transform)
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotQuat;
        glm::decompose(transform, m_scale, rotQuat, m_position, skew, perspective);

        m_rotation = glm::eulerAngles(rotQuat);

        m_dirty = true;
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
        glm::decompose(localMatrix, transform.m_scale, transform.m_rotation, transform.m_position, skew, perspective);

        return localMatrix;
    }
}
