#include "renderSystem.h"

#include <tiny_gltf.h>
#include <neonRHI/neonRHI.h>

#include "graphicsSystem.h"
#include "assets/materialShader.h"
#include "assets/mesh.h"
#include "asset/assetManager.h"
#include "components/camera.h"
#include "components/pointLight.h"
#include "core/engine.h"
#include "core/eventManager.h"
#include "core/sceneManager.h"
#include "core/components/transformComponent.h"

#include "events/windowResizeEvent.h"
#include "glm/glm.hpp"
#include "glm/gtx/dual_quaternion.hpp"

namespace Neon
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct MaterialUniforms
    {
        float roughness;
        float metalness;
        alignas(16) glm::vec4 albedo;
        int useAlbedoTexture;
    };

    struct PointLightUniform
    {
        glm::vec3 position;
        float power;
        alignas(16) glm::vec3 color;
    };

    struct PointLightUniforms
    {
        int pointLightsCount{};
        alignas(16) PointLightUniform pointLights[32]{};
    };

    void RenderSystem::preStartup()
    {
        m_graphicsSystem = Engine::getSystem<GraphicsSystem>();

        m_window = m_graphicsSystem->getWindow();
        m_device = m_graphicsSystem->getDevice();

        // Use a temporary command list for initialization logic
        const Rc<RHI::CommandList> cl = m_device->createCommandList();

        m_cameraUniformBuffer      = m_device->createUniformBuffer();
        m_pointLightsUniformBuffer = m_device->createUniformBuffer();

        m_screenMesh = Mesh();

        const std::vector screenVertices =
        {
            Vertex { {-1, -1, 0 }, { }, { 0, 0} },
            Vertex { { 1, -1, 0 }, { }, { 1, 0} },
            Vertex { { 1,  1, 0 }, { }, { 1, 1} },
            Vertex { {-1,  1, 0 }, { }, { 0, 1} },
        };

        const std::vector<uint32_t> quadIndices =
        {
            0, 1, 2,
            0, 2, 3
        };

        m_screenMesh.setVertices(screenVertices);
        m_screenMesh.setIndices(quadIndices);

        m_screenMesh.apply();

        cl->begin();

        cl->reserveBuffer(m_cameraUniformBuffer     , sizeof(CameraUniforms));
        cl->reserveBuffer(m_pointLightsUniformBuffer, sizeof(PointLightUniforms));

        m_device->submit(cl);
    }

    void RenderSystem::render()
    {
        auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry();
        const auto& cameras = registry.view<Camera, Transform>();

        for(auto [camEntity, camera, camTransform] : cameras)
        {

        }
    }

    void RenderSystem::event(Event *event)
    {
        if(dynamic_cast<WindowResizeEvent*>(event))
        {

        }
    }
}