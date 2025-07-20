Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);

cbuffer Globals : register(b0)
{
    uint2 PixelOffset;
    uint2 Padding;
};

[numthreads(1, 1, 1)]
void main(uint2 dispatchThreadId : SV_DispatchThreadID)
{
    output[dispatchThreadId] = input[uint2(PixelOffset.x + dispatchThreadId.x, PixelOffset.y - dispatchThreadId.y)];
}
