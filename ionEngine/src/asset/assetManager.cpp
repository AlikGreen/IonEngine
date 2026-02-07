#include "assetManager.h"

#include <iostream>

#include "core/engine.h"

namespace ion
{
    std::string AssetManager::getFullPath(const std::string &filePath)
    {
        for (const auto& dir: Engine::getConfig().resourcePaths)
        {
            auto filepath = grl::Path::join(dir, filePath);
            if(grl::Path::exists(filepath))
            {
                return filepath;
            }
        }
        return filePath;
    }

    AssetId AssetManager::generateID()
    {
        {
            return AssetId(nextHandle++);
        }
    }

    bool AssetManager::isValid(const AssetId id) const
    {
        return id.handle() > 0 && id.handle() < nextHandle;
    }

    std::vector<AssetId> AssetManager::getAllAssetIDs()
    {
        return assetHandles;
    }

    AssetMetadata AssetManager::getMetadata(const AssetId handle)
    {
        return assetsMetadata.at(handle);
    }
}
