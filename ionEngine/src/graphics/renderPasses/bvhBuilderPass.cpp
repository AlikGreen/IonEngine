#include "bvhBuilderPass.h"

namespace ion
{
    struct TriangleBounds
    {
        alignas(16) glm::vec3 aabbMin;
        alignas(16) glm::vec3 centroid;
        alignas(16) glm::vec3 aabbMax;
        uint32_t primitiveIndex;
    };

    struct MeshBoundsConstants
    {
        uint32_t indexCount;
        uint32_t threadGroupCount;
        uint32_t indexOffset;
    };

    struct MeshBounds
    {
        glm::vec3 aabbMin;
        uint32_t leftFirst;

        glm::vec3 aabbMax;
        uint32_t count;
    };

    struct BVHNode
    {
        uint32_t rightChild;
        uint32_t leftChildOrFirstIndex;
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;

        uint32_t parent;
        uint32_t visitCount;
        uint32_t count;
    };

    struct TriBoundsBuildParams
    {
        uint32_t startIndex;
        uint32_t triangleCount;
        uint32_t vertexOffset;
        uint32_t boundsOffset; // offset into triangle bounds buffer
    };

    struct RadixSortConstants
    {
        uint32_t elementCount;
        uint32_t bitShift;          // Which bits to sort (0, 8, 16, 24 for 32-bit)
        uint32_t threadGroupCount;
    };

    struct BvhBuilderConstants
    {
        uint32_t triangleCount;
        uint32_t boundsOffset;      // Where to read bounds from
        uint32_t bvhOffset;         // Where to write BVH nodes
        uint32_t primOffset;        // Where to write primitive indices
        uint32_t leafSize;          // Max triangles per leaf (typically 1-4)
    };

    constexpr uint32_t MaxTriangles = 1u << 18;

    BvhBuilderPass::BvhBuilderPass(const grl::Rc<urhi::Device> &device)
    {
        auto& assetManager = Engine::getAssetManager();
        const auto triBoundsShader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/triBoundsBuilder.slang");
        m_triBoundsPipeline = device->createPipeline(urhi::ComputePipelineDesc{*triBoundsShader});

        const auto meshBoundPass1 = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/meshBoundsPass1.slang");
        m_meshBoundsPass1Pipeline = device->createPipeline(urhi::ComputePipelineDesc{*meshBoundPass1});

        const auto meshBoundPass2 = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/meshBoundsPass2.slang");
        m_meshBoundsPass2Pipeline = device->createPipeline(urhi::ComputePipelineDesc{*meshBoundPass2});

        const auto radixSorPass1Shader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/radixSortPass1.slang");
        m_radixSortPass1Pipeline = device->createPipeline(urhi::ComputePipelineDesc{*radixSorPass1Shader});

        const auto radixSorPass2Shader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/radixSortPass2.slang");
        m_radixSortPass2Pipeline = device->createPipeline(urhi::ComputePipelineDesc{*radixSorPass2Shader});

        const auto radixSorPass3Shader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/radixSortPass3.slang");
        m_radixSortPass3Pipeline = device->createPipeline(urhi::ComputePipelineDesc{*radixSorPass3Shader});

        const auto bvhShader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/bvhBuilder.slang");
        m_bvhPipeline = device->createPipeline(urhi::ComputePipelineDesc{*bvhShader});

        const auto bvhComputeAABBsShader = assetManager.import<grl::Rc<urhi::Shader>>("shaders/raytracing/bvhComputeAABBs.slang");
        m_bvhComputeAABBsPipeline = device->createPipeline(urhi::ComputePipelineDesc{*bvhComputeAABBsShader});

        m_triBoundsStorageBuffer = device->createStorageBuffer();
        m_triBoundsUniformBuffer = device->createUniformBuffer();
        m_triMortonCodes = device->createStorageBuffer();
        m_triIndices = device->createStorageBuffer();

        m_bvhNodesBuffer = device->createStorageBuffer();
        m_bvhConstants = device->createUniformBuffer();

        m_meshGroupBoundsBuffer = device->createStorageBuffer();
        m_meshBoundsConstantsBuffer = device->createUniformBuffer();

        m_outputMortonCodes = device->createStorageBuffer();
        m_outputIndices = device->createStorageBuffer();
        m_radixHistogramBuffer = device->createStorageBuffer();
        m_radixSortConstants = device->createUniformBuffer();

        const auto cmd = device->createCommandList();

        cmd->begin();

        cmd->reserveBuffer(m_bvhNodesBuffer, sizeof(BVHNode) * MaxTriangles * 2);
        cmd->reserveBuffer(m_bvhConstants, sizeof(BvhBuilderConstants));

        cmd->reserveBuffer(m_triBoundsStorageBuffer, sizeof(TriangleBounds) * MaxTriangles);

        cmd->reserveBuffer(m_triIndices, sizeof(uint32_t) * MaxTriangles);
        cmd->reserveBuffer(m_triMortonCodes, sizeof(uint32_t) * MaxTriangles);
        cmd->reserveBuffer(m_outputIndices, sizeof(uint32_t) * MaxTriangles);
        cmd->reserveBuffer(m_outputMortonCodes, sizeof(uint32_t) * MaxTriangles);

        cmd->reserveBuffer(m_triBoundsUniformBuffer, sizeof(TriBoundsBuildParams));

        cmd->reserveBuffer(m_meshGroupBoundsBuffer, sizeof(MeshBounds)*m_meshGroupBoundsBufferSize);
        cmd->reserveBuffer(m_meshBoundsConstantsBuffer, sizeof(MeshBoundsConstants));

        cmd->reserveBuffer(m_radixSortConstants, sizeof(RadixSortConstants));
        cmd->reserveBuffer(m_radixHistogramBuffer, sizeof(uint32_t) * MaxTriangles);

        device->submit(cmd);
    }

