#pragma once
#include "AssetId.h"
#include "assetLoader.h"
#include "assetRef.h"
#include "assetSerializerRegistry.h"
#include "assetStream.h"

namespace ion
{
class AssetRegistry
{
public:
    AssetRegistry() = default;
    AssetRegistry(const AssetRegistry&) = delete;
    AssetRegistry& operator=(const AssetRegistry&) = delete;

    template<typename T, typename... Args>
    requires std::is_constructible_v<T, Args...> && std::is_base_of_v<AssetLoader, T>
    void registerLoader(Args&&... args)
    {
        m_loaders.push_back(grl::makeBox<T>(std::forward<Args>(args)...));
    }

    template<typename T, typename Serializer, typename... Args>
    requires std::is_constructible_v<Serializer, Args...>
            && std::is_base_of_v<AssetSerializer<T>, Serializer>
            && HasTypeId<Serializer>
    Serializer& registerSerializer(Args&&... args)
    {
        m_registeredTypeIds[typeid(T)] = Serializer::typeId;
        return m_serializers.registerSerializer<T, Serializer, Args...>(std::forward<Args>(args)...);
    }

    template<typename T>
    AssetRef<T> add(const grl::Rc<T>& asset, AssetId id = AssetId::invalid())
    {
        if (!id.isValid())
            id = AssetId::random();

        auto* serializer = m_serializers.find<T>();

        AssetEntry entry;
        entry.asset = asset;
        entry.runtimeType = typeid(T);

        const auto it = m_registeredTypeIds.find(typeid(T));
        if(it != m_registeredTypeIds.end())
            entry.stableType = it->second;

        m_assets.emplace(id, std::move(entry));
        return AssetRef<T>(asset, id);
    }

    template<typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
    AssetRef<T> emplace(const AssetId id, Args&&... args)
    {
        return add<T>(grl::makeRc<T>(std::forward<Args>(args)...), id);
    }

    template<typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
    AssetRef<T> create(Args&&... args)
    {
        return add<T>(grl::makeRc<T>(std::forward<Args>(args)...), AssetId::random());
    }

    template<typename T>
    requires std::is_constructible_v<std::decay_t<T>, T&&>
    AssetRef<std::decay_t<T>> adopt(T&& asset, const AssetId id = AssetId::invalid())
    {
        return add<std::decay_t<T>>(
            grl::makeRc<std::decay_t<T>>(std::forward<T>(asset)),
            id
        );
    }
    template<typename T>
    AssetRef<T> load(AssetId id)
    {
        const auto it = m_assets.find(id);
        if(it != m_assets.end())
        {
            if(const auto shared = it->second.asset.lock())
                return AssetRef<T>(std::static_pointer_cast<T>(shared), id);
        }

        for (const auto& loader : m_loaders)
        {
            if (!loader->canLoad(id)) continue;
            auto bytes = loader->load(id);
            if (bytes.empty()) continue;
            auto serializer = m_serializers.find<T>();
            auto stream = AssetStream(bytes);

            auto data = serializer->deserialize(stream, *this);

            AssetEntry entry;
            entry.asset = data;
            entry.stableType = m_registeredTypeIds.at(typeid(T));
            entry.runtimeType = typeid(T);

            m_assets.emplace(id, std::move(entry));

            return AssetRef<T>(data, id);
        }
        return nullptr;
    }

    template<typename T>
    void serialize(AssetStream& stream, const T& asset)
    {
        auto* serializer = m_serializers.find<T>();
        if (serializer) serializer->serialize(stream, *this, asset);
    }

    template<typename T>
    grl::Rc<T> deserialize(AssetStream& stream)
    {
        auto* serializer = m_serializers.find<T>();
        if (!serializer) return nullptr;
        return serializer->deserialize(stream, *this);
    }

    AssetSerializerRegistry& serializers()
    {
        return m_serializers;
    }

    template<typename T>
    [[nodiscard]] bool isType(const uint64_t typeId) const
    {
        const auto it = m_registeredTypeIds.find(typeid(T));
        if(it != m_registeredTypeIds.end())
            return typeId == it->second;

        return false;
    }

    template<typename T>
    [[nodiscard]] uint64_t stableId() const
    {
        const auto it = m_registeredTypeIds.find(typeid(T));
        if(it != m_registeredTypeIds.end())
            return it->second;

        return 0;
    }
private:
    struct AssetEntry
    {
        grl::Weak<void> asset;
        std::type_index runtimeType;
        uint64_t stableType;

        AssetEntry() : runtimeType(typeid(void)) {}
    };
    std::unordered_map<AssetId, AssetEntry> m_assets;
    std::unordered_map<std::type_index, uint64_t> m_registeredTypeIds;

    std::vector<grl::Box<AssetLoader>> m_loaders;
    AssetSerializerRegistry m_serializers;
};
}
