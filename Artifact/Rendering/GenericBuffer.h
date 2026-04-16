#pragma once

class GenericBuffer
{
protected:
	friend class DX12Renderer;

	ID3D12Resource* m_Resource;
	UINT m_ElementCount;

public:
	GenericBuffer();
	virtual ~GenericBuffer() = 0;

	ID3D12Resource* GetResource();
	const bool IsLoaded() const;
	const UINT& GetElementCount() const;

	void Destroy();
};

