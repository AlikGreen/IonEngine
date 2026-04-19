#include "sceneManager.h"

#include "asset/assetRegistry.h"

namespace ion
{
    void SceneManager::addScene(const std::string& name, const AssetRef<Scene>& scene)
    {
        m_scenes.emplace(name, scene);
    }

    void SceneManager::setScene(const std::string &name)
    {
        if(m_currentScene)
            for(const auto& system : m_systems)
            {
                system->sceneUnloaded(*m_currentScene);
            }

        m_currentScene = m_scenes[name];

        for(const auto& system : m_systems)
        {
            system->sceneLoaded(*m_currentScene);
        }
    }

    Scene& SceneManager::getCurrentScene() const
    {
        return *m_currentScene;
    }
}
