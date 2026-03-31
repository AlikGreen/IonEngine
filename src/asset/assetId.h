#pragma once
#include <cstdint>
#include <functional>
#include <random>

namespace ion
{
struct AssetId
{
    static AssetId random()
    {
        static std::mt19937_64 engine(std::random_device{}());
        static std::uniform_int_distribution<uint64_t> dist;
        return AssetId(dist(engine));
    }

    constexpr AssetId() : id(0) {}
    explicit constexpr AssetId(const uint64_t id) : id(id) {}
    explicit constexpr operator uint64_t() const { return id; }
    auto operator<=>(const AssetId&) const = default;
    static constexpr AssetId invalid() { return AssetId(0); }
    [[nodiscard]] bool isValid() const { return id != 0; }
    [[nodiscard]] uint64_t handle() const { return id; }
private:
    uint64_t id;
};
}

template <>
struct std::hash<ion::AssetId>
{
    std::size_t operator()(const ion::AssetId& id) const noexcept
    {
        return id.handle();
    }
};
