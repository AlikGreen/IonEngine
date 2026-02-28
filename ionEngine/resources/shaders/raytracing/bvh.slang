import ray;

namespace BVHConstants
{
    static const float EPSILON = 0.0001;
    static const float MAX_DISTANCE = 1e30;
    static const uint INVALID_INDEX = 0xFFFFFFFF;
    static const uint MAX_STACK_DEPTH = 32;
}

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
};

Vertex getVertex(ByteAddressBuffer vertexBuffer, uint index)
{
    uint offset = index * 32;

    Vertex v;
    v.position = asfloat(vertexBuffer.Load3(offset + 0));
    v.normal = asfloat(vertexBuffer.Load3(offset + 12));
    v.uv = asfloat(vertexBuffer.Load2(offset + 24));

    return v;
}

struct AABB
{
    float3 min_;
    float3 max_;

    __init(float3 min, float3 max)
    {
        min_ = min;
        max_ = max;
    }

    __init()
    {
        min_ = float3(BVHConstants.MAX_DISTANCE);
        max_ = float3(-BVHConstants.MAX_DISTANCE);
    }
    
    // Intersection test with ray
    bool intersects(Ray ray)
    {
        float3 invDir = 1.0 / ray.direction;
        float3 t0 = (this.min_ - ray.origin) * invDir;
        float3 t1 = (this.max_ - ray.origin) * invDir;
        
        float3 tmin = min(t0, t1);
        float3 tmax = max(t0, t1);
        
        float tNear = max(max(tmin.x, tmin.y), tmin.z);
        float tFar = min(min(tmax.x, tmax.y), tmax.z);
        
        return tNear <= tFar && tFar >= 0.0;
    }

    float intersectDist(Ray ray)
    {
        float3 invDir = 1.0 / ray.direction;
        float3 t0 = (this.min_ - ray.origin) * invDir;
        float3 t1 = (this.max_ - ray.origin) * invDir;

        float3 tmin = min(t0, t1);
        float3 tmax = max(t0, t1);

        float tNear = max(max(tmin.x, tmin.y), tmin.z);
        float tFar = min(min(tmax.x, tmax.y), tmax.z);

        if (tNear > tFar || tFar < 0.0)
            return -1;

        return max(0.0, tNear);;
    }

    bool intersectsRobust(Ray ray, float tMin, float tMax)
    {
        for (int i = 0; i < 3; i++)
        {
            float invD = 1.0 / ray.direction[i];
            float t0 = (this.min_[i] - ray.origin[i]) * invD;
            float t1 = (this.max_[i] - ray.origin[i]) * invD;
            
            if (invD < 0.0)
            {
                float temp = t0;
                t0 = t1;
                t1 = temp;
            }
            
            tMin = max(t0, tMin);
            tMax = min(t1, tMax);
            
            if (tMax < tMin)
                return false;
        }
        
        return true;
    }
    
    // Expand to include a point
    [mutating]
    void expand(float3 point)
    {
        min_ = min(min_, point);
        max_ = max(max_, point);
    }
    
    // Expand to include another AABB
    [mutating]
    void expand(AABB other)
    {
        min_ = min(min_, other.min_);
        max_ = max(max_, other.max_);
    }
    
    // Get center
    float3 center()
    {
        return (min_ + max_) * 0.5;
    }
    
    // Get extent/size
    float3 extent()
    {
        return max_ - min_;
    }
    
    // Get surface area (for SAH)
    float surfaceArea()
    {
        float3 e = this.extent();
        return 2.0 * (e.x * e.y + e.y * e.z + e.z * e.x);
    }
    
    // Get squared distance from point to AABB center
    float distanceSqToCenter(float3 point)
    {
        float3 c = this.center();
        float3 diff = c - point;
        return dot(diff, diff);
    }
    
    // Check if valid
    bool isValid()
    {
        return all(min_ <= max_);
    }
};

// =============================================================================
// BVH Node
// =============================================================================

struct BVHNode
{
    float3 aabbMin;
    uint leftChildOrFirstIndex;

    float3 aabbMax;
    uint rightChild;

    uint parent;
    uint visitCount;
    uint count;

    AABB getAABB()
    {
        return AABB(aabbMin, aabbMax);
    }

    bool isLeaf()
    {
        return count > 0;
    }

    bool isInternal()
    {
        return count == 0;
    }

    uint getPrimitiveCount()
    {
        return count;
    }
};
