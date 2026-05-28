#include "sceneManager.h"

#include "asset/assetRegistry.h"

namespace ion
{
    void SceneManager::setScene(const AssetRef<Scene> &scene)
    {
        m_sceneToLoad = scene;
    }

    void SceneManager::loadScene()
    {
        if(m_sceneToLoad == nullptr) return;
        
        if(m_currentScene)
        {
            for(const auto& system : m_sceneUnloadedSystems)
            {
                system->sceneUnloaded(*m_currentScene);
            }
        }

        m_currentScene = m_sceneToLoad;
        m_sceneToLoad = nullptr;

        for(const auto& system : m_sceneLoadedSystems)
        {
            system->sceneLoaded(*m_currentScene);
        }
    }
}
