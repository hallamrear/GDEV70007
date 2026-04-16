#pragma once
#include <Rendering/GenericBuffer.h>

class VertexBuffer : public GenericBuffer
{
protected:
	friend class DX12Renderer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

public:
	VertexBuffer();
	~VertexBuffer();

	const D3D12_VERTEX_BUFFER_VIEW& GetBufferView() const;
};

