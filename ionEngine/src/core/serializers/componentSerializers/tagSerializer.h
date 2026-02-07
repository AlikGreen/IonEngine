#pragma once
#include "asset/assetSerializer.h"
#include "asset/assetStream.h"
#include "core/components/tagComponent.h"

namespace ion
{
class TagSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream& assetStream, AssetManager& assetManager, void* component) override
    {
        const auto* tag = static_cast<Tag*>(component);
        assetStream.write(tag->name);
    }

    void* deserialize(AssetStream& assetStream, AssetManager& assetManager) override
    {
        auto* tag = new Tag();
        assetStream.read(tag->name);
        return tag;
    }
};
}
