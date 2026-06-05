#include "pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
	m_VertexBufferView = {};
}

VertexBuffer::~VertexBuffer()
{
	//assert(!IsLoaded());

	m_VertexBufferView = {};

	if (IsLoaded())
	{
		Destroy();
	}
}

const D3D12_VERTEX_BUFFER_VIEW& VertexBuffer::GetBufferView() const
{
	return m_VertexBufferView;
}
