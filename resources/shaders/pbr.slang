// Slang shader (vertex + fragment). No register annotations.

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
    float4x4 normalMatrix;
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
Texture2D<float4> emissionTexture;
SamplerState emissionSampler;

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
    float3 diffuseLight = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < pointLights.count; ++i)
    {
        PointLight light = pointLights.lights[i];
        float3 lightDir = light.position - input.fragPos;
        float dist = length(lightDir);
        lightDir = lightDir / dist;
        float invDistSquare = 1.0 / (dist * dist);
        float lambertian = max(dot(lightDir, input.normal), 0.0);

        diffuseLight += light.color * light.power * lambertian * invDistSquare;
    }

    diffuseLight += float3(0.1);

    float4 albedo = properties.albedo;
    albedo *= albedoTexture.Sample(albedoSampler, input.uv);

    const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

    float3 colorLinear = albedo.xyz * diffuseLight;

    float3 colorGammaCorrected = pow(colorLinear, float3(1.0 / screenGamma));

    return float4(colorGammaCorrected, 1.0);
}
