#pragma once
#include "scene.h"
#include "system.h"
#include "asset/assetRef.h"

namespace ion
{
class SceneManager
{
public:
    explicit SceneManager(std::vector<grl::Box<System>>& systems) : m_systems(systems) { };

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void addScene(const std::string& name, const AssetRef<Scene>& scene);
    void setScene(const std::string &name);
    [[nodiscard]] Scene& getCurrentScene() const;
private:
    AssetRef<Scene> m_currentScene = nullptr;
    std::unordered_map<std::string, AssetRef<Scene>> m_scenes;
    std::vector<grl::Box<System>>& m_systems;
};
}
