#include "assetManager.h"

#include <iostream>

#include "core/engine.h"

namespace Neon
{
    std::string AssetManager::getFullPath(const std::string &filePath)
    {
        for (const auto& dir: Engine::getConfig().resourcePaths)
        {
            auto filepath = Path::join(dir, filePath);
            if(Path::exists(filepath))
            {
                return filepath;
            }
        }
        return filePath;
    }

    AssetID AssetManager::generateID()
    {
        {
            return AssetID(nextHandle++);
        }
    }

    bool AssetManager::isValid(const AssetID id) const
    {
        return id.handle() > 0 && id.handle() < nextHandle;
    }

    std::vector<AssetID> AssetManager::getAllAssetIDs()
    {
        return assetHandles;
    }

    AssetMetadata AssetManager::getMetadata(const AssetID handle)
    {
        return assetsMetadata.at(handle);
    }
}
