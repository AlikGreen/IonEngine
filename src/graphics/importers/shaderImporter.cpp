#include "shaderImporter.h"

#include <slangCompiler.h>

#include "core/engine.h"
#include "core/resourceFs.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    grl::Box<urhi::ShaderSet> ShaderImporter::import(
    const std::filesystem::path& src,
    const ShaderImportOpts& options)
    {
        const std::string dir = grl::Path::directory(src.string());

        urhi::SlangCompileDesc compileDesc{};

        // Check main file
        auto mainContent = grl::File::read(src.string());
        if (!mainContent.has_value()) {
            throw std::runtime_error("Failed to read main shader: " + src.string());
        }

        compileDesc.modules.push_back({
            src.stem().string(),
            src.string(),
            std::move(mainContent.value()),
        });

        const auto& rfs = Engine::resourceFS();

        for (const auto& path : options.additionalModulePaths)
        {
            auto resolvedPath = rfs.resolve(path).string();
            auto content = grl::File::read(resolvedPath);
            if (!content.has_value())
            {
                throw std::runtime_error("Failed to read additional module: " + path);
            }

            compileDesc.modules.push_back({
                grl::Path::stem(path),
                resolvedPath,
                std::move(content.value()),
            });
        }

        compileDesc.includePaths = options.includeDirs;
        compileDesc.includePaths.push_back(dir);
        compileDesc.typeSpecializations = options.typeSpecializations;

        auto entryPoints = urhi::SlangCompiler::compile(compileDesc);
        return grl::makeBox<urhi::ShaderSet>(std::move(entryPoints));
    }

    bool ShaderImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".slang" || src.extension() == ".shader";
    }
}
