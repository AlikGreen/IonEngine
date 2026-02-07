#pragma once
#include <any>
#include <unordered_map>

#include <clogr.h>

namespace ion
{
class RenderContext
{
public:
    template<typename T>
    void set(const std::string& name, T data)
    {
        m_data[name] = data;
    }

    // Gets data with name or fails
    template<typename T>
    T get(const std::string& name)
    {
        if (const auto it = m_data.find(name); it != m_data.end())
        {
            return std::any_cast<T>(it->second);
        }

        clogr::ensure(false, "name was not found in data");
        return nullptr;
    }

    template<typename T>
    T getOr(const std::string& name, T defaultValue)
    {
        if (const auto it = m_data.find(name); it != m_data.end())
        {
            return std::any_cast<T>(it->second);
        }

        return defaultValue;
    }

    bool has(const std::string& name) const
    {
        return m_data.contains(name);
    }
private:
    std::unordered_map<std::string, std::any> m_data;
};
}
