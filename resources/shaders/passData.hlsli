#include "globals.hlsli"

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;

    float4x4 invView;
    float4x4 invProj;
    float4x4 invViewProj;

    float3 position;
};

struct PassData
{
    float time;
    float deltaTime;
    uint  frameCount;

    float2 resolution;
    float2 invResolution;
};

struct SurfaceInput
{
    float3 positionWS;
    float3 normalWS;
    float3 viewDirWS;
    float2 uv;
};

struct SurfaceOutput
{
    float3 baseColor;
    float3 normal;
    float  roughness;
    float  metallic;
    float  opacity;
    float3 emission;
};