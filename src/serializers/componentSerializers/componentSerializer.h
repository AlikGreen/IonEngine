#pragma once

#include "asset/assetDeps.h"
#include "asset/assetRegistry.h"

namespace ion
{
class ComponentSerializerBase
{
public:
    virtual ~ComponentSerializerBase() = default;
};

template<typename T>
class ComponentSerializer : public ComponentSerializerBase
{
    virtual void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const T& component) = 0;
    virtual T deserialize(AssetStream& assetStream, AssetRegistry& assetRegistry) = 0;
};
}
