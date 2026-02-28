struct Ray
{
    float3 origin;
    float3 direction;

    float3 at(float t)
    {
        return this.origin + this.direction * t;
    }

    __init(float3 origin, float3 direction)
    {
        Ray ray;
        ray.origin = origin;
        ray.direction = normalize(direction);
        return ray;
    }

    static Ray generate(uint2 pixelCoord, uint2 resolution, float4x4 camInvViewProj)
    {
        float2 ndc = (float2(pixelCoord) + 0.5) / float2(resolution);
        ndc = ndc * 2.0 - 1.0;

        float4 clipNear = float4(ndc, 0.0, 1.0);
        float4 clipFar = float4(ndc, 1.0, 1.0);

        float4 worldNear = mul(camInvViewProj, clipNear);
        float4 worldFar = mul(camInvViewProj, clipFar);

        worldNear.xyz /= worldNear.w;
        worldFar.xyz /= worldFar.w;

        Ray ray;
        ray.origin = worldNear.xyz;
        ray.direction = normalize(worldFar.xyz - worldNear.xyz);

        return ray;
    }

};

struct HitInfo
{
    static const float MAX_DISTANCE = 1e30;

    bool hit;
    float distance;
    float3 position = { };
    float3 normal = { };
    float2 uv = { };
    uint triangleIndex = 0;
    uint meshIndex = 0;

    __init(bool hit, float distance)
    {
        this.hit = hit;
        this.distance = distance;
    }

    bool isCloserThan(HitInfo other)
    {
        return this.hit && this.distance < other.distance;
    }

    [mutating]
    bool updateIfCloser(HitInfo other)
    {
        if (other.hit && other.distance < this.distance)
        {
            this = other;
            return true;
        }
        return false;
    }

    // Create miss
    static HitInfo createMiss()
    {
        HitInfo hit = HitInfo(false, MAX_DISTANCE);
        return hit;
    }
};