    void BvhBuilderPass::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        if(!ctx.has("all_renderables"))
            return;

        auto& renderables = *ctx.get<std::vector<Renderable>*>("all_renderables");

        uint32_t indexOffset = 0;

        uint32_t renderableIndex = 0;

        for (const auto& renderable: renderables)
        {
            if(renderableIndex > 1) break;
            renderableIndex++;
            const uint32_t indexCount = renderable.mesh->getIndexCount();
            const uint32_t triCount = indexCount/3;
            const uint32_t triOffset = indexOffset/3;

            // Calculate mesh bounds
            {
                uint32_t groups = (indexCount + 255) / 256;

                cmd->setPipeline(m_meshBoundsPass1Pipeline);

                MeshBoundsConstants constants {
                    indexCount,
                    groups,
                    indexOffset,
                };

                cmd->updateBuffer(m_meshBoundsConstantsBuffer, constants);

                if(m_meshGroupBoundsBufferSize < groups)
                {
                    cmd->reserveBuffer(m_meshGroupBoundsBuffer, sizeof(MeshBounds)*groups);
                    m_meshGroupBoundsBufferSize = groups;
                }

                cmd->setUniformBuffer("Constants", m_meshBoundsConstantsBuffer);

                cmd->setStorageBuffer("vertexBuffer", renderable.mesh->getVertexBuffer());
                cmd->setStorageBuffer("indices", renderable.mesh->getIndexBuffer());

                cmd->setStorageBuffer("groupBounds", m_meshGroupBoundsBuffer);


                cmd->dispatch({groups, 1, 1});
                cmd->resourceBarrier(m_triBoundsStorageBuffer);

                if (groups > 1)
                {
                    cmd->setPipeline(m_meshBoundsPass2Pipeline);

                    cmd->setUniformBuffer("Constants", m_meshBoundsConstantsBuffer);
                    cmd->setStorageBuffer("groupBounds", m_meshGroupBoundsBuffer);

                    cmd->dispatch({1, 1, 1});
                }
            }

            // Triangle bounds calculation
            {
                cmd->setPipeline(m_triBoundsPipeline);

                TriBoundsBuildParams params {
                    renderable.mesh->getPrimitives()[renderable.submeshIndex].indexStart,
                    triCount,
                    0,
                    indexOffset/3,
                };

                cmd->updateBuffer(m_triBoundsUniformBuffer, params);

                cmd->setUniformBuffer("BuildParams", m_triBoundsUniformBuffer);
                cmd->setStorageBuffer("triangleBounds", m_triBoundsStorageBuffer);

                cmd->setStorageBuffer("meshBounds", m_meshGroupBoundsBuffer);
                cmd->setStorageBuffer("mortonCodes", m_triMortonCodes);
                cmd->setStorageBuffer("triIndices", m_triIndices);

                cmd->setStorageBuffer("vertexBuffer", renderable.mesh->getVertexBuffer());
                cmd->setStorageBuffer("indexBuffer", renderable.mesh->getIndexBuffer());

                uint32_t groups = (triCount + 255) / 256;
                cmd->dispatch({groups, 1, 1});
                cmd->resourceBarrier(m_triBoundsStorageBuffer);
            }

            // radix sort
            {
                constexpr uint32_t RADIX_BITS = 8;
                constexpr uint32_t NUM_PASSES = 32 / RADIX_BITS;  // 4 passes for 32-bit keys

                constexpr uint32_t THREADS_PER_GROUP = 256;
                const uint32_t groups = (triCount + THREADS_PER_GROUP - 1) / THREADS_PER_GROUP;

                auto currentInputKeys = m_triMortonCodes;
                auto currentOutputKeys = m_outputMortonCodes;

                auto currentInputValues = m_triIndices;
                auto currentOutputValues = m_outputIndices;


                for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
                {
                    RadixSortConstants constants{};
                    constants.elementCount = triCount;
                    constants.bitShift = pass * RADIX_BITS;
                    constants.threadGroupCount = groups;

                    cmd->updateBuffer(m_radixSortConstants, constants);

                    // PASS 1: local histogram
                    cmd->setPipeline(m_radixSortPass1Pipeline);

                    cmd->setUniformBuffer("Constants", m_radixSortConstants);
                    cmd->setStorageBuffer("inputKeys", currentInputKeys);
                    cmd->setStorageBuffer("globalHistogram", m_radixHistogramBuffer);

                    cmd->dispatch({groups, 1, 1});
                    cmd->resourceBarrier(m_radixHistogramBuffer);

                    // PASS 2 : prefix sum
                    cmd->setPipeline(m_radixSortPass2Pipeline);

                    cmd->setUniformBuffer("Constants", m_radixSortConstants);
                    cmd->setStorageBuffer("globalHistogram", m_radixHistogramBuffer);

                    cmd->dispatch({1, 1, 1});
                    cmd->resourceBarrier(m_radixHistogramBuffer);

                    // PASS 3 : scatter
                    cmd->setPipeline(m_radixSortPass3Pipeline);

                    cmd->setUniformBuffer("Constants", m_radixSortConstants);
                    cmd->setStorageBuffer("inputKeys", currentInputKeys);
                    cmd->setStorageBuffer("inputValues", currentInputValues);
                    cmd->setStorageBuffer("globalHistogram", m_radixHistogramBuffer);
                    cmd->setStorageBuffer("outputKeys", currentOutputKeys);
                    cmd->setStorageBuffer("outputValues", currentOutputValues);

                    cmd->dispatch({groups, 1, 1});


                    std::swap(currentInputKeys, currentOutputKeys);
                    std::swap(currentInputValues, currentOutputValues);
                }

                m_outputMortonCodes = currentOutputKeys;
                m_outputIndices = currentOutputValues;

                m_triMortonCodes = currentInputKeys;
                m_triIndices = currentInputValues;
            }

            // BVH building
            {
                uint32_t internalNodeCount = triCount - 1;
                uint32_t groups = (internalNodeCount + 255) / 256;

                BvhBuilderConstants constants
                {
                    triCount,
                    triOffset,
                    triOffset,
                    triOffset,
                    4,
                };

                cmd->setPipeline(m_bvhPipeline);
                cmd->updateBuffer(m_bvhConstants, constants);

                cmd->setStorageBuffer("bvhNodes", m_bvhNodesBuffer);
                cmd->setStorageBuffer("triBounds", m_triBoundsStorageBuffer);
                cmd->setStorageBuffer("triIndices", m_triIndices);
                cmd->setStorageBuffer("mortonCodes", m_triMortonCodes);

                cmd->setUniformBuffer("Constants", m_bvhConstants);

                cmd->dispatch({groups, 1, 1});
                cmd->resourceBarrier(m_bvhNodesBuffer);

                cmd->setPipeline(m_bvhComputeAABBsPipeline);
                cmd->setStorageBuffer("bvhNodes", m_bvhNodesBuffer);
                cmd->setStorageBuffer("triBounds", m_triBoundsStorageBuffer);
                cmd->setStorageBuffer("triIndices", m_triIndices);

                cmd->setUniformBuffer("Constants", m_bvhConstants);

                groups = (triCount + 255) / 256;
                cmd->dispatch({groups, 1, 1});
                cmd->resourceBarrier(m_bvhNodesBuffer);
            }

            indexOffset += indexCount;
        }

        ctx.set("bvh_buffer", m_bvhNodesBuffer);
        ctx.set("sorted_tri_indices_buffer", m_triIndices);
    }
}
