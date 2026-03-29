#include "assetStream.h"

namespace ion
{
    AssetStream::AssetStream(std::span<const uint8_t> bytes): m_buffer(bytes.begin(), bytes.end())
    {  }

    AssetStream::AssetStream(const std::span<const std::byte> bytes)
    : m_buffer(reinterpret_cast<const uint8_t*>(bytes.data()),
               reinterpret_cast<const uint8_t*>(bytes.data()) + bytes.size())
    {
    }

    bool AssetStream::write(const void *data, const size_t size)
    {
        if(m_cursor >= 128000)
            return false;

        auto* byteData = static_cast<const uint8_t*>(data);
        const size_t writeEnd = m_cursor + size;

        if (writeEnd > m_buffer.size())
        {
            m_buffer.reserve(static_cast<size_t>(static_cast<double>(m_buffer.size()) * 1.5));
            m_buffer.resize(writeEnd);
        }

        std::copy(byteData, byteData + size, m_buffer.begin() + m_cursor);
        m_cursor += size;
        return true;
    }

    bool AssetStream::write(const std::string &string)
    {
        if(!write<uint32_t>(string.size()))
            return false;

        if(!write(string.data(), string.size()))
            return false;

        return true;
    }


    void AssetStream::writeZeroes(uint64_t count)
    {
        m_buffer.insert(m_buffer.end(), count, 0);
        m_cursor += count;
    }

    bool AssetStream::read(void *data, const size_t size)
    {
        if(m_cursor + size > m_buffer.size())
            return false;

        memcpy(data, m_buffer.data() + m_cursor, size);
        m_cursor += size;
        return true;
    }

    bool AssetStream::read(std::string &string)
    {
        uint32_t size = 0;
        if(!read<uint32_t>(size))
            return false;

        string.resize(size);

        if(size > 0)
        {
            if(!read(string.data(), size))
                return false;
        }

        return true;
    }

    void AssetStream::skip(const uint32_t bytes)
    {
        m_cursor += bytes;
    }

    size_t AssetStream::getCursor() const
    {
        return m_cursor;
    }

    void AssetStream::setCursor(const size_t pos)
    {
        m_cursor = pos;
    }

    std::vector<uint8_t> AssetStream::buffer() const
    {
        return m_buffer;
    }

    size_t AssetStream::size() const
    {
        return m_buffer.size();
    }
}
