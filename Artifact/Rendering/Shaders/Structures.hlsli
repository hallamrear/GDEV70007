
cbuffer FrameConstantBuffer : register(b0)
{
    float4x4 View;
    float4x4 Projection;
    float4 CameraPosition;
    float4 CameraDirection;
    float4 FCBPadding[6];
};

cbuffer PushConstantBuffer : register(b1)
{
    float4x4 World;
    float4x4 PCBPadding;
}

struct VS_STANDARD_VERTEX_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
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