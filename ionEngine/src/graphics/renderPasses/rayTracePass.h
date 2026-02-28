#pragma once
#include "graphics/renderPass.h"
#include <entis/entis.h>

#include "graphics/components/meshRenderer.h"

namespace ion
{
class RayTracePass final : public RenderPass
{
public:
    explicit RayTracePass(const grl::Rc<urhi::Device> &device);

    void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx) override;
private:
    grl::Rc<urhi::Pipeline> m_rayTracePipeline;
    grl::Rc<urhi::Buffer> m_modelUniformBuffer;
};
}
