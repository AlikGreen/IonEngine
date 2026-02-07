#include "scene.h"

#include "components/parentComponent.h"
#include "components/tagComponent.h"
#include "components/transformComponent.h"

namespace ion
{

    entis::Registry& Scene::getRegistry()
    {
        return registry;
    }

    entis::Entity Scene::createEntity(const std::string& name)
    {
        entis::Entity entity = registry.createEntity();

        entity.emplace<Transform>();
        entity.emplace<Tag>(name);
        entity.emplace<Parent>();

        return entity;
    }

    entis::Entity Scene::import(Scene& scene)
    {
        const std::vector<entis::Entity> newEntities = registry.merge(scene.getRegistry());

        const entis::Entity parent = createEntity(scene.name);

        for(auto entity : newEntities)
        {
            if(!entity.has<Parent>())
                continue;

            auto& p = entity.get<Parent>();

            if(!p.hasParent())
            {
                p.setParent(parent);
            }
        }

        return parent;
    }
}
