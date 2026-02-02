#include "sceneSerializer.h"

#include "asset/assetManager.h"
#include "asset/assetStream.h"
#include "core/scene.h"

namespace Neon
{
    void SceneSerializer::serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset)
    {
        const auto* scene = static_cast<Scene*>(asset);
        assetStream.write(scene->name);
    }

    void* SceneSerializer::deserialize(AssetStream &assetStream, AssetManager &assetManager)
    {
        const auto prefab = new Scene();
        assetStream.read(prefab->name);

        return prefab;
    }
}
