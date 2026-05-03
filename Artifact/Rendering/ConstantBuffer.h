#pragma once
#include "Lighting/Light.h"

/// <summary>
/// Constant buffer has to be 256-byte aligned in DX12.
/// </summary>
class ConstantBuffer
{
public:
	Matrix4x4 View;
	Matrix4x4 Projection;
	Vector4 CameraPosition;
	Vector4 CameraDirection;
	float DeltaTime;
	Vector3 PaddingThree;
	Vector4 Padding[5];

	ConstantBuffer();
	~ConstantBuffer();
};

struct PushConstants
{
	/* 64b */ Matrix4x4 World;
	Matrix4x4 Padding;

	PushConstants();
	~PushConstants();
};

class LightBuffer
{
public:
	Light Lights[MAX_LIGHT_COUNT];
};