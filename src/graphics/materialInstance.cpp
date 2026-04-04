#include "materialInstance.h"

#include "graphicsSystem.h"

namespace ion
{
    MaterialInstance::MaterialInstance(const AssetRef<MaterialTemplate>& templ)
        : m_template(templ)
    {
        m_cpuBuffer.resize(m_template->propertiesBufferSize(), 0);

        if (m_template->propertiesBufferSize() > 0)
        {
            const auto device = Engine::getSystem<GraphicsSystem>()->getDevice();
            m_propertyBuffer = device->createBuffer({
                urhi::BufferUsage::Uniform,
                m_template->propertiesBufferSize()
            });
        }
    }

    bool MaterialInstance::setTexture(const std::string &name, const AssetRef<Image> &image)
    {
        const auto& resources = m_template->resources();
        const auto it = resources.find(name);
        if (it == resources.end() || it->second.type != urhi::ShaderReflection::ResourceType::Texture)
            return false;

        m_textures[name] = image;
        return true;
    }

    bool MaterialInstance::setSampler(const std::string &name, const AssetRef<Image> &image)
    {
        const auto& resources = m_template->resources();
        const auto it = resources.find(name);
        if (it == resources.end() || it->second.type != urhi::ShaderReflection::ResourceType::Sampler)
            return false;

        m_samplers[name] = image;
        return true;
    }

    void MaterialInstance::bind(const grl::Rc<urhi::CommandList> &cmd, const grl::Rc<urhi::RenderPass> &pass)
    {
        if (m_dirty && m_propertyBuffer && !m_cpuBuffer.empty())
        {
            cmd->updateBuffer(m_propertyBuffer, m_cpuBuffer);
            m_dirty = false;
        }

        if (m_propertyBuffer)
            pass->setUniformBuffer("properties", m_propertyBuffer);

        std::unordered_set<std::string> boundNames;
        for (const auto& [name, image] : m_textures)
        {
            pass->setTexture(name, image->textureView());
            boundNames.insert(name);
        }

        for (const auto& [name, image] : m_samplers)
        {
            pass->setSampler(name, image->sampler());
            boundNames.insert(name);
        }

        // Bind defaults for unbound resources
        for (const auto& [name, res] : m_template->resources())
        {
            if (boundNames.contains(name))
                continue;

            switch (res.type)
            {
                case urhi::ShaderReflection::ResourceType::Texture:
                    pass->setTexture(name, m_template->defaultTexture());
                break;
                case urhi::ShaderReflection::ResourceType::Sampler:
                    pass->setSampler(name, m_template->defaultSampler());
                break;
                default:
                    break;
            }
        }
    }

    void MaterialInstance::setPropertyBuffer(const std::vector<uint8_t> &data)
    {
        const size_t copySize = std::min(data.size(), m_cpuBuffer.size());
        std::memcpy(m_cpuBuffer.data(), data.data(), copySize);
        m_dirty = true;
    }
}
