#include "renderer.h"

#include "components/meshRenderer.h"
#include "components/pointLight.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"

namespace ion
{
    CameraData Renderer::createCameraUniformData(entis::Entity camEntity)
    {
        const auto worldMat = Transform::getWorldMatrix(camEntity);
        return createCameraUniformData(worldMat, camEntity.get<Camera>().getProjectionMatrix());
    }

    CameraData Renderer::createCameraUniformData(glm::mat4 camTransform, const glm::mat4 &camProj)
    {
        const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
        const glm::mat4 invViewMat = camTransform * flip;
        const glm::mat4 viewMat = glm::inverse(invViewMat);
        const glm::mat4 projMat = camProj;
        const glm::mat4 viewProjMat = projMat * viewMat;

        CameraData cameraUniformData{};
        cameraUniformData.view = viewMat;
        cameraUniformData.projection = projMat;
        cameraUniformData.viewProjection = viewProjMat;
        cameraUniformData.invView = invViewMat;
        cameraUniformData.invProjection = glm::inverse(projMat);
        cameraUniformData.invViewProjection = glm::inverse(viewProjMat);
        cameraUniformData.position = xyz(camTransform[3]);

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
            pointLightData.position = transform.position();
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
