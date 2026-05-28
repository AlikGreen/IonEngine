#include "sceneSerializer.h"

#include "asset/assetStream.h"
#include "core/scene.h"
#include "core/components/parentComponent.h"
#include "core/components/tagComponent.h"

namespace ion
{
    void SceneSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Scene& scene)
    {
        assetStream.write(scene.name);

        auto& registry = scene.registry();
        auto& view = registry.view<Tag>();

        const uint32_t entityCountCursor = assetStream.getCursor();
        assetStream.write<uint32_t>(0);

        uint32_t entityCount = 0;

        for(const auto& [entity, tag] : view)
        {
            assetStream.write<uint32_t>(entity.id());

            // Write component count
            const uint32_t componentCountCursor = assetStream.getCursor();
            assetStream.write<uint32_t>(0); // component count placeholder

            uint32_t componentCount = 0;

            // Serialize each component type
            for(auto& [typeId, serializer] : m_componentSerializerFuncs)
            {
                const uint32_t beforeWrite = assetStream.getCursor();
                assetStream.write<uint32_t>(0);      // size placeholder
                assetStream.write<uint64_t>(typeId);

                const uint32_t beforeData = assetStream.getCursor();
                serializer(assetRegistry, assetStream, deps, entity);
                const uint32_t afterData = assetStream.getCursor();

                if (afterData == beforeData)
                {
                    assetStream.setCursor(beforeWrite);
                    continue;
                }

                assetStream.writeAt(beforeWrite, afterData - beforeWrite);
                componentCount++;
            }

            assetStream.writeAt(componentCountCursor, componentCount);

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
            uint32_t entityId = 0;
            assetStream.read(entityId);

            const auto entity = registry.createEntityWithId(entityId);

            uint32_t componentCount = 0;
            assetStream.read(componentCount);

            // Deserialize each component
            for(uint32_t j = 0; j < componentCount; ++j)
            {
                const uint32_t componentStartCursor = assetStream.getCursor();

                uint32_t componentSize = 0;
                assetStream.read(componentSize);

                uint64_t typeId = 0;
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
