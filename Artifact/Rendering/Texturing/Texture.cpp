#include "pch.h"
#include "Texture.h"
#include <d3d12.h>

void Texture::Destroy()
{
	if (m_IsLoaded)
	{
		if (m_Resource != nullptr)
		{
			m_Resource->Release();
			m_Resource = nullptr;
			m_IsLoaded = false;
			m_Height = -1;
			m_Width = -1;
		}
	}
}

Texture::Texture()
{
	m_Resource = nullptr;
	m_Width = -1;
	m_Height = -1;
	m_IsLoaded = false;
}

Texture::~Texture()
{
	if (m_IsLoaded)
	{
		Destroy();
	}
}

const bool& Texture::IsLoaded() const
{
	return m_IsLoaded;
}
