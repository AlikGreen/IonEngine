#include "camera.h"

#include "glm/ext/matrix_clip_space.hpp"


namespace Neon
{
    void Camera::setAspectRatio(const float aspectRatio)
    {
        if(this->aspectRatio == aspectRatio) return;

        this->aspectRatio = aspectRatio;
        projectionDirty = true;
    }

    float Camera::getAspectRatio() const
    {
        return aspectRatio;
    }

    void Camera::setFov(const float fov)
    {
        if(this->fov == fov) return;

        this->fov = fov;
        projectionDirty = true;
    }

    float Camera::getFov() const
    {
        return fov;
    }

    void Camera::setNear(const float near)
    {
        if(nearClip == near) return;

        nearClip = near;
        projectionDirty = true;
    }

    float Camera::getNear() const
    {
        return nearClip;
    }

    void Camera::setFar(const float far)
    {
        if(farClip == far) return;

        farClip = far;
        projectionDirty = true;

    }

    float Camera::getFar() const
    {
        return farClip;
    }

    Frustum Camera::getFrustum(const glm::mat4 &view) const
    {
        if(projectionDirty || view != this->viewMatrix)
        {
            this->viewMatrix = view;
            updateProjectionMatrix();
        }

        return frustum;
    }


    const glm::mat4 & Camera::getProjectionMatrix() const
    {
        if (projectionDirty)
        {
            updateProjectionMatrix();
        }
        return projectionMatrix;
    }


    void Camera::updateProjectionMatrix() const
    {
        projectionMatrix = glm::perspective(
            glm::radians(fov),
            aspectRatio,
            nearClip,
            farClip
        );

        frustum.update(projectionMatrix, viewMatrix);

        projectionDirty = false;
    }
}
