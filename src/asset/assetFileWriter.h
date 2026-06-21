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
    void addSection(uint64_t id, std::string_view text);

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    void addSection(uint64_t id, const T& value)
    {
        addSection(id, std::as_bytes(std::span{ &value, 1 }));
    }

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
    std::optional<std::string> readSectionString(uint64_t id);

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    std::optional<T> readSection(const uint64_t id)
    {
        const auto bytes = readSection(id);
        if(!bytes || bytes->size() != sizeof(T))
        {
            return std::nullopt;
        }

        T value;
        std::memcpy(&value, bytes->data(), sizeof(T));
        return value;
    }

    std::vector<SectionEntry> sections();
    std::vector<std::byte> readBody();

    AssetFileWriter toWriter();
private:
    AssetFileReader() = default;
    std::fstream m_file;

    std::string m_name;
    AssetHeader m_header{};
    std::vector<SectionEntry> m_sections;
    std::unordered_map<uint64_t, SectionEntry> m_sectionMap;
};

    namespace SectionIds
    {
        inline constexpr uint64_t Name         = grl::Hash::fnv1a64("ion::Meta::Name");
        inline constexpr uint64_t ParentId     = grl::Hash::fnv1a64("ion::Meta::ParentId");
    }
}
