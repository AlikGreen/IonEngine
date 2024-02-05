#version 330 core
in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fPos;

out vec4 FragColor;

struct Material
{
    sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    bool useAlbedoMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
};

struct DirectionalLight
{
    vec3 position;
    vec3 color;
};

struct PointLight 
{
    vec3 position;
    vec3 color;
    float radius;
    float fallOff; 
};


uniform Material mat;

uniform DirectionalLight directionalLights[10]; 
uniform int numDirectionalLights; 

uniform PointLight pointLights[20]; 
uniform int numPointLights; 
uniform vec3 ambientLightColor;

uniform vec3 camPos;

const float PI = 3.14159265359;

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}


void main()
{
    vec3 albedo = mat.useAlbedoMap ? texture(mat.albedoMap, fTexCoord).rgb : mat.albedo;
    float metallic = mat.useMetallicMap ? texture(mat.metallicMap, fTexCoord).r : mat.metallic;
    float roughness = mat.useRoughnessMap ? texture(mat.roughnessMap, fTexCoord).r : mat.roughness;
    float ao = mat.useAoMap ? texture(mat.aoMap, fTexCoord).r : mat.ao;

    vec3 N = normalize(fNormal);
    vec3 viewDir = normalize(camPos - fPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < numDirectionalLights; ++i)
    {
        vec3 L = normalize(directionalLights[i].position - fPos);
        vec3 H = normalize(viewDir + L);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, viewDir, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, viewDir), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = vec3(1.0);
        vec3 diffuse = irradiance * albedo;

        vec3 ambient = kD * diffuse * ao;
        vec3 cookTorrance = (kD * diffuse + specular) * max(dot(N, L), 0.0);

        Lo += directionalLights[i].color * (ambient + cookTorrance);
    }

    for(int i = 0; i < numPointLights; ++i)
    {
        vec3 L = normalize(pointLights[i].position - fPos);
        vec3 H = normalize(viewDir + L);

        float distance = length(pointLights[i].position - fPos);
        float attenuation = max(0.0, 1.0 - pow(distance / pointLights[i].radius, pointLights[i].fallOff));

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, viewDir, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, viewDir), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = vec3(1.0);
        vec3 diffuse = irradiance * albedo;

        vec3 ambient = kD * diffuse * ao;
        vec3 cookTorrance = (kD * diffuse + specular) * max(dot(N, L), 0.0);

        Lo += attenuation * pointLights[i].color * (ambient + cookTorrance);
    }

    Lo += ambientLightColor * mat.albedo;

    FragColor = vec4(Lo, 1.0);
}