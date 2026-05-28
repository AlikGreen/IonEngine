#pragma once
#include "asset/assetSerializer.h"
#include "slang/compiler.h"

namespace ion
{
class SlangModuleSerializer final : public AssetSerializer<urhi::slang::Module>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("urhi::slang::Module");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const urhi::slang::Module &module) override;
    grl::Rc<urhi::slang::Module> deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
