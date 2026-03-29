#pragma once
#include "asset/assetSerializer.h"
#include "asset/assetStream.h"
#include "core/components/transformComponent.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
class MeshRendererSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset) override
    {
        const MeshRenderer& renderer = *static_cast<MeshRenderer*>(asset);

        assetStream.write<uint32_t>(renderer.materials.size());
        for(const auto material : renderer.materials)
        {
            assetStream.write(material);
        }

        assetStream.write(renderer.mesh);
    }

    void* deserialize(AssetStream &assetStream, AssetManager &assetManager) override
    {
        auto* renderer = new MeshRenderer();
        std::vector<AssetRef<MaterialShader>> materials;
        uint32_t materialCount = 0;
        assetStream.read(materialCount);
        materials.resize(materialCount);

        for(size_t i = 0; i < materialCount; i++)
        {
            assetStream.read(materials[i]);
        }

        assetStream.read(renderer->mesh);

        return renderer;
    }
};
}
