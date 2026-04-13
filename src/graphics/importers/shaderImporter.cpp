#include "shaderImporter.h"

#include "core/engine.h"
#include "core/resourceFs.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    grl::Box<urhi::slang::Module> ShaderImporter::import(
        const std::filesystem::path& src,
        const ShaderImportOpts& options)
    {
        auto source = grl::File::read(src.string());
        clogr::ensure(source.has_value(), "Failed to read shader file: {}", src.string());

        urhi::slang::CompileDesc compileDesc{};
        compileDesc.moduleName = src.stem().string();
        compileDesc.modulePath = src.string();

        compileDesc.optimize = true;
        compileDesc.includePaths = options.includeDirs;
        compileDesc.includePaths.push_back(src.parent_path().string());
        compileDesc.defines = options.defines;

        compileDesc.moduleSource = source.value();

        urhi::slang::Diagnostics diags;
        auto module = urhi::slang::Compiler::compileModule(compileDesc, &diags);
        return grl::makeBox<urhi::slang::Module>(std::move(module));
    }

    bool ShaderImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".slang" || src.extension() == ".shader";
    }
}
