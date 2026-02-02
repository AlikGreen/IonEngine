#pragma once
#include <cstdint>
#include <functional>

namespace Neon
{
struct AssetID
{
    constexpr AssetID() : id(0) {}
    explicit constexpr AssetID(const uint64_t id) : id(id) {}
    explicit constexpr operator uint64_t() const { return id; }
    auto operator<=>(const AssetID&) const = default;
    static constexpr AssetID invalid() { return AssetID(0); }
    [[nodiscard]] bool isValid() const { return id != 0; }
    [[nodiscard]] uint64_t handle() const { return id; }
private:
    uint64_t id;
};
}

template <>
struct std::hash<Neon::AssetID>
{
    std::size_t operator()(const Neon::AssetID& id) const noexcept
    {
        return std::hash<uint64_t>{}(id.handle());
    }
};
