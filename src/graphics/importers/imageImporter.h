#pragma once
#include "asset/assetImporter.h"
#include "graphics/image.h"

namespace ion
{
class ImageImporter final : public AssetImporter<Image>
{
public:
    grl::Box<Image> import(const std::filesystem::path& filepath, const NoOptions&) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
