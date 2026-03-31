#pragma once
#include "assetHeader.h"
#include "assetStream.h"

namespace ion
{
class AssetFileWriter
{
public:
    explicit AssetFileWriter(AssetId assetId, uint64_t typeId);

    void addSection(uint64_t id, std::span<const std::byte> data);
    void removeSection(uint64_t id);
    AssetStream& bodyStream() { return m_bodyStream; }

    void write(const std::filesystem::path& path) const;
private:
    struct SectionData{ SectionEntry entry; std::vector<std::byte> data; };
    AssetStream m_bodyStream;
    std::vector<SectionData> m_sections;
    AssetId m_assetId;
    uint64_t m_typeId;
};

class AssetFileReader
{
public:
    static AssetFileReader open(const std::filesystem::path& path);

    [[nodiscard]] const AssetHeader& header() const;
    [[nodiscard]] bool isValid() const;
    std::optional<std::vector<std::byte>> readSection(uint64_t id);
    std::vector<SectionEntry> sections();
    std::vector<std::byte> readBody();

    AssetFileWriter toWriter();
private:
    AssetFileReader() = default;
    std::fstream m_file;

    AssetHeader m_header{};
    std::vector<SectionEntry> m_sections;
    std::unordered_map<uint64_t, SectionEntry> m_sectionMap;
};
}
