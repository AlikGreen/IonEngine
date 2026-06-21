module genericForwardShader;

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

struct ModelInfo
{
    float4x4 matrix;
    float4x4 normalMatrix;
};

struct PointLight
{
    float3 position;
    float power;
    float3 color;
    float radius;
};

struct PointLights
{
    int count;
    PointLight[64] lights;
};


type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> material;

ConstantBuffer<PointLights> pointLights;
StructuredBuffer<ModelInfo> modelData;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint primitiveIndex : SV_StartInstanceLocation)
{
    ModelInfo model = modelData[primitiveIndex];

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
    float3 viewDir = normalize(camera.position - input.fragPos);

    SurfaceInput si;
    si.normalWS = normalize(input.normal);
    si.positionWS = input.fragPos;
    si.viewDirWS = viewDir;
    si.uv = input.uv;

    SurfaceOutput so; // = material.evaluate(si);
    so.baseColor = float3(1.0, 1.0, 1.0);

    float3 accumulatedLight = float3(0.0);

    // TODO implement pbr

    for(int i = 0; i < pointLights.count; i++)
    {
        PointLight pointLight = pointLights.lights[i];

        float3 L = pointLight.position - input.fragPos;
        float distSquared = dot(L, L);
        float3 lightDir = normalize(L);

        float nDotL = max(0.0, dot(si.normalWS, lightDir));

        float attenuation = pointLight.power / (distSquared + 0.0001);
        float3 diffuse = pointLight.color * nDotL * attenuation;

        accumulatedLight += diffuse;
    }

    float3 ambient = so.baseColor * 0.1;
    float3 finalColor = accumulatedLight * so.baseColor + ambient;

    // Tone mapping
    float3 toneMapped = finalColor / (finalColor + float3(1.0));

    return float4(toneMapped, 1.0);
}