#ifndef NonUniformResourceIndex
    #define NonUniformResourceIndex(index) index
#endif

Texture2D gTextures[];
SamplerState gSamplers[];

float4 Sample(uint textureIndex, uint samplerIndex, float2 uv)
{
    return gTextures[NonUniformResourceIndex(textureIndex)].Sample(gSamplers[NonUniformResourceIndex(samplerIndex)], uv);
}
