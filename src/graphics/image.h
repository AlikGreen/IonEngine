#pragma once
#include <tiny_gltf.h>

#include "sampler.h"
#include "spirv_common.hpp"
#include "texture.h"
#include "textureData.h"
#include "asset/assetRef.h"

namespace ion
{
class Image
{
public:
    Image(const grl::Rc<urhi::Texture>& texture, const grl::Rc<urhi::Sampler>& sampler);
    explicit Image(const TextureData &data, const grl::Rc<urhi::Sampler> &sampler = nullptr);

    grl::Rc<urhi::Texture> texture();
    grl::Rc<urhi::TextureView> textureView();
    grl::Rc<urhi::Sampler> sampler();

    [[nodiscard]] std::vector<uint8_t> pixels() const;
    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }
    [[nodiscard]] urhi::PixelFormat format() const { return m_format; }
private:
    [[nodiscard]] uint32_t sizeInBytes() const;
    void upload();

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    urhi::PixelFormat m_format = urhi::PixelFormat::RGBA8UNorm;
    std::vector<uint8_t> m_pixels;
    bool m_uploaded = false;

    grl::Rc<urhi::Texture> m_texture;
    grl::Rc<urhi::TextureView> m_textureView;
    grl::Rc<urhi::Sampler> m_sampler;
};
}
