#include "image.h"

#include "graphicsSystem.h"

namespace ion
{
    Image::Image(const grl::Rc<urhi::Texture>& texture, const grl::Rc<urhi::Sampler>& sampler)
        : texture(texture), sampler(sampler)
    {  }

    urhi::PixelFormat getFormat(const urhi::PixelType type)
    {
        switch (type)
        {
            case urhi::PixelType::UnsignedByte:
                return urhi::PixelFormat::R8G8B8A8Unorm;
            case urhi::PixelType::Float:
                return urhi::PixelFormat::R32G32B32A32Float;
            default:
                return urhi::PixelFormat::R8G8B8A8Unorm;
        }
    }

    Image::Image(const AssetRef<TextureData> &data, grl::Rc<urhi::Sampler> sampler)
    {
        const grl::Rc<urhi::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
        if(sampler == nullptr)
        {
            constexpr urhi::SamplerDesc samplerDesc;
            sampler = device->createSampler(samplerDesc);
        }

        this->sampler = sampler;

        urhi::TextureDesc texDesc{};
        texDesc.width = data->width;
        texDesc.height = data->height;
        texDesc.format = getFormat(data->pixelType);

        texture = device->createTexture(texDesc);

        urhi::TextureUploadDesc uploadDesc{};

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
