#pragma once
#include "asset/assetImporter.h"
#include "graphics/shaders/shaderModule.h"

namespace ion
{
class ShaderImporter final : public AssetImporter<ShaderModule>
{
public:
    grl::Box<ShaderModule> import(const std::filesystem::path &path, const NoOptions&) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
