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

StructuredBuffer<uint> triIndices; // sorted indices for tri bounds and morton codes

StructuredBuffer<TriangleBounds> triBounds;
StructuredBuffer<uint> mortonCodes;

uint getMorton(uint index)
{
    return mortonCodes[index];
}

uint countLeadingZeros(uint x)
{
    return (x == 0) ? 32 : (31 - firstbithigh(x));
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint nodeIndex = dispatchThreadID.x;
    if (nodeIndex >= triangleCount - 1)
        return;

    int deltaNext = delta(nodeIndex, nodeIndex + 1);
    int deltaPrev = delta(nodeIndex, nodeIndex - 1);
    int direction = (deltaNext >= deltaPrev) ? 1 : -1;

    int deltaMin = delta(nodeIndex, nodeIndex - direction);
    int maxLength = 2;
    while (delta(nodeIndex, nodeIndex + maxLength * direction) > deltaMin)
        maxLength *= 2;

    int rangeLength = 0;
    for (int step = maxLength / 2; step >= 1; step /= 2)
    {
        if (delta(nodeIndex, nodeIndex + (rangeLength + step) * direction) > deltaMin)
            rangeLength += step;
    }
    int rangeEnd = nodeIndex + rangeLength * direction;

    int deltaNode = delta(nodeIndex, rangeEnd);
    int splitOffset = 0;
    int stride = rangeLength;

    do
    {
        stride = (stride + 1) >> 1;  // Ceiling division by 2
        if (delta(nodeIndex, nodeIndex + (splitOffset + stride) * direction) > deltaNode)
        {
            splitOffset += stride;
        }
    } while (stride > 1);

    int splitPosition = nodeIndex + splitOffset * direction + min(direction, 0);

    uint leftChildIdx = (min(nodeIndex, rangeEnd) == splitPosition) ?
            (bvhOffset + triangleCount - 1 + splitPosition) :  // Leaf
            (bvhOffset + splitPosition);                        // Internal

    uint rightChildIdx = (max(nodeIndex, rangeEnd) == splitPosition + 1) ?
        (bvhOffset + triangleCount - 1 + splitPosition + 1) : // Leaf
        (bvhOffset + splitPosition + 1);

    uint internalNodeIdx = bvhOffset + nodeIndex;

    BVHNode node = bvhNodes[internalNodeIdx];
    node.leftChildOrFirstIndex = leftChildIdx;
    node.rightChild = rightChildIdx;
    node.count = 0;
    node.visitCount = 0;
    // node.parent = 0xFFFFFFFF;

    if (nodeIndex == 0)
        bvhNodes[internalNodeIdx].parent = internalNodeIdx;

    bvhNodes[internalNodeIdx] = node;

    // Store parent links (needed for bottom-up AABB pass)
    bvhNodes[leftChildIdx].parent = internalNodeIdx;
    bvhNodes[rightChildIdx].parent = internalNodeIdx;
}


int delta(uint i, uint j)
{
    if (j < 0 || j >= triangleCount)
        return -1;

    uint code_i = getMorton(i);
    uint code_j = getMorton(j);

    if (code_i == code_j)
    {
        // Use index to break ties
        return 32 + countLeadingZeros(i ^ j);
    }
    return countLeadingZeros(code_i ^ code_j);
}
