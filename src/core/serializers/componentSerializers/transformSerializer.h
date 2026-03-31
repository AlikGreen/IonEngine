#pragma once
#include "asset/assetStream.h"
#include "core/components/transformComponent.h"

namespace ion
{
class TransformSerializer final : public ComponentSerializer<Transform>
{
public:
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, const Transform &transform) override
    {
        assetStream.write(transform.position);
        assetStream.write(transform.rotation);
        assetStream.write(transform.scale);
    }

    Transform deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        Transform transform;

        assetStream.read(transform.position);
        assetStream.read(transform.rotation);
        assetStream.read(transform.scale);

        return transform;
    }
};
}
