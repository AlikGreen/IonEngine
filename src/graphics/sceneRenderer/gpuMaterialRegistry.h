#pragma once
#include "asset/assetRef.h"
#include "graphics/materialInstance.h"

namespace ion
{
    class GpuMaterialRegistry
    {
    public:
        struct TemplateInfo
        {
            uint32_t index{};
            uint32_t totalMaterialCount{};
            uint32_t totalPrimitiveCount{};
        };

        struct MaterialEntry
        {
            uint32_t templateIndex{};
            uint32_t materialIndex{};
        };

        explicit GpuMaterialRegistry(const dg::Ref<dg::IRenderDevice> &device);

        const MaterialEntry& registerPrimitiveMaterial(const AssetRef<MaterialInstance>& mat);
        void updateMaterial(const AssetRef<MaterialInstance>& mat, const dg::Ref<dg::IDeviceContext>& ctx);
        void syncLayout(const dg::Ref<dg::IDeviceContext>& ctx);

        AssetRef<MaterialInstance> defaultMaterial() { return m_defaultMaterial; }

        [[nodiscard]] const dg::Ref<dg::IBuffer>& templateBaseOffsetsBuffer() const { return m_templateBaseOffsetsBuffer; }

        const std::vector<AssetRef<MaterialTemplate>>& templates() const { return m_templates; }
        const std::vector<TemplateInfo>& templateInfos() const { return m_templateInfos; }
    private:
        AssetRef<MaterialInstance> m_defaultMaterial;

        std::unordered_map<AssetId, MaterialEntry> m_materialEntries;

        std::unordered_map<AssetId, uint32_t> m_templateIndexLookup;
        std::vector<TemplateInfo> m_templateInfos;
        std::vector<AssetRef<MaterialTemplate>> m_templates;

        std::unordered_map<AssetId, dg::Ref<dg::IBuffer>> m_materialBuffers;

        std::vector<uint32_t> m_templateBaseOffsets;

        dg::Ref<dg::IRenderDevice> m_device;

        dg::Ref<dg::IBuffer> m_templateBaseOffsetsBuffer;
    };
}
