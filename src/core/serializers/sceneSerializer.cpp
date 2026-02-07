#include "sceneSerializer.h"

#include "asset/assetManager.h"
#include "asset/assetStream.h"
#include "core/scene.h"
#include "core/components/parentComponent.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"

namespace ion
{
    void SceneSerializer::serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset)
    {
        auto* scene = static_cast<Scene*>(asset);
        assetStream.write(scene->name);

        auto& registry = scene->getRegistry();
        auto& view = registry.view<Tag>();

        const uint32_t entityCountCursor = assetStream.getCursorPos();
        assetStream.write<uint32_t>(0); // entity count placeholder

        uint32_t entityCount = 0;

        for(auto [entity, tag] : view)
        {
            const uint32_t startOfEntity = assetStream.getCursorPos();
            assetStream.write<uint32_t>(0); // entity size placeholder
            assetStream.write<uint32_t>(entity.id());

            // Write component count
            const uint32_t componentCountCursor = assetStream.getCursorPos();
            assetStream.write<uint32_t>(0); // component count placeholder

            uint32_t componentCount = 0;

            // Serialize each component type
            for(auto& [typeId, serializer] : m_componentSerializers)
            {
                const uint32_t componentStartCursor = assetStream.getCursorPos();
                assetStream.write<uint32_t>(0); // component size placeholder
                assetStream.write<uint32_t>(typeId); // type identifier

                const uint32_t beforeComponentData = assetStream.getCursorPos();
                serializer(assetManager, assetStream, entity);
                const uint32_t afterComponentData = assetStream.getCursorPos();

                // Only count if data was written
                if(afterComponentData > beforeComponentData)
                {
                    const uint32_t componentSize = afterComponentData - componentStartCursor;
                    assetStream.setCursorPos(componentStartCursor);
                    assetStream.write<uint32_t>(componentSize);
                    assetStream.setCursorPos(afterComponentData);
                    componentCount++;
                }
                else
                {
                    // No data written, rewind
                    assetStream.setCursorPos(componentStartCursor);
                }
            }

            // Update component count
            const uint32_t endOfEntity = assetStream.getCursorPos();
            assetStream.setCursorPos(componentCountCursor);
            assetStream.write<uint32_t>(componentCount);
            assetStream.setCursorPos(endOfEntity);

            // Update entity size
            const uint32_t totalWritten = endOfEntity - startOfEntity;
            assetStream.setCursorPos(startOfEntity);
            assetStream.write<uint32_t>(totalWritten);
            assetStream.setCursorPos(endOfEntity);

            entityCount++;
        }

        // Write the final entity count
        const uint32_t finalCursor = assetStream.getCursorPos();
        assetStream.setCursorPos(entityCountCursor);
        assetStream.write<uint32_t>(entityCount);
        assetStream.setCursorPos(finalCursor);
    }

    void* SceneSerializer::deserialize(AssetStream &assetStream, AssetManager &assetManager)
    {
        auto* scene = new Scene();
        assetStream.read(scene->name);

        auto& registry = scene->getRegistry();

        uint32_t entityCount = 0;
        assetStream.read(entityCount);

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            uint32_t entitySize = 0;
            const uint32_t entityStartCursor = assetStream.getCursorPos();
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
                const uint32_t componentStartCursor = assetStream.getCursorPos();
                assetStream.read(componentSize);

                uint32_t typeId = 0;
                assetStream.read(typeId);

                // Find and execute the deserializer
                auto it = m_componentDeserializers.find(typeId);
                if(it != m_componentDeserializers.end())
                {
                    it->second(assetManager, assetStream, entity, registry);
                }
                else
                {
                    // Skip unknown component type
                    assetStream.setCursorPos(componentStartCursor + componentSize);
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
