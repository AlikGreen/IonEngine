#include "shaderPreprocessor.h"

#include <utility>

namespace ion
{
    ShaderPreprocessor::ShaderPreprocessor(const ShaderModule &module, ShaderProcessDesc desc)
        : m_file(module.source), m_processDesc(std::move(desc)), m_module(module)
    {

    }

    static std::unordered_map<std::string, dg::SHADER_TYPE> stageNames
    {
        { "vertex", dg::SHADER_TYPE_VERTEX },
        { "fragment", dg::SHADER_TYPE_PIXEL },
        { "pixel", dg::SHADER_TYPE_PIXEL },
        { "compute", dg::SHADER_TYPE_COMPUTE },
    };

    ProcessedShader ShaderPreprocessor::process(ShaderPreprocessMode mode)
    {
        m_file.reset();
        m_cleaned.clear();

        while (!m_file.done())
        {
            skipLineComment();
            if(m_file.done()) break;
            skipBlockComment();
            if(m_file.done()) break;
            resolveInclude(mode);
            if(m_file.done()) break;

            if(mode == ShaderPreprocessMode::ResolveOnly)
            {
                m_cleaned << m_file.consumeLine() << '\n';
                continue;
            }

            auto attribs = tryParseAttributes();
            if(m_file.done()) break;

            if (auto decl = tryParseDeclaration(); decl.has_value())
            {
                if (const auto* resPtr = std::get_if<ResourceDeclaration>(&decl.value()))
                {
                    ResourceDeclaration res = *resPtr;
                    res.attributes = attribs;
                    m_pendingResources.push_back(res);
                }
                else if (const auto* func = std::get_if<FuncDeclaration>(&decl.value()))
                {
                    for (auto& a : attribs)
                        if (a.name == "shader" && !a.args.empty() && std::holds_alternative<std::string>(a.args.at(0)))
                        {
                            auto stageName = std::get<std::string>(a.args.at(0));
                            m_pendingEntryPoints.push_back({ func->name, stageNames.at(stageName) });
                        }
                }

                continue;
            }

            m_cleaned << m_file.consumeLine() << '\n';
        }

        ProcessedShader shader;
        shader.resources = m_pendingResources;
        shader.hlsl = m_cleaned.str();
        shader.entryPoints = m_pendingEntryPoints;

        return shader;
    }

    // TODO check for circular denendencies
    bool ShaderPreprocessor::resolveInclude(const ShaderPreprocessMode mode)
    {
        auto startPos = m_file.pos();
        if(!m_file.tryConsume("#include"))
            return false;

        const auto includeFileName = m_file.consumeStringLiteral();

        std::string includeSrc{};
        for(const auto& [name, source] : m_processDesc.inlineIncludes)
        {
            if(name == includeFileName)
            {
                includeSrc = source.source;
                break;
            }
        }

        if(includeSrc.empty())
        {
            for(const auto& path : m_processDesc.includePaths)
            {
                auto fullPath = path / includeFileName;
                if(std::filesystem::exists(fullPath))
                {
                    includeSrc = grl::File::read(fullPath.string()).value_or("");
                    if(!includeSrc.empty()) break;
                }
            }
        }

        if(!includeSrc.empty())
        {
            const ShaderModule module{includeSrc};
            ShaderPreprocessor subProcessor(module, m_processDesc);
            auto subResult = subProcessor.process(mode);
            m_pendingResources.insert(m_pendingResources.end(), subResult.resources.begin(), subResult.resources.end());
            m_pendingEntryPoints.insert(m_pendingEntryPoints.end(), subResult.entryPoints.begin(), subResult.entryPoints.end());
            std::string processedInclude = subResult.hlsl;

            m_cleaned << "// ======= Start of include from " << includeFileName << " =======\n\n\n" << processedInclude << "\n\n\n// ======= End of include from " << includeFileName << " =======\n\n\n";
            return true;
        }

        m_file.seek(startPos);
        return false;
    }

