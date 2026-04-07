#pragma once
#include "shaderSet.h"
#include "asset/assetImporter.h"

namespace ion
{
struct ShaderImportOpts
{
    std::vector<std::string> includeDirs{};
    std::vector<std::string> additionalModulePaths{};
    std::vector<std::string> typeSpecializations{};
};

class ShaderImporter final : public AssetImporter<urhi::ShaderSet, ShaderImportOpts>
{
public:
    grl::Box<urhi::ShaderSet> import(const std::filesystem::path &src, const ShaderImportOpts& options) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
