#include "renderer.h"

#include "components/meshRenderer.h"
#include "components/pointLight.h"
#include "core/components/transformComponent.h"

namespace ion
{
    CulledRenderables Renderer::performCulling(Scene &scene, entis::Entity camEntity)
    {
        const auto& camera = camEntity.get<Camera>();
        const auto& meshRenderers = scene.getRegistry().view<MeshRenderer, Transform>();
        std::vector<Renderable> orderedAndCulledEntities;
        orderedAndCulledEntities.reserve(meshRenderers.size());

        for (auto[entity, meshRenderer, transform] : meshRenderers)
        {
            if(meshRenderer.mesh == nullptr) continue;

            const glm::mat4 worldMat = Transform::getWorldMatrix(entity);
            glm::mat4 cameraMat = Transform::getWorldMatrix(camEntity);
            if(camera.getFrustum(glm::inverse(cameraMat)).intersects(meshRenderer.mesh->getBounds()))
            {
                for(size_t i = 0; i < meshRenderer.mesh->getPrimitives().size(); i++)
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

                    orderedAndCulledEntities.emplace_back(renderable);
                }
            }
        }

        CulledRenderables culledEntities{};
        culledEntities.all = orderedAndCulledEntities;

        return culledEntities;
    }

    CameraUniformData Renderer::getCameraUniformData(entis::Entity camEntity)
    {
        const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
        const glm::mat4 invViewMat = Transform::getWorldMatrix(camEntity) * flip;
        const glm::mat4 viewMat = glm::inverse(invViewMat);
        const glm::mat4 projMat = camEntity.get<Camera>().getProjectionMatrix();
        const glm::mat4 viewProjMat = projMat * viewMat;

        CameraUniformData cameraUniformData{};
        cameraUniformData.view = viewMat;
        cameraUniformData.projection = projMat;
        cameraUniformData.viewProjection = viewProjMat;
        cameraUniformData.invView = invViewMat;
        cameraUniformData.invProjection = glm::inverse(projMat);
        cameraUniformData.invViewProjection = glm::inverse(viewProjMat);

        return cameraUniformData;
    }

    PointLightsUniformData Renderer::getPointLightsUniformData(Scene &scene)
    {
        PointLightsUniformData pointLightsData{};

        auto& lightsView = scene.getRegistry().view<PointLight, Transform>();

        int index = 0;
        for(auto [entity, light, transform] : lightsView)
        {
            PointLightUniformData pointLightData{};
            pointLightData.color = light.color;
            pointLightData.position = transform.getPosition();
            pointLightData.intensity = light.power;
            pointLightData.radius = 0.0f; // not used yet

            pointLightsData.lights[index] = pointLightData;
            index++;
        }

        pointLightsData.count = index;

        return pointLightsData;
    }

    void Renderer::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        for(const auto& pass : m_passes)
        {
            pass->execute(cmd, ctx);
        }
    }
}
