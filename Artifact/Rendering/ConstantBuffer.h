#pragma once
#include <DirectXMath.h>

/// <summary>
/// Constant buffer has to be 256-byte aligned in DX12.
/// </summary>
class ConstantBuffer
{
public:
	Matrix4x4 World;
	Matrix4x4 View;
	Matrix4x4 Projection;
	Matrix4x4 Padding;

	ConstantBuffer();
	~ConstantBuffer();
};

