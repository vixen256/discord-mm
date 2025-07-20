Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);

cbuffer Globals : register(b0)
{
    uint2 PixelOffset;
    uint2 Padding;
};

static const float3x3 YCbCrRgbConversion =
{
    1.0,  0.0,     1.5748,
    1.0, -0.1873, -0.4681,
    1.0,  1.8556,  0.0
};

[numthreads(1, 1, 1)]
void main(uint2 dispatchThreadId : SV_DispatchThreadID)
{
    const float CbCr_mult = 256.0 / 255.0;
    const float CbCr_sub = 128.501895 / 255.0;

    float4 YACbCr;
    YACbCr.xy = input.Load(int3(PixelOffset.x + dispatchThreadId.x, PixelOffset.y - dispatchThreadId.y, 0)).xy;
    YACbCr.zw = input.Load(int3((PixelOffset.x + dispatchThreadId.x) / 2, (PixelOffset.y - dispatchThreadId.y) / 2, 1)).xy * CbCr_mult - CbCr_sub;

    output[dispatchThreadId] = float4(mul(YCbCrRgbConversion, YACbCr.xzw), YACbCr.y);
}
