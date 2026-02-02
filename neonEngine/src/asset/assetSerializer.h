#pragma once

namespace Neon
{
    class AssetStream;
    class AssetManager;
}

namespace Neon
{
class AssetSerializer
{
public:
    virtual ~AssetSerializer() = default;

    virtual void serialize(AssetStream& assetStream, AssetManager& assetManager, void* asset) = 0;
    virtual void* deserialize(AssetStream& assetStream, AssetManager& assetManager) = 0;
};
}
