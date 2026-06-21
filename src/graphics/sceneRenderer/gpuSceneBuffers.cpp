#include "gpuSceneBuffers.h"

#include "DrawIndexedIndirectCommand.h"
#include "GpuMaterialRegistry.h"
#include "core/components/transformComponent.h"

namespace ion
{
    GpuSceneBuffers::GpuSceneBuffers(const grl::Rc<urhi::Device> &device)
        : m_device(device)
    {
        constexpr uint32_t kMaxMeshes = 1024*8;
        constexpr uint32_t kMaxPrimitives = 1024*16;
        constexpr uint32_t kMaxMeshlets = 1024*256;

        constexpr uint32_t kMaxVertices = 1024*1024*16;
        constexpr uint32_t kMaxIndices = 1024*1024*32;

        // these values should probably be tuned/data be limited correctly/limits change per device specs

        m_globalIndexBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(uint32_t)*kMaxIndices});
        m_globalVertexBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Vertex, sizeof(Vertex)*kMaxVertices});

        m_globalModelDataBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(ModelData)*kMaxMeshes});
        m_globalMeshletBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(Meshlet)*kMaxMeshlets});
        m_globalMeshletInstanceBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(MeshletInstance)*kMaxMeshlets});

        m_globalPrimitiveBuffer = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(GpuMeshPrimitive)*kMaxPrimitives});
    }

    void GpuSceneBuffers::upload(const entis::Entity entity, const MeshRenderer &mr, GpuMaterialRegistry& matRegistry, const grl::Rc<urhi::CommandList> &cmd)
    {
        auto& mesh = *mr.mesh;

        mesh.apply();

        std::vector<MeshletInstance> meshletInstances;
        meshletInstances.reserve(mesh.meshlets().size());

        std::vector<GpuMeshPrimitive> meshPrimitives(mesh.meshletPrimitives().size());

        const auto alloc = getOrCreateMeshAllocation(mr.mesh, cmd);
        const uint32_t transformIndex = updateTransform(entity, cmd);

        for (size_t i = 0; i < mesh.meshletPrimitives().size(); i++)
        {
            const auto& meshletPrim = mesh.meshletPrimitives().at(i);

            auto material = mr.materials.at(meshletPrim.materialIndex);
            if(material == nullptr)
                material = matRegistry.defaultMaterial();

            auto& mat = matRegistry.registerPrimitiveMaterial(material);

            GpuMeshPrimitive prim{};
            prim.materialIndex = mat.materialIndex;
            prim.templateIndex = mat.templateIndex;
            prim.transformIndex = transformIndex;
            prim.meshletStart = alloc.subMeshes.at(i).meshletStart;
            prim.meshletCount = alloc.subMeshes.at(i).meshletCount;
            prim.vertexStart = alloc.vertexStart;
            prim.indexStart  = alloc.indexStart;
            meshPrimitives[i] = prim;

            for (uint32_t j = 0; j < meshletPrim.meshletCount; j++)
            {
                MeshletInstance inst{};
                inst.primitiveId = m_primitiveCount + i;
                inst.meshletIndex = alloc.meshletStart + meshletPrim.meshletStart + j;
                meshletInstances.push_back(inst);
            }
        }

        cmd->updateBuffer(m_globalMeshletInstanceBuffer, meshletInstances, m_meshletInstanceCount * sizeof(MeshletInstance));
        m_meshletInstanceCount += meshletInstances.size();

        cmd->updateBuffer(m_globalPrimitiveBuffer, meshPrimitives, m_primitiveCount * sizeof(GpuMeshPrimitive));
        m_primitiveCount += mesh.meshletPrimitives().size();
    }

    void GpuSceneBuffers::unload(MeshRenderer &mr)
    {
        clogr::ensure(false, "Not implemented");
    }

    uint32_t GpuSceneBuffers::updateTransform(entis::Entity entity, const grl::Rc<urhi::CommandList> &cmd)
    {
        ModelData modelData{};
        modelData.model = Transform::getWorldMatrix(entity);
        modelData.normalMatrix = glm::transpose(glm::inverse(modelData.model));

        uint32_t index{};

        const auto it = m_entityIndexMap.find(entity);
        if(it != m_entityIndexMap.end())
        {
            index = it->second;
        }else if(m_freeEntityIndices.empty())
        {
            index = m_nextEntityIndex++;
            m_entityIndexMap.emplace(entity, index);
        }else
        {
            index = m_freeEntityIndices.back();
            m_freeEntityIndices.pop_back();
            m_entityIndexMap.emplace(entity, index);
        }

        cmd->updateBuffer(m_globalModelDataBuffer,  modelData, index * sizeof(ModelData));
        return index;
    }

    const GpuSceneBuffers::MeshAllocation& GpuSceneBuffers::getOrCreateMeshAllocation(const AssetRef<Mesh> &mesh, const grl::Rc<urhi::CommandList> &cmd)
    {
        if(const auto it = m_meshAllocations.find(mesh.id()); it != m_meshAllocations.end())
            return it->second;

        MeshAllocation alloc{};
        alloc.indexStart = m_nextFreeGlobalIndex; // maybe change to function acquireNextFreeGlobalIndex so that space can be reused later
        alloc.indexCount = mesh->indices().size();
        m_nextFreeGlobalIndex += alloc.indexCount;

        alloc.vertexStart = m_nextFreeGlobalVertex;
        alloc.vertexCount = mesh->vertices().size();
        m_nextFreeGlobalVertex += alloc.vertexCount;

        alloc.meshletStart = m_nextFreeGlobalMeshlet;
        alloc.meshletCount = mesh->meshlets().size();
        m_nextFreeGlobalMeshlet += alloc.meshletCount;

        cmd->updateBuffer(m_globalIndexBuffer, mesh->indices(), alloc.indexStart * sizeof(uint32_t));
        cmd->updateBuffer(m_globalVertexBuffer, mesh->vertices(), alloc.vertexStart * sizeof(Vertex));
        cmd->updateBuffer(m_globalMeshletBuffer, mesh->meshlets(), alloc.meshletStart * sizeof(Meshlet));

        alloc.subMeshes.reserve(mesh->meshletPrimitives().size());

        for(const auto prim : mesh->meshletPrimitives())
        {
            SubMeshAllocation subAlloc{};
            subAlloc.meshletStart = alloc.meshletStart + prim.meshletStart;
            subAlloc.meshletCount = prim.meshletCount;

            alloc.subMeshes.push_back(subAlloc);
        }

        auto [it, inserted] = m_meshAllocations.emplace(mesh.id(), alloc);
        return it->second;
    }
}
