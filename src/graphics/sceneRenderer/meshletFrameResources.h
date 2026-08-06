#pragma once
#include <grl/mem.h>

#include "buffer.h"

namespace ion
{
struct DrawIndirectCommand
{
    uint32_t NumVertices;
    uint32_t NumInstances;
    uint32_t StartVertexLocation;
    uint32_t StartInstanceLocation;
};

struct DrawIndexedIndirectCommand
{
    uint32_t NumIndices;
    uint32_t NumInstances;
    uint32_t StartIndexLocation;
    uint32_t BaseVertexLocation;
    uint32_t StartInstanceLocation;
};

struct VisibleMeshletEntry
{
    uint32_t meshletId;
    uint32_t primitiveId;
};

struct MeshletFrameResources
{
    dg::Ref<dg::IBuffer> visibleMeshlets{};
    dg::Ref<dg::IBuffer> indirectDispatchArgs{};
    dg::Ref<dg::IBuffer> indexCountOrOffsetPerPrimitive{};
    dg::Ref<dg::IBuffer> globalIndexAllocationCounter{};
    dg::Ref<dg::IBuffer> indexScratchBuffer{};
    dg::Ref<dg::IBuffer> drawCmdBuffer{};
    dg::Ref<dg::IBuffer> drawCountBuffer{};

    dg::Ref<dg::IBuffer> debugLinesBuffer{};
    dg::Ref<dg::IBuffer> linesDrawCmdBuffer{};

    explicit MeshletFrameResources(const dg::Ref<dg::IRenderDevice> &device)
    {
        // these values should probably be tuned/data be limited correctly/limits change per device specs
        constexpr uint32_t kMaxPrimitives = 1024*16;

        constexpr uint32_t kMaxVisibleMeshlets = 1024*128;
        constexpr uint32_t kMaxVisibleIndices = 1024*1024*32;

        constexpr uint32_t kMaxMaterials = 1024;
        constexpr uint32_t kMaxDebugLines = 1024*1024;

        dg::BufferDesc indexScratchDesc{};
        indexScratchDesc.Name      = "Index Scratch Buffer";
        indexScratchDesc.Size      = sizeof(uint32_t) * kMaxVisibleIndices;
        indexScratchDesc.Usage     = dg::USAGE_DEFAULT;
        indexScratchDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_INDEX_BUFFER | dg::BIND_SHADER_RESOURCE;
        indexScratchDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        indexScratchDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer(indexScratchDesc, nullptr, &indexScratchBuffer);

        dg::BufferDesc visibleMeshletsDesc{};
        visibleMeshletsDesc.Name      = "Visible Meshlets Buffer";
        visibleMeshletsDesc.Size      = sizeof(VisibleMeshletEntry) * kMaxVisibleMeshlets;
        visibleMeshletsDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        visibleMeshletsDesc.Usage     = dg::USAGE_DEFAULT;
        visibleMeshletsDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        visibleMeshletsDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        visibleMeshletsDesc.ElementByteStride = sizeof(VisibleMeshletEntry);
        device->CreateBuffer(visibleMeshletsDesc, nullptr, &visibleMeshlets);

        dg::BufferDesc indexCountDesc{};
        indexCountDesc.Name      = "Index Count Or Offset Per Primitive Buffer";
        indexCountDesc.Size      = sizeof(uint32_t) * kMaxPrimitives;
        indexCountDesc.Usage     = dg::USAGE_DEFAULT;
        indexCountDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        indexCountDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        indexCountDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        indexCountDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer(indexCountDesc, nullptr, &indexCountOrOffsetPerPrimitive);

        dg::BufferDesc drawCmdDesc{};
        drawCmdDesc.Name      = "Draw Cmd Buffer";
        drawCmdDesc.Size      = sizeof(DrawIndexedIndirectCommand) * kMaxPrimitives;
        drawCmdDesc.Usage     = dg::USAGE_DEFAULT;
        drawCmdDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_INDIRECT_DRAW_ARGS | dg::BIND_SHADER_RESOURCE;
        drawCmdDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        drawCmdDesc.ElementByteStride = sizeof(DrawIndexedIndirectCommand);
        device->CreateBuffer(drawCmdDesc, nullptr, &drawCmdBuffer);

        dg::BufferDesc drawCountDesc{};
        drawCountDesc.Name      = "Draw Count Buffer";
        drawCountDesc.Size      = sizeof(uint32_t) * kMaxMaterials;
        drawCountDesc.Usage     = dg::USAGE_DEFAULT;
        drawCountDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_INDIRECT_DRAW_ARGS | dg::BIND_SHADER_RESOURCE;
        drawCountDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        drawCountDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer(drawCountDesc, nullptr, &drawCountBuffer);

        dg::BufferDesc globalCounterDesc{};
        globalCounterDesc.Name      = "Global Index Allocation Counter";
        globalCounterDesc.Size      = sizeof(uint32_t);
        globalCounterDesc.Usage     = dg::USAGE_DEFAULT;
        globalCounterDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        globalCounterDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        globalCounterDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        globalCounterDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer(globalCounterDesc, nullptr, &globalIndexAllocationCounter);

        dg::BufferDesc dispatchArgsDesc{};
        dispatchArgsDesc.Name      = "Indirect Dispatch Args Buffer";
        dispatchArgsDesc.Size      = sizeof(uint32_t) * 3;
        dispatchArgsDesc.Usage     = dg::USAGE_DEFAULT;
        dispatchArgsDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_INDIRECT_DRAW_ARGS | dg::BIND_SHADER_RESOURCE;
        dispatchArgsDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        dispatchArgsDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer(dispatchArgsDesc, nullptr, &indirectDispatchArgs);

        dg::BufferDesc debugLinesDesc{};
        debugLinesDesc.Name      = "Debug Lines Buffer";
        debugLinesDesc.Size      = sizeof(glm::vec3) * kMaxDebugLines;
        debugLinesDesc.Usage     = dg::USAGE_DEFAULT;
        debugLinesDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_VERTEX_BUFFER | dg::BIND_SHADER_RESOURCE;
        debugLinesDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        debugLinesDesc.ElementByteStride = sizeof(glm::vec3);
        device->CreateBuffer(debugLinesDesc, nullptr, &debugLinesBuffer);

        dg::BufferDesc linesDrawCmdDesc{};
        linesDrawCmdDesc.Name      = "Lines Draw Cmd Buffer";
        linesDrawCmdDesc.Size      = sizeof(DrawIndirectCommand);
        linesDrawCmdDesc.Usage     = dg::USAGE_DEFAULT;
        linesDrawCmdDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_INDIRECT_DRAW_ARGS | dg::BIND_SHADER_RESOURCE;
        linesDrawCmdDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        linesDrawCmdDesc.ElementByteStride = sizeof(DrawIndirectCommand);
        device->CreateBuffer(linesDrawCmdDesc, nullptr, &linesDrawCmdBuffer);
    }
};
}
