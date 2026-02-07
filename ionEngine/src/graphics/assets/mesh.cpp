#include "mesh.h"

#include <tiny_gltf.h>

#include <clogr.h>
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    void Mesh::apply()
    {
        if(primitives.empty()) primitives.emplace_back(0, indices.size());

        const auto device = Engine::getSystem<GraphicsSystem>()->getDevice();

        vertexBuffer = device->createVertexBuffer();
        indexBuffer  = device->createIndexBuffer();

        const auto cl = device->createCommandList();
        cl->begin();

        cl->reserveBuffer(vertexBuffer, vertices.size() * sizeof(Vertex));
        cl->updateBuffer(vertexBuffer, vertices);

        cl->reserveBuffer(indexBuffer, indices.size() * sizeof(uint32_t));
        cl->updateBuffer(indexBuffer, indices);

        device->submit(cl);

        recalculateBounds();

        this->vertices.clear();
        this->indices.clear();

        verticesDirty = false;
        indicesDirty = false;
    }

    void Mesh::recalculateBounds()
    {
        bounds.min = glm::vec3(std::numeric_limits<float>::min());
        bounds.max = glm::vec3(std::numeric_limits<float>::max());

        for(Vertex vert : vertices)
        {
            bounds.min = glm::min(bounds.min, vert.position);
            bounds.max = glm::max(bounds.max, vert.position);
        }
    }

    void Mesh::setVertices(const std::vector<Vertex> &vertices)
    {
        vertexCount = vertices.size();
        this->vertices = vertices;
        verticesDirty = true;
    }

    void Mesh::setIndices(const std::vector<uint32_t> &indices)
    {
        indexCount = indices.size();
        this->indices = indices;
        indicesDirty = true;
    }

    void Mesh::setPrimitives(const std::vector<Primitive> &primitives)
    {
        this->primitives = primitives;
    }

    void Mesh::addPrimitive(uint32_t startIndex, uint32_t indexCount)
    {
        primitives.emplace_back(startIndex, indexCount);
    }

    const std::vector<Primitive> & Mesh::getPrimitives() const
    {
        return primitives;
    }

    const std::vector<Vertex>& Mesh::getVertices() const
    {
        clogr::ensure(readable, "Mesh is not readable on the CPU.");
        return vertices;
    }

    const std::vector<uint32_t>& Mesh::getIndices() const
    {
        clogr::ensure(readable, "Mesh is not readable on the CPU.");
        return indices;
    }

    AABB Mesh::getBounds()
    {
        if(verticesDirty) recalculateBounds();
        return bounds;
    }

    grl::Rc<urhi::Buffer> Mesh::getVertexBuffer()
    {
        if(verticesDirty)
        {
            apply();
        }
        return vertexBuffer;
    }

    grl::Rc<urhi::Buffer> Mesh::getIndexBuffer()
    {
        if(indicesDirty)
        {
            apply();
        }
        return indexBuffer;
    }

    size_t Mesh::getVertexCount() const
    {
        return vertexCount;
    }

    size_t Mesh::getIndexCount() const
    {
        return indexCount;
    }

    void Mesh::setReadable(const bool readable)
    {
        this->readable = readable;
    }
}
