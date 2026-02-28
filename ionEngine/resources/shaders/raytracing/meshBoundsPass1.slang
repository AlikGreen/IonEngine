import bvh;

struct MeshBounds
{
    float3 min;
    float3 max;
};

cbuffer Constants
{
    uint indexCount;
    uint threadGroupCount;
    uint indexOffset;
};

ByteAddressBuffer vertexBuffer;
StructuredBuffer<uint> indices;
RWStructuredBuffer<MeshBounds> groupBounds;

groupshared float3 sharedMin[256];
groupshared float3 sharedMax[256];

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
                   uint3 GTid : SV_GroupThreadID,
                   uint3 Gid : SV_GroupID)
{
    uint tid = GTid.x;
    uint globalIdx = DTid.x;

    float3 localMin = float3(1e30, 1e30, 1e30);
    float3 localMax = float3(-1e30, -1e30, -1e30);

    for (uint i = globalIdx; i < indexCount; i += 256 * threadGroupCount)
    {
        uint vertexIndex = indices[i + indexOffset];
        float3 v = getVertex(vertexBuffer, vertexIndex).position;

        localMin = min(localMin, v);
        localMax = max(localMax, v);
    }

    sharedMin[tid] = localMin;
    sharedMax[tid] = localMax;
    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint s = 128; s > 0; s >>= 1)
    {
        if (tid < s)
        {
            sharedMin[tid] = min(sharedMin[tid], sharedMin[tid + s]);
            sharedMax[tid] = max(sharedMax[tid], sharedMax[tid + s]);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (tid == 0)
    {
        groupBounds[Gid.x].min = sharedMin[0];
        groupBounds[Gid.x].max = sharedMax[0];
    }
}
