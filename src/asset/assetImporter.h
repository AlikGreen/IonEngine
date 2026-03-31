#pragma once

#include <filesystem>

namespace ion
{
template<typename T>
class AssetImporter
{
public:
    virtual ~AssetImporter() = default;

    virtual grl::Box<T> import(const std::filesystem::path& src) = 0;
    [[nodiscard]] virtual bool canImport(const std::filesystem::path& src) const = 0;
};
}
