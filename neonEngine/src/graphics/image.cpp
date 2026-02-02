#include "image.h"

#include "graphicsSystem.h"

namespace Neon
{
    Image::Image(const Rc<RHI::Texture>& texture, const Rc<RHI::Sampler>& sampler)
        : texture(texture), sampler(sampler)
    {  }

    RHI::PixelFormat getFormat(const RHI::PixelType type)
    {
        switch (type)
        {
            case RHI::PixelType::UnsignedByte:
                return RHI::PixelFormat::R8G8B8A8Unorm;
            case RHI::PixelType::Float:
                return RHI::PixelFormat::R32G32B32A32Float;
            default:
                return RHI::PixelFormat::R8G8B8A8Unorm;
        }
    }

    Image::Image(const AssetRef<TextureData> &data, Rc<RHI::Sampler> sampler)
    {
        const Rc<RHI::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
        if(sampler == nullptr)
        {
            constexpr RHI::SamplerDesc samplerDesc;
            sampler = device->createSampler(samplerDesc);
        }

        this->sampler = sampler;

        RHI::TextureDesc texDesc{};
        texDesc.width = data->width;
        texDesc.height = data->height;
        texDesc.format = getFormat(data->pixelType);

        texture = device->createTexture(texDesc);

        RHI::TextureUploadDesc uploadDesc{};

        uploadDesc.data = data->data;
        uploadDesc.pixelType = data->pixelType;
        uploadDesc.pixelLayout = data->pixelLayout;
        uploadDesc.width = data->width;
        uploadDesc.height = data->height;

        const auto cl = device->createCommandList();

        cl->begin();
        cl->updateTexture(texture, uploadDesc);

        device->submit(cl);
    }
}
