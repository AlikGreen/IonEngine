#pragma once
#include <urhi/urhi.h>
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

        explicit GpuMaterialRegistry(const grl::Rc<urhi::Device> &device);

        const MaterialEntry& registerPrimitiveMaterial(const AssetRef<MaterialInstance>& mat);
        void updateMaterial(const AssetRef<MaterialInstance>& mat, const grl::Rc<urhi::CommandList>& cmd);
        void syncLayout(const grl::Rc<urhi::CommandList>& cmd);

        AssetRef<MaterialInstance> defaultMaterial() { return m_defaultMaterial; }

        [[nodiscard]] const grl::Rc<urhi::Buffer>& templateBaseOffsetsBuffer() const { return m_templateBaseOffsetsBuffer; }

        const std::vector<AssetRef<MaterialTemplate>>& templates() const { return m_templates; }
        const std::vector<TemplateInfo>& templateInfos() const { return m_templateInfos; }
    private:
        AssetRef<MaterialInstance> m_defaultMaterial;

        std::unordered_map<AssetId, MaterialEntry> m_materialEntries;

        std::unordered_map<AssetId, uint32_t> m_templateIndexLookup;
        std::vector<TemplateInfo> m_templateInfos;
        std::vector<AssetRef<MaterialTemplate>> m_templates;

        std::unordered_map<AssetId, grl::Rc<urhi::Buffer>> m_materialBuffers;

        std::vector<uint32_t> m_templateBaseOffsets;

        grl::Rc<urhi::Device> m_device;

        grl::Rc<urhi::Buffer> m_templateBaseOffsetsBuffer;
    };
}
