#pragma once
#include <tiny_gltf.h>

#include "sampler.h"
#include "texture.h"
#include "textureData.h"

namespace ion
{
class Image
{
public:
    Image(const dg::Ref<dg::ITexture>& texture, const dg::Ref<dg::ISampler>& sampler);
    explicit Image(const TextureData &data, const dg::Ref<dg::ISampler> &sampler = {});

    dg::Ref<dg::ITexture> texture();
    dg::Ref<dg::ITextureView> srv();
    dg::Ref<dg::ISampler> sampler();

    [[nodiscard]] std::vector<uint8_t> pixels() const;
    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }
    [[nodiscard]] dg::TEXTURE_FORMAT format() const { return m_format; }
private:
    void upload();

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    dg::TEXTURE_FORMAT m_format = dg::TEX_FORMAT_RGBA8_UNORM;
    std::vector<uint8_t> m_pixels;
    bool m_uploaded = false;

    dg::Ref<dg::ITexture> m_texture;
    dg::Ref<dg::ITextureView> m_srv;
    dg::Ref<dg::ISampler> m_sampler;
};
}
