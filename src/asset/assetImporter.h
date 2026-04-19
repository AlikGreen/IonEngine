#pragma once

#include <filesystem>

namespace ion
{
struct NoOptions {};


template<typename T, typename Opt = NoOptions>
class AssetImporter
{
public:
    using AssetType = T;
    using OptionsType = Opt;

    virtual ~AssetImporter() = default;

    virtual grl::Box<T> import(const std::filesystem::path& src, const Opt& options) = 0;
    [[nodiscard]] virtual bool canImport(const std::filesystem::path& src) const = 0;
};
}
