#include "Structures.hlsli"

Texture2D<float4> DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    return DiffuseTexture.SampleLevel(LinearSampler, input.UV, 0);
}