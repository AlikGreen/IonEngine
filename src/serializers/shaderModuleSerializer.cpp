#include "shaderModuleSerializer.h"

#include "asset/assetStream.h"

namespace ion
{
    void ShaderModuleSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const ShaderModule &module)
    {
        assetStream.write(module.name);
        assetStream.write(module.path);
        assetStream.write(module.source);
    }

    grl::Rc<ShaderModule> ShaderModuleSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        grl::Rc<ShaderModule> module = grl::makeRc<ShaderModule>();

        assetStream.read(module->name);
        assetStream.read(module->path);
        assetStream.read(module->source);

        return module;
    }
}
