#include "materialInstanceSerializer.h"

#include "asset/assetDeps.h"

namespace ion
{
    void MaterialInstanceSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const MaterialInstance &material)
    {
        deps.require(material.materialTemplate());
        assetStream.write(material.materialTemplate().id());

        assetStream.write(material.cpuBuffer());

        assetStream.write<uint32_t>(material.textures().size());

        for(auto& [name, texture] : material.textures())
        {
            assetStream.write(name);
            assetStream.write(texture.id());
            deps.require(texture);
        }

        assetStream.write<uint32_t>(material.samplers().size());

        for(auto& [name, sampler] : material.samplers())
        {
            assetStream.write(name);
            assetStream.write(sampler.id());
            deps.require(sampler);
        }
    }

    grl::Rc<MaterialInstance> MaterialInstanceSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        AssetId templateId{};
        assetStream.read(templateId);
        auto materialTemplate = assetRegistry.load<MaterialTemplate>(templateId);

        auto material = grl::makeRc<MaterialInstance>(materialTemplate);

        std::vector<uint8_t> cpuBuffer{};
        assetStream.read(cpuBuffer);

        material->setPropertyBuffer(cpuBuffer);

        uint32_t textureCount{};
        assetStream.read(textureCount);

        for(size_t i = 0; i < textureCount; i++)
        {
            std::string name;
            assetStream.read(name);
            AssetId imageId{};
            assetStream.read(imageId);
            auto image = assetRegistry.load<Image>(imageId);

            material->setTexture(name, image);
        }

        uint32_t samplerCount{};
        assetStream.read(samplerCount);

        for(size_t i = 0; i < samplerCount; i++)
        {
            std::string name;
            assetStream.read(name);
            AssetId imageId{};
            assetStream.read(imageId);
            auto image = assetRegistry.load<Image>(imageId);

            material->setSampler(name, image);
        }

        return material;
    }
}
