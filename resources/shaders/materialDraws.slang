import meshletPipelineUtil;

struct DrawIndexedIndirectCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  baseVertex;
    uint baseInstance;
};

RWStructuredBuffer<uint> gIndexCountPerPrimitive;
RWStructuredBuffer<Atomic<uint>> gDrawOffsetPerTemplate;
RWStructuredBuffer<Atomic<uint>> gDrawCountPerTemplate;

StructuredBuffer<MeshPrimitive> gPrimitives;

RWStructuredBuffer<DrawIndexedIndirectCommand> gDrawCommands;

RWStructuredBuffer<Atomic<uint>> gGlobalIndexAllocationCounter;

struct PushConstants
{
    uint primitiveCount;
};

[[vk::push_constant]]
PushConstants pc;


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

    uint first = gGlobalIndexAllocationCounter[0].add(count);

    uint drawCmdIndex = gDrawOffsetPerTemplate[prim.templateIndex].add(1);
    gDrawCountPerTemplate[prim.templateIndex]++;

    DrawIndexedIndirectCommand cmd;
    cmd.indexCount    = count;
    cmd.instanceCount = (count > 0) ? 1 : 0;
    cmd.firstIndex    = first;
    cmd.baseVertex    = prim.vertexStart;
    cmd.baseInstance  = primID;
    gDrawCommands[drawCmdIndex] = cmd;

    gIndexCountPerPrimitive[primID] = first;
}
