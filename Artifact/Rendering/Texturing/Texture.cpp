#include "pch.h"
#include "Texture.h"
#include <Rendering/DX12Includes.h>
#include <Rendering/Renderer.h>

//void Texture::Destroy()
//{
//	if (m_IsLoaded && m_Resource != nullptr)
//	{
//		m_Resource->Release();
//		m_Resource = nullptr;
//		m_IsLoaded = false;
//		m_Height = -1;
//		m_Width = -1;
//		m_ID = -1;
//		m_CPUHandle = {};
//		m_DisplayName = "Unnamed Texture";
//	}
//}

Texture::Texture()
{
	m_ID = -1;
	m_Resource = nullptr;
	m_Width = -1;
	m_Height = -1;
	m_IsLoaded = false;
	m_CPUHandle = {};
	m_GPUHandle = {};
	m_DisplayName = "Unnamed Texture";
	m_FileLocation = "NOT LOADED";
}

Texture::~Texture()
{
	if (m_IsLoaded)
	{
		//Destroy();
	}
}

ID3D12Resource* Texture::GetResource() const
{
	if (IsLoaded())
	{
		return m_Resource;
	}

	printf("Trying to acquire a texture that isn't loaded.\n");
	return nullptr;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Texture::GetCPUHandle() const
{
	return m_CPUHandle;
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Texture::GetGPUHandle() const
{
	return m_GPUHandle;
}

const int& Texture::GetID() const
{
	return m_ID;
}

const bool& Texture::IsLoaded() const
{
	return m_IsLoaded;
}

const std::string& Texture::GetDisplayName() const
{
	return m_DisplayName;
}
