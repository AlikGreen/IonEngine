#pragma once
#include <grl/mem.h>

#include "buffer.h"
#include "DrawIndexedIndirectCommand.h"

namespace ion
{
struct VisibleMeshletEntry
{
    uint32_t meshletId;
    uint32_t primitiveId;
};

struct MeshletFrameResources
{
    grl::Rc<urhi::Buffer> visibleMeshlets;
    grl::Rc<urhi::Buffer> indirectDispatchArgs;
    grl::Rc<urhi::Buffer> indexCountOrOffsetPerPrimitive;
    grl::Rc<urhi::Buffer> globalIndexAllocationCounter;
    grl::Rc<urhi::Buffer> indexScratchBuffer;
    grl::Rc<urhi::Buffer> drawCmdBuffer;
    grl::Rc<urhi::Buffer> drawCountBuffer;

    grl::Rc<urhi::Buffer> debugLinesBuffer;
    grl::Rc<urhi::Buffer> linesDrawCmdBuffer;

    explicit MeshletFrameResources(const grl::Rc<urhi::Device> &device)
    {
        // these values should probably be tuned/data be limited correctly/limits change per device specs
        constexpr uint32_t kMaxPrimitives = 1024*16;

        constexpr uint32_t kMaxVisibleMeshlets = 1024*128;
        constexpr uint32_t kMaxVisibleIndices = 1024*1024*32;

        constexpr uint32_t kMaxMaterials = 1024;
        constexpr uint32_t kMaxDebugLines = 1024*1024;

        indexScratchBuffer = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Index, sizeof(uint32_t)*kMaxVisibleIndices});
        visibleMeshlets = device->createBuffer({ urhi::BufferUsage::Storage, sizeof(VisibleMeshletEntry)*kMaxVisibleMeshlets});

        indexCountOrOffsetPerPrimitive = device->createBuffer({ urhi::BufferUsage::Storage, sizeof(uint32_t)*kMaxPrimitives});

        drawCmdBuffer = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Indirect, sizeof(urhi::DrawIndexedIndirectCommand)*kMaxPrimitives});
        drawCountBuffer = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Indirect, sizeof(uint32_t)*kMaxMaterials});

        globalIndexAllocationCounter = device->createBuffer({ urhi::BufferUsage::Storage, sizeof(uint32_t)});
        indirectDispatchArgs = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Indirect, sizeof(uint32_t)*3});

        debugLinesBuffer = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Vertex, sizeof(glm::vec3)*kMaxDebugLines});
        linesDrawCmdBuffer = device->createBuffer({ urhi::BufferUsage::Storage | urhi::BufferUsage::Indirect, sizeof(urhi::DrawIndirectCommand)});
    }
};
}
