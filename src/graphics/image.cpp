#include "image.h"

#include "graphicsSystem.h"
#include "core/engine.h"

namespace ion
{
    Image::Image(const grl::Rc<urhi::Texture>& texture, const grl::Rc<urhi::Sampler>& sampler)
        : m_texture(texture), m_sampler(sampler)
    {
        m_format = m_texture->format();
        m_width = m_texture->width();
        m_height = m_texture->height();
    }

    Image::Image(const TextureData &data, const grl::Rc<urhi::Sampler> &sampler)
    {
        m_sampler = sampler;
        if(m_sampler == nullptr)
        {
            const grl::Rc<urhi::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
            m_sampler = device->createSampler({});
        }

        m_pixels = data.data;
        m_width = data.width;
        m_height = data.height;
        m_format = data.pixelFormat;
    }

    grl::Rc<urhi::Texture> Image::texture()
    {
        if(m_texture == nullptr)
            upload();

        return m_texture;
    }

    grl::Rc<urhi::TextureView> Image::textureView()
    {
        if(m_textureView == nullptr)
        {
            if(m_texture == nullptr)
                upload();

            const grl::Rc<urhi::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
            m_textureView = device->createTextureView(m_texture);
        }

        return m_textureView;
    }

    grl::Rc<urhi::Sampler> Image::sampler()
    {
        return m_sampler;
    }

    std::vector<uint8_t> Image::pixels() const
    {
        return m_pixels;
    }

    uint32_t Image::width() const
    {
        return m_width;
    }

    uint32_t Image::height() const
    {
        return m_height;
    }

    uint32_t Image::sizeInBytes() const
    {
        return m_width * m_height * (m_format == urhi::PixelFormat::RGBA8UNorm ? 4 : 16);
    }

    void Image::upload()
    {
        const grl::Rc<urhi::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();

        urhi::TextureDesc texDesc{};
        texDesc.width = m_width;
        texDesc.height = m_height;
        texDesc.format = m_format;

        m_texture = device->createTexture(texDesc);

        urhi::TextureUploadDesc uploadDesc{};

        uploadDesc.data = m_pixels.data();
        uploadDesc.texture = m_texture;
        uploadDesc.width = m_width;
        uploadDesc.height = m_height;

        const auto cl = device->acquireCommandList(urhi::QueueType::Graphics); // TODO use transfer queue when it works

        cl->begin();
        cl->updateTexture(uploadDesc);

        device->submit(cl);

        m_pixels.clear();
        m_pixels.shrink_to_fit();
    }
}
