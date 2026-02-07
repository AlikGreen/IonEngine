
struct VertexInput
{
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inUV       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 vUV      : TEXCOORD1;
};

cbuffer MatrixUniforms
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 modelMatrix;
};

Texture2D<float4> gizmoTexture;
SamplerState gizmoSampler;

float4 circle(float2 uv, float2 pos, float rad, float3 color)
{
	float d = length(pos - uv) - rad;
	float t = clamp(d, 0.0, 1.0);
	return float4(color, 1.0 - t);
}


[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    float4 fragPos = mul(modelMatrix, float4(input.inPosition, 1.0));
    output.position = mul(projMatrix, mul(viewMatrix, fragPos));
    output.vUV = input.inUV;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput inV) : SV_Target
{
    float4 albedo = gizmoTexture.Sample(gizmoSampler, inV.vUV);
    return albedo;
}
