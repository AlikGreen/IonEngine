#pragma once
#include "assetId.h"

namespace ion
{
enum Compression : uint8_t
{
    None, Zstd
};

struct AssetHeader
{
    uint8_t   magic[4]{};
    uint32_t  version{};
    AssetId  assetId{};
    uint64_t typeId{};
    Compression compression{};
    uint32_t  flags{};

    uint32_t  sectionCount{};

    uint64_t  bodyOffset{};
    uint64_t  bodySize{};
};

struct SectionEntry
{
    uint64_t  id;
    uint64_t  offset;
    uint64_t  size;
};
}
