#pragma once
#include "asset/assetSerializer.h"
#include "asset/assetStream.h"
#include "core/components/transformComponent.h"

namespace ion
{
class TransformSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset) override
    {
        const Transform& transform = *static_cast<Transform*>(asset);

        assetStream.write(transform.position);
        assetStream.write(transform.rotation);
        assetStream.write(transform.scale);
    }

    void* deserialize(AssetStream &assetStream, AssetManager &assetManager) override
    {
        auto* transform = new Transform();

        assetStream.read(transform->position);
        assetStream.read(transform->rotation);
        assetStream.read(transform->scale);

        return transform;
    }
};
}
