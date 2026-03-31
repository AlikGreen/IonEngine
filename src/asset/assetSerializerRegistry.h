#pragma once
#include <typeindex>

#include "assetSerializer.h"

namespace ion
{
template<typename T>
concept HasTypeId = requires
{
    { T::typeId } -> std::convertible_to<uint64_t>;
};


class AssetSerializerRegistry
{
public:
    template<typename T, typename Serializer, typename... Args>
    requires std::is_constructible_v<Serializer, Args...>
            && std::is_base_of_v<AssetSerializer<T>, Serializer>
            && HasTypeId<Serializer>
    Serializer& registerSerializer(Args&&... args)
    {
        auto wrapper = grl::makeBox<SerializerWrapper<T>>();
        wrapper->inner = grl::makeBox<Serializer>(std::forward<Args>(args)...);
        auto raw = wrapper->inner.get();
        m_serializers[typeid(T)] = std::move(wrapper);
        return *dynamic_cast<Serializer*>(raw);
    }

    template<typename T>
    AssetSerializer<T>* find()
    {
        const auto it = m_serializers.find(typeid(T));
        if(it == m_serializers.end()) return nullptr;
        return dynamic_cast<SerializerWrapper<T>*>(it->second.get())->inner.get();
    }
private:
    struct SerializerBase;

    std::unordered_map<std::type_index, grl::Rc<SerializerBase>> m_serializers;

    struct SerializerBase
    {
        virtual ~SerializerBase() = default;
    };

    template<typename T>
    struct SerializerWrapper final : SerializerBase
    {
        grl::Box<AssetSerializer<T>> inner;
    };
};
}
