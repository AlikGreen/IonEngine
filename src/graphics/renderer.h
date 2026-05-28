#pragma once
#include "renderContext.h"
#include "renderPass.h"
#include "components/camera.h"
#include "core/scene.h"

namespace ion
{
struct CulledRenderables
{
    std::vector<Renderable> all;
    std::vector<Renderable> opaques;
    std::vector<Renderable> transparent;
};

class Renderer
{
public:
    Renderer() = default;

    static CulledRenderables performCulling(Scene& scene, entis::Entity camEntity);
    static CulledRenderables performCulling(Scene& scene, glm::mat4 camTransform, const Camera& camera);

    static CameraData createCameraUniformData(entis::Entity camEntity);
    static CameraData createCameraUniformData(glm::mat4 camTransform, const glm::mat4 &camProj);

    static PointLightsData createPointLightsUniformData(Scene &scene);
    static PassData createPassData(glm::vec2 resolution);

    void execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext& ctx);

    template<typename T, typename ...Args>
    requires std::derived_from<T, RenderPass> && std::is_constructible_v<T, Args...>
    void addPass(Args&&... args)
    {
        m_passes.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
private:
    std::vector<grl::Box<RenderPass>> m_passes{};
};
}
