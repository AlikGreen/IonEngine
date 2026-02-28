static const uint RADIX_SIZE = 256;

cbuffer Constants
{
    uint elementCount;
    uint bitShift;
    uint threadGroupCount;
}

RWStructuredBuffer<uint> globalHistogram;

groupshared uint scanTemp[RADIX_SIZE * 2];

[numthreads(RADIX_SIZE, 1, 1)]
void prefixSum(uint3 GTid : SV_GroupThreadID)
{
    uint digit = GTid.x;  // One thread per digit (0-255)

    // Step 1: Sum this digit across all thread groups
    uint totalCount = 0;
    for (uint group = 0; group < threadGroupCount; group++)
    {
        totalCount += globalHistogram[group * RADIX_SIZE + digit];
    }

    scanTemp[digit] = totalCount;
    GroupMemoryBarrierWithGroupSync();

    // Step 2: Exclusive prefix sum of totals (Blelloch scan)
    uint n = RADIX_SIZE;

    // Up-sweep
    uint offset = 1;
    for (uint d = n >> 1; d > 0; d >>= 1)
    {
        if (digit < d)
        {
            uint ai = offset * (2 * digit + 1) - 1;
            uint bi = offset * (2 * digit + 2) - 1;
            scanTemp[bi] += scanTemp[ai];
        }
        offset *= 2;
        GroupMemoryBarrierWithGroupSync();
    }

    if (digit == 0)
        scanTemp[n - 1] = 0;
    GroupMemoryBarrierWithGroupSync();

    // Down-sweep
    for (uint d = 1; d < n; d *= 2)
    {
        offset >>= 1;
        if (digit < d)
        {
            uint ai = offset * (2 * digit + 1) - 1;
            uint bi = offset * (2 * digit + 2) - 1;

            uint t = scanTemp[ai];
            scanTemp[ai] = scanTemp[bi];
            scanTemp[bi] += t;
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Step 3: Distribute prefix sums back to each group
    uint digitPrefixSum = scanTemp[digit];
    uint runningSum = 0;

    for (uint group = 0; group < threadGroupCount; group++)
    {
        uint localCount = globalHistogram[group * RADIX_SIZE + digit];
        globalHistogram[group * RADIX_SIZE + digit] = digitPrefixSum + runningSum;
        runningSum += localCount;
    }
}