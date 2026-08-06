#pragma once
#include <utility>

#include "shaderPreprocessor.h"
#include "spirv_reflect.h"

namespace ion
{
struct ShaderMember
{
    std::string name;
    uint32_t size;
    uint32_t offset;
    dg::VALUE_TYPE type;
    uint32_t componentCount;
};

struct ShaderResource
{
    std::string name;
    dg::SHADER_RESOURCE_TYPE type;
    uint32_t size; // The size of an element

    dg::SHADER_TYPE stages;

    std::vector<ShaderMember> members;
    std::vector<ShaderAttribute> attributes;
};


struct ShaderReflection
{
    std::vector<ShaderResource> resources;
    std::vector<dg::LayoutElement> layoutElements{};
};


class ShaderReflector
{
public:
    explicit ShaderReflector(ProcessedShader processedShader) : m_processedShader(std::move(processedShader)) {}

    ShaderReflection reflect() const;
private:
    static std::vector<uint32_t> compileToSpirV(const std::string &hlsl, const std::string &entryPoint, dg::SHADER_TYPE stage);

    static std::wstring stageToTarget(dg::SHADER_TYPE stage);
    static uint32_t getFormatSize(SpvReflectFormat format);
    static dg::VALUE_TYPE getValueType(SpvReflectFormat format);
    static uint32_t getComponentCount(const SpvReflectTypeDescription* type);
    static dg::SHADER_RESOURCE_TYPE convertResourceType(const SpvReflectDescriptorBinding* binding);
    static dg::VALUE_TYPE convertValueType(const SpvReflectTypeDescription* type);

    ProcessedShader m_processedShader;
};
}
