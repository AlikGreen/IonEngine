#include "materialTemplateSerializer.h"

#include "asset/assetDeps.h"
#include "asset/assetStream.h"

namespace ion
{
    void MaterialTemplateSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps,
        const MaterialTemplate &material)
    {
        assetStream.write(material.name());
        assetStream.write(material.isOpaque());
        assetStream.write(material.isLit());

        deps.require(material.m_module);
        assetStream.write(material.m_module.id());
    }

    grl::Rc<MaterialTemplate> MaterialTemplateSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        std::string name{};
        assetStream.read(name);
        bool isOpaque{};
        assetStream.read(isOpaque);
        bool isLit{};
        assetStream.read(isLit);

        AssetId moduleId{};
        assetStream.read(moduleId);

        auto module = assetRegistry.load<urhi::slang::Module>(moduleId);

        return grl::makeRc<MaterialTemplate>(name, isOpaque, isLit, module);
    }
}
