#pragma once

#include "asset/assetStream.h"
#include "core/components/tagComponent.h"

namespace ion
{
class TagSerializer final : public ComponentSerializer<Tag>
{
public:
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Tag &tag) override
    {
        assetStream.write(tag.name);
    }

    Tag deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override
    {
        Tag tag;
        assetStream.read(tag.name);
        return tag;
    }
};
}
