static const uint RADIX_SIZE = 256;
static const uint THREADS_PER_GROUP = 256;

cbuffer Constants
{
    uniform uint elementCount;
    uniform uint bitShift;
    uniform uint threadGroupCount;
}

StructuredBuffer<uint> inputKeys;
RWStructuredBuffer<uint> globalHistogram;

groupshared uint sharedHistogram[RADIX_SIZE];

[shader("compute")]
[numthreads(THREADS_PER_GROUP, 1, 1)]
void localHistogram(
    uint3 DTid : SV_DispatchThreadID,
    uint3 GTid : SV_GroupThreadID,
    uint3 Gid : SV_GroupID)
{
    uint tid = GTid.x;
    uint groupId = Gid.x;

    if (tid < RADIX_SIZE)
        sharedHistogram[tid] = 0;
    GroupMemoryBarrierWithGroupSync();

    for (uint i = DTid.x; i < elementCount; i += THREADS_PER_GROUP * threadGroupCount)
    {
        uint key = inputKeys[i];
        uint digit = (key >> bitShift) & 0xFF;
        InterlockedAdd(sharedHistogram[digit], 1);
    }
    GroupMemoryBarrierWithGroupSync();

    if (tid < RADIX_SIZE)
    {
        globalHistogram[groupId * RADIX_SIZE + tid] = sharedHistogram[tid];
    }
}