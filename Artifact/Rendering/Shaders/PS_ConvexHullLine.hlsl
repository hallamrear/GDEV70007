#include "Structures.hlsli"

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    colour.x = PCBPadding[0][0];
    colour.y = PCBPadding[0][1];
    colour.z = PCBPadding[0][2];

    return float4(colour.xyz, 1.0f);
}