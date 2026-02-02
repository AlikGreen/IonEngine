#pragma once
#include "graphics/renderPass.h"

namespace Neon::Editor
{
class ObjectPickingPass final : public RenderPass
{
public:
    explicit ObjectPickingPass(const Rc<RHI::Device> &device);
    void execute(const Rc<RHI::CommandList>& cmd, RenderContext &ctx) override;
private:
    MaterialShader m_pickingMaterial;
    Rc<RHI::Buffer> m_modelUniformBuffer;
};
}
