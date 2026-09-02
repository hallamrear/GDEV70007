#include "pch.h"
#include "Engine.h"
#include <Rendering/Renderer.h>
#include <System/AssetManagement.h>
#include <World/World.h>
#include <Physics/Rigidbody.h>
#include <World/Entity.h>
#include <time.h>
#include <filesystem>

std::filesystem::path Engine::m_ContentFolderLocation = std::filesystem::path();
std::filesystem::path Engine::m_ExecutableLocation = std::filesystem::path();
std::chrono::duration<long double> Engine::UpdateDelta = {};
std::chrono::duration<long double> Engine::RenderDelta = {};

Engine::Engine()
{
	m_ContentFolderLocation = std::filesystem::path();
	m_ExecutableLocation = std::filesystem::path();
	m_IsInitialised = false;
	m_Renderer = nullptr;
	m_World = nullptr;
	m_AssetManager = nullptr;
	m_IsRunning = false;
	m_FrameTime = 0.0f;
	m_SampleFramesRemaining = -1;
	m_SampleFrameData = {};
	m_SampleFramesRemaining = -1;
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

	m_SampleFrameData = {};

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
	m_UpdateStart = std::chrono::steady_clock::now();

	InputListener::UpdateInputStates();

	if (!m_IsInitialised || !m_IsRunning)
		return;

	if (m_InputListener.GetKeyDown(VK_NUMPAD5)) { IIMGUIRenderable::DrawIMGUI = !IIMGUIRenderable::DrawIMGUI; }
		
	if (m_World != nullptr)
	{
		m_World->FixedUpdate();
	}
}

