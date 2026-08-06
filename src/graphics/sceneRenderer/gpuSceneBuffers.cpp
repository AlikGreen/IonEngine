#include "gpuSceneBuffers.h"

#include "GpuMaterialRegistry.h"
#include "core/components/transformComponent.h"

namespace ion
{
    GpuSceneBuffers::GpuSceneBuffers(const dg::Ref<dg::IRenderDevice> &device)
        : m_device(device)
    {
        constexpr uint32_t kMaxMeshes = 1024*8;
        constexpr uint32_t kMaxPrimitives = 1024*16;
        constexpr uint32_t kMaxMeshlets = 1024*256;

        constexpr uint32_t kMaxVertices = 1024*1024*16;
        constexpr uint32_t kMaxIndices = 1024*1024*32;

        dg::BufferDesc globalIndexDesc{};
        globalIndexDesc.Name              = "Global Index Buffer";
        globalIndexDesc.Size              = sizeof(uint32_t) * kMaxIndices;
        globalIndexDesc.Usage             = dg::USAGE_DEFAULT;
        globalIndexDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalIndexDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalIndexDesc.ElementByteStride = sizeof(uint32_t);
        m_device->CreateBuffer(globalIndexDesc, nullptr, &m_globalIndexBuffer);

        dg::BufferDesc globalVertexDesc{};
        globalVertexDesc.Name              = "Global Vertex Buffer";
        globalVertexDesc.Size              = sizeof(Vertex) * kMaxVertices;
        globalVertexDesc.Usage             = dg::USAGE_DEFAULT;
        globalVertexDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_VERTEX_BUFFER | dg::BIND_SHADER_RESOURCE;
        globalVertexDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalVertexDesc.ElementByteStride = sizeof(Vertex);
        m_device->CreateBuffer(globalVertexDesc, nullptr, &m_globalVertexBuffer);

        dg::BufferDesc globalModelDataDesc{};
        globalModelDataDesc.Name              = "Global Model Data Buffer";
        globalModelDataDesc.Size              = sizeof(ModelData) * kMaxMeshes;
        globalModelDataDesc.Usage             = dg::USAGE_DEFAULT;
        globalModelDataDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalModelDataDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalModelDataDesc.ElementByteStride = sizeof(ModelData);
        m_device->CreateBuffer(globalModelDataDesc, nullptr, &m_globalModelDataBuffer);

        dg::BufferDesc globalMeshletDesc{};
        globalMeshletDesc.Name              = "Global Meshlet Buffer";
        globalMeshletDesc.Size              = sizeof(Meshlet) * kMaxMeshlets;
        globalMeshletDesc.Usage             = dg::USAGE_DEFAULT;
        globalMeshletDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalMeshletDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalMeshletDesc.ElementByteStride = sizeof(Meshlet);
        m_device->CreateBuffer(globalMeshletDesc, nullptr, &m_globalMeshletBuffer);

        dg::BufferDesc globalMeshletInstDesc{};
        globalMeshletInstDesc.Name              = "Global Meshlet Instance Buffer";
        globalMeshletInstDesc.Size              = sizeof(MeshletInstance) * kMaxMeshlets;
        globalMeshletInstDesc.Usage             = dg::USAGE_DEFAULT;
        globalMeshletInstDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalMeshletInstDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalMeshletInstDesc.ElementByteStride = sizeof(MeshletInstance);
        m_device->CreateBuffer(globalMeshletInstDesc, nullptr, &m_globalMeshletInstanceBuffer);

        dg::BufferDesc globalPrimDesc{};
        globalPrimDesc.Name              = "Global Primitive Buffer";
        globalPrimDesc.Size              = sizeof(GpuMeshPrimitive) * kMaxPrimitives;
        globalPrimDesc.Usage             = dg::USAGE_DEFAULT;
        globalPrimDesc.BindFlags         = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalPrimDesc.Mode              = dg::BUFFER_MODE_STRUCTURED;
        globalPrimDesc.ElementByteStride = sizeof(GpuMeshPrimitive);
        m_device->CreateBuffer(globalPrimDesc, nullptr, &m_globalPrimitiveBuffer);
    }

    void GpuSceneBuffers::upload(const entis::Entity entity, const MeshRenderer &mr, GpuMaterialRegistry& matRegistry, const dg::Ref<dg::IDeviceContext>& ctx)
    {
        auto& mesh = *mr.mesh;

        mesh.apply();

        std::vector<MeshletInstance> meshletInstances;
        meshletInstances.reserve(mesh.meshlets().size());

        std::vector<GpuMeshPrimitive> meshPrimitives(mesh.meshletPrimitives().size());

        const auto alloc = getOrCreateMeshAllocation(mr.mesh, ctx);
        const uint32_t transformIndex = updateTransform(entity, ctx);

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

        ctx->UpdateBuffer(
            m_globalMeshletInstanceBuffer,
            m_meshletInstanceCount * sizeof(MeshletInstance),
            meshletInstances.size() * sizeof(MeshletInstance),
            meshletInstances.data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_meshletInstanceCount += meshletInstances.size();

        ctx->UpdateBuffer(
            m_globalPrimitiveBuffer,
            m_primitiveCount * sizeof(GpuMeshPrimitive),
            meshPrimitives.size() * sizeof(GpuMeshPrimitive),
            meshPrimitives.data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_primitiveCount += meshPrimitives.size();
    }

    void GpuSceneBuffers::unload(MeshRenderer &mr)
    {
        clogr::ensure(false, "Not implemented");
    }

    uint32_t GpuSceneBuffers::updateTransform(entis::Entity entity, const dg::Ref<dg::IDeviceContext>& ctx)
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

        ctx->UpdateBuffer(m_globalModelDataBuffer, index * sizeof(ModelData), sizeof(ModelData), &modelData, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        return index;
    }

    const GpuSceneBuffers::MeshAllocation& GpuSceneBuffers::getOrCreateMeshAllocation(const AssetRef<Mesh> &mesh, const dg::Ref<dg::IDeviceContext>& ctx)
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

        ctx->UpdateBuffer(
            m_globalIndexBuffer,
            alloc.indexStart * sizeof(uint32_t),
            mesh->indices().size() * sizeof(uint32_t),
            mesh->indices().data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        ctx->UpdateBuffer(
            m_globalVertexBuffer,
            alloc.vertexStart * sizeof(Vertex),
            mesh->vertices().size() * sizeof(Vertex),
            mesh->vertices().data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        ctx->UpdateBuffer(
            m_globalMeshletBuffer,
            alloc.meshletStart * sizeof(Meshlet),
            mesh->meshlets().size() * sizeof(Meshlet),
            mesh->meshlets().data(),
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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
