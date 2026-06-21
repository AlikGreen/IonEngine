#pragma once
#include "GpuMaterialRegistry.h"
#include "../../../external/entis/src/entity.h"
#include "graphics/mesh.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
class GpuSceneBuffers
{
public:
    struct ModelData
    {
        glm::mat4 model{};
        glm::mat4 normalMatrix{};
    };

    struct MeshletInstance
    {
        uint32_t meshletIndex;
        uint32_t primitiveId;
    };

    struct GpuMeshPrimitive
    {
        uint32_t meshletStart;
        uint32_t meshletCount;
        uint32_t transformIndex;
        uint32_t materialIndex;
        uint32_t templateIndex;
        uint32_t vertexStart;
        uint32_t indexStart;
        uint32_t padding;
    };

    explicit GpuSceneBuffers(const grl::Rc<urhi::Device> &device);

    void upload(entis::Entity entity, const MeshRenderer& mr, GpuMaterialRegistry& matRegistry, const grl::Rc<urhi::CommandList>& cmd);
    void unload(MeshRenderer& mr);

    uint32_t updateTransform(entis::Entity entity, const grl::Rc<urhi::CommandList>& cmd);

    [[nodiscard]] uint32_t meshletInstanceCount() const { return m_meshletInstanceCount; }
    [[nodiscard]] uint32_t primitiveCount() const { return m_primitiveCount; }

    [[nodiscard]] const grl::Rc<urhi::Buffer>& vertexBuffer() const { return m_globalVertexBuffer; }
    [[nodiscard]] const grl::Rc<urhi::Buffer>& indexBuffer() const { return m_globalIndexBuffer; }
    [[nodiscard]] const grl::Rc<urhi::Buffer>& meshletBuffer() const { return m_globalMeshletBuffer; }
    [[nodiscard]] const grl::Rc<urhi::Buffer>& primitiveBuffer() const { return m_globalPrimitiveBuffer; }
    [[nodiscard]] const grl::Rc<urhi::Buffer>& meshletInstanceBuffer() const { return m_globalMeshletInstanceBuffer; }
    [[nodiscard]] const grl::Rc<urhi::Buffer>& transformBuffer() const { return m_globalModelDataBuffer; }
private:
    struct SubMeshAllocation
    {
        uint32_t meshletStart;
        uint32_t meshletCount;
    };

    struct MeshAllocation
    {
        uint32_t indexStart;
        uint32_t indexCount;
        uint32_t vertexStart;
        uint32_t vertexCount;
        uint32_t meshletStart;
        uint32_t meshletCount;
        std::vector<SubMeshAllocation> subMeshes;
    };

    const MeshAllocation& getOrCreateMeshAllocation(const AssetRef<Mesh> &mesh, const grl::Rc<urhi::CommandList>& cmd);

    std::unordered_map<entis::Entity, uint32_t> m_entityIndexMap;
    std::vector<uint32_t> m_freeEntityIndices;
    uint32_t m_nextEntityIndex = 0;

    std::unordered_map<AssetId, MeshAllocation> m_meshAllocations;

    uint32_t m_primitiveCount = 0;
    uint32_t m_meshletInstanceCount = 0;

    grl::Rc<urhi::Device> m_device;

    grl::Rc<urhi::Buffer> m_globalIndexBuffer;
    grl::Rc<urhi::Buffer> m_globalVertexBuffer;
    grl::Rc<urhi::Buffer> m_globalModelDataBuffer;
    grl::Rc<urhi::Buffer> m_globalMeshletBuffer;
    grl::Rc<urhi::Buffer> m_globalMeshletInstanceBuffer;
    grl::Rc<urhi::Buffer> m_globalPrimitiveBuffer;

    uint32_t m_nextFreeGlobalIndex = 0;
    uint32_t m_nextFreeGlobalVertex = 0;
    uint32_t m_nextFreeGlobalMeshlet = 0;
};
}
