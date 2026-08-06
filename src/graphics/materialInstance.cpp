#include "materialInstance.h"

#include "graphicsSystem.h"
#include "core/engine.h"

namespace ion
{
    MaterialInstance::MaterialInstance(const AssetRef<MaterialTemplate>& templ)
        : m_template(templ)
    {
        m_cpuBuffer.resize(m_template->propertiesBufferSize(), 0);

        if (m_template->propertiesBufferSize() > 0)
        {
            const auto device = Engine::getSystem<GraphicsSystem>()->device();

            dg::BufferDesc cameraBufferDesc{};
            cameraBufferDesc.Name      = "Material property buffer";
            cameraBufferDesc.Size      = sizeof(m_template->propertiesBufferSize());
            cameraBufferDesc.Usage     = dg::USAGE_DYNAMIC;
            cameraBufferDesc.BindFlags = dg::BIND_UNIFORM_BUFFER;
            device->CreateBuffer(cameraBufferDesc, nullptr, &m_propertyBuffer);
        }
    }

    bool MaterialInstance::setTexture(const std::string &name, const AssetRef<Image> &image)
    {
        const std::string qualifiedName = "material."+name;
        const auto& resources = m_template->resources();
        const auto it = resources.find(qualifiedName);
        if (it == resources.end() || it->second.type != dg::SHADER_RESOURCE_TYPE_TEXTURE_SRV)
            return false;

        m_textures[name] = image;
        return true;
    }

    bool MaterialInstance::setSampler(const std::string &name, const AssetRef<Image> &image)
    {
        const std::string qualifiedName = "material."+name;
        const auto& resources = m_template->resources();
        const auto it = resources.find(qualifiedName);
        if (it == resources.end() || it->second.type != dg::SHADER_RESOURCE_TYPE_SAMPLER)
            return false;

        m_samplers[name] = image;
        return true;
    }


    // void MaterialInstance::applyBindings(const dg::Ref<dg::IDeviceContext>& ctx)
    // {
    //     if (m_dirty && m_propertyBuffer && !m_cpuBuffer.empty())
    //     {
    //         cmd->updateBuffer(m_propertyBuffer, m_cpuBuffer);
    //         m_dirty = false;
    //     }
    //
    //     if (m_propertyBuffer)
    //         pass.setBuffer("material", m_propertyBuffer);
    //
    //     std::unordered_set<std::string> boundNames;
    //     for (const auto& [name, image] : m_textures)
    //     {
    //         pass.setTexture("material."+name, image->textureView());
    //         boundNames.insert("material."+name);
    //     }
    //
    //     for (const auto& [name, image] : m_samplers)
    //     {
    //         pass.setSampler("material."+name, image->sampler());
    //         boundNames.insert("material."+name);
    //     }
    //
    //     // Bind defaults for unbound resources
    //     for (const auto& [name, res] : m_template->resources())
    //     {
    //         if (boundNames.contains(name))
    //             continue;
    //
    //         switch (res.type)
    //         {
    //             case SlangResType::Texture:
    //                 pass.setTexture(name, m_template->defaultTexture());
    //             break;
    //             case SlangResType::Sampler:
    //                 pass.setSampler(name, m_template->defaultSampler());
    //             break;
    //             default:
    //                 break;
    //         }
    //     }
    // }

    void MaterialInstance::setPropertyBuffer(const std::vector<uint8_t> &data)
    {
        const size_t copySize = std::min(data.size(), m_cpuBuffer.size());
        std::memcpy(m_cpuBuffer.data(), data.data(), copySize);
        m_dirty = true;
    }
}
