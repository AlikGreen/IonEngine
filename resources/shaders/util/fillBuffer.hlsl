RWByteAddressBuffer gBuffer;

struct PushConstants
{
    uint offset;
    uint size;
    uint value;
};

[[ion::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if(dispatchThreadID.x >= pc.size) return;

    uint index = dispatchThreadID.x + pc.offset;
    gBuffer.Store(index * 4, pc.value);
}

