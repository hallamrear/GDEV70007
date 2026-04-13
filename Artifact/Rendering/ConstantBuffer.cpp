#include "pch.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
	World = DirectX::XMFLOAT4X4();
	View = DirectX::XMFLOAT4X4();
	Projection = DirectX::XMFLOAT4X4();
}

ConstantBuffer::~ConstantBuffer()
{
	World = DirectX::XMFLOAT4X4();
	View = DirectX::XMFLOAT4X4();
	Projection = DirectX::XMFLOAT4X4();
}
