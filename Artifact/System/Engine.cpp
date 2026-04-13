#include "pch.h"
#include "Engine.h"
#include <Rendering/Renderer.h>
#include <World/World.h>

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
	m_IsInitialised = false;
	m_Renderer = nullptr;
	m_World = nullptr;
}

Engine::~Engine()
{
	assert(m_IsInitialised);
	m_ContentFolderLocation = std::filesystem::path();
}

bool Engine::InitialiseSubsystems(const std::filesystem::path& contentFolderLocation)
{
	if (m_IsInitialised)
	{
		printf("Engine is already initialised.");
		return true;
	}

	m_Renderer = Renderer::CreateRenderer();

	if (m_Renderer == nullptr)
	{
		printf("Failed to create renderer.");
		return false;
	}

	m_World = World::CreateWorld();

	if (m_World == nullptr)
	{
		printf("Failed to create world.");
		return false;
	}

	m_IsInitialised = (m_World != nullptr) && (m_Renderer != nullptr);

	if (m_IsInitialised)
	{
		m_ContentFolderLocation = contentFolderLocation;
	}

	return m_IsInitialised;
}

Engine* Engine::CreateEngine(const std::filesystem::path& contentFolderLocation)
{
	Engine* engine = new Engine();

	bool initialised = engine->InitialiseSubsystems(contentFolderLocation);

	if (initialised == false)
	{
		printf("Failed to initialise subsystems.");
		delete engine;
		engine = nullptr;
	}

	return engine;
}

bool Engine::DestroyEngine(Engine* engine)
{
	if (engine == nullptr)
	{
		printf("Trying to destroy an engine that doesn't exist.");
		return false;
	}

	if (engine->m_IsInitialised)
	{
		assert(World::DestroyWorld(engine->m_World));
		assert(Renderer::DestroyRenderer(engine->m_Renderer));
	}

	return true;
}

void Engine::Update(const float& deltaTime)
{
	if (!m_IsInitialised)
		return;

	m_World->Update(deltaTime);

	m_World->PostUpdate(deltaTime);
}

void Engine::Render()
{
	if (!m_IsInitialised)
		return;

	m_World->Render();
}
