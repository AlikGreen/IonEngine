import material;

struct PbrMaterial : IMaterial
{
    Texture2D   albedoMap;
    SamplerState albedoSampler;

    Texture2D   normalMap;
    SamplerState normalSampler;

    Texture2D   metallicRoughnessMap;
    SamplerState metallicRoughnessSampler;

    bool hasNormalMap;

    float3 baseColor = float3(1.0, 1.0, 1.0);
    float  roughness = 0.5;
    float  metallic = 0.0;
    float  normalScale = 1.0;
    float  alphaCutoff = 0.5;

    SurfaceOutput evaluateSurface(SurfaceInput i)
    {
        float4 albedoSample = albedoMap.Sample(albedoSampler, i.uv);

        float4 mrSample = metallicRoughnessMap.Sample(metallicRoughnessSampler, i.uv);

        float3 normalTS = float3(0.0, 0.0, 1.0);
        if (hasNormalMap)
        {
            normalTS = normalMap.Sample(normalSampler, i.uv).xyz * 2.0 - 1.0;
            normalTS.xy *= normalScale;
            normalTS = normalize(normalTS);
        }

        SurfaceOutput o;
        o.baseColor = baseColor * albedoSample.rgb;
        o.normalWS = i.normalWS;
        o.roughness = saturate(roughness * mrSample.g);
        o.metallic = saturate(metallic * mrSample.b);
        o.opacity = albedoSample.a;
        o.emission = float3(0.0, 0.0, 0.0);

        return o;
    }
};