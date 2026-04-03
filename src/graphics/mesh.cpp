#include "mesh.h"

#include <tiny_gltf.h>

#include <clogr.h>
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    void Mesh::apply()
    {
        if(m_primitives.empty()) m_primitives.emplace_back(0, m_indices.size());

        const auto device = Engine::getSystem<GraphicsSystem>()->getDevice();

        m_vertexBuffer = device->createBuffer({ urhi::BufferUsage::Vertex, m_vertices.size() * sizeof(Vertex) });
        m_indexBuffer  = device->createBuffer({ urhi::BufferUsage::Index, m_indices.size() * sizeof(uint32_t) });

        const auto cl = device->acquireCommandList(urhi::QueueType::Graphics);
        cl->begin();

        cl->updateBuffer(m_vertexBuffer, m_vertices);
        cl->updateBuffer(m_indexBuffer, m_indices);

        device->submit(cl);

        recalculateBounds();

        if(!m_keepCpuData)
        {
            this->m_vertices.clear();
            this->m_indices.clear();
        }

        m_verticesDirty = false;
        m_indicesDirty = false;
    }

    void Mesh::recalculateBounds()
    {
        m_bounds.min = glm::vec3(std::numeric_limits<float>::min());
        m_bounds.max = glm::vec3(std::numeric_limits<float>::max());

        for(Vertex vert : m_vertices)
        {
            m_bounds.min = glm::min(m_bounds.min, vert.position);
            m_bounds.max = glm::max(m_bounds.max, vert.position);
        }
    }

    Mesh::Mesh(const bool keepCpuData)
        : m_keepCpuData(keepCpuData) {  }

    void Mesh::vertices(std::vector<Vertex> vertices)
    {
        m_vertexCount = vertices.size();
        m_vertices = std::move(vertices);
        m_verticesDirty = true;
    }

    void Mesh::indices(std::vector<uint32_t> indices)
    {
        m_indexCount = indices.size();
        m_indices = std::move(indices);
        m_indicesDirty = true;
    }

    void Mesh::primitives(std::vector<Primitive> primitives)
    {
        m_primitives = std::move(primitives);
    }

    void Mesh::addPrimitive(uint32_t startIndex, uint32_t indexCount)
    {
        m_primitives.emplace_back(startIndex, indexCount);
    }

    const std::vector<Primitive> & Mesh::primitives() const
    {
        return m_primitives;
    }

    const std::vector<Vertex>& Mesh::vertices() const
    {
        clogr::ensure(m_keepCpuData, "Mesh CPU data is not retained. Set keepCpuData = true or use readbackVertices().");
        return m_vertices;
    }

    const std::vector<uint32_t>& Mesh::indices() const
    {
        clogr::ensure(m_keepCpuData, "Mesh CPU data is not retained. Set keepCpuData = true or use readbackIndices().");
        return m_indices;
    }

    AABB Mesh::bounds()
    {
        if(m_verticesDirty) recalculateBounds();
        return m_bounds;
    }

    grl::Rc<urhi::Buffer> Mesh::vertexBuffer()
    {
        if(m_verticesDirty)
        {
            apply();
        }
        return m_vertexBuffer;
    }

    grl::Rc<urhi::Buffer> Mesh::indexBuffer()
    {
        if(m_indicesDirty)
        {
            apply();
        }
        return m_indexBuffer;
    }

    size_t Mesh::vertexCount() const
    {
        return m_vertexCount;
    }

    size_t Mesh::indexCount() const
    {
        return m_indexCount;
    }

    std::vector<Vertex> Mesh::readbackVertices() const
    {
        if(m_vertices.size() == m_vertexCount)
            return m_vertices;

        const auto device = Engine::getSystem<GraphicsSystem>()->getDevice();

        const auto cl = device->acquireCommandList(urhi::QueueType::Graphics);
        cl->begin();

        const auto request = cl->readback(m_vertexBuffer);

        device->submit(cl);

        request->wait();
        const auto* data = static_cast<const Vertex*>(request->data());
        return {data, data + request->size() / sizeof(Vertex)};
    }

    std::vector<uint32_t> Mesh::readbackIndices() const
    {
        if(m_indices.size() == m_indexCount)
            return m_indices;

        const auto device = Engine::getSystem<GraphicsSystem>()->getDevice();

        const auto cl = device->acquireCommandList(urhi::QueueType::Graphics);
        cl->begin();

        const auto request = cl->readback(m_indexBuffer);

        device->submit(cl);

        request->wait();
        const auto* data = static_cast<const uint32_t*>(request->data());
        return {data, data + request->size() / sizeof(uint32_t)};
    }
}
