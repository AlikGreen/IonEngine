#include "assetManager.h"

#include <iostream>

#include "core/engine.h"

namespace ion
{
    AssetId AssetManager::generateID()
    {
        return AssetId(nextHandle++);
    }

    bool AssetManager::isValid(const AssetId id) const
    {
        return id.handle() > 0 && id.handle() < nextHandle;
    }

    AssetManager::AssetManager(ResourceFS& resourceFS)
        : resourceFS(resourceFS) {  }

    std::vector<AssetId> AssetManager::getAllAssetIDs()
    {
        return assetHandles;
    }

    AssetMetadata AssetManager::getMetadata(const AssetId handle)
    {
        return assetsMetadata.at(handle);
    }
}
