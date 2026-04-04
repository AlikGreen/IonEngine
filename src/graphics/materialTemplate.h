#pragma once
#include <urhi/urhi.h>

#include "shaderSet.h"
#include "asset/assetRef.h"

namespace ion
{
struct MaterialDescription
{
    std::string name{};
    AssetRef<urhi::ShaderSet> shaders{};
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

class MaterialTemplate
{
public:
    explicit MaterialTemplate(const MaterialDescription& desc);

    const grl::Rc<urhi::Pipeline>& pipeline() const { return m_pipeline; }
    const grl::Rc<urhi::TextureView>& defaultTexture() const { return m_defaultTexture; }
    const grl::Rc<urhi::Sampler>& defaultSampler() const { return m_defaultSampler; }

    const auto& resources() const { return m_resources; }
    const auto& properties() const { return m_properties; }

    uint32_t propertiesBufferSize() const { return m_propertiesSize; }

    const std::string& name() const { return m_desc.name; }
    bool isOpaque() const { return !m_desc.blendEnabled; }

    const MaterialDescription& description() const { return m_desc; }
private:
    MaterialDescription m_desc;

    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::Pipeline> m_pipeline;
    grl::Rc<urhi::TextureView> m_defaultTexture;
    grl::Rc<urhi::Sampler> m_defaultSampler;

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> m_resources;
    std::unordered_map<std::string, urhi::ShaderReflection::Member> m_properties;
    uint32_t m_propertiesSize = 0;
};

class MaterialTemplates
{
public:
    static AssetRef<MaterialTemplate> pbr();
    static AssetRef<MaterialTemplate> billboard();
    static AssetRef<MaterialTemplate> equirectangularSkybox();
private:
    static AssetRef<MaterialTemplate> s_pbr;
    static AssetRef<MaterialTemplate> s_billboard;
    static AssetRef<MaterialTemplate> s_equirectangularSkybox;
};
}
