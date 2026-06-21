struct ModelInfo
{
    float4x4 matrix;
    float4x4 normalMatrix;
};

struct MeshPrimitive
{
    uint meshletStart;
    uint meshletCount;
    uint transformIndex;
    uint materialIndex;
    uint templateIndex;
    uint vertexStart;
    uint indexStart;
    uint padding;
};

struct Meshlet
{
    float3 aabbMin;
    uint   indexStart;
    float3 aabbMax;
    uint   indexCount;

    float3 coneApex;
    float  coneCutoff;
    float3 coneAxis;
    uint   padding;
};

struct MeshletInstance
{
    uint meshletIndex;
    uint primitiveId;
};

struct DispatchIndirectArgs
{
    Atomic<uint> threadGroupsX;
    Atomic<uint> threadGroupsY;
    Atomic<uint> threadGroupsZ;
};

struct DrawIndirectArgs
{
    Atomic<uint> vertexCount;
    Atomic<uint> instanceCount;
    Atomic<int>  baseVertex;
    Atomic<uint> baseInstance;
};

struct VisibleMeshletEntry
{
    uint meshletId;
    uint primitiveId;
};
