module material;

public struct Camera
{
    public float4x4 view;
    public float4x4 proj;
    public float4x4 viewProj;

    public float4x4 invView;
    public float4x4 invProj;
    public float4x4 invViewProj;

    public float3 position;
};

public struct PassData
{
    public float time;
    public float deltaTime;
    public uint  frameCount;

    public float2 resolution;
    public float2 invResolution;

    // Texture2D<float> depthBuffer;
    // Texture2D<float4> colorBuffer;
    // TextureCube       environmentMap;
    // SamplerState      linearSampler;

    // float sampleDepth(float2 uv) { return depthBuffer.SampleLevel(linearSampler, uv, 0); }
};


public ConstantBuffer<Camera> camera;
public ParameterBlock<PassData> pass;


public struct SurfaceInput
{
    public float3 positionWS;
    public float3 normalWS;
    public float3 viewDirWS;
    public float2 uv;
};

public struct SurfaceOutput
{
    public float3 baseColor;
    public float3 normal;
    public float  roughness;
    public float  metallic;
    public float  opacity;
    public float3 emission;
};

public interface IMaterial
{
    public SurfaceOutput evaluate(SurfaceInput i);
};
