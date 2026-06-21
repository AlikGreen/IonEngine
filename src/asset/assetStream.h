#pragma once
#include <cstdint>
#include <vector>

#include "assetId.h"

namespace ion
{
class AssetStream
{
public:
    AssetStream() = default;
    explicit AssetStream(std::span<const uint8_t> bytes);
    explicit AssetStream(std::span<const std::byte> bytes);

    bool write(const void* data, size_t size);

    template<typename T>
    requires (std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>)
    bool write(const T& val)
    {
        return write(&val, sizeof(T));
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    bool write(std::vector<T> vector)
    {
        if(!write<uint32_t>(vector.size()))
            return false;

        if(!write(vector.data(), vector.size() * sizeof(T)))
            return false;

        return true;
    }

    bool write(const AssetId assetId)
    {
        write<uint64_t>(assetId.handle());
        return true;
    }

    bool write(const std::string &string);

    void writeAt(const uint64_t offset, auto value)
    {
        const uint64_t saved = getCursor();
        setCursor(offset);
        write(value);
        setCursor(saved);
    }

    void writeZeroes(uint64_t count);

    bool read(void* data, size_t size);

    template<typename T>
    requires (std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>)
    bool read(T& out)
    {
        return read(&out, sizeof(T));
    }

    bool read(AssetId& assetId)
    {
        uint64_t id = 0;
        read<uint64_t>(id);
        assetId = AssetId(id);

        return true;
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    bool read(std::vector<T>& vector)
    {
        uint32_t size = 0;
        if(!read<uint32_t>(size))
            return false;

        vector.resize(size);

        if(!read(vector.data(), sizeof(T) * size))
            return false;

        return true;
    }

    bool read(std::string& string);

    template<typename T>
    void skip()
    {
        m_cursor += sizeof(T);
    }

    void skip(uint32_t bytes);

    [[nodiscard]] size_t getCursor() const;
    void setCursor(size_t pos);

    [[nodiscard]] std::vector<uint8_t> buffer() const;
    [[nodiscard]] size_t size() const;
private:
    uint64_t m_cursor = 0;
    std::vector<uint8_t> m_buffer{};
};
}
