#pragma once
#include "textureView.h"

namespace ion
{
class RenderTarget
{
public:
    bool resize(uint32_t w, uint32_t h);

    dg::Ref<dg::ITextureView>  getColorRTV() { return m_colorRTV; }
    dg::Ref<dg::ITextureView>  getColorSRV() { return m_colorSRV; }

    dg::Ref<dg::ITextureView>  getDepthDTV() { return m_depthDSV; }
    dg::Ref<dg::ITextureView>  getDepthSRV() { return m_depthSRV; }

    [[nodiscard]] uint32_t getWidth() const { return m_width; }
    [[nodiscard]] uint32_t getHeight() const { return m_height; }
private:
    friend class GraphicsSystem;
    RenderTarget(const dg::Ref<dg::IRenderDevice>& device, uint32_t w, uint32_t h, bool useDepth);

    dg::Ref<dg::IRenderDevice> m_device;

    dg::Ref<dg::ITextureView> m_colorRTV;
    dg::Ref<dg::ITextureView> m_colorSRV;

    dg::Ref<dg::ITextureView> m_depthDSV;
    dg::Ref<dg::ITextureView> m_depthSRV;

    bool m_useDepth;
    uint32_t m_width{};
    uint32_t m_height{};
};
}
