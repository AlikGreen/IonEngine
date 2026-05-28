#pragma once
#include "scene.h"
#include "system.h"
#include "asset/assetRef.h"

namespace ion
{
class SceneManager
{
public:
    explicit SceneManager(std::vector<System*>& sceneLoadedSystems, std::vector<System*>& sceneUnloadedSystems)
        : m_sceneLoadedSystems(sceneLoadedSystems), m_sceneUnloadedSystems(sceneUnloadedSystems) { };

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void setScene(const AssetRef<Scene>& scene);
    [[nodiscard]] AssetRef<Scene> activeScene() const { return m_currentScene; }
private:
    friend class Engine;
    void loadScene();

    AssetRef<Scene> m_currentScene = nullptr;
    AssetRef<Scene> m_sceneToLoad = nullptr;

    std::vector<System*>& m_sceneLoadedSystems;
    std::vector<System*>& m_sceneUnloadedSystems;
};
}
