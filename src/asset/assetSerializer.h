#pragma once

namespace ion
{
    class AssetStream;
    class AssetRegistry;
    class AssetDeps;
}

namespace ion
{
template<typename T>
class AssetSerializer
{
public:
    using AssetType = T;

    virtual ~AssetSerializer() = default;

    virtual void serialize(AssetStream& assetStream, AssetRegistry& assetRegistry, AssetDeps& deps, const T& asset) = 0;
    virtual grl::Rc<T> deserialize(AssetStream& assetStream, AssetRegistry& assetRegistry) = 0;
};
}
