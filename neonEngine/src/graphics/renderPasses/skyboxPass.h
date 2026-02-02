#pragma once
#include "graphics/renderPass.h"

namespace Neon
{
class SkyboxRenderPass final : public RenderPass
{
public:
    SkyboxRenderPass();
    void execute(const Rc<RHI::CommandList> &cmd, RenderContext &ctx) override;
private:
    Mesh m_screenMesh;
};
}
