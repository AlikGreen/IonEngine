import bvh;

struct TriangleBounds
{
    float3 aabbMin;
    float3 centroid_;
    float3 aabbMax;
    uint primitiveIndex;
};


cbuffer BuildParams
{
    uint startIndex;
    uint triangleCount;
    uint vertexOffset;
    uint boundsOffset;
};

RWStructuredBuffer<TriangleBounds> triangleBounds;
RWStructuredBuffer<uint> mortonCodes;
RWStructuredBuffer<uint> triIndices;

ByteAddressBuffer vertexBuffer;
StructuredBuffer<uint> indexBuffer;

struct MeshBounds
{
    float3 min;
    float3 max;
};

StructuredBuffer<MeshBounds> meshBounds;

uint expandBits(uint v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

uint morton3D(float3 pos)
{
    // Normalize to [0, 1]
    pos = saturate((pos - meshBounds[0].min) / (meshBounds[0].max - meshBounds[0].min));

    // Scale to 10-bit integers [0, 1023]
    uint x = clamp(uint(pos.x * 1024.0f), 0u, 1023u);
    uint y = clamp(uint(pos.y * 1024.0f), 0u, 1023u);
    uint z = clamp(uint(pos.z * 1024.0f), 0u, 1023u);

    // Interleave bits
    return expandBits(x) | (expandBits(y) << 1) | (expandBits(z) << 2);
}


[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint localTriIdx = dispatchThreadID.x;
    if (localTriIdx >= triangleCount)
        return;

    // Get triangle vertices
    uint globalTriIdx = startIndex + localTriIdx;
    uint i0 = indexBuffer[globalTriIdx * 3 + 0];
    uint i1 = indexBuffer[globalTriIdx * 3 + 1];
    uint i2 = indexBuffer[globalTriIdx * 3 + 2];

    float3 v0 = getVertex(vertexBuffer, i0).position;
    float3 v1 = getVertex(vertexBuffer, i1).position;
    float3 v2 = getVertex(vertexBuffer, i2).position;

    // Compute AABB
    float3 aabbMin = min(min(v0, v1), v2);
    float3 aabbMax = max(max(v0, v1), v2);
    float3 centroid_ = (v0 + v1 + v2) / 3.0;

    // Store bounds
    TriangleBounds bounds;
    bounds.aabbMin = aabbMin;
    bounds.aabbMax = aabbMax;
    bounds.centroid_ = centroid_;
    bounds.primitiveIndex = localTriIdx;

    uint index = boundsOffset + localTriIdx;
    triangleBounds[index] = bounds;
    mortonCodes[index] = morton3D(centroid_);
    triIndices[index] = index;
}