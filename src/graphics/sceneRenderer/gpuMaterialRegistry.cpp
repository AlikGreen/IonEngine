#include "GpuMaterialRegistry.h"
#include "asset/assetRegistry.h"
#include "core/engine.h"

namespace ion
{
    GpuMaterialRegistry::GpuMaterialRegistry(const grl::Rc<urhi::Device> &device)
        : m_device(device)
    {
        m_templateBaseOffsetsBuffer = m_device->createBuffer({urhi::BufferUsage::Storage, sizeof(uint32_t) * 512});

        auto& assets = Engine::assetRegistry();
        m_defaultMaterial = assets.create<MaterialInstance>(MaterialTemplates::pbr());
    }

    const GpuMaterialRegistry::MaterialEntry& GpuMaterialRegistry::registerPrimitiveMaterial(const AssetRef<MaterialInstance> &mat)
    {
        auto matIt = m_materialEntries.find(mat.id());
        if(matIt == m_materialEntries.end())
        {
            const auto templ = mat->materialTemplate();


            auto [templIt, inserted] = m_templateIndexLookup.emplace(templ.id(), m_templateInfos.size());
            if(inserted)
            {
                templIt->second = m_templateInfos.size();
                m_templateInfos.push_back(TemplateInfo{static_cast<uint32_t>(m_templates.size())});
                m_templates.push_back(templ);
            }

            auto& templInfo = m_templateInfos.at(templIt->second);

            const uint32_t matIndex = templInfo.totalMaterialCount++;
            auto [newIt, _] = m_materialEntries.emplace(mat.id(), MaterialEntry{ templInfo.index, matIndex });
            matIt = newIt;
        }

        m_templateInfos[matIt->second.templateIndex].totalPrimitiveCount++;

        return matIt->second;
    }

    void GpuMaterialRegistry::updateMaterial(const AssetRef<MaterialInstance> &mat, const grl::Rc<urhi::CommandList> &cmd)
    {
        if(!mat->dirty()) return;

        const auto templ = mat->materialTemplate();

        const uint32_t propsBufSize = templ->propertiesBufferSize();

        auto& buffer = m_materialBuffers[mat.id()];
        if(buffer == nullptr)
            buffer = m_device->createBuffer({urhi::BufferUsage::Storage, propsBufSize * 512});

        const auto it = m_materialEntries.find(mat.id());
        if(it == m_materialEntries.end())
            return;

        cmd->updateBuffer(buffer, mat->cpuBuffer(), it->second.materialIndex * propsBufSize);

        mat->dirty(false);
    }

    void GpuMaterialRegistry::syncLayout(const grl::Rc<urhi::CommandList> &cmd)
    {
        m_templateBaseOffsets.resize(m_templateInfos.size(), 0);

        uint32_t offset = 0;
        for (const auto& info : m_templateInfos)
        {
            m_templateBaseOffsets[info.index] = offset;
            offset += info.totalPrimitiveCount;
        }
        cmd->updateBuffer(m_templateBaseOffsetsBuffer, m_templateBaseOffsets);
    }

}
