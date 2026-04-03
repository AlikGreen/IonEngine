#pragma once
#include "asset/assetSerializer.h"
#include "graphics/materialInstance.h"

namespace ion
{
class MaterialSerializer final : public AssetSerializer<MaterialInstance>
{
public:
    static constexpr uint64_t typeId = grl::hash64("ion::MaterialShader");


    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const MaterialInstance &material) override;
    grl::Rc<MaterialInstance> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
