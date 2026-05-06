#include "Structures.hlsli"
#include "Lighting.hlsli"

Texture2D<float4> DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    float4 sampleColour = DiffuseTexture.SampleLevel(LinearSampler, input.UV, 0);

    float4 lighting = CalculateLighting(sampleColour, input.PositionW.xyz, input.NormalW);

    return lighting * sampleColour;
}