#include "pch.h"
#include "GenericBuffer.h"
#include <Rendering/Renderer.h>

GenericBuffer::GenericBuffer()
{
	m_Resource = nullptr;
	m_ElementCount = (UINT)-1;
}

GenericBuffer::~GenericBuffer()
{
    assert(!IsLoaded());
}

ID3D12Resource* GenericBuffer::GetResource()
{
	if (IsLoaded())
	{
		return m_Resource;
	}

	printf("Trying to acquire a buffer that isn't loaded.\n");
	return nullptr;
}

const bool GenericBuffer::IsLoaded() const
{
	return (m_Resource != nullptr);
}

const UINT& GenericBuffer::GetElementCount() const
{
	return m_ElementCount;
}

void GenericBuffer::Destroy()
{
	if (IsLoaded())
	{
		m_Resource->Release();
		m_Resource = nullptr;
	}
}
