#include "assetManager.h"

#include <iostream>
#include <random>

#include "core/engine.h"

namespace ion
{
    AssetId AssetManager::generateID()
    {
        static std::mt19937_64 engine(std::random_device{}());
        static std::uniform_int_distribution<uint64_t> dist;
        return AssetId(dist(engine));
    }

    bool AssetManager::isValid(const AssetId id) const
    {
        return id.handle() > 0;
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
