#pragma once

#include "commandList.h"
#include "renderContext.h"
#include <entis/entis.h>

#include "frustum.h"
#include "materialInstance.h"
#include "mesh.h"

#include "frustum.h"

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
    float power;
    glm::vec3 color;
    float radius;
};

struct alignas(16) PointLightsUniformData
{
    int count;
    int padding[3];
    PointLightUniformData lights[64];
};

struct Renderable
{
    glm::mat4 worldMatrix{};

    Mesh* mesh{};
    MaterialInstance* material{};
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
