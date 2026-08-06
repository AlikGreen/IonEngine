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

struct VertexInput
{
    float4 position : ATTRIB0;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

ConstantBuffer<Camera> gCamera;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(gCamera.proj, mul(gCamera.view, float4(input.position.xyz, 1.0)));
    return output;
}

[shader("pixel")]
float4 fragmentMain() : SV_Target
{
    return float4(0.0, 1.0, 0.0, 1.0);
}
