#pragma once
#include <slang/compiler.h>

#include "asset/assetImporter.h"

namespace ion
{
struct ShaderImportOpts
{
    std::vector<std::string> includeDirs{};
    std::vector<std::string> defines{};
};

class ShaderImporter final : public AssetImporter<urhi::slang::Module, ShaderImportOpts>
{
public:
    grl::Box<urhi::slang::Module> import(const std::filesystem::path &src, const ShaderImportOpts& options) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
