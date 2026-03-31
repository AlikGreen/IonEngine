#pragma once
#include <grl/mem.h>

#include "AssetId.h"

namespace ion
{
class AssetLoader
{
public:
    virtual ~AssetLoader() = default;
    virtual std::vector<std::byte> load(AssetId id) = 0;
    [[nodiscard]] virtual bool canLoad(AssetId id) const = 0;
};
}
