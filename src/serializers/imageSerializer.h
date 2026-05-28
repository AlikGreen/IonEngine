#pragma once
#include "asset/assetSerializer.h"
#include "graphics/image.h"

namespace ion
{
class ImageSerializer final : public AssetSerializer<Image>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::Image");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const Image &image) override;
    grl::Rc<Image> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
