#include "shaderImporter.h"

#include "shaderCompiler.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    void* ShaderImporter::load(const std::string &filepath)
    {
        const auto& device = Engine::getSystem<GraphicsSystem>()->getDevice();

        const std::string dir = grl::Path::directory(filepath);

        urhi::ShaderCompileDescription compileDesc{};
        compileDesc.path = filepath;
        compileDesc.source = grl::File::read(filepath).value();
        compileDesc.includePaths.push_back(dir);

        auto spirv = urhi::ShaderCompiler::compile(compileDesc);
        auto shader = device->createShader(spirv);
        shader->compile();
        return new grl::Rc(std::move(shader));
    }
}
