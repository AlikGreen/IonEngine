#pragma once

#include "asset/assetStream.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
class MeshRendererSerializer final : public ComponentSerializer<MeshRenderer>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::MeshRenderer");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const MeshRenderer& renderer) override
    {
        deps.require(renderer.mesh);
        for (const auto& mat : renderer.materials)
            deps.require(mat);
        
        assetStream.write<uint32_t>(renderer.materials.size());
        for(const auto& material : renderer.materials)
        {
            assetStream.write(material.id());
        }

        assetStream.write(renderer.mesh.id());
    }

    MeshRenderer deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        MeshRenderer renderer;
        uint32_t materialCount = 0;
        assetStream.read(materialCount);
        renderer.materials.resize(materialCount);

        for(size_t i = 0; i < materialCount; i++)
        {
            AssetId id;
            assetStream.read(id);
            renderer.materials[i] = assetRegistry.load<MaterialInstance>(id);
        }

        AssetId id;
        assetStream.read(id);
        renderer.mesh = assetRegistry.load<Mesh>(id);

        return renderer;
    }
};
}
