#include "slangModuleSerializer.h"

#include "asset/assetStream.h"

namespace ion
{
    void SlangModuleSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const urhi::slang::Module &module)
    {
        assetStream.write(module.name);
        assetStream.write(module.path);
        assetStream.write(module.ir);
    }

    grl::Rc<urhi::slang::Module> SlangModuleSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        auto module = grl::makeRc<urhi::slang::Module>();

        assetStream.read(module->name);
        assetStream.read(module->path);
        assetStream.read(module->ir);

        return module;
    }
}
