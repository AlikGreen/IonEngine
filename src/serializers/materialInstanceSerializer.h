#pragma once
#include "asset/assetSerializer.h"
#include "graphics/materialInstance.h"

namespace ion
{
class MaterialInstanceSerializer final : public AssetSerializer<MaterialInstance>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::MaterialInstance");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const MaterialInstance &material) override;
    grl::Rc<MaterialInstance> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
