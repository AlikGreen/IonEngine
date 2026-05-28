#pragma once
#include "graphics/renderPass.h"


#include "graphics/components/meshRenderer.h"

namespace ion
{
class OpaqueForwardPass final : public RenderPass
{
public:
    OpaqueForwardPass();
    void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx) override;
private:
    urhi::GraphicsPipelineDesc m_pipelineDesc;
    AssetRef<urhi::slang::Module> m_shaderModule;
};
}
