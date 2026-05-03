#ifndef __STRUCTURES_HLSL__
#define __STRUCTURES_HLSL__

#define MAX_LIGHT_COUNT 96

struct Light
{
	int Type;
	float SpecularStrength;
	float SpecularPower;
	float AmbientStrength;
	float3 Position;
	float InnerCutoff;
	float OuterCutoff;
	float3 Direction;
	float4 Colour;
	float4 Diffuse;
	float4 Attenuation;
	float4 Strength;
	bool IsEnabled;
	bool Padding[15];
};

cbuffer FrameConstantBuffer : register(b0)
{
	float4x4 View;
	float4x4 Projection;
	float4 CameraPosition;
	float4 CameraDirection;
	float DeltaTime;
	float3 PaddingThree;
	float4 Padding[5];
};

cbuffer PushConstantBuffer : register(b1)
{
    float4x4 World;
    float4x4 PCBPadding;
};

cbuffer LightingDataBuffer : register(b2)
{
    Light LightData[MAX_LIGHT_COUNT];
};

struct VS_STANDARD_VERTEX_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POSITION;
    float3 Normal : NORMAL0;
    float3 NormalW : NORMAL1;
    float3 Tangent : TANGENT0;
    float3 TangentW : TANGENT1;
    float2 UV : TEXCOORD;
};

struct VS_STANDARD_VERTEX_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

#endif //__STRUCTURES_HLSL__