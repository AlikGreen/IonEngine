#include "materialSerializer.h"

#include "asset/assetStream.h"

namespace ion
{
    void MaterialSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const MaterialInstance& material)
    {
    }


    grl::Rc<MaterialInstance> MaterialSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        return nullptr;
    }
}
