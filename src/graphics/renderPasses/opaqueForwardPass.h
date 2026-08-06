#pragma once
#include "graphics/renderPass.h"


#include "graphics/components/meshRenderer.h"

namespace ion
{
class OpaqueForwardPass final : public RenderPass
{
public:
    OpaqueForwardPass(const dg::Ref<dg::IRenderDevice>& device);
    void execute(const dg::Ref<dg::IDeviceContext>& dc, RenderContext& ctx) override;
private:
    AssetRef<ShaderModule> m_shaderModule;
};
}
