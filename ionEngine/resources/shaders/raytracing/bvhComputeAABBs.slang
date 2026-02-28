import bvh;

struct TriangleBounds
{
    float3 aabbMin;
    float3 centroid;
    float3 aabbMax;
    uint primitiveIndex;
};

cbuffer Constants
{
    uint triangleCount;
    uint boundsOffset;      // Where to read bounds from
    uint bvhOffset;         // Where to write BVH nodes
    uint primOffset;        // Where to write primitive indices
    uint leafSize;          // Max triangles per leaf (typically 1-4)
};


RWStructuredBuffer<BVHNode> bvhNodes;

StructuredBuffer<uint> triIndices;

StructuredBuffer<TriangleBounds> triBounds;

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint leafIndex = dispatchThreadID.x;
    if (leafIndex >= triangleCount)
        return;

    // Initialize leaf node AABB
    uint leafNodeIdx = bvhOffset + triangleCount - 1 + leafIndex;
    uint triangleIndex = triIndices[primOffset + leafIndex];

    TriangleBounds bounds = triBounds[boundsOffset + triangleIndex];
    bvhNodes[leafNodeIdx].aabbMin = bounds.aabbMin;
    bvhNodes[leafNodeIdx].aabbMax = bounds.aabbMax;
    bvhNodes[leafNodeIdx].leftChildOrFirstIndex = triangleIndex;
    bvhNodes[leafNodeIdx].count = 1;

    // Traverse up the tree
    uint currentNodeIdx = leafNodeIdx;
    uint rootNodeIdx = bvhOffset;

    // SAFETY: Limit iterations to tree height (log2(N) + margin)
    const uint maxIterations = 64;  // Safety for trees up to 2^64 nodes

    for (uint iteration = 0; iteration < maxIterations; iteration++)
    {
        if (currentNodeIdx == rootNodeIdx)
            break;  // Reached root

        uint parentNodeIdx = bvhNodes[currentNodeIdx].parent;

        // SAFETY: Check for invalid parent
        if (parentNodeIdx < bvhOffset || parentNodeIdx >= bvhOffset + triangleCount - 1)
            break;  // Invalid parent index

        // Atomic counter to ensure both children are processed
        uint previousVisitCount = 0;
        InterlockedAdd(bvhNodes[parentNodeIdx].visitCount, 1, previousVisitCount);

        if (previousVisitCount == 0)
            break;  // First child to arrive, wait for sibling

        // Second child arrived, compute parent AABB
        uint leftChildIdx = bvhNodes[parentNodeIdx].leftChildOrFirstIndex;
        uint rightChildIdx = bvhNodes[parentNodeIdx].rightChild;

        float3 minBound = min(bvhNodes[leftChildIdx].aabbMin, bvhNodes[rightChildIdx].aabbMin);
        float3 maxBound = max(bvhNodes[leftChildIdx].aabbMax, bvhNodes[rightChildIdx].aabbMax);

        bvhNodes[parentNodeIdx].aabbMin = minBound;
        bvhNodes[parentNodeIdx].aabbMax = maxBound;

        currentNodeIdx = parentNodeIdx;
    }
}