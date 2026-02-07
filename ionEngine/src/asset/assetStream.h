#pragma once
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include "assetManager.h"
#include "assetRef.h"

namespace ion
{
class AssetStream
{
public:
    AssetStream() = default;
    explicit AssetStream(std::vector<uint8_t> bytes)
        : buffer(std::move(bytes))
        {  }

    bool write(const void* data, const size_t size)
    {
        if(m_cursor >= 128000)
            return false;

        auto* byteData = static_cast<const uint8_t*>(data);
        const size_t writeEnd = m_cursor + size;

        if (writeEnd > buffer.size())
        {
            buffer.reserve(static_cast<float>(buffer.size()) * 1.5f);
            buffer.resize(writeEnd);
        }

        std::copy(byteData, byteData + size, buffer.begin() + m_cursor);
        m_cursor += size;
        return true;
    }

    template<typename T>
    requires (std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>)
    bool write(const T& val)
    {
        return write(&val, sizeof(T));
    }

    template<typename T>
    bool write(std::vector<T> vector)
    {
        if(!write<uint32_t>(vector.size()))
            return false;

        if(!write(vector.data(), vector.size() * sizeof(T)))
            return false;

        return true;
    }

    template<typename T>
    bool write(AssetRef<T> assetRef)
    {
        write<uint64_t>(assetRef.id().handle());
        return true;
    }

    bool write(const std::string &string)
    {
        if(!write<uint32_t>(string.size()))
            return false;

        if(!write(string.data(), string.size()))
            return false;

        return true;
    }

    bool read(void* data, const size_t size)
    {
        if(m_cursor + size > buffer.size())
            return false;

        memcpy(data, buffer.data() + m_cursor, size);
        m_cursor += size;
        return true;
    }

    template<typename T>
    requires (std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>)
    bool read(T& out)
    {
        return read(&out, sizeof(T));
    }

    template<typename T>
    bool read(AssetRef<T>& assetRef)
    {
        uint64_t id = 0;
        read<uint64_t>(id);

        assetRef = Engine::getAssetManager().getAsset<T>(AssetId(id));
        return true;
    }

    template<typename T>
    bool read(std::vector<T>& vector)
    {
        uint32_t size = 0;
        if(!read<uint32_t>(size))
            return false;

        void* data = nullptr;
        if(!read(&data, sizeof(T)*size))
            return false;

        vector.resize(size);
        vector.assign(data, data + size);

        return true;
    }

    bool read(std::string& string)
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

    template<typename T>
    void skip()
    {
        m_cursor += sizeof(T);
    }

    void skip(const uint32_t bytes)
    {
        m_cursor += bytes;
    }

    [[nodiscard]] size_t getCursorPos() const
    {
        return m_cursor;
    }

    void setCursorPos(const size_t pos)
    {
        m_cursor = pos;
    }

    [[nodiscard]] std::vector<uint8_t> getBuffer() const
    {
        return buffer;
    }
private:
    size_t m_cursor = 0;
    std::vector<uint8_t> buffer{};
};
}
