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

RWStructuredBuffer<MeshBounds> groupBounds;

groupshared float3 sharedMin[256];
groupshared float3 sharedMax[256];

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
                   uint3 GTid : SV_GroupThreadID)
{
    uint tid = GTid.x;

    // Initialize to extremes
    float3 localMin = float3(1e30, 1e30, 1e30);
    float3 localMax = float3(-1e30, -1e30, -1e30);

    // Load from previous pass results
    if (DTid.x < threadGroupCount)
    {
        localMin = groupBounds[DTid.x].min;
        localMax = groupBounds[DTid.x].max;
    }

    sharedMin[tid] = localMin;
    sharedMax[tid] = localMax;
    GroupMemoryBarrierWithGroupSync();

    // Parallel reduction
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

    // Write final result to first element
    if (tid == 0)
    {
        groupBounds[0].min = sharedMin[0];
        groupBounds[0].max = sharedMax[0];
    }
}