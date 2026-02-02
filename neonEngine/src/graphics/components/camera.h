#pragma once

#include "asset/assetRef.h"
#include "glm/glm.hpp"
#include "graphics/frustum.h"
#include "graphics/assets/materialShader.h"
#include "graphics/renderTarget.h"

namespace Neon
{
struct Camera
{
public:
    glm::vec4 bgColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    void setAspectRatio(float aspectRatio);
    [[nodiscard]] float getAspectRatio() const;

    void setFov(float fov);
    [[nodiscard]] float getFov() const;

    void setNear(float near);
    [[nodiscard]] float getNear() const;

    void setFar(float far);
    [[nodiscard]] float getFar() const;

    Frustum getFrustum(const glm::mat4 &view) const;
    [[nodiscard]] const glm::mat4& getProjectionMatrix() const;

    Rc<RenderTarget> renderTarget{};
    AssetRef<MaterialShader> skyboxMaterial{};
private:
    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    float aspectRatio = 1.0f;

    mutable bool projectionDirty = true;

    mutable glm::mat4 projectionMatrix{};
    mutable glm::mat4 viewMatrix{};

    mutable Frustum frustum{};

    void updateProjectionMatrix() const;
};
}
