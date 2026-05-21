#include "pch.h"
#include "Engine.h"
#include <Rendering/Renderer.h>
#include <System/AssetManagement.h>
#include <World/World.h>

#include <World/Entity.h>

std::filesystem::path Engine::m_ContentFolderLocation = std::filesystem::path();
std::filesystem::path Engine::m_ExecutableLocation = std::filesystem::path();

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
	m_ExecutableLocation = std::filesystem::path();
	m_IsInitialised = false;
	m_Renderer = nullptr;
	m_World = nullptr;
	m_AssetManager = nullptr;
	m_IsRunning = false;
}

Engine::~Engine()
{
	assert(!m_IsInitialised);
	m_ContentFolderLocation = std::filesystem::path();
}

bool Engine::InitialiseSubsystems(HWND windowHandle, const std::filesystem::path& contentFolderLocation)
{
	char execLocationBuffer[256];
	bool foundExecLocation = (GetModuleFileName(NULL, execLocationBuffer, 256) != 0);

	if (foundExecLocation == false)
	{
		printf("Failed to find executable's file location for content folder.\n");
		return false;
	}

	m_ExecutableLocation = execLocationBuffer;
	m_ExecutableLocation.remove_filename();

	m_ContentFolderLocation = m_ExecutableLocation / contentFolderLocation;

	if (m_IsInitialised)
	{
		printf("Engine is already initialised.\n");
		return true;
	}

	m_Renderer = Renderer::CreateRenderer(windowHandle);

	if (m_Renderer == nullptr)
	{
		printf("Failed to create renderer.\n");
		return false;
	}

	ServiceLocator::Provide(m_Renderer);

	m_AssetManager = AssetManager::CreateAssetDatabase();

	if (m_AssetManager == nullptr)
	{
		printf("Failed to create asset manager.\n");
		return false;
	}

	ServiceLocator::Provide(m_AssetManager);

	//m_AssetManager->PreloadFolder(m_ContentFolderLocation);

	m_World = World::CreateWorld();

	if (m_World == nullptr)
	{
		printf("Failed to create world.\n");
		return false;
	}

	ServiceLocator::Provide(m_World);

	m_IsInitialised = (m_World != nullptr) && (m_Renderer != nullptr);

	if (m_IsInitialised)
	{
		m_Renderer->PostAssetInitialisation();
		InputListener::EnableControllerSupport(true);
	}

	return m_IsInitialised;
}

Engine* Engine::CreateEngine(HWND windowHandle, const std::filesystem::path& contentFolderLocation)
{
	Engine* engine = new Engine();

	bool initialised = engine->InitialiseSubsystems(windowHandle, contentFolderLocation);

	if (initialised == false)
	{
		InputListener::DisableControllerSupport();
		m_ContentFolderLocation = "";
		m_ExecutableLocation = "";
		printf("Failed to initialise subsystems.\n");
		delete engine;
		engine = nullptr;
	}

	return engine;
}