    void ShaderPreprocessor::skipLineComment()
    {
        if(!m_file.tryConsume("//")) return;
        m_cleaned << "//" << m_file.consumeLine() << '\n';
    }

    void ShaderPreprocessor::skipBlockComment()
    {
        if(!m_file.tryConsume("/*")) return;

        m_cleaned << "/*";
        while((m_file.peek() != '*' || m_file.peek(1) != '/') && !m_file.done())
        {
            m_cleaned << m_file.peek();
            m_file.skip();
        }
        m_file.tryConsume("*/");
        m_cleaned << "*/";
    }

    std::vector<ShaderAttribute> ShaderPreprocessor::tryParseAttributes()
    {
        std::vector<ShaderAttribute> attributes;

        while (true)
        {
            std::optional<ShaderAttribute> attrib = tryParseAttribute();

            if(attrib == std::nullopt)
                break;

            attributes.push_back(attrib.value());
        }

        return attributes;
    }

    std::optional<ShaderAttribute> ShaderPreprocessor::tryParseAttribute()
    {
        if(!m_file.tryConsume('[')) return std::nullopt;

        std::ostringstream attribText;
        attribText << "[";

        const bool doubleOpen = m_file.tryConsume("[");

        const auto attribName = m_file.consumeIdent();
        attribText << attribName;

        std::vector<ArgType> attribArgs{};
        if(m_file.tryConsume("("))
        {
            attribText << "(";
            const auto startPos = m_file.pos();
            attribArgs = parseArgs();
            attribText << m_file.subStr(startPos, m_file.pos());
            m_file.tryConsume(')');
            attribText << ")";
        }

        m_file.tryConsume("]");
        attribText << "]";
        if(doubleOpen)
            m_file.tryConsume("]");
        else
            m_cleaned << attribText.str();

        ShaderAttribute attrib;
        attrib.name = attribName;
        attrib.args = attribArgs;
        return attrib;
    }

    std::optional<ShaderDeclaration> ShaderPreprocessor::tryParseDeclaration()
    {
        const auto startPos = m_file.pos();

        const auto type = std::string(m_file.consumeIdent());
        if (type.empty())
            return std::nullopt;

        std::vector<ArgType> templateArgs;
        if (m_file.tryConsume('<'))
        {
            templateArgs = parseArgs();
            m_file.tryConsume('>');
        }

        m_file.skipWhitespace();
        const auto name = std::string(m_file.consumeIdent());
        if (name.empty())
        {
            m_file.seek(startPos); // not a declaration
            return std::nullopt;
        }

        m_file.skipWhitespace();

        if (m_file.peek() == '(')
            return finishFunctionDecl(type, name);

        if(type == "struct")
            return finishStructDecl(name);

        if(isResourceType(type))
            return finishResource(type, templateArgs, name);

        m_file.seek(startPos);
        return std::nullopt;
    }

    ResourceDeclaration ShaderPreprocessor::finishResource(const std::string &type, const std::vector<ArgType> &templateArgs, const std::string &name)
    {
        m_cleaned << type;
        if(!templateArgs.empty())
        {
            m_cleaned << '<';
            bool first = true;

            for(const auto& arg : templateArgs)
            {
                if(!first)
                    m_cleaned << ", ";
                first = false;

                if(const auto strArg = std::get_if<std::string>(&arg))
                    m_cleaned << *strArg;
                if(const auto intArg = std::get_if<int>(&arg))
                    m_cleaned << *intArg;
            }
            m_cleaned << '>';
        }

        m_cleaned << ' ' << name;

        uint32_t arraySize = 0;
        if(m_file.tryConsume('['))
        {
            std::string arraySizeStr;
            if(std::isdigit(m_file.peek()))
            {
                arraySizeStr = std::string(m_file.consumeIntLiteral());
                std::from_chars(arraySizeStr.data(), arraySizeStr.data() + arraySizeStr.size(), arraySize);
            }
            m_file.tryConsume(']');
            m_cleaned << '[' << arraySizeStr << ']';
        }

        m_file.tryConsume(';');

        const char regSpace = getRegisterSpace(type);
        const int regNumber = m_registerCounters[regSpace]++;
        m_cleaned << " : register(" << regSpace << regNumber << ");\n";

        ResourceDeclaration decl{};
        decl.name = name;
        decl.type = type;
        decl.templateArgs = templateArgs;
        decl.arraySize = arraySize;

        return decl;
    }

