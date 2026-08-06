#pragma once

#include "commandList.h"
#include "renderContext.h"
#include <entis/entis.h>

#include "frustum.h"
#include "materialInstance.h"
#include "mesh.h"


namespace ion
{
struct CameraData
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;

    glm::mat4 invView;
    glm::mat4 invProjection;
    glm::mat4 invViewProjection;

    glm::vec3 position;
};

struct PassData
{
    float time;
    float deltaTime;
    uint32_t  frameCount;

    glm::vec2 resolution;
    glm::vec2 invResolution;
};

struct PointLightData
{
    glm::vec3 position;
    float power;
    glm::vec3 color;
    float radius;
};

struct alignas(16) PointLightsData
{
    int count;
    int padding[3];
    PointLightData lights[64];
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
    virtual void execute(const dg::Ref<dg::IDeviceContext>& dc, RenderContext& ctx) = 0;
};
}
