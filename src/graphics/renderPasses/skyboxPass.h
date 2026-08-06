#pragma once
#include "graphics/renderPass.h"

namespace ion
{
class SkyboxRenderPass final : public RenderPass
{
public:
    SkyboxRenderPass();
    void execute(const dg::Ref<dg::IDeviceContext>& dc, RenderContext &ctx) override;
private:
    dg::Ref<dg::IBuffer> m_screenVertexBuffer;
    dg::Ref<dg::IBuffer> m_screenIndexBuffer;

    AssetRef<ShaderModule> m_shaderModule;
};
}
