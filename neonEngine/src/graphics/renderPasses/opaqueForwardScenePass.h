#pragma once
#include "graphics/renderPass.h"
#include <neonECS/neonECS.h>

#include "graphics/components/meshRenderer.h"

namespace Neon
{
class ForwardSceneRenderPass final : public RenderPass
{
public:
    explicit ForwardSceneRenderPass(const Rc<RHI::Device>& device);
    void execute(const Rc<RHI::CommandList>& cmd, RenderContext &ctx) override;
private:
    void drawRenderable(const Rc<RHI::CommandList>& cmd, const Renderable& renderable, const Rc<RHI::Buffer> &cameraBuffer, const Rc<RHI::Buffer> &pointLightsBuffer) const;
    Rc<RHI::Buffer> m_modelUniformBuffer;
};
}
