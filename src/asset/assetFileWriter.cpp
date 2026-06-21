#include "assetFileWriter.h"

namespace ion
{
    AssetFileWriter::AssetFileWriter(const AssetId assetId, const uint64_t typeId)
        : m_assetId(assetId), m_typeId(typeId) {  }

    void AssetFileWriter::addSection(const uint64_t id, std::span<const std::byte> data)
    {
        SectionData s;
        s.entry.id   = id;
        s.entry.size = data.size();
        s.data.assign(data.begin(), data.end());
        m_sections.push_back(std::move(s));
    }

    void AssetFileWriter::addSection(const uint64_t id, const std::string_view text)
    {
        addSection(id, std::as_bytes(std::span{ text.data(), text.size() }));
    }

    void AssetFileWriter::removeSection(const uint64_t id)
    {
        for(auto it = m_sections.begin(); it != m_sections.end(); ++it)
        {
            if(it->entry.id == id)
            {
                m_sections.erase(it);
                break;
            }
        }
    }


    void AssetFileWriter::write(const std::filesystem::path &path) const
    {
        AssetStream stream;

        // Write asset header
        AssetHeader header{};
        header.magic[0] = 'I';
        header.magic[1] = 'O';
        header.magic[2] = 'N';
        header.magic[3] = 'E';

        header.version = 0;
        header.assetId = m_assetId;
        header.typeId = m_typeId;
        header.flags = 0;

        header.bodySize = m_bodyStream.size();
        header.sectionCount = m_sections.size();

        stream.write(header);

        // Write body
        const auto buf = m_bodyStream.buffer();
        stream.write(buf.data(), buf.size());

        uint64_t sectionDataOffset = stream.getCursor() + sizeof(SectionEntry) * header.sectionCount;

        // Write sections header
        for(const auto& section : m_sections)
        {
            SectionEntry entry = section.entry;
            entry.offset = sectionDataOffset;
            sectionDataOffset += section.data.size();
            stream.write(entry);
        }

        // Write sections
        for(const auto& section : m_sections)
            stream.write(section.data.data(), section.data.size());

        std::fstream file (path, std::ios::out  | std::ios::binary);
        file.write(reinterpret_cast<const char*>(stream.buffer().data()), static_cast<int64_t>(stream.buffer().size()));
        file.close();
    }

    AssetFileReader AssetFileReader::open(const std::filesystem::path &path)
    {
        AssetFileReader reader{};
        reader.m_file.open(path, std::ios::in | std::ios::binary);
        reader.m_file.read(reinterpret_cast<char*>(&reader.m_header), sizeof(AssetHeader));
        reader.m_name = path.stem().string();
        return reader;
    }

    const AssetHeader& AssetFileReader::header() const
    {
        return m_header;
    }

    bool AssetFileReader::isValid() const
    {
        return m_header.magic[0] == 'I' &&
                m_header.magic[1] == 'O' &&
                m_header.magic[2] == 'N' &&
                m_header.magic[3] == 'E' &&
                m_header.version == 0 &&
                m_header.assetId.isValid();
    }

    std::optional<std::vector<std::byte>> AssetFileReader::readSection(const uint64_t id)
    {
        sections();
        if(!m_sectionMap.contains(id)) return std::nullopt;

        const SectionEntry entry = m_sectionMap.at(id);
        m_file.seekg(static_cast<int64_t>(entry.offset), std::ios::beg);

        std::vector<std::byte> result;
        result.resize(entry.size);

        m_file.read(reinterpret_cast<char*>(result.data()), static_cast<int64_t>(entry.size));
        return result;
    }

    std::optional<std::string> AssetFileReader::readSectionString(const uint64_t id)
    {
        const auto bytes = readSection(id);
        if(!bytes)
            return std::nullopt;

        return std::string(
            reinterpret_cast<const char*>(bytes->data()),
            bytes->size()
        );
    }

    std::vector<SectionEntry> AssetFileReader::sections()
    {
        if(!m_sections.empty() || m_header.sectionCount == 0) return m_sections;

        m_file.seekg(sizeof(AssetHeader) + m_header.bodySize, std::ios::beg);

        for(size_t i = 0; i < m_header.sectionCount; i++)
        {
            SectionEntry entry{};
            m_file.read(reinterpret_cast<char*>(&entry), sizeof(SectionEntry));
            m_sections.push_back(entry);
            m_sectionMap.emplace(entry.id, entry);
        }

        return m_sections;
    }


    std::vector<std::byte> AssetFileReader::readBody()
    {
        std::vector<std::byte> bodyData;
        bodyData.resize(m_header.bodySize);

        m_file.seekg(sizeof(AssetHeader), std::ios::beg);
        m_file.read(reinterpret_cast<char*>(bodyData.data()), static_cast<int64_t>(m_header.bodySize));

        return bodyData;
    }

    AssetFileWriter AssetFileReader::toWriter()
    {
        AssetFileWriter writer(m_header.assetId, m_header.typeId);
        for(const auto section : m_sections)
        {
            writer.addSection(section.id, readSection(section.id).value_or(std::vector<std::byte>{}));
        }
        writer.bodyStream() = AssetStream{readBody()};
        return writer;
    }
}
