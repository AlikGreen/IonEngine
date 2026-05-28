#include "materialTemplate.h"

#include <utility>

#include "graphicsSystem.h"
#include "asset/assetImportPipeline.h"
#include "asset/assetRegistry.h"
#include "core/engine.h"
#include "importers/shaderImporter.h"

namespace ion
{
    AssetRef<urhi::slang::Module> MaterialTemplate::s_baseMaterialModule{};
    AssetRef<urhi::slang::Module> MaterialTemplate::s_reflectionShader{};

    grl::Rc<urhi::TextureView> MaterialTemplate::s_defaultTexture{};
    grl::Rc<urhi::Sampler> MaterialTemplate::s_defaultSampler{};

    MaterialTemplate::MaterialTemplate(const MaterialDesc &desc)
        : m_lit(desc.lit), m_opaque(desc.opaque), m_name(desc.name)
    {
        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();
        init(m_device);

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_module = importPipeline.import<urhi::slang::Module>(desc.path);

        compileModule();
    }

    MaterialTemplate::MaterialTemplate(std::string name, const bool opaque, const bool lit, AssetRef<urhi::slang::Module> module)
        : m_lit(lit), m_opaque(opaque), m_name(std::move(name)), m_module(std::move(module))
    {
        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();
        init(m_device);

        compileModule();
    }

    grl::Rc<urhi::Pipeline> MaterialTemplate::getOrCreatePipeline(const urhi::slang::Module& passModule, urhi::GraphicsPipelineDesc pipelineDesc)
    {
        const size_t key = pipelineHash(passModule, pipelineDesc);

        if (const auto it = m_pipelines.find(key); it != m_pipelines.end())
            return it->second;

        // TODO Check disk shader cache
        // Should probably have an engine-wide cache folder for anything that needs caching

        urhi::slang::LinkDesc linkDesc;
        linkDesc.modules     = { *m_module, passModule, *s_baseMaterialModule };
        linkDesc.typeSpecializations = { m_name };

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

    static size_t fnv1a(const void* data, size_t size)
    {
        auto* p = static_cast<const uint8_t*>(data);
        size_t hash = 14695981039346656037ull;
        for (size_t i = 0; i < size; i++)
            hash = (hash ^ p[i]) * 1099511628211ull;
        return hash;
    }

    static void hashCombine(size_t& seed, size_t hash)
    {
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    static size_t irHash(const urhi::slang::Module& module)
    {
        static std::unordered_map<const void*, size_t> s_cache;

        const void* key = module.ir.data();
        auto it = s_cache.find(key);
        if (it != s_cache.end())
            return it->second;

        size_t h = fnv1a(module.ir.data(), module.ir.size());
        s_cache.emplace(key, h);
        return h;
    }

    void MaterialTemplate::compileModule()
    {
        urhi::slang::LinkDesc linkDesc;
        linkDesc.modules     = { *m_module, *s_reflectionShader, *s_baseMaterialModule };
        linkDesc.typeSpecializations = { m_name };

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
    }

    void MaterialTemplate::init(const grl::Rc<urhi::Device> &device)
    {
        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();

        if(!s_baseMaterialModule)
            s_baseMaterialModule = importPipeline.import<urhi::slang::Module>("shaders/material.slang");

        if(!s_reflectionShader)
            s_reflectionShader = importPipeline.import<urhi::slang::Module>("shaders/materialReflection.slang");

        if(!s_defaultTexture)
            s_defaultTexture = Engine::getSystem<GraphicsSystem>()->getDefaultTexture();

        if(!s_defaultSampler)
        {
            urhi::SamplerDesc samplerDesc{};
            samplerDesc.addressModeU = urhi::AddressMode::Repeat;
            samplerDesc.addressModeV = urhi::AddressMode::Repeat;
            s_defaultSampler = device->createSampler(samplerDesc);
        }
    }

    size_t MaterialTemplate::pipelineHash(
    const urhi::slang::Module& passModule,
    const urhi::GraphicsPipelineDesc& desc)
    {
        size_t seed = irHash(passModule);

        hashCombine(seed, fnv1a(&desc.primitiveType,   sizeof(desc.primitiveType)));
        hashCombine(seed, fnv1a(&desc.rasterizerState, sizeof(desc.rasterizerState)));
        hashCombine(seed, fnv1a(&desc.depthState,      sizeof(desc.depthState)));

        for (const auto& att : desc.colorAttachments)
            hashCombine(seed, fnv1a(&att, sizeof(att)));

        const bool hasDepth = desc.depthAttachmentFormat.has_value();
        hashCombine(seed, hasDepth);
        if (hasDepth)
            hashCombine(seed, fnv1a(&*desc.depthAttachmentFormat, sizeof(urhi::PixelFormat)));

        return seed;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::s_pbr{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_billboard{};
    AssetRef<MaterialTemplate> MaterialTemplates::s_equirectangularSkybox{};

    AssetRef<MaterialTemplate> MaterialTemplates::pbr()
    {
        if(s_pbr) return s_pbr;

        MaterialDesc desc{};
        desc.name = "PbrMaterial";
        desc.path = "shaders/pbrMaterial.slang";
        desc.opaque = true;

        s_pbr = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_pbr;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::billboard()
    {
        if(s_billboard) return s_billboard;

        MaterialDesc desc{};
        desc.name = "BillboardMaterial";
        desc.path = "shaders/billboardMaterial.slang";
        desc.opaque = false;

        s_billboard = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_billboard;
    }

    AssetRef<MaterialTemplate> MaterialTemplates::equirectangularSkybox()
    {
        if(s_equirectangularSkybox) return s_equirectangularSkybox;

        MaterialDesc desc{};
        desc.name = "EquirectangularSkyboxMaterial";
        desc.path = "shaders/equirectangularSkyboxMaterial.slang";
        desc.opaque = true;

        s_equirectangularSkybox = Engine::assetRegistry().create<MaterialTemplate>(desc);
        return s_equirectangularSkybox;
    }
}
