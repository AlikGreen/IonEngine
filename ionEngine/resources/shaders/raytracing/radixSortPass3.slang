static const uint RADIX_SIZE = 256;
static const uint THREADS_PER_GROUP = 256;

cbuffer Constants
{
    uniform uint elementCount;
    uniform uint bitShift;
    uniform uint threadGroupCount;
}

StructuredBuffer<uint> inputKeys;
StructuredBuffer<uint> inputValues;
RWStructuredBuffer<uint> globalHistogram;

RWStructuredBuffer<uint> outputKeys;
RWStructuredBuffer<uint> outputValues;

groupshared uint localOffsets[RADIX_SIZE];

[shader("compute")]
[numthreads(THREADS_PER_GROUP, 1, 1)]
void scatter(
    uint3 DTid : SV_DispatchThreadID,
    uint3 GTid : SV_GroupThreadID,
    uint3 Gid : SV_GroupID)
{
    uint tid = GTid.x;
    uint groupId = Gid.x;

    if (tid < RADIX_SIZE)
    {
        localOffsets[tid] = globalHistogram[groupId * RADIX_SIZE + tid];
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint i = DTid.x; i < elementCount; i += THREADS_PER_GROUP * threadGroupCount)
    {
        uint key = inputKeys[i];
        uint value = inputValues[i];
        uint digit = (key >> bitShift) & 0xFF;

        uint offset;
        InterlockedAdd(localOffsets[digit], 1, offset);

        outputKeys[offset] = key;
        outputValues[offset] = value;
    }
}