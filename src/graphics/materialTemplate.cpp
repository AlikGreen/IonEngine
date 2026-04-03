#include "materialTemplate.h"

#include "graphicsSystem.h"
#include "asset/assetImportPipeline.h"
#include "asset/assetRegistry.h"

namespace ion
{
    MaterialTemplate::MaterialTemplate(const MaterialDescription &desc)
        : m_desc(desc)
    {
        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();

        std::vector<grl::Rc<urhi::Shader>> shaderObjects;
        for (const auto& ep : *desc.shaders)
        {
            shaderObjects.push_back(m_device->createShader(ep));
            for (const auto& res : ep.reflection.resources)
            {
                m_resources[res.name] = res;
                if (res.type == urhi::ShaderReflection::ResourceType::ConstantBuffer
                    && res.name == "properties")
                {
                    for (const auto& mem : res.members)
                    {
                        m_properties[mem.name] = mem;
                        m_propertiesSize = std::max(m_propertiesSize, mem.offset + mem.size);
                    }
                }
            }
        }

        urhi::DepthState depthState{};
        depthState.hasDepthTarget   = true;
        depthState.enableDepthTest  = desc.depthTest;
        depthState.enableDepthWrite = desc.depthWrite;

        urhi::RasterizerState rasterState{};
        rasterState.cullMode = desc.cullMode;

        urhi::BlendState blendState{};
        blendState.enableBlend    = desc.blendEnabled;
        blendState.srcColorFactor = desc.srcColorBlendFactor;
        blendState.dstColorFactor = desc.dstColorBlendFactor;
        blendState.srcAlphaFactor = desc.srcAlphaBlendFactor;
        blendState.dstAlphaFactor = desc.dstAlphaBlendFactor;

        urhi::ColorAttachmentDesc colorAttach{};
        colorAttach.blend  = blendState;
        colorAttach.format = desc.colorAttachmentFormat;

        urhi::GraphicsPipelineDesc pipelineDesc{};
        pipelineDesc.shaders          = shaderObjects;
        pipelineDesc.depthState       = depthState;
        pipelineDesc.rasterizerState  = rasterState;
        pipelineDesc.colorAttachments = { colorAttach };

        if (desc.depthTest || desc.depthWrite)
            pipelineDesc.depthAttachmentFormat = desc.depthAttachmentFormat;

        m_pipeline = m_device->createPipeline(pipelineDesc);
        m_defaultTexture = Engine::getSystem<GraphicsSystem>()->getDefaultTexture();

        urhi::SamplerDesc samplerDesc{};
        samplerDesc.addressModeU = urhi::AddressMode::Repeat;
        samplerDesc.addressModeV = urhi::AddressMode::Repeat;
        m_defaultSampler = m_device->createSampler(samplerDesc);
    }

    AssetRef<MaterialTemplate> MaterialTemplates::s_pbr = nullptr;
    AssetRef<MaterialTemplate> MaterialTemplates::s_billboard = nullptr;
    AssetRef<MaterialTemplate> MaterialTemplates::s_equirectangularSkybox = nullptr;

    AssetRef<MaterialTemplate> MaterialTemplates::pbr()
    {
        if(s_pbr) return s_pbr;

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        const auto shaders = importPipeline.import<std::vector<urhi::ShaderEntryPoint>>("shaders/pbr.slang");

        MaterialDescription desc{};
        desc.name = "PBR";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = true;

        s_pbr = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_pbr;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::billboard()
    {
        if(s_billboard) return s_billboard;

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        const auto shaders = importPipeline.import<std::vector<urhi::ShaderEntryPoint>>("shaders/billboard.slang");

        MaterialDescription desc{};
        desc.name = "Billboard";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = true;
        desc.srcColorBlendFactor = urhi::BlendFactor::SrcAlpha;
        desc.dstColorBlendFactor = urhi::BlendFactor::InvSrcAlpha;
        desc.srcAlphaBlendFactor = urhi::BlendFactor::One;
        desc.dstAlphaBlendFactor = urhi::BlendFactor::InvSrcAlpha;
        desc.depthTest = false;
        desc.depthWrite = false;

        s_billboard = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_billboard;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::equirectangularSkybox()
    {
        if(s_equirectangularSkybox) return s_equirectangularSkybox;

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        const auto shaders = importPipeline.import<std::vector<urhi::ShaderEntryPoint>>("shaders/skybox.slang");

        MaterialDescription desc{};
        desc.name = "Skybox equirectangular";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = false;

        s_equirectangularSkybox = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_equirectangularSkybox;
    }
}
