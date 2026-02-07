#include "sceneManager.h"

namespace ion
{
    SceneManager::SceneManager()
    {
        currentScene = Engine::getAssetManager().addAsset(new Scene());
    }

    void SceneManager::setScene(std::string name)
    {
    }

    Scene& SceneManager::getCurrentScene() const
    {
        return *currentScene;
    }
}
