#pragma once
#include <unordered_set>
#include "asset/assetRef.h"

namespace ion
{
struct MaterialDescription
{
    std::string name{};
    AssetRef<std::vector<urhi::ShaderEntryPoint>> shaders{};
    bool depthWrite{};
    bool depthTest{};
    bool blendEnabled{};

    urhi::CullMode cullMode = urhi::CullMode::None;
    urhi::BlendFactor srcColorBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor dstColorBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor srcAlphaBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor dstAlphaBlendFactor = urhi::BlendFactor::One;

    urhi::PixelFormat colorAttachmentFormat = urhi::PixelFormat::RGBA8UNorm;
    urhi::PixelFormat depthAttachmentFormat = urhi::PixelFormat::Depth32Float;
};

class MaterialShader
{
public:
    MaterialShader() = default;
    explicit MaterialShader(const MaterialDescription& desc);

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    bool setProperty(const std::string& name, T const& value)
    {
        const auto it = properties.find(name);

        if(it == properties.end())
        {
            clogr::error("MaterialShader::setProperty: Member not found: '{}'", name);
            return false;
        }

        const auto member = it->second;

        if(member.size != sizeof(T))
        {
            clogr::error("MaterialShader::setProperty: member size mismatch on member: '{}'", name);
            return false;
        }

        auto const* src = reinterpret_cast<uint8_t const*>(&value);
        uint8_t* dst = cpuData.data() + member.offset;

        boundResources.emplace(name);

        if(memcmp(dst, src, member.size) == 0)
            return true;

        memcpy(dst, src, member.size);
        dirty = true;
        return true;
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    std::optional<T> getProperty(const std::string& name) const
    {
        const auto it = properties.find(name);

        if(it == properties.end())
            return std::nullopt;

        const auto member = it->second;

        T out;
        memcpy(&out, cpuData.data() + member.offset, sizeof(T));
        return out;
    }

    bool setTexture(std::string name, grl::Rc<urhi::TextureView> texture);
    bool setSampler(std::string name, grl::Rc<urhi::Sampler> sampler);

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> getResources() const;
    std::unordered_map<std::string, urhi::ShaderReflection::Member>  getProperties() const;

    grl::Rc<urhi::Pipeline> getPipeline();
    void bindUniforms(const grl::Rc<urhi::CommandList>& cmd, const grl::Rc<urhi::RenderPass> &pass);

    bool isOpaque() const;

    static MaterialShader createBillboard();
    static MaterialShader createPBR();
    static MaterialShader createEquirectangularSkybox();
private:
    std::string name;
    bool opaque{};

    grl::Rc<urhi::Device> device;
    grl::Rc<urhi::Pipeline> pipeline;
    grl::Rc<urhi::TextureView> defaultTexture;
    grl::Rc<urhi::Sampler> defaultSampler;

    std::unordered_set<std::string> boundResources;
    std::unordered_map<std::string, grl::Rc<urhi::TextureView>> textures;
    std::unordered_map<std::string, grl::Rc<urhi::Sampler>> samplers;

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> resources;
    std::unordered_map<std::string, urhi::ShaderReflection::Member> properties;

    bool dirty = true;
    std::vector<uint8_t> cpuData;
    grl::Rc<urhi::Buffer> propertiesBuffer;
};
}
