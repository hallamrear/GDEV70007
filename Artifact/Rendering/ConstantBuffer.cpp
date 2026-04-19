#include "pch.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
	View = Matrix4x4();
	Projection = Matrix4x4();
	CameraPosition = Vector4();
	CameraDirection = Vector4();
	DeltaTime = 0.0f;
	PaddingThree = Vector3();

	memset(&Padding, 0, sizeof(Padding));
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
