#include "assetFileWriter.h"

namespace ion
{
    void AssetFileWriter::writeSection(const uint64_t id, std::span<const uint8_t> data)
    {
        SectionData s;
        s.entry.id   = id;
        s.entry.size = data.size();
        s.data.assign(data.begin(), data.end());
        m_sections.push_back(std::move(s));
    }

    std::vector<uint8_t> AssetFileWriter::finalize(const AssetId assetId) const
    {
        AssetStream stream;

        AssetHeader header{};
        header.magic[0] = 'I';
        header.magic[1] = 'O';
        header.magic[2] = 'N';
        header.magic[3] = 'E';

        header.version = 0;
        header.assetId = assetId.handle();
        header.flags = 0;

        header.sectionCount = m_sections.size();

        const uint64_t sectionTableEndOffset = sizeof(AssetHeader) + sizeof(SectionEntry) * m_sections.size();
        uint64_t bodyOffset = sectionTableEndOffset;
        for(const auto& section : m_sections)
        {
            bodyOffset += section.data.size();
        }

        header.bodyOffset = bodyOffset;
        header.bodySize = m_bodyStream.size();

        stream.write(header);

        uint64_t sectionDataOffset = sectionTableEndOffset;

        for(const auto& section : m_sections)
        {
            SectionEntry entry = section.entry;
            entry.offset = sectionDataOffset;
            sectionDataOffset += section.data.size();
            stream.write(entry);
        }

        for(const auto& section : m_sections)
            stream.write(section.data);

        stream.write(m_bodyStream.buffer());

        return stream.buffer();
    }

    AssetFileReader::AssetFileReader(std::span<const uint8_t> fileData)
    {
        AssetStream stream(fileData);
        stream.read(m_header);
        if (!isValid()) return;

        for (uint32_t i = 0; i < m_header.sectionCount; i++)
        {
            SectionEntry entry{};
            stream.read(entry);
            m_sectionTable[entry.id] = entry;
        }

        for (auto& [id, entry] : m_sectionTable)
        {
            std::vector<uint8_t> data(entry.size);
            stream.setCursor(entry.offset);
            stream.read(data);
            m_sectionData[id] = std::move(data);
        }

        m_bodyData.resize(m_header.bodySize);
        stream.setCursor(m_header.bodyOffset);
        stream.read(m_bodyData.data(), m_bodyData.size());
    }

    const AssetHeader& AssetFileReader::header() const
    {
        return m_header;
    }

    bool AssetFileReader::isValid() const
    {
        return m_header.magic[0] == 'I' && m_header.magic[0] == 'O' && m_header.magic[0] == 'N' && m_header.magic[0] == 'E';
    }

    bool AssetFileReader::hasSection(const uint64_t id) const
    {
        return m_sectionTable.contains(id);
    }

    std::optional<std::span<const uint8_t>> AssetFileReader::readSection(const uint64_t id) const
    {
        if(!m_sectionData.contains(id)) return std::nullopt;

        return m_sectionData.at(id);
    }

    std::span<const uint8_t> AssetFileReader::bodyBytes() const
    {
        return m_bodyData;
    }

    AssetStream AssetFileReader::bodyStream() const
    {
        return AssetStream(m_bodyData);
    }
}
