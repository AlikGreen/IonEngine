// Slang shader (vertex + fragment). No register annotations.

struct VertexInput
{
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inUV       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position  : SV_Position;
    float3 vWorldDir : TEXCOORD1;
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

Texture2D<float4> skyboxTexture;
SamplerState linearSampler;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.position = float4(input.inPosition, 1.0);

    float4x4 invProj = invProjMatrix;
    float3x3 viewRot = (float3x3)viewMatrix;
    float3x3 invViewRot = transpose(viewRot);

    float4 viewPos = mul(invProj, float4(input.inPosition.xy, 1.0, 1.0));
    output.vWorldDir = mul(invViewRot, viewPos.xyz);

    return output;
}

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

[shader("fragment")]
float4 fragmentMain(VertexOutput inV, out float oDepth : SV_Depth) : SV_Target
{

    oDepth = 1.0 - 1e-5;
    float3 dir = normalize(inV.vWorldDir);
    float2 uv = dirToEquirectUv(dir);
    float3 envColor = skyboxTexture.Sample(linearSampler, uv).rgb;

    envColor = envColor / (envColor + float3(1.0, 1.0, 1.0));
    envColor = pow(envColor, float3(1.0 / 2.2));

    return float4(envColor, 1.0);
}
