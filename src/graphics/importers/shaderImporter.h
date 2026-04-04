#pragma once
#include "shaderSet.h"
#include "asset/assetImporter.h"

namespace ion
{
class ShaderImporter final : public AssetImporter<urhi::ShaderSet>
{
public:
    grl::Box<urhi::ShaderSet> import(const std::filesystem::path &src, ImportOptions<urhi::ShaderSet> options) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
