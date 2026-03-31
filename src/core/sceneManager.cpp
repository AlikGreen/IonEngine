#include "sceneManager.h"

#include "asset/assetRegistry.h"

namespace ion
{
    SceneManager::SceneManager()
    {
        currentScene = Engine::assetRegistry().create<Scene>();
    }

    void SceneManager::setScene(std::string name)
    {
    }

    Scene& SceneManager::getCurrentScene() const
    {
        return *currentScene;
    }
}
