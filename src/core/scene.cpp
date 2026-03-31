#include "scene.h"

#include "components/parentComponent.h"
#include "components/tagComponent.h"
#include "components/transformComponent.h"

namespace ion
{

    entis::Registry& Scene::registry()
    {
        return m_registry;
    }

    const entis::Registry& Scene::registry() const
    {
        return m_registry;
    }

    entis::Entity Scene::createEntity(const std::string& name)
    {
        entis::Entity entity = m_registry.createEntity();

        entity.emplace<Transform>();
        entity.emplace<Tag>(name);
        entity.emplace<Parent>();

        return entity;
    }

    entis::Entity Scene::import(Scene& scene)
    {
        const std::vector<entis::Entity> newEntities = m_registry.merge(scene.registry());

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
