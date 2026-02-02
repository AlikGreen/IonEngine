#include "materialShader.h"

#include <utility>

#include "graphics/graphicsSystem.h"

namespace Neon
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    MaterialShader::MaterialShader(const MaterialDescription& description)
    {
        name = description.name;
        device = Engine::getSystem<GraphicsSystem>()->getDevice();
        reflection = (*description.shader)->getShaderReflection();

        uint32_t requiredSize = 0;

        for(const auto& resource : reflection.resources)
        {
            if(resource.type == RHI::ShaderReflection::ResourceType::ConstantBuffer &&  resource.name == "Properties")
            {
                for (const auto& mem : resource.members)
                {
                    memberInfos.push_back(mem);
                    requiredSize = std::max(requiredSize, mem.offset + mem.size);
                }
            }

            if(resource.type == RHI::ShaderReflection::ResourceType::Sampler)
                samplerInfos.push_back(resource);
            if(resource.type == RHI::ShaderReflection::ResourceType::Texture)
                textureInfos.push_back(resource);
        }


        if (cpuData.size() < requiredSize)
            cpuData.resize(requiredSize);

        RHI::InputLayout inputLayout{};
        inputLayout.addVertexBuffer<Vertex>(0);
        inputLayout.addVertexAttribute<glm::vec3>(0, 0); // Pos
        inputLayout.addVertexAttribute<glm::vec3>(0, 1); // Normal
        inputLayout.addVertexAttribute<glm::vec2>(0, 2); // UV

        RHI::DepthState depthState{};
        depthState.hasDepthTarget   = true;
        depthState.enableDepthTest  = description.depthTest;
        depthState.enableDepthWrite = description.depthWrite;

        RHI::RasterizerState rasterizerState{};
        rasterizerState.cullMode = RHI::CullMode::Back;


        RHI::BlendState blendState{};
        blendState.enableBlend = description.blendEnabled;
        blendState.srcColorFactor = description.srcColorBlendFactor;
        blendState.dstColorFactor = description.dstColorBlendFactor;
        blendState.srcAlphaFactor = description.srcAlphaBlendFactor;
        blendState.dstAlphaFactor = description.dstAlphaBlendFactor;

        RHI::GraphicsPipelineDesc pipelineDescription{};
        pipelineDescription.shader             = *description.shader;
        pipelineDescription.inputLayout        = inputLayout;
        pipelineDescription.targetsDescription = {};
        pipelineDescription.depthState         = depthState;
        pipelineDescription.rasterizerState    = rasterizerState;
        pipelineDescription.blendState         = blendState;

        pipeline = device->createPipeline(pipelineDescription);

        propertiesBuffer = device->createUniformBuffer();

        defaultTexture = Engine::getSystem<GraphicsSystem>()->getDefaultTexture();
        RHI::SamplerDesc samplerDescription{};
        defaultSampler = device->createSampler(samplerDescription);
    }

    bool MaterialShader::setTexture(std::string name, Rc<RHI::TextureView> texture)
    {
        bool containsTexture = false;

        for(const auto& textureInfo : textureInfos)
        {
            if(textureInfo.name == name)
            {
                containsTexture = true;
                break;
            }
        }

        if(!containsTexture || texture.get() == nullptr)
            return false;

        if(textures.contains(name) && textures.at(name) == texture)
            return true;

        textures.emplace(name, texture);

        return true;
    }

    bool MaterialShader::setSampler(std::string name, Rc<RHI::Sampler> sampler)
    {
        bool containsSampler = false;

        for(const auto& samplerInfo : samplerInfos)
        {
            if(samplerInfo.name == name)
            {
                containsSampler = true;
                break;
            }
        }

        if(!containsSampler || sampler.get() == nullptr)
            return false;

        if(samplers.contains(name) && samplers.at(name) == sampler)
            return true;

        samplers.emplace(name, sampler);

        return true;
    }

    std::vector<RHI::ShaderReflection::Resource> MaterialShader::getSamplers() const
    {
        return samplerInfos;
    }

    std::vector<RHI::ShaderReflection::Resource> MaterialShader::getTextures() const
    {
        return textureInfos;
    }

    std::vector<RHI::ShaderReflection::Member> MaterialShader::getProperties() const
    {
        return memberInfos;
    }

    RHI::ShaderReflection MaterialShader::getReflection()
    {
        return reflection;
    }

    Rc<RHI::Pipeline> MaterialShader::getPipeline()
    {
        return pipeline;
    }

    void MaterialShader::bindUniforms(const Rc<RHI::CommandList>& commandList)
    {
        if(dirty && !cpuData.empty())
        {
            commandList->reserveBuffer(propertiesBuffer, cpuData.size());
            commandList->updateBuffer(propertiesBuffer, cpuData);
            dirty = false;
        }

        if(propertiesBuffer != nullptr && !cpuData.empty())
            commandList->setUniformBuffer("Properties", propertiesBuffer);

        for(const auto& textureInfo : textureInfos)
        {
            if(!textures.contains(textureInfo.name))
                commandList->setTexture(textureInfo.name, defaultTexture);
        }

        for(const auto& samplerInfo : samplerInfos)
        {
            if(!samplers.contains(samplerInfo.name))
                commandList->setSampler(samplerInfo.name, defaultSampler);
        }

        for(const auto& [name, texture] : textures)
        {
            commandList->setTexture(name, texture);
        }

        for(const auto& [name, sampler] : samplers)
        {
            commandList->setSampler(name, sampler);
        }
    }

    MaterialShader MaterialShader::createBillboard()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shader = assetManager.import<Rc<RHI::Shader>>("shaders/billboard.slang");

        MaterialDescription desc{};
        desc.name = "Billboard";
        desc.shader = shader;
        desc.cullMode = RHI::CullMode::Back;
        desc.blendEnabled = true;
        desc.srcColorBlendFactor = RHI::BlendFactor::SrcAlpha;
        desc.dstColorBlendFactor = RHI::BlendFactor::InvSrcAlpha;
        desc.srcAlphaBlendFactor = RHI::BlendFactor::One;
        desc.dstAlphaBlendFactor = RHI::BlendFactor::InvSrcAlpha;
        desc.depthTest = false;
        desc.depthWrite = false;

        return MaterialShader(desc);
    }

    MaterialShader MaterialShader::createPBR()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shader = assetManager.import<Rc<RHI::Shader>>("shaders/pbr.slang");

        MaterialDescription desc{};
        desc.name = "PBR";
        desc.shader = shader;
        desc.cullMode = RHI::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = true;

        return MaterialShader(desc);
    }

    MaterialShader MaterialShader::createEquirectangularSkybox()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shader = assetManager.import<Rc<RHI::Shader>>("shaders/skybox.slang");

        MaterialDescription desc{};
        desc.name = "Skybox equirectangular";
        desc.shader = shader;
        desc.cullMode = RHI::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = true;
        desc.depthWrite = false;

        return MaterialShader(desc);
    }
}
