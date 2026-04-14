#include "pch.h"
#include "Engine.h"
#include <Rendering/Renderer.h>
#include <World/World.h>

std::filesystem::path Engine::m_ContentFolderLocation = std::filesystem::path();

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
	m_IsInitialised = false;
	m_Renderer = nullptr;
	m_World = nullptr;
}

Engine::~Engine()
{
	assert(!m_IsInitialised);
	m_ContentFolderLocation = std::filesystem::path();
}

bool Engine::InitialiseSubsystems(HWND windowHandle, const std::filesystem::path& contentFolderLocation)
{
	if (m_IsInitialised)
	{
		printf("Engine is already initialised.");
		return true;
	}

	m_Renderer = Renderer::CreateRenderer(windowHandle);

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

Engine* Engine::CreateEngine(HWND windowHandle, const std::filesystem::path& contentFolderLocation)
{
	Engine* engine = new Engine();

	bool initialised = engine->InitialiseSubsystems(windowHandle, contentFolderLocation);

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

	engine->m_IsRunning = false;
	return true;
}

void Engine::Start()
{
	m_IsRunning = true;
}

void Engine::Stop()
{
	m_IsRunning = false;
}

const bool& Engine::IsRunning() const
{
	return m_IsRunning;
}

LRESULT Engine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(wParam);

	switch (message)
	{
		case WM_SIZE:
		{
			if (m_Renderer != nullptr)
			{
				if (m_Renderer->IsInitialised() == false)
					break;

				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				return m_Renderer->ResizeSwapchain(width, height);
			}
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Engine::Update(const float& deltaTime)
{
	if (!m_IsInitialised || !m_IsRunning)
		return;

	static float timeElapsed = 0.0f;
	timeElapsed += deltaTime;

	Vector4 clearColour = m_Renderer->GetClearColour();
	clearColour.x = (sin(timeElapsed + 0) * 127 + 127) / 255.0f;
	clearColour.y = (sin(timeElapsed + 2) * 127 + 127) / 255.0f;
	clearColour.z = (sin(timeElapsed + 4) * 127 + 127) / 255.0f;
	clearColour.w = 1.0f;
	m_Renderer->SetClearColour(clearColour);

	m_World->Update(deltaTime);

	m_World->PostUpdate(deltaTime);
}

void Engine::Render()
{
	if (!m_IsInitialised || !m_IsRunning)
		return;

	m_Renderer->ClearFrame();

	m_World->Render();

	m_Renderer->PresentFrame();
}
