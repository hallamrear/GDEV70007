#include "Structures.hlsli"

SamplerState LinearSampler : register(s0);
Texture2D<float4> BroadPhaseTexture : register(t1);
Texture2D<float4> NarrowPhaseTexture : register(t2);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (PCBPadding[2][2] > 0.0f)
    {
        if(PCBPadding[2][2] > 1.0f)
        {
            return float4(0.0f, 1.0f / PCBPadding[2][2], 0.0f, 1.0f);
        }
        else
        {
            return float4(0.0f, 0.0f, 1.0f, 1.0f);
        }
    }
    
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}