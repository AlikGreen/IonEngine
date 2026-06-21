#pragma once
#include <unordered_set>

#include "image.h"
#include "materialTemplate.h"

namespace ion
{
class MaterialInstance
{
public:
    MaterialInstance() = default;
    explicit MaterialInstance(const AssetRef<MaterialTemplate> &templ);

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    bool set(const std::string& name, const T& value)
    {
        const auto& props = m_template->properties();
        const auto it = props.find(name);
        if (it == props.end())
            return false;

        const auto& member = it->second;
        if (member.size != sizeof(T))
            return false;

        auto* dst = m_cpuBuffer.data() + member.offset;
        if (std::memcmp(dst, &value, sizeof(T)) == 0)
            return true;

        std::memcpy(dst, &value, sizeof(T));
        m_dirty = true;
        return true;
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    std::optional<T> get(const std::string& name) const
    {
        const auto& props = m_template->properties();
        const auto it = props.find(name);
        if (it == props.end())
            return std::nullopt;

        T out;
        std::memcpy(&out, m_cpuBuffer.data() + it->second.offset, sizeof(T));
        return out;
    }

    bool setTexture(const std::string& name, const AssetRef<Image>& image);
    bool setSampler(const std::string& name, const AssetRef<Image>& image);

    void applyBindings(const grl::Rc<urhi::CommandList>& cmd, urhi::RenderPass& pass);

    [[nodiscard]] const AssetRef<MaterialTemplate>& materialTemplate() const { return m_template; }

    [[nodiscard]] const std::unordered_map<std::string, AssetRef<Image>>& textures() const { return m_textures; }
    [[nodiscard]] const std::unordered_map<std::string, AssetRef<Image>>& samplers() const { return m_samplers; }
    [[nodiscard]] const std::vector<uint8_t>& cpuBuffer() const { return m_cpuBuffer; }

    void setPropertyBuffer(const std::vector<uint8_t>& data);

    [[nodiscard]] const std::string& name() const { return m_template->name(); }
    [[nodiscard]] bool isOpaque() const { return m_template->isOpaque(); }

    [[nodiscard]] const auto& resources() const { return m_template->resources(); }
    [[nodiscard]] const auto& properties() const { return m_template->properties(); }

    bool dirty() const { return m_dirty; }
    void dirty(const bool dirty) { m_dirty = dirty; }
private:
    AssetRef<MaterialTemplate> m_template;

    std::vector<uint8_t> m_cpuBuffer;
    std::unordered_map<std::string, AssetRef<Image>> m_textures;
    std::unordered_map<std::string, AssetRef<Image>> m_samplers;

    grl::Rc<urhi::Buffer> m_propertyBuffer;
    bool m_dirty = true;
};
}
