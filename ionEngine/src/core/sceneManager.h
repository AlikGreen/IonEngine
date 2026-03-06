#pragma once
#include "scene.h"
#include "asset/assetRef.h"

namespace ion
{
class SceneManager
{
public:
    SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void setScene(std::string name);
    Scene& getCurrentScene() const;
private:
    AssetRef<Scene> currentScene;
    std::unordered_map<std::string, AssetRef<Scene>> scenes;
};
}
