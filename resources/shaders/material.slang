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
    float3 normalWS;
    float  roughness;
    float  metallic;
    float  opacity;
    float3 emission;
};

interface IMaterial
{
    SurfaceOutput evaluate(SurfaceInput i);
}