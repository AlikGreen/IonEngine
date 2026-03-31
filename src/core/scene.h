#pragma once
#include <entis/entis.h>


namespace ion
{
class Scene
{
public:
    entis::Registry& registry();
    [[nodiscard]] const entis::Registry& registry() const;
    entis::Entity createEntity(const std::string& name = "Entity");
    entis::Entity import(Scene& scene);
    std::string name;
private:
    friend class SceneSerializer;
    entis::Registry m_registry = entis::Registry();
};
}
