struct VertexInput
{
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inUV       : TEXCOORD0;
};

struct VertexOutput
{
  float4 position : SV_Position;
};

cbuffer CameraUniforms
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 viewProjMatrix;

    float4x4 invViewMatrix;
    float4x4 invProjMatrix;
    float4x4 invViewProjMatrix;
};


cbuffer ModelUniforms
{
    float4x4 matrix;
    uint entityId;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    float4 fragPos = mul(matrix, float4(input.inPosition, 1.0));

    output.position = mul(projMatrix, mul(viewMatrix, fragPos));

    return output;
}

[shader("fragment")]
uint fragmentMain(VertexOutput inV) : SV_Target
{
    return entityId;
}
