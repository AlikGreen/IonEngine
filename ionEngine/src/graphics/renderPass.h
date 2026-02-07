#pragma once

#include "commandList.h"
#include "renderContext.h"
#include <entis/entis.h>

#include "frustum.h"
#include "assets/materialShader.h"
#include "assets/mesh.h"

namespace ion
{
struct CameraUniformData
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;

    glm::mat4 invView;
    glm::mat4 invProjection;
    glm::mat4 invViewProjection;
};

struct PointLightUniformData
{
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float intensity;
};

struct PointLightsUniformData
{
    int count;
    PointLightUniformData lights[64];
};

struct Renderable
{
    glm::mat4 worldMatrix{};

    Mesh* mesh{};
    MaterialShader* material{};
    size_t submeshIndex{};

    float distanceToCamera{};

    entis::Entity entity = entis::Entity::null();
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext& ctx) = 0;
};
}
