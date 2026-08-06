#include "meshletPipelineUtil.hlsli"

struct DrawIndexedIndirectCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  baseVertex;
    uint baseInstance;
};

RWStructuredBuffer<uint> gIndexCountPerPrimitive;
RWStructuredBuffer<uint> gDrawOffsetPerTemplate;
RWStructuredBuffer<uint> gDrawCountPerTemplate;

StructuredBuffer<MeshPrimitive> gPrimitives;

RWStructuredBuffer<DrawIndexedIndirectCommand> gDrawCommands;

RWStructuredBuffer<uint> gGlobalIndexAllocationCounter;

struct PushConstants
{
    uint primitiveCount;
};

[[ion::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint primID = threadID.x;
    if (primID >= pc.primitiveCount)
        return;

    MeshPrimitive prim = gPrimitives[primID];

    uint count = gIndexCountPerPrimitive[primID];

    if (count == 0)
        return;

    uint first = 0;
    InterlockedAdd(gGlobalIndexAllocationCounter[0], count, first);

    uint drawCmdIndex = 0;
    InterlockedAdd(gDrawOffsetPerTemplate[prim.templateIndex], 1, drawCmdIndex);
    InterlockedAdd(gDrawCountPerTemplate[prim.templateIndex], 1);

    DrawIndexedIndirectCommand cmd;
    cmd.indexCount    = count;
    cmd.instanceCount = (count > 0) ? 1 : 0;
    cmd.firstIndex    = first;
    cmd.baseVertex    = prim.vertexStart;
    cmd.baseInstance  = primID;
    gDrawCommands[drawCmdIndex] = cmd;

    gIndexCountPerPrimitive[primID] = first;
}
