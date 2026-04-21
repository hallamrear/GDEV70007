#include "pch.h"
#include "Renderer.h"

#ifdef RENDERER_DX12
#include <Rendering/DX12Renderer.h>
#endif // RENDERER_DX12

#ifdef RENDERER_DX11
#include <Rendering/DX11Renderer.h>
#endif // RENDERER_DX11

const int Renderer::m_SwapChainBufferCount = 2;
const TEXTURE_FORMAT Renderer::m_BackbufferFormat = (TEXTURE_FORMAT)DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
const TEXTURE_FORMAT Renderer::m_DepthStencilBufferFormat = (TEXTURE_FORMAT)DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

Renderer::Renderer()
{
	m_ClearColour = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_WindowWidth = 0;
	m_WindowHeight = 0;
	m_IsInitialised = false;
	m_WindowHandle = NULL;
}

Renderer::~Renderer()
{
	assert(!m_IsInitialised);
}

Renderer* Renderer::CreateRenderer(HWND windowHandle)
{
#if defined RENDERER_DX12
	Renderer* renderer = new DX12Renderer();
#elif defined RENDERER_DX11
	DX11Renderer* renderer = new DX11Renderer();
#else
	printf("No rendering API defined.\n");
	return nullptr;
#endif

	bool initialised = renderer->Initialise(windowHandle);

	if (initialised == false)
	{
		printf("Failed to initialise renderer.\n");
		delete renderer;
		renderer = nullptr;
	}

	return renderer;
}

bool Renderer::DestroyRenderer(Renderer* renderer)
{
	if (renderer == nullptr)
	{
		printf("Trying to destroy a renderer that doesn't exist.\n");
		return false;
	}

	if (renderer->IsInitialised() == false)
	{
		printf("Trying to destroy a renderer that doesn't exist.\n");
		return false;
	}

	renderer->Shutdown();

	return true;
}

const HWND& Renderer::GetWindowHandle() const
{
	return m_WindowHandle;
}

HWND& Renderer::GetWindowHandle()
{
	return m_WindowHandle;
}

const bool& Renderer::IsInitialised() const
{
	return m_IsInitialised;
}

const TEXTURE_FORMAT Renderer::GetBackbufferFormat()
{
	return m_BackbufferFormat;
}

const TEXTURE_FORMAT Renderer::GetDepthStencilBufferFormat()
{
	return m_DepthStencilBufferFormat;
}

const int Renderer::GetSwapChainBufferCount()
{
	return m_SwapChainBufferCount;
}

const int& Renderer::GetWindowWidth() const
{
	return m_WindowWidth;
}

const int& Renderer::GetWindowHeight() const
{
	return m_WindowHeight;
}

const Vector4& Renderer::GetClearColour() const
{
	return m_ClearColour;
}

void Renderer::SetClearColour(const Vector4& newColour)
{
	m_ClearColour = newColour;
}