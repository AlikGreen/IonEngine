#pragma once
#include "asset/assetSerializer.h"
#include "graphics/shaders/shaderModule.h"

namespace ion
{
class ShaderModuleSerializer final : public AssetSerializer<ShaderModule>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::ShaderModule");

    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const ShaderModule &module) override;
    grl::Rc<ShaderModule> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
