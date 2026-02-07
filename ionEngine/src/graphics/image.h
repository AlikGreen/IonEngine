#pragma once
#include <tiny_gltf.h>

#include "sampler.h"
#include "spirv_common.hpp"
#include "texture.h"
#include "asset/assetRef.h"
#include "assets/textureData.h"

namespace ion
{
class Image
{
public:
    Image(const grl::Rc<urhi::Texture>& texture, const grl::Rc<urhi::Sampler>& sampler);
    Image(const AssetRef<TextureData> &data, grl::Rc<urhi::Sampler> sampler = nullptr);

    grl::Rc<urhi::Texture> texture;
    grl::Rc<urhi::Sampler> sampler;
};
}
