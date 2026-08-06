#pragma once
#include <vector>

#include "shaderDiagnostics.h"
#include "shaderModule.h"
#include "util/stringParser.h"

namespace ion
{
struct ShaderProcessDesc
{
    std::vector<std::filesystem::path> includePaths{};
    std::vector<std::pair<std::string, ShaderModule>> inlineIncludes{};
    std::vector<std::string> entryPoints{};
    dg::SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType = dg::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
};

using ArgType = std::variant<std::string, int>;

struct ShaderAttribute
{
    std::string name;
    std::vector<ArgType> args;
};

struct EntryPointInfo
{
    std::string   name;
    dg::SHADER_TYPE stage;
};

struct ResourceDeclaration
{
    std::string type;
    std::vector<ArgType> templateArgs;
    std::string name;
    uint32_t    arraySize;
    std::vector<ShaderAttribute> attributes;
};

struct FuncDeclaration
{
    std::string returnType;
    std::string name;
};

struct StructDeclaration
{
    std::string name;
};

struct ProcessedShader
{
    std::string hlsl;
    std::vector<ResourceDeclaration> resources;
    std::vector<EntryPointInfo> entryPoints;
};

using ShaderDeclaration = std::variant<ResourceDeclaration, FuncDeclaration, StructDeclaration>;

enum class ShaderPreprocessMode
{
    ResolveOnly,
    FullProcess
};

class ShaderPreprocessor
{
public:
    explicit ShaderPreprocessor(const ShaderModule& module, ShaderProcessDesc desc);
    ProcessedShader process(ShaderPreprocessMode mode = ShaderPreprocessMode::FullProcess);
private:
    StringParser       m_file;
    ShaderProcessDesc  m_processDesc;
    ShaderModule       m_module;

    std::ostringstream m_cleaned;
    std::unordered_map<char, int> m_registerCounters{};

    std::vector<ResourceDeclaration> m_pendingResources;
    std::vector<EntryPointInfo> m_pendingEntryPoints;

    DiagnosticsSink    m_diagnostics;

    bool resolveInclude(ShaderPreprocessMode mode);

    void skipLineComment(); 
    void skipBlockComment();
    std::vector<ShaderAttribute> tryParseAttributes();
    std::optional<ShaderAttribute> tryParseAttribute();
    std::optional<ShaderDeclaration> tryParseDeclaration();

    ResourceDeclaration finishResource(const std::string &type, const std::vector<ArgType> &templateArgs, const std::string &name);
    FuncDeclaration finishFunctionDecl(const std::string &type, const std::string &name);
    StructDeclaration finishStructDecl(const std::string &name);

    std::vector<ArgType> parseArgs();

    static bool isResourceType(std::string_view type);

    static char getRegisterSpace(std::string_view type);
};
}
