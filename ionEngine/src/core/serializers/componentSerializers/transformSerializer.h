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
        const glm::vec3& pos = transform.getPosition();
        const glm::vec3& rot = transform.getRotation();
        const glm::vec3& scl = transform.getScale();

        assetStream.write(pos);
        assetStream.write(rot);
        assetStream.write(scl);
    }

    void* deserialize(AssetStream &assetStream, AssetManager &assetManager) override
    {
        glm::vec3 pos;
        glm::vec3 rot;
        glm::vec3 scl;

        assetStream.read(pos);
        assetStream.read(rot);
        assetStream.read(scl);

        auto* transform = new Transform();
        transform->setPosition(pos);
        transform->setRotation(rot);
        transform->setScale(scl);

        return transform;
    }
};
}
