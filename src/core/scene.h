#pragma once
#include <entis/entis.h>


namespace ion
{
class Scene
{
public:
    [[nodiscard]] entis::Registry& getRegistry();
    entis::Entity createEntity(const std::string& name = "Entity");
    entis::Entity import(Scene& scene);
    std::string name;
private:
    friend class SceneSerializer;
    entis::Registry registry = entis::Registry();
};
}
