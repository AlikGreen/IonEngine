#pragma once
#include <vector>

#include <urhi/urhi.h>
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
    };

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;

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
    };

    class Mesh
    {
    public:
        explicit Mesh(bool keepCpuData = false);

        bool isDrawable() const;

        void vertices(std::vector<Vertex> vertices);
        void indices(std::vector<uint32_t> indices);

        void primitives(std::vector<Primitive> primitives);
        [[nodiscard]] std::vector<Primitive> primitives() const;
        void addPrimitive(uint32_t startIndex, uint32_t indexCount);

        AABB bounds();

        [[nodiscard]] const std::vector<Vertex>& vertices() const;
        [[nodiscard]] const std::vector<uint32_t>& indices() const;

        [[nodiscard]] grl::Rc<urhi::Buffer> vertexBuffer();
        [[nodiscard]] grl::Rc<urhi::Buffer> indexBuffer();
        [[nodiscard]] size_t vertexCount() const;
        [[nodiscard]] size_t indexCount() const;

        [[nodiscard]] std::vector<Vertex> readbackVertices() const;
        [[nodiscard]] std::vector<uint32_t> readbackIndices() const;
    private:
        void apply();
        void recalculateBounds();

        bool m_verticesDirty = false;
        bool m_indicesDirty = false;
        bool m_boundsDirty = false;

        bool m_keepCpuData = false;

        std::vector<Vertex>    m_vertices{};
        std::vector<uint32_t>  m_indices{};
        std::vector<Primitive> m_primitives{};

        AABB m_bounds{};

        grl::Rc<urhi::Buffer> m_vertexBuffer = nullptr;
        grl::Rc<urhi::Buffer> m_indexBuffer = nullptr;

        size_t m_vertexCount = 0;
        size_t m_indexCount = 0;
    };
}
