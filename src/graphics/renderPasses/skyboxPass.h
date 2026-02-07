#pragma once
#include "graphics/renderPass.h"

namespace ion
{
class SkyboxRenderPass final : public RenderPass
{
public:
    SkyboxRenderPass();
    void execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx) override;
private:
    Mesh m_screenMesh;
};
}
