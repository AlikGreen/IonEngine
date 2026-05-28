#pragma once
#include <urhi/urhi.h>

#include "shaderSet.h"
#include "asset/assetRef.h"
#include "slang/compiler.h"

namespace ion
{
struct MaterialDesc
{
    std::string name{};
    std::string path{};

    bool opaque = true;
    bool lit = true;
};

class MaterialTemplate
{
public:
    static AssetRef<urhi::slang::Module> s_reflectionShader;

    explicit MaterialTemplate(const MaterialDesc& desc);
    MaterialTemplate(std::string name, bool opaque, bool lit, AssetRef<urhi::slang::Module> module);

    grl::Rc<urhi::Pipeline> getOrCreatePipeline(const urhi::slang::Module& passModule,
                                                urhi::GraphicsPipelineDesc pipelineDesc);

    [[nodiscard]] const grl::Rc<urhi::TextureView>& defaultTexture() const { return s_defaultTexture; }
    [[nodiscard]] const grl::Rc<urhi::Sampler>& defaultSampler() const { return s_defaultSampler; }

    [[nodiscard]] const auto& resources() const { return m_resources; }
    [[nodiscard]] const auto& properties() const { return m_properties; }

    [[nodiscard]] uint32_t propertiesBufferSize() const { return m_propertiesSize; }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] bool isOpaque() const { return m_opaque; }
    [[nodiscard]] bool isLit() const { return m_lit; }
private:
    friend class MaterialTemplateSerializer;

    void compileModule();
    static void init(const grl::Rc<urhi::Device> &device);

    [[nodiscard]] static size_t pipelineHash(const urhi::slang::Module& passModule, const urhi::GraphicsPipelineDesc &desc);

    bool m_lit = true;
    bool m_opaque = true;
    std::string m_name{};

    AssetRef<urhi::slang::Module> m_module;

    grl::Rc<urhi::Device> m_device;

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> m_resources;
    std::unordered_map<std::string, urhi::ShaderReflection::Member> m_properties;
    uint32_t m_propertiesSize = 0;

    std::unordered_map<size_t, grl::Rc<urhi::Pipeline>> m_pipelines;

    static AssetRef<urhi::slang::Module> s_baseMaterialModule;
    static grl::Rc<urhi::TextureView> s_defaultTexture;
    static grl::Rc<urhi::Sampler> s_defaultSampler;
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
