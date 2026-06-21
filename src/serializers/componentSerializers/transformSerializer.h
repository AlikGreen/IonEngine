#pragma once
#include "asset/assetStream.h"
#include "core/components/transformComponent.h"

namespace ion
{
class TransformSerializer final : public ComponentSerializer<Transform>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::Transform");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Transform &transform) override
    {
        assetStream.write(transform.position());
        assetStream.write(transform.rotation());
        assetStream.write(transform.scale());
    }

    Transform deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        Transform transform;

        assetStream.read(transform.m_position);
        assetStream.read(transform.m_rotation);
        assetStream.read(transform.m_scale);

        return transform;
    }
};
}
