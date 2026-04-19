#pragma once
#include <vector>

namespace ion
{
class ScriptAssemblyData
{
public:
    explicit ScriptAssemblyData(const std::vector<std::byte> &data) : m_data(data) { }
    [[nodiscard]] const std::vector<std::byte>& data() const { return m_data; }
private:
    std::vector<std::byte> m_data;
};
}
