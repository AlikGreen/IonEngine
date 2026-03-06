// Slang shader (vertex + fragment). No register annotations.

static const int MAX_POINT_LIGHTS = 64;

struct VertexInput
{
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inUV       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 vNormal  : TEXCOORD1;
    float2 vUV      : TEXCOORD2;
    float3 vFragPos : TEXCOORD3;
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

struct Properties
{
    float roughness;
    float metanless;
    float4 albedo;
};

struct PointLight
{
    float3 position;
    float  power;
    float3 color;
    float  radius;
};

struct PointLights
{
    int count;
    PointLight lights[MAX_POINT_LIGHTS];
};

struct ModelInfo
{
    float4x4 matrix;
};

ConstantBuffer<Properties> properties;
ConstantBuffer<Camera> camera;
ConstantBuffer<PointLights> pointLights;

[[vk::push_constant]]
ModelInfo model;

Texture2D<float4> albedoTexture;
SamplerState albedoSampler;
Texture2D<float4> metallicRoughnessTexture;
SamplerState metallicRoughnessSampler;
Texture2D<float4> normalTexture;
SamplerState normalSampler;
Texture2D<float4> occlusionTexture;
SamplerState occlusionSampler;
Texture2D<float4> emmisionTexture;
SamplerState emmisionSampler;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    output.vNormal = input.inNormal;
    output.vUV = input.inUV;

    float4 fragPos = mul(model.matrix, float4(input.inPosition, 1.0));
    output.vFragPos = fragPos.xyz;

    output.position = mul(camera.proj, mul(camera.view, fragPos));

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput inV) : SV_Target
{
    float3 diffuseLight = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < pointLights.count; ++i)
    {
        PointLight light = pointLights.lights[i];
        float dist = length(inV.vFragPos - light.position);
        float3 lightDir = light.position - inV.vFragPos;

        diffuseLight += light.color * (1.0 / dist) * light.power * dot(lightDir, inV.vNormal);
    }

    diffuseLight = max(diffuseLight, float3(0.2, 0.2, 0.2));

    float4 albedo = properties.albedo;
    albedo *= albedoTexture.Sample(albedoSampler, inV.vUV);

    return albedo * float4(diffuseLight, 1.0);
}
