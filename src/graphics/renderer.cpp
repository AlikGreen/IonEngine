#include "renderer.h"

#include "components/meshRenderer.h"
#include "components/pointLight.h"
#include "core/components/transformComponent.h"

namespace ion
{
    CulledRenderables Renderer::performCulling(Scene &scene, entis::Entity camEntity)
    {
        const auto& camera = camEntity.get<Camera>();
        const auto& meshRenderers = scene.registry().view<MeshRenderer, Transform>();
        std::vector<Renderable> renderables;
        renderables.reserve(meshRenderers.size());
        std::vector<Renderable> opaqueRenderables;
        opaqueRenderables.reserve(meshRenderers.size());
        std::vector<Renderable> transparentRenderables;
        transparentRenderables.reserve(meshRenderers.size());

        for (auto[entity, meshRenderer, transform] : meshRenderers)
        {
            if(meshRenderer.mesh == nullptr) continue;

            const glm::mat4 worldMat = Transform::getWorldMatrix(entity);
            glm::mat4 cameraMat = Transform::getWorldMatrix(camEntity);
            if(camera.getFrustum(glm::inverse(cameraMat)).intersects(meshRenderer.mesh->bounds()))
            {
                for(size_t i = 0; i < meshRenderer.mesh->primitives().size(); i++)
                {
                    Renderable renderable{};
                    renderable.entity = entity;

                    if(meshRenderer.materials.size() > i)
                        renderable.material = meshRenderer.materials[i].get();
                    else
                        renderable.material = meshRenderer.getMaterial().get();

                    renderable.mesh = meshRenderer.mesh.get();
                    renderable.submeshIndex = i;
                    renderable.worldMatrix = worldMat;

                    auto camPos = glm::vec3(cameraMat[3]);
                    auto meshPos = glm::vec3(worldMat[3]);

                    renderable.distanceToCamera = glm::distance(camPos, meshPos);

                    renderables.emplace_back(renderable);

                    if(renderable.material->isOpaque())
                        opaqueRenderables.emplace_back(renderable);
                    else
                        transparentRenderables.emplace_back(renderable);
                }
            }
        }

        renderables.shrink_to_fit();
        opaqueRenderables.shrink_to_fit();
        transparentRenderables.shrink_to_fit();

        CulledRenderables culledEntities{};
        culledEntities.all = renderables;
        culledEntities.opaques = opaqueRenderables;
        culledEntities.transparent = transparentRenderables;

        return culledEntities;
    }


    CameraData Renderer::createCameraUniformData(entis::Entity camEntity)
    {
        const auto worldMat = Transform::getWorldMatrix(camEntity);
        const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
        const glm::mat4 invViewMat = worldMat * flip;
        const glm::mat4 viewMat = glm::inverse(invViewMat);
        const glm::mat4 projMat = camEntity.get<Camera>().getProjectionMatrix();
        const glm::mat4 viewProjMat = projMat * viewMat;

        CameraData cameraUniformData{};
        cameraUniformData.view = viewMat;
        cameraUniformData.projection = projMat;
        cameraUniformData.viewProjection = viewProjMat;
        cameraUniformData.invView = invViewMat;
        cameraUniformData.invProjection = glm::inverse(projMat);
        cameraUniformData.invViewProjection = glm::inverse(viewProjMat);
        cameraUniformData.position = xyz(worldMat[3]);

        return cameraUniformData;
    }

    PointLightsData Renderer::createPointLightsUniformData(Scene &scene)
    {
        PointLightsData pointLightsData{};

        auto& lightsView = scene.registry().view<PointLight, Transform>();

        int index = 0;
        for(const auto& [entity, light, transform] : lightsView)
        {
            PointLightData pointLightData{};
            pointLightData.color = light.color;
            pointLightData.position = transform.position;
            pointLightData.power = light.power;
            pointLightData.radius = 0.0f; // not used yet

            pointLightsData.lights[index] = pointLightData;
            index++;
        }

        pointLightsData.count = index;

        return pointLightsData;
    }

    PassData Renderer::createPassData(const glm::vec2 resolution)
    {
        PassData pass{};
        constexpr double timeRepeatValue = glm::pi<double>() * 2 * 1024;
        pass.time = static_cast<float>(glm::mod(Engine::getTime(), timeRepeatValue));
        pass.deltaTime = Engine::getDeltaTime();
        pass.frameCount = Engine::getFrames();
        pass.resolution = resolution;
        pass.invResolution = 1.0f / resolution;
        return pass;
    }

    void Renderer::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        for(const auto& pass : m_passes)
        {
            pass->execute(cmd, ctx);
        }
    }
}
