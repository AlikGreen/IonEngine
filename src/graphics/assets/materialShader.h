#pragma once
#include "shader.h"
#include <variant>

#include "asset/assetRef.h"

namespace ion
{
struct MaterialDescription
{
    std::string name{};
    AssetRef<grl::Rc<urhi::Shader>> shader;
    bool depthWrite{};
    bool depthTest{};
    bool blendEnabled{};
    urhi::CullMode cullMode = urhi::CullMode::None;
    urhi::BlendFactor srcColorBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor dstColorBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor srcAlphaBlendFactor = urhi::BlendFactor::One;
    urhi::BlendFactor dstAlphaBlendFactor = urhi::BlendFactor::One;
};

class MaterialShader
{
public:
    MaterialShader() = default;
    explicit MaterialShader(const MaterialDescription& description);

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    bool setProperty(const std::string& name, T const& value)
    {
        std::optional<urhi::ShaderReflection::Member> member = std::nullopt;
        for(const auto& mem : memberInfos)
        {
            if(mem.name == name)
                member = mem;
        }

        if(!member.has_value())
        {
            clogr::error("MaterialShader::setProperty: Member not found: '{}'", name);
            return false;
        }

        if(member->size != sizeof(T))
        {
            clogr::error("MaterialShader::setProperty: member size mismatch on member: '{}'", name);
            return false;
        }

        auto const* src = reinterpret_cast<uint8_t const*>(&value);
        uint8_t* dst = cpuData.data() + member->offset;

        if(memcmp(dst, src, member->size) == 0)
            return true; // no change

        memcpy(dst, src, member->size);
        dirty = true;
        return true;
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    std::optional<T> getProperty(const std::string& name) const
    {
        std::optional<urhi::ShaderReflection::Member> member = std::nullopt;
        for(const auto& mem : memberInfos)
        {
            if(mem.name == name)
                member = mem;
        }

        if(member == std::nullopt)
            return std::nullopt;

        T out;
        memcpy(&out, cpuData.data() + member->offset, sizeof(T));
        return out;
    }

    bool setTexture(std::string name, grl::Rc<urhi::TextureView> texture);
    bool setSampler(std::string name, grl::Rc<urhi::Sampler> sampler);

    std::vector<urhi::ShaderReflection::Resource> getSamplers() const;
    std::vector<urhi::ShaderReflection::Resource> getTextures() const;
    std::vector<urhi::ShaderReflection::Member> getProperties() const;
    urhi::ShaderReflection getReflection() const;
    MaterialDescription getDescription() const;

    grl::Rc<urhi::Pipeline> getPipeline();
    void bindUniforms(const grl::Rc<urhi::CommandList>& commandList);

    static MaterialShader createBillboard();
    static MaterialShader createPBR();
    static MaterialShader createEquirectangularSkybox();
private:
    std::string name;
    MaterialDescription description;

    grl::Rc<urhi::Device> device;
    grl::Rc<urhi::Pipeline> pipeline;
    grl::Rc<urhi::TextureView> defaultTexture;
    grl::Rc<urhi::Sampler> defaultSampler;

    std::unordered_map<std::string, grl::Rc<urhi::TextureView>> textures;
    std::unordered_map<std::string, grl::Rc<urhi::Sampler>> samplers;
    std::vector<urhi::ShaderReflection::Resource> samplerInfos;
    std::vector<urhi::ShaderReflection::Resource> textureInfos;
    std::vector<urhi::ShaderReflection::Member> memberInfos;

    bool dirty = true;
    std::vector<uint8_t> cpuData;
    grl::Rc<urhi::Buffer> propertiesBuffer;
    urhi::ShaderReflection reflection;
};
}
