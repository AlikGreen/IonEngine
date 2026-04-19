#pragma once
#include <functional>

#include <entis/entis.h>

#include "asset/assetRegistry.h"
#include "asset/assetSerializer.h"
#include "componentSerializers/componentSerializer.h"
#include "core/scene.h"
#include "grl/error.h"

namespace ion
{
class SceneSerializer final : public AssetSerializer<Scene>
{
public:
    static constexpr uint64_t typeId = grl::hash64("ion::Scene");
    void serialize(AssetStream &assetStream, AssetRegistry& assetRegistry, AssetDeps& deps, const Scene& scene) override;
    grl::Rc<Scene> deserialize(AssetStream &assetStream, AssetRegistry& assetRegistry) override;

    template<size_t TypeId, typename Serializer, typename... Args>
    requires std::is_constructible_v<Serializer, Args...> && std::is_base_of_v<ComponentSerializer<typename Serializer::AssetType>, Serializer>
    void registerComponentSerializer(Args&&... args)
    {
        using T = typename Serializer::AssetType;
        constexpr uint32_t typeId = TypeId;

        m_componentSerializers[typeId] = grl::makeBox<Serializer>(std::forward<Args>(args)...);

        m_componentSerializerFuncs[typeId] = [typeId, this](AssetRegistry& assetRegistry, AssetStream &assetStream, AssetDeps& deps, entis::Entity entity)
        {
            if(!entity.has<T>()) return;

            const T& component = entity.get<T>();
            auto serializer = static_cast<Serializer*>(m_componentSerializers[typeId].get());
            serializer->serialize(assetStream, assetRegistry, deps, component);
        };

        m_componentDeserializerFuncs[typeId] = [this, typeId](AssetRegistry& assetRegistry, AssetStream &assetStream, const entis::Entity entity, entis::Registry& registry)
        {
            auto serializer = static_cast<Serializer*>(m_componentSerializers[typeId].get());
            auto component = serializer->deserialize(assetStream, assetRegistry);

            registry.assign<T>(entity, std::move(component));
        };
    }

private:
    using SerializerFunc = std::function<void(AssetRegistry&, AssetStream&, AssetDeps&, entis::Entity)>;
    using DeserializerFunc = std::function<void(AssetRegistry&, AssetStream&, entis::Entity, entis::Registry&)>;

    std::unordered_map<uint32_t, SerializerFunc> m_componentSerializerFuncs{};
    std::unordered_map<uint32_t, DeserializerFunc> m_componentDeserializerFuncs{};
    std::unordered_map<uint32_t, grl::Box<ComponentSerializerBase>> m_componentSerializers{};
};
}
