#include "image.h"

#include "graphicsSystem.h"
#include "core/engine.h"
#include "helpers/gfx.h"
#include "helpers/textureReadback.h"

namespace ion
{
    Image::Image(const dg::Ref<dg::ITexture>& texture, const dg::Ref<dg::ISampler>& sampler)
        : m_texture(texture), m_sampler(sampler)
    {
        const auto texDesc = m_texture->GetDesc();
        m_format = texDesc.Format;
        m_width = texDesc.Width;
        m_height = texDesc.Height;
        m_uploaded = true;
    }

    Image::Image(const TextureData &data, const dg::Ref<dg::ISampler> &sampler)
    {
        m_sampler = sampler;
        if(m_sampler == nullptr)
        {
            const dg::Ref<dg::IRenderDevice> device = Engine::getSystem<GraphicsSystem>()->device();
            device->CreateSampler({}, &m_sampler);
        }

        m_pixels = data.data;
        m_width = data.width;
        m_height = data.height;
        m_format = data.pixelFormat;
    }

    dg::Ref<dg::ITexture> Image::texture()
    {
        if(m_texture == nullptr)
            upload();

        return m_texture;
    }

    dg::Ref<dg::ITextureView> Image::srv()
    {
        if(m_srv == nullptr)
        {
            if(m_texture == nullptr)
                upload();

            const dg::Ref<dg::IRenderDevice> device = Engine::getSystem<GraphicsSystem>()->device();
            m_srv = m_texture->GetDefaultView(dg::TEXTURE_VIEW_SHADER_RESOURCE);
        }

        return m_srv;
    }

    dg::Ref<dg::ISampler> Image::sampler()
    {
        return m_sampler;
    }

    std::vector<uint8_t> Image::pixels() const
    {
        if(!m_uploaded)
        {
            return m_pixels;
        }

        const auto device = Engine::getSystem<GraphicsSystem>()->device();
        const auto ctx = Engine::getSystem<GraphicsSystem>()->imContext();

        TextureReadbackDesc desc;
        desc.width = m_width;
        desc.height = m_height;
        desc.srcTexture = m_texture;

        TextureReadback readback = TextureReadback::Create(device, ctx, desc);

        return readback.getData();
    }

    void Image::upload()
    {
        const auto graphicsSys = Engine::getSystem<GraphicsSystem>();
        const auto device = graphicsSys->device();
        const auto ctx = graphicsSys->imContext();

        dg::TextureDesc texDesc{};
        texDesc.Width = m_width;
        texDesc.Height = m_height;
        texDesc.Format = m_format;
        texDesc.Type = dg::RESOURCE_DIMENSION::RESOURCE_DIM_TEX_2D;
        texDesc.BindFlags = dg::BIND_SHADER_RESOURCE;
        texDesc.MipLevels = 1;
        texDesc.Usage = dg::USAGE_DEFAULT;

        const uint32_t texSize = gfx::getTextureSize(m_format, m_width, m_height, 1);

        dg::TextureSubResData subResData{};
        subResData.pData = m_pixels.data();
        subResData.Stride = texSize / m_height;
        subResData.DepthStride = texSize;

        dg::TextureData texData{};
        texData.pContext = ctx;
        texData.NumSubresources = 1;
        texData.pSubResources = &subResData;

        device->CreateTexture(texDesc, &texData, &m_texture);

        m_pixels.clear();
        m_pixels.shrink_to_fit();
        m_uploaded = true;
    }
}
