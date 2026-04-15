#pragma once
#include <Rendering/DX12Includes.h>

struct ID3D12Resource;

class Texture
{
private:
	friend class TextureLoader;
	int m_ID;
	int m_Width;
	int m_Height;
	bool m_IsLoaded;
	ID3D12Resource* m_Resource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;

	void Destroy();

public:
	Texture();
	~Texture();

	ID3D12Resource* GetResource() const;
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const;
	const int& GetID() const;
	const bool& IsLoaded() const;
};

typedef std::shared_ptr<class Texture> TextureRef;
typedef std::weak_ptr<class Texture> TexturePtr;