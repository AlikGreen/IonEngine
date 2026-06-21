import meshletPipelineUtil;

StructuredBuffer<Meshlet> gMeshlets;
StructuredBuffer<VisibleMeshletEntry> gVisibleMeshlets;
RWStructuredBuffer<Atomic<uint>> gCurrentOffsetPerPrimitive;

StructuredBuffer<uint> gGlobalIndices;
RWStructuredBuffer<uint> gScratchIndices;

StructuredBuffer<MeshPrimitive> gPrimitives;

groupshared uint groupDstOffset;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 localThreadID : SV_GroupThreadID,
    uint3 groupId : SV_GroupID)
{
    VisibleMeshletEntry entry = gVisibleMeshlets[groupId.x];
    Meshlet meshlet = gMeshlets[entry.meshletId];
    uint primIndex = entry.primitiveId;

    if (localThreadID.x == 0)
    {
        groupDstOffset = gCurrentOffsetPerPrimitive[primIndex].add(meshlet.indexCount);
    }

    GroupMemoryBarrierWithGroupSync();

    MeshPrimitive prim = gPrimitives[primIndex];

    for (uint i = localThreadID.x; i < meshlet.indexCount; i += 64)
    {
        gScratchIndices[groupDstOffset + i] = gGlobalIndices[prim.indexStart + meshlet.indexStart + i];
    }
}

