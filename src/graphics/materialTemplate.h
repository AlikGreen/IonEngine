#pragma once

#include "graphicsSystem.h"
#include "asset/assetRef.h"

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
    explicit MaterialTemplate(const MaterialDesc& desc);
    MaterialTemplate(std::string name, bool opaque, bool lit, AssetRef<ShaderModule> module);

    std::pair<dg::Ref<dg::IPipelineState>, dg::Ref<dg::IShaderResourceBinding>> getOrCreatePipeline(const ShaderModule& passModule, const PassDefinition& passDef);

    [[nodiscard]] static const dg::Ref<dg::ITextureView>& defaultTexture() { return s_defaultTexture; }
    [[nodiscard]] static const dg::Ref<dg::ISampler>& defaultSampler() { return s_defaultSampler; }

    [[nodiscard]] const std::unordered_map<std::string, ShaderResource>& resources() const { return m_resources; }
    [[nodiscard]] const std::unordered_map<std::string, ShaderMember>& properties() const { return m_properties; }

    [[nodiscard]] uint32_t propertiesBufferSize() const { return m_propertiesSize; }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] bool isOpaque() const { return m_opaque; }
    [[nodiscard]] bool isLit() const { return m_lit; }
private:
    friend class MaterialTemplateSerializer;

    void reflectModule();
    static void init(const dg::Ref<dg::IRenderDevice>& device);

    bool m_lit = true;
    bool m_opaque = true;
    std::string m_name{};

    AssetRef<ShaderModule> m_module;

    GraphicsSystem* m_graphicsSystem;
    dg::Ref<dg::IRenderDevice> m_device;

    uint32_t m_propertiesSize = 0;

    std::unordered_map<dg::IPipelineState*, dg::Ref<dg::IShaderResourceBinding>> m_srbCache;

    std::unordered_map<std::string, ShaderResource> m_resources;
    std::unordered_map<std::string, ShaderMember> m_properties;

    static AssetRef<ShaderModule> s_reflectionShader;

    static dg::Ref<dg::ITextureView> s_defaultTexture;
    static dg::Ref<dg::ISampler> s_defaultSampler;
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
