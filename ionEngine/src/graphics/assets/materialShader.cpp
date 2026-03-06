#include "materialShader.h"

#include <ranges>
#include <utility>

#include "graphics/graphicsSystem.h"

namespace ion
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    MaterialShader::MaterialShader(const MaterialDescription& desc)
    {
        name = desc.name;
        device = Engine::getSystem<GraphicsSystem>()->getDevice();
        opaque = !desc.blendEnabled;

        uint32_t requiredSize = 0;

        std::vector<grl::Rc<urhi::Shader>> shaders{};

        for(const auto& entryPoint : *desc.shaders)
        {
            shaders.push_back(device->createShader(entryPoint));

            for(const auto& resource : entryPoint.reflection.resources)
            {
                resources[resource.name] = resource;

                if(resource.type == urhi::ShaderReflection::ResourceType::ConstantBuffer && resource.name == "properties")
                {
                    for (const auto& mem : resource.members)
                    {
                        properties[mem.name] = mem;
                        requiredSize = std::max(requiredSize, mem.offset + mem.size);
                    }
                }
            }
        }


        cpuData.resize(requiredSize);

        if(requiredSize > 0)
            propertiesBuffer = device->createBuffer({ urhi::BufferUsage::Uniform, requiredSize });

        urhi::DepthState depthState{};
        depthState.hasDepthTarget   = true;
        depthState.enableDepthTest  = desc.depthTest;
        depthState.enableDepthWrite = desc.depthWrite;

        urhi::RasterizerState rasterizerState{};
        rasterizerState.cullMode = desc.cullMode;

        urhi::BlendState blendState{};
        blendState.enableBlend    = desc.blendEnabled;
        blendState.srcColorFactor = desc.srcColorBlendFactor;
        blendState.dstColorFactor = desc.dstColorBlendFactor;
        blendState.srcAlphaFactor = desc.srcAlphaBlendFactor;
        blendState.dstAlphaFactor = desc.dstAlphaBlendFactor;

        urhi::ColorAttachmentDesc colorAttachment;
        colorAttachment.blend = blendState;
        colorAttachment.format = desc.colorAttachmentFormat;

        urhi::GraphicsPipelineDesc pipelineDesc{};
        pipelineDesc.shaders          = shaders;
        pipelineDesc.depthState       = depthState;
        pipelineDesc.rasterizerState  = rasterizerState;
        pipelineDesc.colorAttachments = { colorAttachment };

        if(desc.depthTest || desc.depthWrite)
            pipelineDesc.depthAttachmentFormat = desc.depthAttachmentFormat;

        pipeline = device->createPipeline(pipelineDesc);

        defaultTexture = Engine::getSystem<GraphicsSystem>()->getDefaultTexture();

        urhi::SamplerDesc samplerDesc{};
        samplerDesc.addressModeU = urhi::AddressMode::Repeat;
        samplerDesc.addressModeV = urhi::AddressMode::Repeat;
        defaultSampler = device->createSampler(samplerDesc);
    }

    bool MaterialShader::setTexture(std::string name, grl::Rc<urhi::TextureView> texture)
    {
        const auto it = resources.find(name);
        if(it == resources.end() || it->second.type != urhi::ShaderReflection::ResourceType::Texture)
            return false;

        if(texture.get() == nullptr)
            return false;

        textures.emplace(name, texture);
        boundResources.emplace(name);

        return true;
    }

    bool MaterialShader::setSampler(std::string name, grl::Rc<urhi::Sampler> sampler)
    {
        const auto it = resources.find(name);
        if(it == resources.end() || it->second.type != urhi::ShaderReflection::ResourceType::Sampler)
            return false;

        if(sampler.get() == nullptr)
            return false;

        samplers.emplace(name, sampler);
        boundResources.emplace(name);

        return true;
    }

    std::unordered_map<std::string, urhi::ShaderReflection::Resource> MaterialShader::getResources() const
    {
        return resources;
    }

    std::unordered_map<std::string, urhi::ShaderReflection::Member> MaterialShader::getProperties() const
    {
        return properties;
    }

    grl::Rc<urhi::Pipeline> MaterialShader::getPipeline()
    {
        return pipeline;
    }

    void MaterialShader::bindUniforms(const grl::Rc<urhi::CommandList>& cmd, const grl::Rc<urhi::RenderPass> &pass)
    {
        if(dirty && propertiesBuffer && !cpuData.empty())
        {
            cmd->updateBuffer(propertiesBuffer, cpuData);
            dirty = false;
        }

        if(propertiesBuffer)
            pass->setUniformBuffer("properties", propertiesBuffer);

        for(const auto& resource : resources | std::ranges::views::values)
        {
            if(boundResources.contains(resource.name)) continue;

            switch(resource.type)
            {
                case urhi::ShaderReflection::ResourceType::Texture:
                    pass->setTexture(resource.name, defaultTexture);
                case urhi::ShaderReflection::ResourceType::Sampler:
                    pass->setSampler(resource.name, defaultSampler);
                default:
                    break;
            }
        }

        for(const auto& [name, texture] : textures)
        {
            pass->setTexture(name, texture);
        }

        for(const auto& [name, sampler] : samplers)
        {
            pass->setSampler(name, sampler);
        }
    }

    bool MaterialShader::isOpaque() const
    {
        return opaque;
    }

    MaterialShader MaterialShader::createBillboard()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shaders = assetManager.import<std::vector<urhi::ShaderEntryPoint>>("shaders/billboard.slang");

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

        return MaterialShader(desc);
    }

    MaterialShader MaterialShader::createPBR()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shaders = assetManager.import<std::vector<urhi::ShaderEntryPoint>>("shaders/pbr.slang");

        MaterialDescription desc{};
        desc.name = "PBR";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = true;

        return MaterialShader(desc);
    }

    MaterialShader MaterialShader::createEquirectangularSkybox()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shaders = assetManager.import<std::vector<urhi::ShaderEntryPoint>>("shaders/skybox.slang");

        MaterialDescription desc{};
        desc.name = "Skybox equirectangular";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = false;

        return MaterialShader(desc);
    }
}
