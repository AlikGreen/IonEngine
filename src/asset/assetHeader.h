#pragma once

namespace ion
{
struct  AssetHeader
{
    uint8_t   magic[4];
    uint32_t  version;
    uint64_t  assetId;
    uint32_t  flags;

    uint32_t  sectionCount;

    uint64_t  bodyOffset;
    uint64_t  bodySize;
};

struct SectionEntry
{
    uint64_t  id;
    uint64_t  offset;
    uint64_t  size;
};
}
