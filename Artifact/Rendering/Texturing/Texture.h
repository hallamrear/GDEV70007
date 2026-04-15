#pragma once

struct ID3D12Resource;
struct D3D12_CPU_DESCRIPTOR_HANDLE;

class Texture
{
private:
	friend class AssetLoader;
	friend class DX12Renderer;

	int m_ID;
	int m_Width;
	int m_Height;
	bool m_IsLoaded;
	ID3D12Resource* m_Resource;
	std::string m_DisplayName;
	std::string m_FileLocation;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;

public:
	Texture();
	~Texture();

	const std::string& GetDisplayName() const;

	ID3D12Resource* GetResource() const;
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const;
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const;
	const int& GetID() const;
	const bool& IsLoaded() const;
};

typedef std::shared_ptr<class Texture> TextureRef;
typedef std::weak_ptr<class Texture> TexturePtr;