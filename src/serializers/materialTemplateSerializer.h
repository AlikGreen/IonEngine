#pragma once
#include "asset/assetSerializer.h"
#include "graphics/materialTemplate.h"

namespace ion
{
class MaterialTemplateSerializer final : public AssetSerializer<MaterialTemplate>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::MaterialTemplate");

    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const MaterialTemplate &material) override;
    grl::Rc<MaterialTemplate> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
