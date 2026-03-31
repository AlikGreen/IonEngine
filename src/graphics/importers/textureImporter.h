#pragma once

#include "asset/assetImporter.h"
#include "graphics/assets/textureData.h"

namespace ion
{
class TextureImporter final : public AssetImporter<TextureData>
{
public:
    grl::Box<TextureData> import(const std::filesystem::path& filepath) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
