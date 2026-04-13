#include "materialTemplate.h"

#include "graphicsSystem.h"
#include "asset/assetImportPipeline.h"
#include "asset/assetRegistry.h"
#include "importers/shaderImporter.h"

namespace ion
{
    AssetRef<urhi::slang::Module> MaterialTemplate::s_baseMaterialModule{};
    AssetRef<urhi::slang::Module> MaterialTemplate::s_reflectionShader{};

    MaterialTemplate::MaterialTemplate(const MaterialDescription &desc)
        : m_desc(desc)
    {
        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();

        if(!s_baseMaterialModule)
            s_baseMaterialModule = importPipeline.import<urhi::slang::Module>("shaders/material.slang");

        if(!s_reflectionShader)
            s_reflectionShader = importPipeline.import<urhi::slang::Module>("shaders/materialReflection.slang");

        m_module = importPipeline.import<urhi::slang::Module>(desc.path);

        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();


        urhi::slang::LinkDesc linkDesc;
        linkDesc.modules     = { *m_module, *s_reflectionShader, *s_baseMaterialModule };
        linkDesc.typeSpecializations = { m_desc.name };

        urhi::slang::Diagnostics diags;
        const auto shaderSet = urhi::slang::Compiler::linkToShaderSet(linkDesc, &diags);

        for (const auto& res : shaderSet.stages()[0].reflection.resources)
        {
            m_resources[res.name] = res;

            if (res.type == urhi::ShaderReflection::ResourceType::ConstantBuffer
                && res.name == "material")
            {
                for (const auto& mem : res.members)
                {
                    m_properties[mem.name] = mem;
                    m_propertiesSize = std::max(m_propertiesSize, mem.offset + mem.size);
                }
            }
        }

        m_defaultTexture = Engine::getSystem<GraphicsSystem>()->getDefaultTexture();

        urhi::SamplerDesc samplerDesc{};
        samplerDesc.addressModeU = urhi::AddressMode::Repeat;
        samplerDesc.addressModeV = urhi::AddressMode::Repeat;
        m_defaultSampler = m_device->createSampler(samplerDesc);
    }

    grl::Rc<urhi::Pipeline> MaterialTemplate::getOrCreatePipeline(const urhi::slang::Module& passModule, urhi::GraphicsPipelineDesc pipelineDesc)
    {
        const size_t key = pipelineHash(passModule, pipelineDesc);

        if (const auto it = m_pipelines.find(key); it != m_pipelines.end())
            return it->second;

        // TODO Check disk shader cache

        urhi::slang::LinkDesc linkDesc;
        linkDesc.modules     = { *m_module, passModule, *s_baseMaterialModule };
        linkDesc.typeSpecializations = { m_desc.name };

        urhi::slang::Diagnostics diags;
        const auto shaderSet = urhi::slang::Compiler::linkToShaderSet(linkDesc, &diags);

        std::vector<grl::Rc<urhi::Shader>> shaders;
        for(const auto& ep : shaderSet.stages())
        {
            auto shader = m_device->createShader(ep);
            shaders.push_back(shader);
        }

        pipelineDesc.shaders = shaders;
        auto pipeline  = m_device->createPipeline(pipelineDesc);

        m_pipelines[key] = pipeline;
        return pipeline;
    }

    static void hashCombine(size_t& seed, const size_t hash)
    {
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    static void hashBytes(size_t& seed, const void* data, const size_t size)
    {
        auto* p = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; i++)
            hashCombine(seed, std::hash<uint8_t>{}(p[i]));
    }


    size_t MaterialTemplate::pipelineHash(
        const urhi::slang::Module& passModule,
        const urhi::GraphicsPipelineDesc& desc)
    {
        size_t seed = 0;

        hashBytes(seed, passModule.ir.data(),  passModule.ir.size());

        hashBytes(seed, &desc.primitiveType,   sizeof(desc.primitiveType));
        hashBytes(seed, &desc.rasterizerState, sizeof(desc.rasterizerState));
        hashBytes(seed, &desc.depthState,      sizeof(desc.depthState));

        for (const auto& att : desc.colorAttachments)
            hashBytes(seed, &att, sizeof(att));

        const bool hasDepth = desc.depthAttachmentFormat.has_value();
        hashBytes(seed, &hasDepth, sizeof(hasDepth));
        if (hasDepth)
            hashBytes(seed, &*desc.depthAttachmentFormat, sizeof(urhi::PixelFormat));

        return seed;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::s_pbr{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_billboard{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_equirectangularSkybox{};

    AssetRef<MaterialTemplate> MaterialTemplates::pbr()
    {
        if(s_pbr) return s_pbr;

        MaterialDescription desc{};
        desc.name = "PbrMaterial";
        desc.path = "shaders/pbrMaterial.slang";
        desc.opaque = true;

        s_pbr = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_pbr;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::billboard()
    {
        if(s_billboard) return s_billboard;

        MaterialDescription desc{};
        desc.name = "BillboardMaterial";
        desc.path = "shaders/billboardMaterial.slang";
        desc.opaque = false;

        s_billboard = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_billboard;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::equirectangularSkybox()
    {
        if(s_equirectangularSkybox) return s_equirectangularSkybox;

        MaterialDescription desc{};
        desc.name = "EquirectangularSkyboxMaterial";
        desc.path = "shaders/equirectangularSkyboxMaterial.slang";
        desc.opaque = true;

        s_equirectangularSkybox = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_equirectangularSkybox;
    }
}
