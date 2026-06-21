import material;

static const int MAX_POINT_LIGHTS = 64;

struct VertexInput
{
    float2 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position  : SV_Position;
    float3 clipPos  : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

struct ModelInfo
{
    float4x4 matrix;
    float4x4 normalMatrix;
};

type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> material;

[[vk::push_constant]]
ModelInfo model;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.position = float4(input.position, 1.0, 1.0);
    output.uv = input.uv;
    output.clipPos = float3(input.position.xy, 0.0);

    return output;
}


[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    SurfaceInput si;
    si.normalWS = float3(0, 1, 0);
    si.positionWS = input.clipPos;
    si.viewDirWS = normalize(input.position.xyz - camera.view[3].xyz);
    si.uv = input.uv;

    SurfaceOutput so = material.evaluate(si);

    return float4(so.baseColor, 1.0);
}
