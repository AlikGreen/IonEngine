#include "shaderRegistry.h"

namespace ion
{
    ShaderRegistry::ShaderRegistry(const dg::Ref<dg::IRenderDevice> &device)
        : m_device(device)
    {

    }

    ShaderBundle ShaderRegistry::getOrCreate(const ShaderModule &module, const ShaderProcessDesc &desc)
    {
        const uint32_t key = createShaderVariantKey(desc, module);

        if(const auto it = m_bundleCache.find(key); it != m_bundleCache.end())
            return it->second;

        ShaderPreprocessor preprocessor{module, desc};
        ProcessedShader processedShader = preprocessor.process();

        grl::File::write(R"(C:\Users\alikg\Downloads\)"+module.name+".hlsl", processedShader.hlsl);

        ShaderReflector reflector{processedShader};
        ShaderReflection refl = reflector.reflect();

        ShaderBundle bundle;
        bundle.resources = refl.resources;
        bundle.layoutElements = refl.layoutElements;

        for(const auto& [name, stage] : processedShader.entryPoints)
        {
            dg::Ref<dg::IShader> shader;

            dg::ShaderCreateInfo shaderDesc{};
            shaderDesc.Desc.ShaderType = stage;
            shaderDesc.Source = processedShader.hlsl.c_str();
            shaderDesc.SourceLanguage = dg::SHADER_SOURCE_LANGUAGE_HLSL;
            shaderDesc.EntryPoint = name.c_str();

            m_device->CreateShader(shaderDesc, &shader);

            switch (stage)
            {
                case dg::SHADER_TYPE_PIXEL:
                    bundle.ps = shader;
                break;
                case dg::SHADER_TYPE_VERTEX:
                    bundle.vs = shader;
                        break;
                case dg::SHADER_TYPE_COMPUTE:
                    bundle.cs = shader;
                    break;
                default:
                    break;
            }
        }

        m_bundleCache.emplace(key, bundle);

        return bundle;
    }


    uint32_t ShaderRegistry::createShaderVariantKey(const ShaderProcessDesc &parseDesc, const ShaderModule &module)
    {
        uint32_t key = grl::Hash::fnv1a32(module.source);

        for(const auto& ep : parseDesc.entryPoints)
        {
            const uint32_t epHash = grl::Hash::fnv1a32(ep);
            grl::Hash::hashCombine(key, epHash);
        }

        for(const auto& path : parseDesc.includePaths)
        {
            const uint32_t pathHash = grl::Hash::fnv1a32(path.string());
            grl::Hash::hashCombine(key, pathHash);
        }

        for(const auto& [name, src] : parseDesc.inlineIncludes)
        {
            const uint32_t nameHash = grl::Hash::fnv1a32(name);
            const uint32_t srcHash = grl::Hash::fnv1a32(src.source);
            grl::Hash::hashCombine(key, nameHash);
            grl::Hash::hashCombine(key, srcHash);
        }

        grl::Hash::hashCombine(key, parseDesc.defaultVariableType);
        return key;
    }
}
