#include "renderTarget.h"

namespace ion
{
    bool RenderTarget::resize(const uint32_t w, const uint32_t h)
    {
        if(m_width == w && m_height == h)
            return false;

        m_width = std::max(w, 1u);
        m_height = std::max(h, 1u);

        std::vector<dg::RenderPassAttachmentDesc> attachments{};
        std::vector<dg::ITextureView*> attachmentViews{};

        dg::TextureDesc colTexDesc{};
        colTexDesc.Type = dg::RESOURCE_DIM_TEX_2D;
        colTexDesc.Width = m_width;
        colTexDesc.Height = m_height;
        colTexDesc.Format = dg::TEX_FORMAT_RGBA8_UNORM;
        colTexDesc.BindFlags = dg::BIND_RENDER_TARGET | dg::BIND_SHADER_RESOURCE;

        dg::Ref<dg::ITexture> colTex;
        m_device->CreateTexture(colTexDesc, nullptr, &colTex);
        m_colorRTV = colTex->GetDefaultView(dg::TEXTURE_VIEW_RENDER_TARGET);
        m_colorSRV = colTex->GetDefaultView(dg::TEXTURE_VIEW_SHADER_RESOURCE);

        dg::RenderPassAttachmentDesc colorAttachment;
        colorAttachment.Format = dg::TEX_FORMAT_RGBA8_UNORM;

        attachments.push_back(colorAttachment);
        attachmentViews.push_back(m_colorRTV);

        if(m_useDepth)
        {
            dg::TextureDesc depthTexDesc{};
            depthTexDesc.Type = dg::RESOURCE_DIM_TEX_2D;
            depthTexDesc.Width = m_width;
            depthTexDesc.Height = m_height;
            depthTexDesc.Format = dg::TEX_FORMAT_D32_FLOAT;
            depthTexDesc.BindFlags = dg::BIND_DEPTH_STENCIL | dg::BIND_SHADER_RESOURCE;

            dg::Ref<dg::ITexture> depthTex;
            m_device->CreateTexture(depthTexDesc, nullptr, &depthTex);
            m_depthDSV = depthTex->GetDefaultView(dg::TEXTURE_VIEW_DEPTH_STENCIL);
            m_depthSRV = depthTex->GetDefaultView(dg::TEXTURE_VIEW_SHADER_RESOURCE);

            dg::RenderPassAttachmentDesc depthAttachment;
            depthAttachment.Format = dg::TEX_FORMAT_D32_FLOAT;

            attachments.push_back(depthAttachment);
            attachmentViews.push_back(m_depthDSV);
        }

        return true;
    }

    RenderTarget::RenderTarget(const dg::Ref<dg::IRenderDevice>& device, const uint32_t w, const uint32_t h, const bool useDepth)
    : m_device(device), m_useDepth(useDepth)
    {
        resize(w, h);
    }
}
