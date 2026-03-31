#include "shaderImporter.h"

#include "shaderCompiler.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    grl::Box<std::vector<urhi::ShaderEntryPoint>> ShaderImporter::import(const std::filesystem::path &src)
    {
        const std::string dir = grl::Path::directory(src.string());

        urhi::ShaderCompileDesc compileDesc{};
        compileDesc.path = src.string();
        compileDesc.source = grl::File::read(src.string()).value();
        compileDesc.includePaths.push_back(dir);

        auto entryPoints = urhi::ShaderCompiler::compile(compileDesc);
        return grl::makeBox<std::vector<urhi::ShaderEntryPoint>>(std::move(entryPoints));
    }

    bool ShaderImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".slang" || src.extension() == ".shader";
    }
}
