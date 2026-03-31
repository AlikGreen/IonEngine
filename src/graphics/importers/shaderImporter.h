#pragma once
#include "asset/assetImporter.h"
#include "descriptions/shaderEntryPoint.h"

namespace ion
{
class ShaderImporter final : public AssetImporter<std::vector<urhi::ShaderEntryPoint>>
{
public:
    grl::Box<std::vector<urhi::ShaderEntryPoint>> import(const std::filesystem::path &src) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
