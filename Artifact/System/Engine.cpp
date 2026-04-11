#include "pch.h"
#include "Engine.h"
#include "ISubsystem.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
	m_IsInitialised = false;
}

Engine::~Engine()
{
	if (m_IsInitialised)
	{

	}

	m_ContentFolderLocation = std::filesystem::path();
}

bool Engine::InitialiseSubsystems()
{



	return true;
}

Engine* Engine::CreateEngine(const std::filesystem::path& _contentFolderLocation)
{
	Engine* engine = new Engine();

	bool initialised = engine->InitialiseSubsystems();

	if (initialised == false)
	{
		printf("Failed to initialise subsystems.");
		delete engine;
		engine = nullptr;
	}

	return engine;
}

bool Engine::DestroyEngine(Engine* _engine)
{
	if (_engine == nullptr)
	{
		printf("Trying to destroy an engine that doesn't exist.");
		return false;
	}

	return true;
}

void Engine::Update(const float& _deltaTime)
{
	if (m_World != nullptr)
	{
		if (m_World->IsLoaded())
		{

		}
	}
}

void Engine::Render()
{

}
