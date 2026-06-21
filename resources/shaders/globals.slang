Texture2D gTexture2Ds[];

struct Texture2DHandle
{
    uint index;

    float4 sample(SamplerState sampler, float2 uv)
    {
        return gTexture2Ds[index].Sample(sampler, uv);
    }
};