    FuncDeclaration ShaderPreprocessor::finishFunctionDecl(const std::string &type, const std::string &name)
    {
        FuncDeclaration decl{};
        decl.name = name;
        decl.returnType = type;

        m_file.tryConsume('(');
        const auto argsText = m_file.readUntil(")");
        m_file.tryConsume(')');

        m_cleaned << '\n' << type << ' ' << name << '(' << argsText << ")";

        if(m_file.tryConsume(':'))
        {
            m_cleaned << " : " << m_file.consumeIdent();
        }

        m_file.tryConsume('{');

        int depth = 1;
        const auto bodyStart = m_file.pos();
        while (!m_file.done() && depth > 0)
        {
            const char c = m_file.peek();
            m_file.skip();
            if (c == '{') depth++;
            else if (c == '}') depth--;
        }
        const auto body = m_file.subStr(bodyStart, m_file.pos() - 1);

        m_cleaned << "\n{" << body << "}\n";

        return decl;
    }

    StructDeclaration ShaderPreprocessor::finishStructDecl(const std::string& name)
    {
        m_file.skipWhitespace();

        m_cleaned << "struct " << name << ' ';

        m_file.skipWhitespace();
        m_file.tryConsume('{');

        int depth = 1;
        const auto bodyStart = m_file.pos();
        while (!m_file.done() && depth > 0)
        {
            char c = m_file.peek();
            m_file.skip();
            if (c == '{') depth++;
            else if (c == '}') depth--;
        }
        const auto body = m_file.subStr(bodyStart, m_file.pos() - 1);

        m_file.tryConsume(';');

        m_cleaned << "\n{" << body << "};\n";
        return StructDeclaration{name};
    }

    std::vector<ArgType> ShaderPreprocessor::parseArgs()
    {
        std::vector<ArgType> args;

        while (true)
        {
            m_file.skipWhitespace();

            if (m_file.peek() == '"')
            {
                args.emplace_back(std::string(m_file.consumeStringLiteral()));
            }
            else if (std::isdigit(m_file.peek()) || m_file.peek() == '+' || m_file.peek() == '-')
            {
                auto text = m_file.consumeIntLiteral();
                int value;
                if (std::from_chars(text.data(), text.data() + text.size(), value).ec == std::errc{})
                    args.emplace_back(value);
            }
            else if (std::isalpha(m_file.peek()) || m_file.peek() == '_')
            {
                args.emplace_back(std::string(m_file.consumeIdent()));   // <-- new: type names, "FrameConstants" etc.
            }
            else
            {
                break;
            }

            m_file.skipWhitespace();
            if (!m_file.tryConsume(','))
                break;
        }

        return args;
    }

    bool ShaderPreprocessor::isResourceType(std::string_view type)
    {
        return type.starts_with("Texture") ||
               type.starts_with("RW") ||
               type.starts_with("Sampler") ||
               type.starts_with("Buffer") ||
               type.starts_with("StructuredBuffer") ||
               type.starts_with("ByteAddressBuffer") ||
               type.starts_with("AppendStructuredBuffer") ||
               type.starts_with("ConsumeStructuredBuffer") ||
               type.starts_with("ConstantBuffer") ||
               type.starts_with("RaytracingAccelerationStructure");
    }

    char ShaderPreprocessor::getRegisterSpace(const std::string_view type)
    {
        if(type.starts_with("RW"))            return 'u';
        if(type == "AppendStructuredBuffer")       return 'u';
        if(type == "ConsumeStructuredBuffer")      return 'u';
        if(type.starts_with("Sampler"))       return 's';
        if(type == "ConstantBuffer")               return 'b';
        return 't';
    }
}
