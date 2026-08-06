#include "pipelineRegistry.h"

#include <utility>

namespace ion
{
    PipelineRegistry::PipelineRegistry(dg::Ref<dg::IRenderDevice> device)
        : m_device(std::move(device))
    {

    }

    dg::Ref<dg::IPipelineState> PipelineRegistry::getOrCreateGraphics(const ShaderBundle& shader, const PassDefinition& definition)
    {
        dg::GraphicsPipelineStateCreateInfo ci{};
        ci.pVS = shader.vs;
        ci.pPS = shader.ps;

        ci.PSODesc.PipelineType = dg::PIPELINE_TYPE_GRAPHICS;

        ci.GraphicsPipeline.BlendDesc = createBlendState(definition.overrides.blend.value_or(BlendPreset::Opaque), definition.rtvFormats.size());
        ci.GraphicsPipeline.DepthStencilDesc = createDepthState(definition.overrides.depth.value_or(DepthPreset::Disabled));

        ci.GraphicsPipeline.PrimitiveTopology = definition.topology;

        ci.GraphicsPipeline.NumRenderTargets = definition.rtvFormats.size();

        for(size_t i = 0; i < definition.rtvFormats.size(); i++)
            ci.GraphicsPipeline.RTVFormats[i] = definition.rtvFormats[i];

        ci.GraphicsPipeline.DSVFormat = definition.dtvFormat;

        ci.GraphicsPipeline.RasterizerDesc.CullMode = definition.overrides.cullMode.value_or(dg::CULL_MODE_BACK);
        ci.GraphicsPipeline.RasterizerDesc.FillMode = definition.overrides.fillMode.value_or(dg::FILL_MODE_SOLID);
        ci.GraphicsPipeline.RasterizerDesc.ScissorEnable = definition.overrides.scissorEnabled.value_or(false);

        const uint32_t key = pipelineHash(ci);

        if(const auto it = m_psoCache.find(key); it != m_psoCache.end())
        {
            return it->second;
        }

        ci.GraphicsPipeline.InputLayout.NumElements = shader.layoutElements.size();
        ci.GraphicsPipeline.InputLayout.LayoutElements = shader.layoutElements.data();

        const auto variables = createResourceLayout(shader.resources);

        ci.PSODesc.ResourceLayout.Variables = variables.data();
        ci.PSODesc.ResourceLayout.NumVariables = variables.size();
        ci.PSODesc.ResourceLayout.DefaultVariableType = dg::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        dg::Ref<dg::IPipelineState> pso;
        m_device->CreatePipelineState(ci, &pso);
        m_psoCache[key] = pso;
        return pso;
    }

    dg::Ref<dg::IPipelineState> PipelineRegistry::getOrCreateCompute(const ShaderBundle &shader)
    {
        dg::ComputePipelineStateCreateInfo ci{};
        ci.pCS = shader.cs;
        ci.PSODesc.PipelineType = dg::PIPELINE_TYPE_COMPUTE;

        const uint32_t key = grl::Hash::hashObject32(shader.cs.RawPtr());

        if(const auto it = m_psoCache.find(key); it != m_psoCache.end())
        {
            return it->second;
        }

        const auto variables = createResourceLayout(shader.resources);

        ci.PSODesc.ResourceLayout.Variables = variables.data();
        ci.PSODesc.ResourceLayout.NumVariables = variables.size();

        dg::Ref<dg::IPipelineState> pso;
        m_device->CreatePipelineState(ci, &pso);
        m_psoCache[key] = pso;
        return pso;
    }

    std::vector<dg::ShaderResourceVariableDesc> PipelineRegistry::createResourceLayout(const std::vector<ShaderResource> &resources)
    {
        std::vector<dg::ShaderResourceVariableDesc> variables{};

        for(const auto& resource : resources)
        {
            dg::ShaderResourceVariableDesc varDesc{};
            varDesc.Name = resource.name.c_str();
            varDesc.Type = dg::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
            varDesc.ShaderStages = resource.stages;

            if(!std::ranges::contains(resource.attributes, "ion::allow_dynamic_offset", &ShaderAttribute::name))
            {
                if(resource.type == dg::SHADER_RESOURCE_TYPE_BUFFER_SRV ||
                    resource.type == dg::SHADER_RESOURCE_TYPE_BUFFER_UAV ||
                    resource.type == dg::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER)
                    varDesc.Flags = dg::SHADER_VARIABLE_FLAG_NO_DYNAMIC_BUFFERS;
            }

            if(std::ranges::contains(resource.attributes, "ion::push_constant", &ShaderAttribute::name))
            {
                varDesc.Type = dg::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
                varDesc.Flags = dg::SHADER_VARIABLE_FLAG_INLINE_CONSTANTS;
            }
            else if(std::ranges::contains(resource.attributes, "ion::static", &ShaderAttribute::name))
                varDesc.Type = dg::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
            else if(std::ranges::contains(resource.attributes, "ion::mutable", &ShaderAttribute::name))
                varDesc.Type = dg::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
            else if(std::ranges::contains(resource.attributes, "ion::dynamic", &ShaderAttribute::name))
                varDesc.Type = dg::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;

            variables.push_back(varDesc);
        }

        return variables;
    }

