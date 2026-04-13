#include "Structures.hlsli"

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
	return float4(input.Normal, 1.0f);
}