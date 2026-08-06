#pragma once
#include <vector>

#include "glm/glm.hpp"

namespace ion
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct Primitive
    {
        uint32_t indexStart;
        uint32_t indexCount;
        uint32_t materialIndex;
    };

    struct AABB
    {
        glm::vec3 min =  glm::vec3(FLT_MAX);
        glm::vec3 max = -glm::vec3(FLT_MAX);

        void expand(const glm::vec3 point)
        {
            min = glm::min(point, min);
            max = glm::max(point, max);
        }

        AABB transformed(const glm::mat4& mat)
        {
            glm::vec3 corners[8] = {
                {min.x, min.y, min.z},
                {min.x, min.y, max.z},
                {min.x, max.y, min.z},
                {min.x, max.y, max.z},
                {max.x, min.y, min.z},
                {max.x, min.y, max.z},
                {max.x, max.y, min.z},
                {max.x, max.y, max.z},
            };

            glm::vec3 newMin(FLT_MAX);
            glm::vec3 newMax(-FLT_MAX);

            for (auto corner : corners)
            {
                glm::vec4 transformed = mat * glm::vec4(corner, 1.0f);
                glm::vec3 p = glm::vec3(transformed) / transformed.w;

                newMin = glm::min(newMin, p);
                newMax = glm::max(newMax, p);
            }

            return { newMin, newMax };
        }

        [[nodiscard]] glm::vec3 center() const
        {
            return (min + max) * 0.5f;
        }

        float surfaceArea() const
        {
            const glm::vec3 dist = max - min;
            return 2*(dist.x*dist.y + dist.y*dist.z + dist.z*dist.x);
        }
    };


    struct Meshlet
    {
        glm::vec3 aabbMin;
        uint32_t   indexStart;
        glm::vec3 aabbMax;
        uint32_t   indexCount;

        glm::vec3 coneApex;
        float  coneCutoff;
        glm::vec3 coneAxis;
        uint32_t   materialIndex;
    };

    struct MeshletPrimitive
    {
        uint32_t meshletStart;
        uint32_t meshletCount;
        uint32_t materialIndex;
    };

    class Mesh
    {
    public:
        struct PrimitiveMeshletInfo;

        explicit Mesh(bool keepCpuData = false);

        void vertices(std::vector<Vertex> vertices);
        void indices(std::vector<uint32_t> indices);

        void primitives(std::vector<Primitive> primitives);
        [[nodiscard]] std::vector<Primitive> primitives() const;
        void addPrimitive(uint32_t startIndex, uint32_t indexCount);

        AABB bounds();

        [[nodiscard]] const std::vector<Vertex>& vertices() const { return m_vertices; }
        [[nodiscard]] const std::vector<uint32_t>& indices() const { return m_indices; }
        [[nodiscard]] const std::vector<uint32_t>& cookedIndices() const { return m_cookedIndices; }
        [[nodiscard]] const std::vector<Meshlet>& meshlets() const { return m_meshlets; }
        [[nodiscard]] const std::vector<MeshletPrimitive>& meshletPrimitives() const { return m_meshletPrimitives; }

        void apply();
    private:
        void buildInternals();
        void recalculateBounds();

        bool m_dirty = false;

        uint32_t m_indexCount{};
        uint32_t m_vertexCount{};
        uint32_t m_meshletCount{};

        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<Primitive> m_primitives;

        std::vector<uint32_t> m_cookedIndices;
        std::vector<Meshlet>  m_meshlets;
        std::vector<MeshletPrimitive>  m_meshletPrimitives;

        AABB m_bounds{};
    public:
        struct PrimitiveMeshletInfo
        {
            uint32_t meshletOffset;
            uint32_t meshletCount;
            uint32_t bvhRoot;
        };
    };
}
