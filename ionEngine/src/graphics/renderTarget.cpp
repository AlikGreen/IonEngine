#include "renderTarget.h"

namespace ion
{
    bool RenderTarget::resize(const uint32_t w, const uint32_t h)
    {
        if(m_width == w && m_height == h)
            return false;

        m_width = glm::max(w, 1u);
        m_height = glm::max(h, 1u);

        const urhi::TextureDesc colTexDesc = urhi::TextureDesc::Texture2D(m_width, m_height, urhi::PixelFormat::R8G8B8A8Unorm, urhi::TextureUsage::ColorTarget);
        const grl::Rc<urhi::Texture> colTex = m_device->createTexture(colTexDesc);
        const urhi::TextureViewDesc colViewDesc(colTex);
        m_colorAttachment = m_device->createTextureView(colViewDesc);

        if(m_useDepth)
        {
            const urhi::TextureDesc depthTexDesc = urhi::TextureDesc::Texture2D(m_width, m_height, urhi::PixelFormat::D24UnormS8Uint, urhi::TextureUsage::DepthStencilTarget);
            const grl::Rc<urhi::Texture> depthTex = m_device->createTexture(depthTexDesc);
            const urhi::TextureViewDesc depthViewDesc(depthTex);
            m_depthAttachment = m_device->createTextureView(depthViewDesc);
        }

        return true;
    }

    RenderTarget::RenderTarget(const grl::Rc<urhi::Device> &device, const uint32_t w, const uint32_t h, const bool useDepth)
    : m_device(device), m_useDepth(useDepth)
    {
        resize(w, h);
    }
}
