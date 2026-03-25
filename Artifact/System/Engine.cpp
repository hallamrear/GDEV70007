#include "pch.h"
#include "Engine.h"

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
}

Engine::~Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
}

bool Engine::InitialiseSubSystems()
{

}

Engine* Engine::CreateEngine(const std::filesystem::path& _contentFolderLocation)
{
	Engine* engine = new Engine();

	bool initialised = engine->InitialiseSubSystems();

	if (initialised == false)
	{
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

}

void Engine::Render()
{

}
