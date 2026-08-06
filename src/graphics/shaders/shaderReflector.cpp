#include "shaderReflector.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <WinAdapter.h>
#endif

#include <dxcapi.h>

namespace ion
{
    ShaderReflection ShaderReflector::reflect() const
    {
        std::vector<dg::LayoutElement> inputLayoutElements{};
        std::vector<ShaderResource> shaderResources{};

        for(const auto& [name, stage] : m_processedShader.entryPoints)
        {
            auto spriv = compileToSpirV(m_processedShader.hlsl, name, stage);

            SpvReflectShaderModule module;
            const SpvReflectResult result = spvReflectCreateShaderModule(spriv.size()*4, spriv.data(), &module);
            clogr::ensure(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect shader entry point ({})", name);

            // --- Reflect Input Layout ---
            if(stage == dg::SHADER_TYPE_VERTEX)
            {
                uint32_t inputVarCount = 0;
                spvReflectEnumerateInputVariables(&module, &inputVarCount, nullptr);

                std::vector<SpvReflectInterfaceVariable*> inputVars(inputVarCount);
                spvReflectEnumerateInputVariables(&module, &inputVarCount, inputVars.data());

                uint32_t offset = 0;
                uint32_t index = 0;
                for (const SpvReflectInterfaceVariable* inputVar : inputVars)
                {
                    dg::LayoutElement layoutElement{};
                    layoutElement.BufferSlot = 0;
                    layoutElement.InputIndex = index;
                    layoutElement.RelativeOffset = offset;
                    layoutElement.IsNormalized = false;
                    layoutElement.ValueType = getValueType(inputVar->format);
                    layoutElement.NumComponents = getComponentCount(inputVar->type_description);

                    inputLayoutElements.push_back(layoutElement);

                    offset += getFormatSize(inputVar->format);
                    index++;
                }

                for(auto layoutElement : inputLayoutElements)
                {
                    layoutElement.Stride = offset;
                }
            }

            // --- Reflect Shader Resources ---
            uint32_t descriptorCount = 0;
            spvReflectEnumerateDescriptorBindings(&module, &descriptorCount, nullptr);

            std::vector<SpvReflectDescriptorBinding*> descriptors(descriptorCount);
            spvReflectEnumerateDescriptorBindings(&module, &descriptorCount, descriptors.data());

            for(auto binding : descriptors)
            {
                ShaderResource resource{};

                resource.name = binding->name;

                for(auto res : m_processedShader.resources)
                {
                    if(res.name == resource.name)
                    {
                        resource.attributes = res.attributes;
                        break;
                    }
                }

                resource.type = convertResourceType(binding);

                if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                {
                    resource.size = binding->block.size;
                }

                resource.stages = stage;

                SpvReflectBlockVariable& block = binding->block;

                for (uint32_t i = 0; i < block.member_count; i++)
                {
                    ShaderMember shaderMember{};

                    SpvReflectBlockVariable& member = block.members[i];

                    shaderMember.offset = member.offset;
                    shaderMember.size = member.padded_size;
                    shaderMember.type = convertValueType(member.type_description);
                    shaderMember.componentCount = getComponentCount(member.type_description);

                    resource.members.push_back(shaderMember);
                }

                shaderResources.push_back(resource);
            }
        }

        return { shaderResources, inputLayoutElements };
    }

    std::vector<uint32_t> ShaderReflector::compileToSpirV(const std::string &hlsl, const std::string &entryPoint, dg::SHADER_TYPE stage)
    {
        IDxcLibrary*         library  = nullptr;
        IDxcCompiler*        compiler = nullptr;
        IDxcBlobEncoding*    source   = nullptr;
        IDxcOperationResult* result   = nullptr;

        DxcCreateInstance(CLSID_DxcLibrary,  IID_PPV_ARGS(&library));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

        library->CreateBlobWithEncodingFromPinned(
            hlsl.c_str(),
            static_cast<UINT32>(hlsl.size()),
            CP_UTF8,
            &source
        );

        const auto wEntry = std::wstring(entryPoint.begin(), entryPoint.end());

        LPCWSTR args[] = { L"-spirv", L"-fvk-use-gl-layout", L"-fspv-reflect", L"-HV", L"2021" };

        compiler->Compile(
            source,
            L"shader.hlsl",
            wEntry.c_str(),
            stageToTarget(stage).c_str(),
            args, _countof(args),
            nullptr, 0,
            nullptr,
            &result
        );

        HRESULT hr;
        result->GetStatus(&hr);
        if (FAILED(hr))
        {
            IDxcBlobEncoding* errors = nullptr;
            result->GetErrorBuffer(&errors);
            const std::string msg(
                static_cast<const char*>(errors->GetBufferPointer()),
                errors->GetBufferSize()
            );
            errors->Release();
            clogr::abort("{}", msg.c_str());
            return {};
        }

        IDxcBlob* spirvBlob = nullptr;
        result->GetResult(&spirvBlob);

        const uint32_t* data      = static_cast<const uint32_t*>(spirvBlob->GetBufferPointer());
        size_t          wordCount = spirvBlob->GetBufferSize() / sizeof(uint32_t);
        std::vector<uint32_t> spirv(data, data + wordCount);

        spirvBlob->Release();
        result->Release();
        source->Release();
        compiler->Release();
        library->Release();

        return spirv;
    }

    std::wstring ShaderReflector::stageToTarget(dg::SHADER_TYPE stage)
    {
        switch (stage)
        {
            case dg::SHADER_TYPE_VERTEX:   return L"vs_6_0";
            case dg::SHADER_TYPE_PIXEL:    return L"ps_6_0";
            case dg::SHADER_TYPE_COMPUTE:  return L"cs_6_0";
            case dg::SHADER_TYPE_GEOMETRY: return L"gs_6_0";
            case dg::SHADER_TYPE_HULL:     return L"hs_6_0";
            case dg::SHADER_TYPE_DOMAIN:   return L"ds_6_0";
            default:
                assert(false && "Unsupported shader stage");
            return L"vs_6_0";
        }
    }

    uint32_t ShaderReflector::getFormatSize(const SpvReflectFormat format)
    {
        switch (format)
        {
            case SPV_REFLECT_FORMAT_R32_SFLOAT:
                return 4;
            case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
                return 8;
            case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
                return 12;
            case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
                return 16;

            case SPV_REFLECT_FORMAT_R32_SINT:
                return 4;
            case SPV_REFLECT_FORMAT_R32G32_SINT:
                return 8;
            case SPV_REFLECT_FORMAT_R32G32B32_SINT:
                return 12;
            case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
                return 16;

            case SPV_REFLECT_FORMAT_R32_UINT:
                return 4;
            case SPV_REFLECT_FORMAT_R32G32_UINT:
                return 8;
            case SPV_REFLECT_FORMAT_R32G32B32_UINT:
                return 12;
            case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
                return 16;

            case SPV_REFLECT_FORMAT_R16_SFLOAT:
                return 2;
            case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
                return 4;
            case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
                return 8;

            case SPV_REFLECT_FORMAT_R16_SINT:
                return 2;
            case SPV_REFLECT_FORMAT_R16G16_SINT:
                return 4;
            case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
                return 8;

            case SPV_REFLECT_FORMAT_R16_UINT:
                return 2;
            case SPV_REFLECT_FORMAT_R16G16_UINT:
                return 4;
            case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
                return 8;
            default:
                return 0;
        }
    }

    dg::VALUE_TYPE ShaderReflector::getValueType(const SpvReflectFormat format)
    {
        switch (format)
        {
            case SPV_REFLECT_FORMAT_R32_SFLOAT:
            case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
                return dg::VT_FLOAT32;

            case SPV_REFLECT_FORMAT_R32_SINT:
            case SPV_REFLECT_FORMAT_R32G32_SINT:
            case SPV_REFLECT_FORMAT_R32G32B32_SINT:
            case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
                return dg::VT_INT32;

            case SPV_REFLECT_FORMAT_R32_UINT:
            case SPV_REFLECT_FORMAT_R32G32_UINT:
            case SPV_REFLECT_FORMAT_R32G32B32_UINT:
            case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
                return dg::VT_UINT32;

            case SPV_REFLECT_FORMAT_R16_SFLOAT:
            case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
            case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
                return dg::VT_FLOAT16;

            case SPV_REFLECT_FORMAT_R16_SINT:
            case SPV_REFLECT_FORMAT_R16G16_SINT:
            case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
                return dg::VT_INT16;

            case SPV_REFLECT_FORMAT_R16_UINT:
            case SPV_REFLECT_FORMAT_R16G16_UINT:
            case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
                return dg::VT_UINT16;
            default:
                return dg::VT_UNDEFINED;
        }
    }

    uint32_t ShaderReflector::getComponentCount(const SpvReflectTypeDescription* type)
    {
        if (type == nullptr)
            return 0;

        if (type->op == SpvOpTypeVector)
            return type->traits.numeric.vector.component_count;

        return 1;
    }

    dg::SHADER_RESOURCE_TYPE ShaderReflector::convertResourceType(const SpvReflectDescriptorBinding* binding)
    {
        switch (binding->descriptor_type)
        {
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                return dg::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                return dg::SHADER_RESOURCE_TYPE_BUFFER_UAV;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return dg::SHADER_RESOURCE_TYPE_TEXTURE_SRV;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                return dg::SHADER_RESOURCE_TYPE_TEXTURE_UAV;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                return dg::SHADER_RESOURCE_TYPE_SAMPLER;
            case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                return dg::SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT;

            default:
                return dg::SHADER_RESOURCE_TYPE_UNKNOWN;
        }
    }

    dg::VALUE_TYPE ShaderReflector::convertValueType(const SpvReflectTypeDescription* type)
    {
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
        {
            return dg::VT_UINT8;
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
        {
            switch (type->traits.numeric.scalar.width)
            {
                case 16:
                    return dg::VT_FLOAT16;
                case 32:
                    return dg::VT_FLOAT32;
                case 64:
                    return dg::VT_FLOAT64;
                default:
                    return dg::VT_UNDEFINED;
            }
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        {
            const bool isSigned = type->traits.numeric.scalar.signedness != 0;

            switch (type->traits.numeric.scalar.width)
            {
                case 8:
                    return isSigned ? dg::VT_INT8 : dg::VT_UINT8;
                case 16:
                    return isSigned ? dg::VT_INT16 : dg::VT_UINT16;
                case 32:
                    return isSigned ? dg::VT_INT32 : dg::VT_UINT32;
                default:
                    return dg::VT_UNDEFINED;
            }
        }

        return dg::VT_UNDEFINED;
    }
}
