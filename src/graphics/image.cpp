#include "image.h"

#include "graphicsSystem.h"

namespace ion
{
    Image::Image(const grl::Rc<urhi::Texture>& texture, const grl::Rc<urhi::Sampler>& sampler)
        : texture(texture), sampler(sampler)
    {  }

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
        texDesc.format = data->pixelFormat;

        texture = device->createTexture(texDesc);

        urhi::TextureUploadDesc uploadDesc{};

        uploadDesc.data = data->data;
        uploadDesc.texture = texture;
        uploadDesc.width = data->width;
        uploadDesc.height = data->height;

        const auto cl = device->acquireCommandList(urhi::QueueType::Graphics);

        cl->begin();
        cl->updateTexture(uploadDesc);

        device->submit(cl);
    }
}
