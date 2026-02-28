import ray;
import bvh;

ByteAddressBuffer vertexBuffer;
StructuredBuffer<uint> indices;

RWTexture2D<float4> outputImage;

StructuredBuffer<BVHNode> bvh;

cbuffer CameraUniforms
{
    float4x4 camView;
    float4x4 camProj;
    float4x4 camViewProj;

    float4x4 camInvView;
    float4x4 camInvProj;
    float4x4 camInvViewProj;
};

cbuffer ModelUniforms
{
  float4x4 modelMatrix;
  float4x4 normalMatrix;
  int startIndex;
  int indexCount;
};


HitInfo intersectTriangle(Ray ray, uint triIndex)
{
    HitInfo hit;
    hit.hit = false;
    hit.distance = 1e30;

    // Get triangle vertex indices
    uint i0 = indices[triIndex * 3 + 0];
    uint i1 = indices[triIndex * 3 + 1];
    uint i2 = indices[triIndex * 3 + 2];

    // Load vertices using byte buffer
    Vertex v0_data = getVertex(vertexBuffer, i0);
    Vertex v1_data = getVertex(vertexBuffer, i1);
    Vertex v2_data = getVertex(vertexBuffer, i2);

    // Transform positions to world space
    float3 v0 = mul(modelMatrix, float4(v0_data.position, 1.0)).xyz;
    float3 v1 = mul(modelMatrix, float4(v1_data.position, 1.0)).xyz;
    float3 v2 = mul(modelMatrix, float4(v2_data.position, 1.0)).xyz;

    // ... Möller-Trumbore intersection code stays the same ...
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    float3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);

    if (abs(a) < 0.0001)
        return hit;

    float f = 1.0 / a;
    float3 s = ray.origin - v0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return hit;

    float3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);

    if (v < 0.0 || u + v > 1.0)
        return hit;

    float t = f * dot(edge2, q);

    if (t > 0.0001)
    {
        hit.hit = true;
        hit.distance = t;
        hit.position = ray.origin + ray.direction * t;

        // Transform normals to world space using normal matrix
        float3 n0 = mul((float3x3)normalMatrix, v0_data.normal);
        float3 n1 = mul((float3x3)normalMatrix, v1_data.normal);
        float3 n2 = mul((float3x3)normalMatrix, v2_data.normal);

        float w = 1.0 - u - v;
        hit.normal = normalize(n0 * w + n1 * u + n2 * v);

        // UVs are NOT transformed - they stay in texture space
        hit.uv = v0_data.uv * w + v1_data.uv * u + v2_data.uv * v;
    }

    return hit;
}

// Trace ray through scene
HitInfo traceRay(Ray ray, uint triangleStart, uint count)
{
    HitInfo closestHit;
    closestHit.hit = false;
    closestHit.distance = 1e30;

    for (uint i = 0; i < count; i++)
    {
        HitInfo hit = intersectTriangle(ray, triangleStart + i);  // use directly!
        if (hit.hit && hit.distance < closestHit.distance)
        {
            closestHit = hit;
        }
    }

    return closestHit;
}

float3 shade(HitInfo hit)
{
    if (!hit.hit)
        return float3(0.2, 0.3, 0.4);

    float3 lightDir = normalize(float3(1, 1, 1));
    float ndotl = max(0.0, dot(hit.normal, lightDir));

    float3 albedo = float3(0.8, 0.8, 0.8);
    float3 ambient = float3(0.1, 0.1, 0.1);

    return albedo * (ambient + ndotl * 0.9);
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixelCoord = dispatchThreadID.xy;
    uint2 resolution;
    outputImage.GetDimensions(resolution.x, resolution.y);

    if (pixelCoord.x >= resolution.x || pixelCoord.y >= resolution.y)
        return;

    uint triangleCount = indexCount / 3;

    Ray ray = Ray::generate(pixelCoord, resolution, camInvViewProj);

    #define MAX_STACK_DEPTH 32

    if (!bvh[0].getAABB().intersects(ray))
    {
        outputImage[pixelCoord] = float4(0.0, 0.0, 0.3, 1.0);
        return;
    }

    uint nodeStack[MAX_STACK_DEPTH];
    nodeStack[0] = 0;
    uint stackPtr = 1;

    HitInfo closestHit;
    closestHit.hit = false;
    closestHit.distance = 1e30;
    uint leavesVisited = 0;

    while (stackPtr > 0)
    {
        BVHNode current = bvh[nodeStack[--stackPtr]];

        if (current.isLeaf())
        {
            leavesVisited++;
            HitInfo hit = traceRay(ray, current.leftChildOrFirstIndex, current.count);

            if (hit.hit && hit.distance < closestHit.distance)
            {
                closestHit = hit;
            }
            continue;
        }

        uint leftIdx = current.leftChildOrFirstIndex;
        uint rightIdx = current.rightChild;

        float tLeft = bvh[leftIdx].getAABB().intersectDist(ray);
        float tRight = bvh[rightIdx].getAABB().intersectDist(ray);

        bool hitLeft = (tLeft >= 0.0);  // Remove: && tLeft < closestHit.distance
        bool hitRight = (tRight >= 0.0);

        if (hitLeft && hitRight)
        {
            if (tLeft < tRight)
            {
                nodeStack[stackPtr++] = rightIdx;
                nodeStack[stackPtr++] = leftIdx;
            } else
            {
                nodeStack[stackPtr++] = leftIdx;
                nodeStack[stackPtr++] = rightIdx;
            }
        }
        else if (hitLeft) nodeStack[stackPtr++] = leftIdx;
        else if (hitRight) nodeStack[stackPtr++] = rightIdx;
    }

    outputImage[pixelCoord] = float4(float(leavesVisited) / 12.0, 0.0, 0.0, 1.0);
}