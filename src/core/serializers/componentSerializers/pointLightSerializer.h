#pragma once
#include "asset/assetSerializer.h"
#include "asset/assetStream.h"
#include "core/components/transformComponent.h"
#include "graphics/components/pointLight.h"

namespace ion
{
class PointLightSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset) override
    {
        const PointLight& light = *static_cast<PointLight*>(asset);
        assetStream.write(light.power);
        assetStream.write(light.color);
    }

    void* deserialize(AssetStream &assetStream, AssetManager &assetManager) override
    {
        auto* light = new PointLight();

        assetStream.read(light->power);
        assetStream.read(light->color);

        return light;
    }
};
}
