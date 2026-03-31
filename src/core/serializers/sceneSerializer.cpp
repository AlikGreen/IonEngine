#include "sceneSerializer.h"

#include "asset/assetStream.h"
#include "core/scene.h"
#include "core/components/parentComponent.h"
#include "core/components/tagComponent.h"

namespace ion
{
    void SceneSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, const Scene& scene)
    {
        assetStream.write(scene.name);

        auto& registry = scene.registry();
        auto& view = registry.view<Tag>();

        const uint32_t entityCountCursor = assetStream.getCursor();
        assetStream.write<uint32_t>(0);

        uint32_t entityCount = 0;

        for(const auto& [entity, tag] : view)
        {
            const uint32_t startOfEntity = assetStream.getCursor();
            assetStream.write<uint32_t>(0); // entity size placeholder
            assetStream.write<uint32_t>(entity.id());

            // Write component count
            const uint32_t componentCountCursor = assetStream.getCursor();
            assetStream.write<uint32_t>(0); // component count placeholder

            uint32_t componentCount = 0;

            // Serialize each component type
            for(auto& [typeId, serializer] : m_componentSerializerFuncs)
            {
                const uint32_t componentStartCursor = assetStream.getCursor();
                assetStream.write<uint32_t>(0);
                assetStream.write<uint32_t>(typeId);

                serializer(assetRegistry, assetStream, entity);
                const uint32_t afterComponentData = assetStream.getCursor();

                const uint32_t componentSize = afterComponentData - componentStartCursor;
                assetStream.writeAt(componentStartCursor, componentSize);
                componentCount++;
            }

            const uint32_t endOfEntity = assetStream.getCursor();
            assetStream.writeAt(componentCountCursor, componentCount);

            const uint32_t totalWritten = endOfEntity - startOfEntity;
            assetStream.writeAt(startOfEntity, totalWritten);

            entityCount++;
        }

        assetStream.writeAt(entityCountCursor, entityCount);
    }

    grl::Rc<Scene> SceneSerializer::deserialize(AssetStream &assetStream, AssetRegistry& assetRegistry)
    {
        auto scene = grl::makeRc<Scene>();
        assetStream.read(scene->name);

        auto& registry = scene->registry();

        uint32_t entityCount = 0;
        assetStream.read(entityCount);

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            uint32_t entitySize = 0;
            assetStream.read(entitySize);

            uint32_t entityId = 0;
            assetStream.read(entityId);

            const auto entity = registry.createEntityWithId(entityId);

            uint32_t componentCount = 0;
            assetStream.read(componentCount);

            // Deserialize each component
            for(uint32_t j = 0; j < componentCount; ++j)
            {
                uint32_t componentSize = 0;
                const uint32_t componentStartCursor = assetStream.getCursor();
                assetStream.read(componentSize);

                uint32_t typeId = 0;
                assetStream.read(typeId);

                // Find and execute the deserializer
                auto it = m_componentDeserializerFuncs.find(typeId);
                if(it != m_componentDeserializerFuncs.end())
                {
                    it->second(assetRegistry, assetStream, entity, registry);
                }
                else
                {
                    // Skip unknown component type
                    assetStream.setCursor(componentStartCursor + componentSize);
                }
            }
        }

        auto& parentView = registry.view<Parent>();

        for(auto [entity, parent] : parentView)
        {
            entis::Entity parentEntity = parent.getParent();
            parentEntity = entis::Entity(&registry ,parentEntity.id());
            parent.setParent(parentEntity);
        }

        return scene;
    }
}
