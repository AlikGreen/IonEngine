#pragma once
#include "graphics/renderPass.h"
#include <entis/entis.h>

#include "graphics/components/meshRenderer.h"

namespace ion
{
class OpaqueForwardPass final : public RenderPass
{
public:
    explicit OpaqueForwardPass(const grl::Rc<urhi::Device>& device);
    void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx) override;
private:
    void drawRenderable(const grl::Rc<urhi::CommandList>& cmd, const Renderable& renderable, const grl::Rc<urhi::Buffer> &cameraBuffer, const grl::Rc<urhi::Buffer> &pointLightsBuffer) const;
    grl::Rc<urhi::Buffer> m_modelUniformBuffer;
};
}
