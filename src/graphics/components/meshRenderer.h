#pragma once
#include "asset/assetRef.h"
#include "graphics/materialInstance.h"
#include "graphics/mesh.h"

namespace ion
{
struct MeshRenderer
{
    std::vector<AssetRef<MaterialInstance>> materials{};
    AssetRef<Mesh> mesh{};

    void setMaterial(const AssetRef<MaterialInstance>& material)
    {
        if(materials.empty())
            materials.push_back(material);
        else
            materials[0] = material;
    }

    [[nodiscard]] AssetRef<MaterialInstance> getMaterial() const
    {
        if(materials.empty()) return nullptr;
        return materials[0];
    }
};
}
