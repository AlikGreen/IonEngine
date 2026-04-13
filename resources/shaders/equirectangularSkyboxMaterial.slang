import material;

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

struct EquirectangularSkyboxMaterial : IMaterial
{
    Texture2D   albedoMap;
    SamplerState albedoSampler;

    float4 baseColor = float4(1.0, 1.0, 1.0, 1.0);

    SurfaceOutput evaluate(SurfaceInput i)
    {
        float4 worldPos = mul(camera.invViewProj, float4(i.positionWS, 1.0));
        float3 worldPos3 = worldPos.xyz / worldPos.w;
        float3 viewDir = normalize(worldPos3 - camera.position);

        float2 texCoord = dirToEquirectUv(viewDir);
        float4 albedoSample = albedoMap.SampleLevel(albedoSampler, texCoord, 0);

        SurfaceOutput o;
        o.baseColor = albedoSample.rgb;
        o.opacity = 1.0;
        o.emission = float3(0.0, 0.0, 0.0);

        return o;
    }
};