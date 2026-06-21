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
    grl::Rc<urhi::Buffer> m_screenVertexBuffer;
    grl::Rc<urhi::Buffer> m_screenIndexBuffer;

    urhi::GraphicsPipelineDesc m_pipelineDesc;
    AssetRef<urhi::slang::Module> m_shaderModule;
};
}
