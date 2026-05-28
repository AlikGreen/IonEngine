#pragma once

#include "asset/assetStream.h"
#include "graphics/components/pointLight.h"

namespace ion
{
class PointLightSerializer final : public ComponentSerializer<PointLight>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::PointLight");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const PointLight &light) override
    {
        assetStream.write(light.power);
        assetStream.write(light.color);
    }

    PointLight deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        PointLight light;

        assetStream.read(light.power);
        assetStream.read(light.color);

        return light;
    }
};
}
