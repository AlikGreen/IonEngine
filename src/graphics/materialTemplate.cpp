#include "materialTemplate.h"

#include <utility>

#include "graphicsSystem.h"
#include "asset/assetImportPipeline.h"
#include "asset/assetRegistry.h"
#include "core/engine.h"
#include "importers/shaderImporter.h"

namespace ion
{
    AssetRef<ShaderModule> MaterialTemplate::s_reflectionShader{};
    dg::Ref<dg::ITextureView> MaterialTemplate::s_defaultTexture{};
    dg::Ref<dg::ISampler> MaterialTemplate::s_defaultSampler{};

    MaterialTemplate::MaterialTemplate(const MaterialDesc &desc)
        : m_lit(desc.lit), m_opaque(desc.opaque), m_name(desc.name)
    {
        m_graphicsSystem = Engine::getSystem<GraphicsSystem>();
        m_device = m_graphicsSystem->device();
        init(m_device);

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_module = importPipeline.import<ShaderModule>(desc.path);

        reflectModule();
    }

    MaterialTemplate::MaterialTemplate(std::string name, const bool opaque, const bool lit, AssetRef<ShaderModule> module)
        : m_lit(lit), m_opaque(opaque), m_name(std::move(name)), m_module(std::move(module))
    {
        m_graphicsSystem = Engine::getSystem<GraphicsSystem>();
        m_device = m_graphicsSystem->device();
        init(m_device);

        reflectModule();
    }

    std::pair<dg::Ref<dg::IPipelineState>, dg::Ref<dg::IShaderResourceBinding>> MaterialTemplate::getOrCreatePipeline(const ShaderModule& passModule, const PassDefinition& passDef)
    {
        ShaderProcessDesc processDesc{};
        processDesc.inlineIncludes.emplace_back("material", *m_module);
        processDesc.defaultVariableType = dg::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        auto shaderBundle = m_graphicsSystem->shaderRegistry().getOrCreate(passModule, processDesc);
        auto pso = m_graphicsSystem->pipelineRegistry().getOrCreateGraphics(shaderBundle, passDef);

        if(auto it = m_srbCache.find(pso.RawPtr()); it != m_srbCache.end())
        {
            return { pso, it->second };
        }

        dg::Ref<dg::IShaderResourceBinding> srb;
        pso->CreateShaderResourceBinding(&srb);

        auto [it, _] = m_srbCache.emplace(pso.RawPtr(), srb);
        return { pso, it->second };
    }

    void MaterialTemplate::reflectModule()
    {
        ShaderProcessDesc parseDesc{};
        parseDesc.inlineIncludes.emplace_back("material", *m_module);
        parseDesc.defaultVariableType = dg::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        const auto shaderBundle = m_graphicsSystem->shaderRegistry().getOrCreate(*s_reflectionShader, parseDesc);

        for (const auto& res : shaderBundle.resources)
        {
            m_resources[res.name] = res;

            if (res.type == dg::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER
                && res.name == "material")
            {
                for (const auto& mem : res.members)
                {
                    m_properties[mem.name] = mem;
                    m_propertiesSize = std::max(m_propertiesSize, mem.offset + mem.size);
                }
            }
        }
    }

    void MaterialTemplate::init(const dg::Ref<dg::IRenderDevice> &device)
    {
        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();

        if(!s_reflectionShader)
            s_reflectionShader = importPipeline.import<ShaderModule>("shaders/materialReflection.hlsl");

        if(!s_defaultTexture)
            s_defaultTexture = Engine::getSystem<GraphicsSystem>()->defaultTexture();

        if(!s_defaultSampler)
        {
            dg::SamplerDesc samplerDesc{};
            samplerDesc.AddressU = dg::TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = dg::TEXTURE_ADDRESS_WRAP;
            device->CreateSampler(samplerDesc, &s_defaultSampler);
        }
    }

    AssetRef<MaterialTemplate> MaterialTemplates::s_pbr{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_billboard{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_equirectangularSkybox{};

    AssetRef<MaterialTemplate> MaterialTemplates::pbr()
    {
        if(s_pbr) return s_pbr;

        MaterialDesc desc{};
        desc.name = "Pbr";
        desc.path = "shaders/pbrMaterial.hlsli";
        desc.opaque = true;

        s_pbr = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_pbr;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::billboard()
    {
        if(s_billboard) return s_billboard;

        MaterialDesc desc{};
        desc.name = "Billboard";
        desc.path = "shaders/billboardMaterial.hlsli";
        desc.opaque = false;

        s_billboard = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_billboard;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::equirectangularSkybox()
    {
        if(s_equirectangularSkybox) return s_equirectangularSkybox;

        MaterialDesc desc{};
        desc.name = "EquirectangularSkybox";
        desc.path = "shaders/equirectangularSkyboxMaterial.hlsli";
        desc.opaque = true;

        s_equirectangularSkybox = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_equirectangularSkybox;
    }
}
