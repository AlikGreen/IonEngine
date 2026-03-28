#pragma once
#include "assetHeader.h"
#include "assetManager.h"
#include "assetStream.h"

namespace ion
{
class AssetFileWriter
{
public:
    AssetStream& bodyStream() { return m_bodyStream; }
    void writeSection(uint64_t id, std::span<const uint8_t> data);
    [[nodiscard]] std::vector<uint8_t> finalize(AssetId assetId) const;
private:
    struct SectionData{ SectionEntry entry; std::vector<uint8_t> data; };
    AssetStream m_bodyStream;
    std::vector<SectionData> m_sections;
};

class AssetFileReader
{
public:
    explicit AssetFileReader(std::span<const uint8_t> fileData);

    [[nodiscard]] const AssetHeader& header() const;
    [[nodiscard]] bool isValid() const;

    [[nodiscard]] bool hasSection(uint64_t id) const;
    [[nodiscard]] std::optional<std::span<const uint8_t>> readSection(uint64_t id) const;

    [[nodiscard]] std::span<const uint8_t> bodyBytes() const;
    [[nodiscard]] AssetStream bodyStream() const;

private:
    AssetHeader m_header;
    std::vector<uint8_t> m_bodyData;
    std::unordered_map<uint64_t, SectionEntry> m_sectionTable;
    std::unordered_map<uint64_t, std::vector<uint8_t>> m_sectionData;
};
}
