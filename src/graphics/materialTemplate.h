#pragma once
#include <urhi/urhi.h>

#include "shaderSet.h"
#include "asset/assetRef.h"
#include "slang/compiler.h"

namespace ion
{
struct MaterialDescription
{
    std::string name{};
    std::string path{};

    bool opaque = false;
    bool lit = true;
};

class MaterialTemplate
{
public:
    static AssetRef<urhi::slang::Module> s_reflectionShader;

    explicit MaterialTemplate(const MaterialDescription& desc);

    grl::Rc<urhi::Pipeline> getOrCreatePipeline(const urhi::slang::Module& passModule,
                                                urhi::GraphicsPipelineDesc pipelineDesc);

    [[nodiscard]] const grl::Rc<urhi::TextureView>& defaultTexture() const { return m_defaultTexture; }
    [[nodiscard]] const grl::Rc<urhi::Sampler>& defaultSampler() const { return m_defaultSampler; }

    [[nodiscard]] const auto& resources() const { return m_resources; }
    [[nodiscard]] const auto& properties() const { return m_properties; }

    [[nodiscard]] uint32_t propertiesBufferSize() const { return m_propertiesSize; }

    [[nodiscard]] const std::string& name() const { return m_desc.name; }
    [[nodiscard]] bool isOpaque() const { return m_desc.opaque; }

    [[nodiscard]] const MaterialDescription& description() const { return m_desc; }
private:
    [[nodiscard]] static size_t pipelineHash(const urhi::slang::Module& passModule, const urhi::GraphicsPipelineDesc &desc);
    static AssetRef<urhi::slang::Module> s_baseMaterialModule;

    MaterialDescription m_desc;
    AssetRef<urhi::slang::Module> m_module;

    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::TextureView> m_defaultTexture;
    grl::Rc<urhi::Sampler> m_defaultSampler;

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> m_resources;
    std::unordered_map<std::string, urhi::ShaderReflection::Member> m_properties;
    uint32_t m_propertiesSize = 0;

    std::unordered_map<size_t, grl::Rc<urhi::Pipeline>> m_pipelines;
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
