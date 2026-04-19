#pragma once
#include "scriptAssemblyData.h"
#include "asset/assetImporter.h"

namespace ion
{
class ScriptDllImporter final : public AssetImporter<ScriptAssemblyData>
{
public:
    grl::Box<ScriptAssemblyData> import(const std::filesystem::path &src, const NoOptions &options) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
