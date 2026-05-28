#pragma once

#include "asset/assetStream.h"
#include "core/components/parentComponent.h"

namespace ion
{
class ParentSerializer final : public ComponentSerializer<Parent>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::Parent");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Parent &parent) override
    {
        assetStream.write<uint32_t>(parent.getParent().id());
    }

    Parent deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        entis::EntityId entityId = 0;
        assetStream.read<uint32_t>(entityId);
        const auto entity = entis::Entity(nullptr, entityId);

        Parent parent;
        parent.setParent(entity);
        return parent;
    }
};
}
