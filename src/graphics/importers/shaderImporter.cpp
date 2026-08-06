#include "shaderImporter.h"

#include "core/engine.h"
#include "core/resourceFs.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    grl::Box<ShaderModule> ShaderImporter::import(const std::filesystem::path &path, const NoOptions &)
    {
        const auto source = grl::File::read(path.string());
        clogr::ensure(source.has_value(), "Failed to read shader file: {}", path.string());

        const ShaderModule module{source.value()};
        ShaderProcessDesc desc{};
        desc.includePaths = { path.parent_path() };

        ShaderPreprocessor preprocessor{module, desc};
        std::string processedSource = preprocessor.process(ShaderPreprocessMode::ResolveOnly).hlsl;

        return grl::makeBox<ShaderModule>(processedSource, path.string(), path.stem().string());
    }

    bool ShaderImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".hlsl" || src.extension() == ".hlsli" || src.extension() == ".shader";
    }
}
