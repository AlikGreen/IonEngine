#pragma once
#include <functional>

#include <entis/entis.h>

#include "asset/assetManager.h"
#include "asset/assetSerializer.h"

namespace ion
{
class SceneSerializer final : public AssetSerializer
{
public:
    void serialize(AssetStream &assetStream, AssetManager &assetManager, void *asset) override;
    void* deserialize(AssetStream &assetStream, AssetManager &assetManager) override;

    template<typename T>
    void registerComponentSerializer(const uint32_t typeId)
    {
        m_componentSerializers[typeId] = [](AssetManager &assetManager, AssetStream &assetStream, entis::Entity entity)
        {
            if(!entity.has<T>()) return;

            T& component = entity.get<T>();
            assetManager.serialize<T>(assetStream, component);
        };

        m_componentDeserializers[typeId] = [](AssetManager &assetManager, AssetStream &assetStream, entis::Entity entity, entis::Registry& registry)
        {
            auto component = assetManager.deserialize<T>(assetStream);
            registry.assign<T>(entity, std::move(*component));
        };
    }
private:
    using SerializerFunc = std::function<void(AssetManager&, AssetStream&, entis::Entity)>;
    using DeserializerFunc = std::function<void(AssetManager&, AssetStream&, entis::Entity, entis::Registry&)>;

    std::unordered_map<uint32_t, SerializerFunc> m_componentSerializers{};
    std::unordered_map<uint32_t, DeserializerFunc> m_componentDeserializers{};
};
}
