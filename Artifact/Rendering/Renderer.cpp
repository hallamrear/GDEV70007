#include "pch.h"
#include "Renderer.h"

Renderer::Renderer()
{
	m_IsInitialised = false;
}

Renderer::~Renderer()
{
	assert(m_IsInitialised);
}

Renderer* Renderer::CreateRenderer()
{
	Renderer* renderer = new Renderer();

	bool initialised = renderer->Initialise();

	if (initialised == false)
	{
		printf("Failed to initialise renderer.");
		delete renderer;
		renderer = nullptr;
	}

	return renderer;
}

bool Renderer::DestroyRenderer(Renderer* renderer)
{
	if (renderer == nullptr)
	{
		printf("Trying to destroy a renderer that doesn't exist.");
		return false;
	}

	if (renderer->IsInitialised() == false)
	{
		printf("Trying to destroy a renderer that doesn't exist.");
		return false;
	}

	renderer->Shutdown();

	return true;
}

const bool& Renderer::IsInitialised() const
{
	return m_IsInitialised;
}