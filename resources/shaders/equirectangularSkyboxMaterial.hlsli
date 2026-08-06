#include "material.hlsli"

static const float PI = 3.14159265359;

float2 dirToEquirectUv(float3 dir)
{
    dir = normalize(dir);

    float phi = atan2(dir.z, dir.x);
    float theta = asin(clamp(-dir.y, -1.0, 1.0));

    float u = phi / (2.0 * PI) + 0.5;
    float v = 0.5 - (theta / PI);

    return float2(u, v);
}

struct MaterialData
{
    uint albedoMap;
    uint albedoSampler;

    float4 baseColor;
};

StructuredBuffer<MaterialData> mat;

SurfaceOutput evaluate(SurfaceInput i, int index)
{
    float4 worldPos = mul(camera.invViewProj, float4(i.positionWS, 1.0));
    float3 worldPos3 = worldPos.xyz / worldPos.w;
    float3 viewDir = normalize(worldPos3 - camera.position);

    float2 texCoord = dirToEquirectUv(viewDir);
    float4 albedoSample = Sample(mat[index].albedoMap, mat[index].albedoSampler, texCoord);

    SurfaceOutput o;
    o.baseColor = albedoSample.rgb;
    o.opacity = 1.0;
    o.emission = float3(0.0, 0.0, 0.0);

    return o;
}