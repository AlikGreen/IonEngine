#include "GpuMaterialRegistry.h"
#include "asset/assetRegistry.h"
#include "core/engine.h"

namespace ion
{
    GpuMaterialRegistry::GpuMaterialRegistry(const dg::Ref<dg::IRenderDevice> &device)
        : m_device(device)
    {
        dg::BufferDesc desc{};
        desc.Size      = sizeof(uint32_t) * 512;
        desc.Usage     = dg::USAGE_DEFAULT;
        desc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        desc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        desc.ElementByteStride = sizeof(uint32_t);

        m_device->CreateBuffer(desc, nullptr, &m_templateBaseOffsetsBuffer);

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

    void GpuMaterialRegistry::updateMaterial(const AssetRef<MaterialInstance> &mat, const dg::Ref<dg::IDeviceContext> &ctx)
    {
        if(!mat->dirty()) return;

        const auto templ = mat->materialTemplate();

        const uint32_t propsBufSize = templ->propertiesBufferSize();

        auto& buffer = m_materialBuffers[mat.id()];
        if(buffer == nullptr)
        {
            dg::BufferDesc desc{};
            desc.Size      = propsBufSize * 512;
            desc.Usage     = dg::USAGE_DEFAULT;
            desc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;

            m_device->CreateBuffer(desc, nullptr, &buffer);
        }

        const auto it = m_materialEntries.find(mat.id());
        if(it == m_materialEntries.end())
            return;


        ctx->UpdateBuffer(
            buffer,
            it->second.materialIndex * propsBufSize,
            mat->cpuBuffer().size(),
            mat->cpuBuffer().data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );

        mat->dirty(false);
    }

    void GpuMaterialRegistry::syncLayout(const dg::Ref<dg::IDeviceContext> &ctx)
    {
        m_templateBaseOffsets.resize(m_templateInfos.size(), 0);

        uint32_t offset = 0;
        for (const auto& info : m_templateInfos)
        {
            m_templateBaseOffsets[info.index] = offset;
            offset += info.totalPrimitiveCount;
        }

        ctx->UpdateBuffer(
            m_templateBaseOffsetsBuffer,
            0,
            m_templateBaseOffsets.size() * sizeof(uint32_t),
            m_templateBaseOffsets.data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );
    }

}
