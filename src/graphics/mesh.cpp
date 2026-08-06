#include "mesh.h"

#include <tiny_gltf.h>

#include <clogr.h>

#include "meshoptimizer.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    Mesh::Mesh(const bool keepCpuData) {  }

    void Mesh::apply()
    {
        if(!m_dirty || m_vertices.empty() || m_indices.empty())
            return;

        const auto device = Engine::getSystem<GraphicsSystem>()->device();

        buildInternals();

        m_dirty = false;
    }

    void Mesh::buildInternals()
    {
        for (const auto& prim : primitives())
        {
            const uint32_t* primIndices = m_indices.data() + prim.indexStart;

            const size_t maxMeshlets = meshopt_buildMeshletsBound(prim.indexCount, 64, 124);

            std::vector<meshopt_Meshlet> rawMeshlets(maxMeshlets);
            std::vector<uint32_t>        rawVerts(maxMeshlets * 64);
            std::vector<uint8_t>         rawTris(maxMeshlets * 124 * 3);

            size_t meshletCount = meshopt_buildMeshlets(
                rawMeshlets.data(), rawVerts.data(), rawTris.data(),
                primIndices, prim.indexCount,
                &m_vertices[0].position.x,
                m_vertices.size(), sizeof(Vertex),
                64, 124, 0.5f
            );

            MeshletPrimitive meshletPrim{};
            meshletPrim.materialIndex = prim.materialIndex;
            meshletPrim.meshletStart = m_meshlets.size();
            meshletPrim.meshletCount = meshletCount;
            m_meshletPrimitives.push_back(meshletPrim);


            for (size_t m = 0; m < meshletCount; m++)
            {
                meshopt_Bounds b = meshopt_computeMeshletBounds(
                    &rawVerts[rawMeshlets[m].vertex_offset],
                    &rawTris[rawMeshlets[m].triangle_offset],
                    rawMeshlets[m].triangle_count,
                    &m_vertices[0].position.x,
                    m_vertices.size(), sizeof(Vertex)
                );

                AABB meshletBounds{};
                for (uint32_t j = 0; j < rawMeshlets[m].vertex_count; j++)
                {
                    uint32_t globalIdx = rawVerts[rawMeshlets[m].vertex_offset + j];
                    meshletBounds.expand(m_vertices[globalIdx].position);
                }

                uint32_t indexStart = m_cookedIndices.size();

                for (uint32_t t = 0; t < rawMeshlets[m].triangle_count; t++)
                {
                    uint32_t base = (rawMeshlets[m].triangle_offset + t) * 3;
                    for (int v = 0; v < 3; v++)
                    {
                        uint8_t  localIdx  = rawTris[base + v];
                        uint32_t globalIdx = rawVerts[rawMeshlets[m].vertex_offset + localIdx];
                        m_cookedIndices.push_back(globalIdx);
                    }
                }

                Meshlet desc{};
                desc.indexStart = indexStart;
                desc.indexCount = rawMeshlets[m].triangle_count * 3;
                desc.coneApex       = {b.cone_apex[0], b.cone_apex[1], b.cone_apex[2]};
                desc.coneAxis       = {b.cone_axis[0], b.cone_axis[1], b.cone_axis[2]};
                desc.coneCutoff     = b.cone_cutoff;
                desc.aabbMin = meshletBounds.min;
                desc.aabbMax = meshletBounds.max;

                desc.materialIndex = prim.materialIndex; // I don't know how this will work yet

                m_meshlets.push_back(desc);
            }
        }
    }

    void Mesh::recalculateBounds()
    {
        if(!m_dirty) return;

        m_bounds = {};

        for(const Vertex& vert : m_vertices)
        {
            m_bounds.expand(vert.position);
        }
    }

    void Mesh::vertices(std::vector<Vertex> vertices)
    {
        m_vertices = std::move(vertices);
        m_dirty = true;
    }

    void Mesh::indices(std::vector<uint32_t> indices)
    {
        m_indexCount = indices.size();
        m_indices = std::move(indices);
        m_dirty = true;
    }

    void Mesh::primitives(std::vector<Primitive> primitives)
    {
        m_primitives = std::move(primitives);
    }

    void Mesh::addPrimitive(uint32_t startIndex, uint32_t indexCount)
    {
        m_primitives.emplace_back(startIndex, indexCount);
    }

    std::vector<Primitive> Mesh::primitives() const
    {
        if(m_primitives.empty() && m_indexCount > 0)
            return {Primitive{0, static_cast<uint32_t>(m_indexCount)}};

        return m_primitives;
    }

    AABB Mesh::bounds()
    {
        recalculateBounds();
        return m_bounds;
    }
}
