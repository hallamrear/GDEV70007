#include "pch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
	m_IndexBufferView = {};
}

IndexBuffer::~IndexBuffer()
{
	assert(!IsLoaded());

	m_IndexBufferView = {};

	if (IsLoaded())
	{
		Destroy();
	}
}

const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetBufferView() const
{
	return m_IndexBufferView;
}