bool Engine::DestroyEngine(Engine* engine)
{
	if (engine == nullptr)
	{
		printf("Trying to destroy an engine that doesn't exist.\n");
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

const std::filesystem::path& Engine::GetContentFolderLocation()
{
	return m_ContentFolderLocation;
}

LRESULT Engine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(wParam);

	switch (message)
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_MOUSEMOVE:
		case WM_MOUSEHOVER:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		{
			InputListener::WndProcCallback(message, lParam, wParam);
			return 0;
		}
		break;

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

void Engine::FixedUpdate()
{
	if (!m_IsInitialised || !m_IsRunning)
		return;

	if (m_World != nullptr)
	{
		m_World->FixedUpdate();
	}
}

void Engine::Update(const float& deltaTime)
{
	InputListener::UpdateInputStates();

	//POINT windowCentre{ m_Renderer->GetWindowCentre().x, m_Renderer->GetWindowCentre().y };
	//ScreenToClient(m_Renderer->GetWindowHandle(), &centre);
	//InputListener::SetMousePosition(m_Renderer->GetWindowCentre());

	static float timeElapsed = 0.0f;
	timeElapsed += deltaTime;

	Vector4 clearColour = m_Renderer->GetClearColour();
	clearColour.x = (sin(timeElapsed + 0) * 127 + 127) / 255.0f;
	clearColour.y = (sin(timeElapsed + 2) * 127 + 127) / 255.0f;
	clearColour.z = (sin(timeElapsed + 4) * 127 + 127) / 255.0f;
	clearColour.w = 1.0f;
	m_Renderer->SetClearColour(clearColour);

	Vector3 forward = m_Renderer->GetCamera().GetForwardVector();
	Vector3 right = m_Renderer->GetCamera().GetRightVector();
	Vector3 up = m_Renderer->GetCamera().GetUpVector();

	const float moveSpeed = +360.0f * deltaTime;
	const float rotationSpeed = 5.0f * deltaTime;

	if (m_InputListener.GetKeyDown(VK_KEY_W)) { m_Renderer->GetCamera().Move(Vector3(forward.x * moveSpeed, forward.y * moveSpeed, forward.z * moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_S)) { m_Renderer->GetCamera().Move(Vector3(forward.x * -moveSpeed, forward.y * -moveSpeed, forward.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_A)) { m_Renderer->GetCamera().Move(Vector3(right.x * -moveSpeed, right.y * -moveSpeed, right.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_D)) { m_Renderer->GetCamera().Move(Vector3(right.x * moveSpeed, right.y * moveSpeed, right.z * moveSpeed)); }

	if (m_InputListener.GetKeyDown(VK_LSHIFT)) { m_Renderer->GetCamera().Move(Vector3(up.x * -moveSpeed, up.y * -moveSpeed, up.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_SPACE)) { m_Renderer->GetCamera().Move(Vector3(up.x * moveSpeed, up.y * moveSpeed, up.z * moveSpeed)); }
	
	if (m_InputListener.GetKeyDown(VK_LEFT)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(0.0f, -rotationSpeed, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_RIGHT)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(0.0f, rotationSpeed, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_UP)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(rotationSpeed, 0.0f, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_DOWN)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(-rotationSpeed, 0.0f, 0.0f)); }

	Vector2 delta = m_InputListener.GetMouseState().GetMouseDelta();
	float sensitivity = 0.0f;

	if (abs(delta.x) > FLT_EPSILON || abs(delta.y) > FLT_EPSILON)
	{
		m_Renderer->GetCamera().RotateEulerDegrees(Vector3(rotationSpeed * -delta.y * sensitivity, rotationSpeed * delta.x * sensitivity, 0.0f));
	}

	if (m_InputListener.IsControllerSupportEnabled())
	{
		Vector2 thumbstickLeft =
		{
			m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_LEFT_THUMBSTICK_X),
			m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_LEFT_THUMBSTICK_Y)
		};

		if ((thumbstickLeft.y > (0.0f + FLT_EPSILON)) || (thumbstickLeft.y < (0.0f - FLT_EPSILON))) { m_Renderer->GetCamera().Move(Vector3(forward.x * moveSpeed * thumbstickLeft.y, forward.y * moveSpeed * thumbstickLeft.y, forward.z * moveSpeed * thumbstickLeft.y)); }
		if ((thumbstickLeft.x > (0.0f + FLT_EPSILON)) || (thumbstickLeft.x < (0.0f - FLT_EPSILON))) { m_Renderer->GetCamera().Move(Vector3(right.x * moveSpeed * thumbstickLeft.x, right.y * moveSpeed * thumbstickLeft.x, right.z * moveSpeed * thumbstickLeft.x)); }

		Vector2 thumbstickRight =
		{
			m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_RIGHT_THUMBSTICK_X),
			m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_RIGHT_THUMBSTICK_Y)
		};

		if (thumbstickRight.x > (0.0f + FLT_EPSILON) || thumbstickRight.x < (0.0f - FLT_EPSILON) ||
			thumbstickRight.y >(0.0f + FLT_EPSILON) || thumbstickRight.y < (0.0f - FLT_EPSILON))
		{
			m_Renderer->GetCamera().RotateEulerDegrees(Vector3(rotationSpeed * -thumbstickRight.y, rotationSpeed * thumbstickRight.x, 0.0f));
		}

		if (m_World->TestBoxA != nullptr)
		{
			Vector3 localForward = m_World->TestBoxA->GetForwardVector();
			Vector3 localRight = m_World->TestBoxA->GetRightVector();
			Vector3 localUp = m_World->TestBoxA->GetUpVector();

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_UP))
			{
				m_World->TestBoxA->Translate(Vector3(localForward.x * moveSpeed * 5 * FIXED_TIMESTEP, localForward.y * moveSpeed * 5 * FIXED_TIMESTEP, localForward.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_RIGHT))
			{
				m_World->TestBoxA->Translate(Vector3(localRight.x * moveSpeed * 5 * FIXED_TIMESTEP, localRight.y * moveSpeed * 5 * FIXED_TIMESTEP, localRight.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_DOWN))
			{
				m_World->TestBoxA->Translate(Vector3(-localForward.x * moveSpeed * 5 * FIXED_TIMESTEP, -localForward.y * moveSpeed * 5 * FIXED_TIMESTEP, -localForward.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_LEFT))
			{
				m_World->TestBoxA->Translate(Vector3(-localRight.x * moveSpeed * 5 * FIXED_TIMESTEP, -localRight.y * moveSpeed * 5 * FIXED_TIMESTEP, -localRight.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_LEFT_SHOULDER))
			{
				m_World->TestBoxA->Translate(Vector3(-localUp.x * moveSpeed * 5 * FIXED_TIMESTEP, -localUp.y * moveSpeed * 5 * FIXED_TIMESTEP, -localUp.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}

			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_RIGHT_SHOULDER))
			{
				m_World->TestBoxA->Translate(Vector3(localUp.x * moveSpeed * 5 * FIXED_TIMESTEP, localUp.y * moveSpeed * 5 * FIXED_TIMESTEP, localUp.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
		}
	}

	if (m_World != nullptr)
	{
		m_World->Update(deltaTime);
	}
}

void Engine::Render()
{
	if (!m_IsInitialised || !m_IsRunning)
		return;

	m_Renderer->ClearFrame();

	m_World->Render(*m_Renderer);

#if defined(_DEBUG)
		m_Renderer->RenderIMGUIFrame();
#endif

	m_Renderer->PresentFrame();
}
