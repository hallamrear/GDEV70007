#pragma once
#include <Rendering/GenericBuffer.h>

class IndexBuffer : public GenericBuffer
{
protected:
	friend class DX12Renderer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

public:
	IndexBuffer();
	~IndexBuffer();

	const D3D12_INDEX_BUFFER_VIEW& GetBufferView() const;
};

