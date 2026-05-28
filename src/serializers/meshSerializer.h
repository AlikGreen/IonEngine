#pragma once
#include "asset/assetSerializer.h"
#include "graphics/mesh.h"

namespace ion
{
class MeshSerializer final : public AssetSerializer<Mesh>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::Mesh");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Mesh &mesh) override;
    grl::Rc<Mesh> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
