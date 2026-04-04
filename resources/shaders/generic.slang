import material;

static const int MAX_POINT_LIGHTS = 64;

struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal  : TEXCOORD1;
    float2 uv      : TEXCOORD2;
    float3 fragPos : TEXCOORD3;
};

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;

    float4x4 invView;
    float4x4 invProj;
    float4x4 invViewProj;
}

struct ModelInfo
{
    float4x4 matrix;
    float4x4 normalMatrix;
};

ConstantBuffer<Camera> camera;
ParameterBlock<IMaterial> material;

[[vk::push_constant]]
ModelInfo model;


[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.normal = normalize(mul((float3x3)model.normalMatrix, input.normal));
    output.uv = input.uv;

    float4 fragPos = mul(model.matrix, float4(input.position, 1.0));
    output.fragPos = fragPos.xyz;

    output.position = mul(camera.proj, mul(camera.view, fragPos));

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    SurfaceInput si;
    si.normalWS = input.normal;
    si.positionWS = input.position.xyz;
    si.viewDirWS = normalize(input.position.xyz - camera.view[3].xyz);
    si.uv = input.uv;

    SurfaceOutput so = material.evaluate(si);

    return float4(so.baseColor, 1.0);
}