static double timeElapsed = 0.0;
void Engine::Update(const long double& deltaTime)
{
	//POINT windowCentre{ m_Renderer->GetWindowCentre().x, m_Renderer->GetWindowCentre().y };
	//ScreenToClient(m_Renderer->GetWindowHandle(), &centre);
	//InputListener::SetMousePosition(m_Renderer->GetWindowCentre());

	m_FrameTime = deltaTime;
	timeElapsed += (double)deltaTime;

	Vector4 clearColour = m_Renderer->GetClearColour();
	clearColour.x = (sin((float)timeElapsed + 0) * 127 + 127) / 255.0f;
	clearColour.y = (sin((float)timeElapsed + 2) * 127 + 127) / 255.0f;
	clearColour.z = (sin((float)timeElapsed + 4) * 127 + 127) / 255.0f;
	clearColour.w = 1.0f;
	clearColour = { 1.0f, 1.0f, 1.0f, 1.0f };
	clearColour = { 210.0f / 255.0f, 210.0f / 255.0f, 210.0f / 255.0f, 1.0f };
	m_Renderer->SetClearColour(clearColour);

	if (m_World != nullptr)
	{
		m_World->Update((double)deltaTime);
	}

	float moveSpeed = +40.0f * (float)deltaTime;
	float rotationSpeed = 5.0f * (float)deltaTime;
	Vector3 forward = m_Renderer->GetCamera().GetForwardVector();
	Vector3 right = m_Renderer->GetCamera().GetRightVector();
	Vector3 up = m_Renderer->GetCamera().GetUpVector();
	
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
	
		if (m_World->ControlledEntity != nullptr)
		{
			float leftTrigger = m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_LEFT_TRIGGER);
			float rightTrigger = m_InputListener.GetControllerAnalogValue(0, CONTROLLER_ANALOG_RIGHT_TRIGGER);
	
			if (leftTrigger > 0)
				m_World->ControlledEntity->Rotate(Vector3(rotationSpeed * leftTrigger, 0.0f, 0.0f));
	
			if (rightTrigger > 0)
				m_World->ControlledEntity->Rotate(Vector3(0.0f, rotationSpeed * rightTrigger, 0.0f));
	
			Vector3 localForward = m_World->ControlledEntity->GetForwardVector();
			Vector3 localRight = m_World->ControlledEntity->GetRightVector();
			Vector3 localUp = m_World->ControlledEntity->GetUpVector();
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_UP) || m_InputListener.GetKeyDown(VK_KEY_I))
			{
				m_World->ControlledEntity->Translate(Vector3(localForward.x * moveSpeed * 5 * FIXED_TIMESTEP, localForward.y * moveSpeed * 5 * FIXED_TIMESTEP, localForward.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_RIGHT) || m_InputListener.GetKeyDown(VK_KEY_L))
			{
				m_World->ControlledEntity->Translate(Vector3(localRight.x * moveSpeed * 5 * FIXED_TIMESTEP, localRight.y * moveSpeed * 5 * FIXED_TIMESTEP, localRight.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_DOWN) || m_InputListener.GetKeyDown(VK_KEY_K))
			{
				m_World->ControlledEntity->Translate(Vector3(-localForward.x * moveSpeed * 5 * FIXED_TIMESTEP, -localForward.y * moveSpeed * 5 * FIXED_TIMESTEP, -localForward.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_DPAD_LEFT) || m_InputListener.GetKeyDown(VK_KEY_J))
			{
				m_World->ControlledEntity->Translate(Vector3(-localRight.x * moveSpeed * 5 * FIXED_TIMESTEP, -localRight.y * moveSpeed * 5 * FIXED_TIMESTEP, -localRight.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_LEFT_SHOULDER) || m_InputListener.GetKeyDown(VK_KEY_U))
			{
				m_World->ControlledEntity->Translate(Vector3(-localUp.x * moveSpeed * 5 * FIXED_TIMESTEP, -localUp.y * moveSpeed * 5 * FIXED_TIMESTEP, -localUp.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
	
			if (m_InputListener.GetControllerButtonDown(0, CONTROLLER_BUTTON_RIGHT_SHOULDER) || m_InputListener.GetKeyDown(VK_KEY_O))
			{
				m_World->ControlledEntity->Translate(Vector3(localUp.x * moveSpeed * 5 * FIXED_TIMESTEP, localUp.y * moveSpeed * 5 * FIXED_TIMESTEP, localUp.z * moveSpeed * 5 * FIXED_TIMESTEP));
			}
		}
	}

	if (m_InputListener.GetKeyDown(VK_KEY_W)) { m_Renderer->GetCamera().Move(Vector3(forward.x * moveSpeed, forward.y * moveSpeed, forward.z * moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_S)) { m_Renderer->GetCamera().Move(Vector3(forward.x * -moveSpeed, forward.y * -moveSpeed, forward.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_A)) { m_Renderer->GetCamera().Move(Vector3(right.x * -moveSpeed, right.y * -moveSpeed, right.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_KEY_D)) { m_Renderer->GetCamera().Move(Vector3(right.x * moveSpeed, right.y * moveSpeed, right.z * moveSpeed)); }

	if (m_InputListener.GetKeyDown(VK_LSHIFT)) { m_Renderer->GetCamera().Move(Vector3(up.x * -moveSpeed, up.y * -moveSpeed, up.z * -moveSpeed)); }
	if (m_InputListener.GetKeyDown(VK_SPACE)) { m_Renderer->GetCamera().Move(Vector3(up.x * moveSpeed, up.y * moveSpeed, up.z * moveSpeed)); }

	if (m_InputListener.GetKeyDown(VK_LEFT)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(0.0f, -rotationSpeed, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_RIGHT)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(0.0f, rotationSpeed, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_UP)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(-rotationSpeed, 0.0f, 0.0f)); }
	if (m_InputListener.GetKeyDown(VK_DOWN)) { m_Renderer->GetCamera().RotateEulerDegrees(Vector3(rotationSpeed, 0.0f, 0.0f)); }

	Vector2 delta = m_InputListener.GetMouseState().GetMouseDelta();
	float sensitivity = 0.0f;

	if (abs(delta.x) > FLT_EPSILON || abs(delta.y) > FLT_EPSILON)
	{
		m_Renderer->GetCamera().RotateEulerDegrees(Vector3(rotationSpeed * -delta.y * sensitivity, rotationSpeed * delta.x * sensitivity, 0.0f));
	}

	m_UpdateEnd = std::chrono::steady_clock::now();
}

void Engine::Render()
{
	if (!m_IsInitialised || !m_IsRunning)
		return;

	m_RenderStart = std::chrono::steady_clock::now();

	m_Renderer->ClearFrame();

	m_World->Render(*m_Renderer);

	m_Renderer->RenderIMGUIFrame();

	m_Renderer->PresentFrame();

	m_RenderEnd = std::chrono::steady_clock::now();
}

void Engine::CalculateTimings()
{
	RenderDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(m_RenderEnd - m_RenderStart);
	UpdateDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(m_UpdateEnd - m_UpdateStart);

	if (m_SampleSetsRemaining > 0)
	{
		if (TimeBetweenSamples > 0.0f)
		{
			TimeBetweenSamples -= (float)m_FrameTime;
			return;
		}

		if (m_SampleFramesRemaining > 0)
		{
			m_SampleFrameData.push_back(std::make_pair(m_FrameTime, std::make_pair(RenderDelta.count(), UpdateDelta.count())));
			m_SampleFramesRemaining--;
		}
		else if (m_SampleFramesRemaining == 0)
		{
			std::time_t currentTime = std::time(nullptr);
			std::tm localTime;
			localtime_s(&localTime, &currentTime);

			char buffer[256];
			asctime_s(&buffer[0], 256, &localTime);

			std::filesystem::create_directories(c_WorldExampleSceneNames[m_World->GetCurrentScene()]);
			std::filesystem::create_directories(c_WorldExampleSceneNames[m_World->GetCurrentScene()] + "//Data//" + std::to_string(m_World->c_TestCount));
			std::filesystem::create_directories(c_WorldExampleSceneNames[m_World->GetCurrentScene()] + "//Data//Details//");

			std::string name = c_WorldExampleSceneNames[m_World->GetCurrentScene()];
			name += "//Data//Details//";
			name += std::to_string(m_World->c_TestCount);
			name += ".txt";

			if (std::filesystem::exists(name) == false)
			{
				std::fstream details(name.c_str(), std::fstream::trunc | std::fstream::out);
				if (details.is_open())
				{
					details.close();
				}
			}

			std::fstream details(name.c_str(), std::fstream::app | std::fstream::out);
			if (details.is_open())
			{
				details << "\n";
				details << m_World->GetExtraDetails();
			}

			details.close();

			name = c_WorldExampleSceneNames[m_World->GetCurrentScene()];
			name += "//Data//";
			name += std::to_string(m_World->c_TestCount);
			name += "//";
			name += std::to_string(currentTime);
			name += ".txt";

			std::fstream file(name.c_str(), std::fstream::trunc | std::fstream::out);

			if (file.is_open())
			{
				for (size_t i = 0; i < m_SampleFrameData.size(); i++)
				{
					file << m_SampleFrameData[i].first << ", " << m_SampleFrameData[i].second.first << ", " << m_SampleFrameData[i].second.second << "," << std::endl;
				}
			}

			file.close();

			m_SampleFrameData.clear();
			m_SampleFramesRemaining = 100;
			m_SampleSetsRemaining--;
			TimeBetweenSamples = 0.5f;
		}
	}
}

void Engine::OnIMGUIRender()
{
	ImGui::Begin("Data");


	bool disabled = m_SampleSetsRemaining > 0;

	if (disabled)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	if (ImGui::Button("Start Samples"))
	{
		m_SampleFrameData.clear();
		m_SampleFramesRemaining = 100;
		m_SampleSetsRemaining = 10;
		TimeBetweenSamples = 0.5f;
	}

	if (disabled)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::Text("Total Elapsed Time : %lf\n", timeElapsed);
	ImGui::Text("Frame Time : %lf\n", m_FrameTime);
	ImGui::Text("FPS: %lf", (1.0 / m_FrameTime));
	ImGui::Text("Render Time (s): %lf", (RenderDelta.count()));
	ImGui::Text("Update Time (s): %lf", (UpdateDelta.count()));
	ImGui::End();
}
