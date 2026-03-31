#pragma once

namespace ion
{
    class AssetStream;
    class AssetRegistry;
}

namespace ion
{
template<typename T>
class AssetSerializer
{
public:
    virtual ~AssetSerializer() = default;

    virtual void serialize(AssetStream& assetStream, AssetRegistry& assetRegistry, const T& asset) = 0;
    virtual grl::Rc<T> deserialize(AssetStream& assetStream, AssetRegistry& assetRegistry) = 0;
};
}
