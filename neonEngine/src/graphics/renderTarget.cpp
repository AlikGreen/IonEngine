#include "renderTarget.h"

namespace Neon
{
    bool RenderTarget::resize(const uint32_t w, const uint32_t h)
    {
        if(m_width == w && m_height == h)
            return false;

        m_width = glm::max(w, 1u);
        m_height = glm::max(h, 1u);

        const RHI::TextureDesc colTexDesc = RHI::TextureDesc::Texture2D(m_width, m_height, RHI::PixelFormat::R8G8B8A8Unorm, RHI::TextureUsage::ColorTarget);
        const Rc<RHI::Texture> colTex = m_device->createTexture(colTexDesc);
        const RHI::TextureViewDesc colViewDesc(colTex);
        m_colorAttachment = m_device->createTextureView(colViewDesc);

        if(m_useDepth)
        {
            const RHI::TextureDesc depthTexDesc = RHI::TextureDesc::Texture2D(m_width, m_height, RHI::PixelFormat::D24UnormS8Uint, RHI::TextureUsage::DepthStencilTarget);
            const Rc<RHI::Texture> depthTex = m_device->createTexture(depthTexDesc);
            const RHI::TextureViewDesc depthViewDesc(depthTex);
            m_depthAttachment = m_device->createTextureView(depthViewDesc);
        }

        return true;
    }

    RenderTarget::RenderTarget(const Rc<RHI::Device> &device, const uint32_t w, const uint32_t h, const bool useDepth)
    : m_device(device), m_useDepth(useDepth)
    {
        resize(w, h);
    }
}
