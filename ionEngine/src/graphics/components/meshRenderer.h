#pragma once
#include "asset/assetRef.h"
#include "graphics/assets/materialShader.h"
#include "graphics/assets/mesh.h"

namespace ion
{
struct MeshRenderer
{
    std::vector<AssetRef<MaterialShader>> materials{};
    AssetRef<Mesh> mesh{};

    void setMaterial(const AssetRef<MaterialShader>& material)
    {
        if(materials.empty())
            materials.push_back(material);
        else
            materials[0] = material;
    }

    AssetRef<MaterialShader> getMaterial() const
    {
        if(materials.empty()) return nullptr;
        return materials[0];
    }
};
}
