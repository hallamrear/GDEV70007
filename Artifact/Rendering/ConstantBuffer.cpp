#include "pch.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
	View = Matrix4x4();
	Projection = Matrix4x4();
	CameraPosition = Vector4();
	CameraDirection = Vector4();
	for (size_t i = 0; i < 6; i++)
	{
		Padding[i] = Vector4();
	}	
}

ConstantBuffer::~ConstantBuffer()
{
	View = Matrix4x4();
	Projection = Matrix4x4();
}

PushConstants::PushConstants()
{
	World = Matrix4x4();
	Padding = Matrix4x4();
}

PushConstants::~PushConstants()
{
	World = Matrix4x4();
	Padding = Matrix4x4();
}
