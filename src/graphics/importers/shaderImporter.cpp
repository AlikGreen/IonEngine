#include "shaderImporter.h"

#include <slangCompiler.h>

#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    grl::Box<urhi::ShaderSet> ShaderImporter::import(const std::filesystem::path &src, ImportOptions<urhi::ShaderSet> options)
    {
        const std::string dir = grl::Path::directory(src.string());

        urhi::SlangCompileDesc compileDesc{};
        compileDesc.modules.push_back(
        {
                src.stem().string(),
                src.string(),
                grl::File::read(src.string()).value(),
        });
        compileDesc.includePaths.push_back(dir);

        auto entryPoints = urhi::SlangCompiler::compile(compileDesc);
        return grl::makeBox<urhi::ShaderSet>(std::move(entryPoints));
    }

    bool ShaderImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".slang" || src.extension() == ".shader";
    }
}
