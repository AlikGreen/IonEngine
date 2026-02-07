#pragma once
#include "graphics/renderPass.h"

namespace ion::Editor
{
class ObjectPickingPass final : public RenderPass
{
public:
    explicit ObjectPickingPass(const grl::Rc<urhi::Device> &device);
    void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx) override;
private:
    MaterialShader m_pickingMaterial;
    grl::Rc<urhi::Buffer> m_modelUniformBuffer;
};
}
