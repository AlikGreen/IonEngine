#include "material.hlsli"


struct MaterialData
{
    uint albedoMap;
    uint albedoSampler;

    uint normalMap;
    uint normalSampler;

    uint metallicRoughnessMap;
    uint metallicRoughnessSampler;

    bool hasNormalMap;

    float4 baseColor;
    float  roughness;
    float  metallic;
    float  normalScale;
    float  alphaCutoff;
}

StructuredBuffer<MaterialData> mat;

SurfaceOutput evaluate(SurfaceInput input, int index)
{
    float4 albedoSample = Sample(mat[index].albedoMap, mat[index].albedoSampler, input.uv);

    float4 mrSample = Sample(mat[index].metallicRoughnessMap, mat[index].metallicRoughnessSampler, input.uv);

    float3 normalTS = float3(0.0, 0.0, 1.0);
    if (mat[index].hasNormalMap)
    {
        normalTS = Sample(mat[index].normalMap, mat[index].normalSampler, input.uv) .xyz * 2.0 - 1.0;
        normalTS.xy *= mat[index].normalScale;
        normalTS = normalize(normalTS);
    }

    SurfaceOutput o;
    o.baseColor = mat[index].baseColor.rgb * albedoSample.rgb;
    o.normal = input.normalWS;
    o.roughness = saturate(mat[index].roughness * mrSample.g);
    o.metallic = saturate(mat[index].metallic * mrSample.b);
    o.opacity = albedoSample.a;
    o.emission = float3(0.0, 0.0, 0.0);

    return o;
}
