#pragma once
#include <stack>
#include <unordered_set>
#include <utility>

#include "assetRegistry.h"

namespace ion
{
class AssetDependency
{
public:
    void serialize(AssetStream& stream, AssetRegistry& registry) const { m_serializeFunc(stream, registry); }
    [[nodiscard]] AssetId id() const { return m_assetId; }
    [[nodiscard]] uint64_t stableTypeId() const { return m_stableTypeId; }
private:
    friend class AssetDeps;
    AssetDependency(const AssetId assetId, const uint64_t stableTypeId, std::function<void(AssetStream&, AssetRegistry&)> serializeFunc)
        : m_assetId(assetId), m_stableTypeId(stableTypeId), m_serializeFunc(std::move(serializeFunc)) {}

    AssetId m_assetId;
    uint64_t m_stableTypeId = 0;
    std::function<void(AssetStream&, AssetRegistry&)> m_serializeFunc;
};

class AssetDeps
{
public:
    explicit AssetDeps(AssetRegistry& registry) : m_registry(registry) {}

    template<typename T>
    void require(const AssetRef<T>& ref)
    {
        if (!ref || m_seen.contains(ref.id())) return;
        m_seen.insert(ref.id());

        m_pending.push(
        {
            ref.id(),
            m_registry.stableId<T>(),
            [ref, this](AssetStream& stream, AssetRegistry& reg)
            {
                reg.serialize(stream, *this, *ref);
            }
        });
    }

    std::stack<AssetDependency>& pending() { return m_pending; }
private:
    AssetRegistry& m_registry;
    std::unordered_set<AssetId> m_seen;
    std::stack<AssetDependency> m_pending;
};
}