    dg::BlendStateDesc PipelineRegistry::createBlendState(const BlendOverride &blendOverride, const size_t rtvCount)
    {
        dg::BlendStateDesc blendStateDesc{};

        if (std::holds_alternative<BlendPreset>(blendOverride))
        {
            const auto preset = std::get<BlendPreset>(blendOverride);

            for(size_t i = 0; i < rtvCount; i++)
            {
                switch (preset)
                {
                    case BlendPreset::Opaque:
                        blendStateDesc.RenderTargets[i].BlendEnable = false;
                        break;
                    case BlendPreset::AlphaBlend:
                        blendStateDesc.RenderTargets[i].BlendEnable = true;
                        blendStateDesc.RenderTargets[i].SrcBlend = dg::BLEND_FACTOR_SRC_ALPHA;
                        blendStateDesc.RenderTargets[i].DestBlend = dg::BLEND_FACTOR_INV_SRC_ALPHA;
                        blendStateDesc.RenderTargets[i].BlendOp = dg::BLEND_OPERATION_ADD;
                        blendStateDesc.RenderTargets[i].SrcBlendAlpha = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].DestBlendAlpha = dg::BLEND_FACTOR_INV_SRC_ALPHA;
                        blendStateDesc.RenderTargets[i].BlendOpAlpha = dg::BLEND_OPERATION_ADD;
                    case BlendPreset::Additive:
                        blendStateDesc.RenderTargets[i].BlendEnable = true;
                        blendStateDesc.RenderTargets[i].SrcBlend = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].DestBlend = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].BlendOp = dg::BLEND_OPERATION_ADD;
                        blendStateDesc.RenderTargets[i].SrcBlendAlpha = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].DestBlendAlpha = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].BlendOpAlpha = dg::BLEND_OPERATION_ADD;
                    case BlendPreset::Premultiplied:
                        blendStateDesc.RenderTargets[i].BlendEnable = true;
                        blendStateDesc.RenderTargets[i].SrcBlend = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].DestBlend = dg::BLEND_FACTOR_INV_SRC_ALPHA;
                        blendStateDesc.RenderTargets[i].BlendOp = dg::BLEND_OPERATION_ADD;
                        blendStateDesc.RenderTargets[i].SrcBlendAlpha = dg::BLEND_FACTOR_ONE;
                        blendStateDesc.RenderTargets[i].DestBlendAlpha = dg::BLEND_FACTOR_INV_SRC_ALPHA;
                        blendStateDesc.RenderTargets[i].BlendOpAlpha = dg::BLEND_OPERATION_ADD;
                }
            }
        }
        else if (std::holds_alternative<dg::BlendStateDesc>(blendOverride))
        {
            blendStateDesc = std::get<dg::BlendStateDesc>(blendOverride);
        }

        return blendStateDesc;
    }

    dg::DepthStencilStateDesc PipelineRegistry::createDepthState(const DepthOverride &depthOverride)
    {
        dg::DepthStencilStateDesc depthStateDesc{};

        if (std::holds_alternative<DepthPreset>(depthOverride))
        {
            const auto preset = std::get<DepthPreset>(depthOverride);

            switch(preset)
            {
                case DepthPreset::Disabled:
                    depthStateDesc.DepthEnable = false;
                    depthStateDesc.DepthWriteEnable = false;
                case DepthPreset::ReadWrite:
                    depthStateDesc.DepthEnable = true;
                    depthStateDesc.DepthWriteEnable = true;
                    depthStateDesc.DepthFunc = dg::COMPARISON_FUNC_LESS;
                case DepthPreset::ReadOnly:
                    depthStateDesc.DepthEnable = true;
                    depthStateDesc.DepthWriteEnable = false;
                    depthStateDesc.DepthFunc = dg::COMPARISON_FUNC_LESS;
            }

        }
        else if (std::holds_alternative<dg::DepthStencilStateDesc>(depthOverride))
        {
            depthStateDesc = std::get<dg::DepthStencilStateDesc>(depthOverride);
        }

        return depthStateDesc;
    }


    size_t PipelineRegistry::pipelineHash(const dg::GraphicsPipelineStateCreateInfo& ci)
    {
        size_t seed = grl::Hash::hashValues32(ci.pVS, ci.pPS);

        grl::Hash::hashCombine(seed, ci.GraphicsPipeline.PrimitiveTopology);
        grl::Hash::hashCombine(seed, ci.GraphicsPipeline.RasterizerDesc);
        grl::Hash::hashCombine(seed, ci.GraphicsPipeline.DepthStencilDesc);

        for (size_t i = 0; i < ci.GraphicsPipeline.NumRenderTargets; i++)
            grl::Hash::hashCombine(seed, ci.GraphicsPipeline.RTVFormats[i]);

        grl::Hash::hashCombine(seed, ci.GraphicsPipeline.DSVFormat);

        return seed;
    }
}
