#pragma once
#include "asset/assetSerializer.h"
#include "asset/assetStream.h"
#include "core/components/parentComponent.h"

namespace ion
{
class ParentSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset) override
    {
        const Parent& parent = *static_cast<Parent*>(asset);

        assetStream.write<uint32_t>(parent.getParent().id());
    }

    void * deserialize(AssetStream &assetStream, AssetManager &assetManager) override
    {
        entis::EntityId entityId = 0;
        assetStream.read<uint32_t>(entityId);
        const auto entity = entis::Entity(nullptr, entityId);

        auto* parent = new Parent();
        parent->setParent(entity);
        return parent;
    }
};
}
