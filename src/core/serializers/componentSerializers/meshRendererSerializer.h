#pragma once

#include "asset/assetStream.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
class MeshRendererSerializer final : public ComponentSerializer<MeshRenderer>
{
public:
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, const MeshRenderer& renderer) override
    {
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
        renderer.materialsepen.resize(materialCount);

        for(size_t i = 0; i < materialCount; i++)
        {
            AssetId id;
            assetStream.read(id);
            renderer.materials[i] = assetRegistry.load<MaterialShader>(id);
        }

        AssetId id;
        assetStream.read(id);
        renderer.mesh = assetRegistry.load<Mesh>(id);

        return renderer;
    }
};
}
