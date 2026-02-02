#pragma once
#include "shader.h"
#include <variant>

#include "asset/assetRef.h"

namespace Neon
{
struct MaterialDescription
{
    std::string name{};
    AssetRef<Rc<RHI::Shader>> shader;
    bool depthWrite{};
    bool depthTest{};
    bool blendEnabled{};
    RHI::CullMode cullMode = RHI::CullMode::None;
    RHI::BlendFactor srcColorBlendFactor = RHI::BlendFactor::One;
    RHI::BlendFactor dstColorBlendFactor = RHI::BlendFactor::One;
    RHI::BlendFactor srcAlphaBlendFactor = RHI::BlendFactor::One;
    RHI::BlendFactor dstAlphaBlendFactor = RHI::BlendFactor::One;
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
        std::optional<RHI::ShaderReflection::Member> member = std::nullopt;
        for(const auto& mem : memberInfos)
        {
            if(mem.name == name)
                member = mem;
        }

        if(!member.has_value())
        {
            Log::error("MaterialShader::setProperty: Member not found: '{}'", name);
            return false;
        }

        if(member->size != sizeof(T))
        {
            Log::error("MaterialShader::setProperty: member size mismatch on member: '{}'", name);
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
        std::optional<RHI::ShaderReflection::Member> member = std::nullopt;
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

    bool setTexture(std::string name, Rc<RHI::TextureView> texture);
    bool setSampler(std::string name, Rc<RHI::Sampler> sampler);

    std::vector<RHI::ShaderReflection::Resource> getSamplers() const;
    std::vector<RHI::ShaderReflection::Resource> getTextures() const;
    std::vector<RHI::ShaderReflection::Member> getProperties() const;
    RHI::ShaderReflection getReflection();

    Rc<RHI::Pipeline> getPipeline();
    void bindUniforms(const Rc<RHI::CommandList>& commandList);

    static MaterialShader createBillboard();
    static MaterialShader createPBR();
    static MaterialShader createEquirectangularSkybox();
private:
    std::string name;

    Rc<RHI::Device> device;
    Rc<RHI::Pipeline> pipeline;
    Rc<RHI::TextureView> defaultTexture;
    Rc<RHI::Sampler> defaultSampler;

    std::unordered_map<std::string, Rc<RHI::TextureView>> textures;
    std::unordered_map<std::string, Rc<RHI::Sampler>> samplers;
    std::vector<RHI::ShaderReflection::Resource> samplerInfos;
    std::vector<RHI::ShaderReflection::Resource> textureInfos;
    std::vector<RHI::ShaderReflection::Member> memberInfos;

    bool dirty = true;
    std::vector<uint8_t> cpuData;
    Rc<RHI::Buffer> propertiesBuffer;
    RHI::ShaderReflection reflection;
};
}
