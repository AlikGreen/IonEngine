#include "shaderImporter.h"

#include "shaderCompiler.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    void* ShaderImporter::load(const std::string &filepath)
    {
        const std::string dir = grl::Path::directory(filepath);

        urhi::ShaderCompileDesc compileDesc{};
        compileDesc.path = filepath;
        compileDesc.source = grl::File::read(filepath).value();
        compileDesc.includePaths.push_back(dir);

        auto entryPoints = urhi::ShaderCompiler::compile(compileDesc);
        return new std::vector(std::move(entryPoints));
    }
}
