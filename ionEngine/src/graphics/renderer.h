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
    static CameraUniformData getCameraUniformData(entis::Entity camEntity);

    static PointLightsUniformData getPointLightsUniformData(Scene &scene);

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
