#include "util.hlsli"
#include "meshletPipelineUtil.hlsli"

struct CameraData
{
    float4 frustumPlanes[6];
    float3 position;
    uint padding;
};


StructuredBuffer<MeshletInstance> gMeshletInstances;
StructuredBuffer<Meshlet> gMeshlets;

StructuredBuffer<MeshPrimitive> gPrimitives;
StructuredBuffer<ModelInfo> gModelData;
// StructuredBuffer<float4x4> gTransforms;

RWStructuredBuffer<VisibleMeshletEntry> gVisibleMeshlets;
RWStructuredBuffer<DispatchIndirectArgs> gIndirectDispatchArgs;

RWStructuredBuffer<uint> gIndexCountPerPrimitive;

ConstantBuffer<CameraData> gCamera;

RWStructuredBuffer<float4> gDebugLines;
RWStructuredBuffer<DrawIndirectArgs> gDebugLinesDraw;

struct PushConstants
{
    uint meshletCount;
};

[[ion::push_constant]]
ConstantBuffer<PushConstants> pc;

bool outsidePlane(float4 plane, float3 center, float3 extents)
{
    float3 n = plane.xyz;
    float d = plane.w;

    float s = dot(n, center) + d;
    float r = dot(abs(n), extents);

    return s + r < 0.0f;
}

bool intersectsFrustum(float3 aabbMin, float3 aabbMax, float4x4 worldMat)
{
    float3 boxCenter  = (aabbMin + aabbMax) * 0.5f;
    float3 boxExtents = (aabbMax - aabbMin) * 0.5f;

    // M^T transforms planes from world space into object space
    float4x4 worldToLocalPlaneMat = transpose(worldMat);

    [unroll]
    for (int i = 0; i < 6; i++)
    {
        float4 localPlane = mul(worldToLocalPlaneMat, gCamera.frustumPlanes[i]);
        if (outsidePlane(localPlane, boxCenter, boxExtents))
            return false;
    }
    return true;
}

bool shouldCull(Meshlet meshlet, ModelInfo model)
{
    // return false;
    return !intersectsFrustum(meshlet.aabbMin, meshlet.aabbMax, model.modelMatrix);
}

static const uint2 edges[12] =
{
    uint2(0,1), uint2(1,2), uint2(2,3), uint2(3,0),
    uint2(4,5), uint2(5,6), uint2(6,7), uint2(7,4),
    uint2(0,4), uint2(1,5), uint2(2,6), uint2(3,7)
};

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= pc.meshletCount) return;

    if (dispatchThreadID.x == 0)
    {
        gIndirectDispatchArgs[0].threadGroupsY = 1;
        gIndirectDispatchArgs[0].threadGroupsZ = 1;

        gDebugLinesDraw[0].instanceCount = 1;
    }

    MeshletInstance inst = gMeshletInstances[dispatchThreadID.x];
    Meshlet meshlet = gMeshlets[inst.meshletIndex];
    MeshPrimitive prim = gPrimitives[inst.primitiveId];
    ModelInfo model = gModelData[prim.transformIndex];

    if(shouldCull(meshlet, model))
        return;

    uint visibleIdx;
    InterlockedAdd(gIndirectDispatchArgs[0].threadGroupsX, 1, visibleIdx);


    if (dispatchThreadID.x < 500)
    {
        int debugIdx = 0;
        InterlockedAdd(gDebugLinesDraw[0].vertexCount, 24, debugIdx);

        float3 localCorners[8] =
        {
            float3(meshlet.aabbMin.x, meshlet.aabbMin.y, meshlet.aabbMin.z),
            float3(meshlet.aabbMax.x, meshlet.aabbMin.y, meshlet.aabbMin.z),
            float3(meshlet.aabbMax.x, meshlet.aabbMax.y, meshlet.aabbMin.z),
            float3(meshlet.aabbMin.x, meshlet.aabbMax.y, meshlet.aabbMin.z),
            float3(meshlet.aabbMin.x, meshlet.aabbMin.y, meshlet.aabbMax.z),
            float3(meshlet.aabbMax.x, meshlet.aabbMin.y, meshlet.aabbMax.z),
            float3(meshlet.aabbMax.x, meshlet.aabbMax.y, meshlet.aabbMax.z),
            float3(meshlet.aabbMin.x, meshlet.aabbMax.y, meshlet.aabbMax.z)
        };

        float3 corners[8];
        for (uint i = 0; i < 8; ++i)
        {
            corners[i] = mul(model.modelMatrix, float4(localCorners[i], 1.0f)).xyz;
        }

        for (uint i = 0; i < 12; ++i)
        {
            gDebugLines[debugIdx + i * 2 + 0] = float4(corners[edges[i].x], 0.0);
            gDebugLines[debugIdx + i * 2 + 1] = float4(corners[edges[i].y], 0.0);
        }
    }

    VisibleMeshletEntry entry;
    entry.meshletId = inst.meshletIndex;
    entry.primitiveId = inst.primitiveId;
    gVisibleMeshlets[visibleIdx] = entry;

    InterlockedAdd(gIndexCountPerPrimitive[inst.primitiveId], meshlet.indexCount);
}

