#include "pch.h"
#include "InputListener.h"
#include <Rendering/IMGUIIncludes.h>

std::vector<InputListener*> InputListener::m_Instances = std::vector<InputListener*>();
InputListener::InputMap InputListener::m_KeyStates = InputListener::InputMap();
MouseState InputListener::m_MouseState = MouseState();
InputListener::ControllerState InputListener::m_ControllerStates[MAX_CONTROLLERS] = { InputListener::ControllerState() };
float InputListener::m_DeadZonePercentageNormalised = DEFAULT_CONTROLLER_DEADZONE;
bool InputListener::m_DeadZoneEnabled = true;
bool InputListener::m_ControllersEnabled = true;

KeyState::KeyState(const int& keycode, const bool& repeating, const bool& down) : KeyCode(keycode)
{
	m_IsRepeating = repeating;
	m_IsDown = down;
}

KeyState::~KeyState()
{
	m_IsRepeating = false;
	m_IsDown = false;
}

const bool& KeyState::IsRepeating() const
{
	return m_IsRepeating;
}

const bool& KeyState::IsDown() const
{
	return m_IsDown;
}

void InputListener::KeyboardInputCallback(const int& keycode, const bool& isDown)
{
	KeyState& keyState = GetKeyStateReference(keycode);
	keyState.m_IsRepeating = (keyState.IsDown() && isDown);
	keyState.m_IsDown = isDown;
}

void InputListener::MouseInputCallback(const MOUSE_BUTTON& buttonCode, const bool& isDown)
{
	MouseButtonState& mouseButtonState = GetMouseButtonStateReference(buttonCode);
	mouseButtonState.m_IsDown = isDown;
}

KeyState& InputListener::GetKeyStateReference(const int& keycode)
{
	InputMap::iterator itr = m_KeyStates.find(keycode);

	if (itr != m_KeyStates.end())
	{
		return *itr->second;
	}

	std::pair<InputMap::iterator, bool> validInsertion = m_KeyStates.insert(std::make_pair(keycode, new KeyState(keycode)));

	if (validInsertion.second == false)
	{
		throw std::exception("Failed to add the keycode to the state listener.\n");
	}
	else
	{
		printf("Added key %i to input map.\n", keycode);
	}

	return *validInsertion.first->second;
}

MouseButtonState& InputListener::GetMouseButtonStateReference(const MOUSE_BUTTON& buttonCode)
{
	return m_MouseState.GetButtonState(buttonCode);
}

InputListener::InputListener()
{
	m_Instances.push_back(this);
}

InputListener::~InputListener()
{
	m_Instances.erase(std::find(m_Instances.begin(), m_Instances.end(), this));
}

void InputListener::EnableControllerSupport(const bool& deadZoneEnabled)
{
	m_ControllersEnabled = true;

	SetDeadZoneEnabled(deadZoneEnabled);
	SetDeadZonePercentage(DEFAULT_CONTROLLER_DEADZONE);
}

const bool& InputListener::IsControllerSupportEnabled()
{
	return m_ControllersEnabled;
}

void InputListener::DisableControllerSupport()
{
	m_ControllersEnabled = false;
}

void InputListener::SetDeadZonePercentage(const float& deadZonePercentageNormalised)
{
	m_DeadZonePercentageNormalised = deadZonePercentageNormalised;
	m_DeadZonePercentageNormalised = std::clamp(m_DeadZonePercentageNormalised, 0.0f, 1.0f);
}

void InputListener::SetDeadZoneEnabled(const bool& deadZoneEnabled)
{
	m_DeadZoneEnabled = deadZoneEnabled;
}

void InputListener::WndProcCallback(const int& msg, LPARAM lParam, WPARAM wParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN: { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_LEFT, true); } break;
	case WM_LBUTTONUP:	 { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_LEFT, false); }	break;
	case WM_MBUTTONDOWN: { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE, true); } break;
	case WM_MBUTTONUP:	 { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_MIDDLE, false); } break;
	case WM_RBUTTONDOWN: { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_RIGHT, true); } break;
	case WM_RBUTTONUP:   { InputListener::MouseInputCallback(MOUSE_BUTTON::MOUSE_BUTTON_RIGHT, false); } break;

	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	{
		WORD fwButton = GET_XBUTTON_WPARAM(wParam);

		switch (fwButton)
		{
		case XBUTTON1: { InputListener::KeyboardInputCallback(XBUTTON1, (msg == WM_XBUTTONDOWN)); } break;
		case XBUTTON2: { InputListener::KeyboardInputCallback(XBUTTON2, (msg == WM_XBUTTONDOWN)); } break;
		default:
			printf("Unrecognised XBUTTON input detection.\n");
			break;
		}
	}
	break;

	case WM_MOUSEWHEEL:
	{

	}
	break;

	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
	{
		POINTS p = MAKEPOINTS(lParam);
		Vector2 pos = Vector2(p.x, p.y);
		m_MouseState.m_LastPolledPosition = pos;
	}
	break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		WORD vkCode = LOWORD(wParam);
		WORD keyFlags = HIWORD(lParam);
		//BOOL isKeyRepeated = (keyFlags & KF_REPEAT) == KF_REPEAT;
		bool isKeyDown = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;

		//FIX: Left/Right Ctrl and Alt currently do not work.
		if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
		{
			WORD scanCode = LOBYTE(keyFlags);
			vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
		}

		InputListener::KeyboardInputCallback(vkCode, isKeyDown);
	}
	break;

	default:
	{
		printf("Message got to Win32InputCallback that is not meant to be there.\n");
	}
	break;
	}
}

void InputListener::UpdateInputStates()
{
	if (m_ControllersEnabled == false)
	{
		return;
	}

	UpdateControllerStates();
	UpdateMouseState();
}

void InputListener::UpdateMouseState()
{
	m_MouseState.m_LastPosition = m_MouseState.m_Position;
	m_MouseState.m_Position = m_MouseState.m_LastPolledPosition;
	m_MouseState.m_FrameDelta = Vector2(m_MouseState.m_Position.x - m_MouseState.m_LastPosition.x, m_MouseState.m_Position.y - m_MouseState.m_LastPosition.y);
}

void InputListener::UpdateControllerStates()
{
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		DWORD dwResult = XInputGetState(i, &m_ControllerStates[i].State);
		m_ControllerStates[i].IsConnected = (dwResult == ERROR_SUCCESS);

		if (m_DeadZoneEnabled)
		{
			const float scaledDeadzoneValue = m_DeadZonePercentageNormalised * float(0x7FFF);

			// Zero value if thumbsticks are within the dead zone 
			if ((m_ControllerStates[i].State.Gamepad.sThumbLX < scaledDeadzoneValue && m_ControllerStates[i].State.Gamepad.sThumbLX > -scaledDeadzoneValue) &&
				(m_ControllerStates[i].State.Gamepad.sThumbLY < scaledDeadzoneValue && m_ControllerStates[i].State.Gamepad.sThumbLY > -scaledDeadzoneValue))
			{
				m_ControllerStates[i].State.Gamepad.sThumbLX = 0;
				m_ControllerStates[i].State.Gamepad.sThumbLY = 0;
			}

			if ((m_ControllerStates[i].State.Gamepad.sThumbRX < scaledDeadzoneValue && m_ControllerStates[i].State.Gamepad.sThumbRX > -scaledDeadzoneValue) &&
				(m_ControllerStates[i].State.Gamepad.sThumbRY < scaledDeadzoneValue && m_ControllerStates[i].State.Gamepad.sThumbRY > -scaledDeadzoneValue))
			{
				m_ControllerStates[i].State.Gamepad.sThumbRX = 0;
				m_ControllerStates[i].State.Gamepad.sThumbRY = 0;
			}
		}
	}
}

const MouseState& InputListener::GetMouseState()
{
	return m_MouseState;
}

const bool& InputListener::GetKeyDown(const int& keycode)
{
	const KeyState& keyState = GetKeyState(keycode);
	return keyState.IsDown();
}

const KeyState& InputListener::GetKeyState(const int& keycode)
{
	return GetKeyStateReference(keycode);
}

const bool& InputListener::GetButtonDown(const MOUSE_BUTTON& buttonCode)
{
	return m_MouseState.GetButtonState(buttonCode).IsDown();
}

const bool InputListener::GetControllerButtonDown(const int& controllerIndex, const CONTROLLER_BUTTON& controllerButton)
{
	if (m_ControllersEnabled == false)
	{
		return false;
	}

	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
	{
		printf("Trying to access a controller with an invalid index (%i).\n", controllerIndex);
		return false;
	}

	if (m_ControllerStates[controllerIndex].IsConnected == false)
	{
		return false;
	}

	return (m_ControllerStates[controllerIndex].State.Gamepad.wButtons & controllerButton);
}

/// <summary>
/// Returns a scaled float from -1 to 1 from edge to edge. 
/// 0.0f means its at rest in the centre.
/// </summary>
/// <param name="controllerIndex">The index of the controller to query.</param>
/// <param name="controllerAnalogStick">Which analog value to query.</param>
/// <returns></returns>
const float InputListener::GetControllerAnalogValue(const int& controllerIndex, const CONTROLLER_ANALOG_STICK& controllerAnalogStick)
{
	if (m_ControllersEnabled == false)
	{
		return 0.0f;
	}

	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
	{
		printf("Trying to access a controller with an invalid index (%i).\n", controllerIndex);
		return 0.0f;
	}

	if (m_ControllerStates[controllerIndex].IsConnected == false)
	{
		return 0.0f;
	}

	switch (controllerAnalogStick)
	{
	case CONTROLLER_ANALOG_LEFT_THUMBSTICK_X:
		return (m_ControllerStates[controllerIndex].State.Gamepad.sThumbLX / (float)SHRT_MAX);
		break;
	case CONTROLLER_ANALOG_LEFT_THUMBSTICK_Y:
		return (m_ControllerStates[controllerIndex].State.Gamepad.sThumbLY / (float)SHRT_MAX);
		break;
	case CONTROLLER_ANALOG_RIGHT_THUMBSTICK_X:
		return (m_ControllerStates[controllerIndex].State.Gamepad.sThumbRX / (float)SHRT_MAX);
		break;
	case CONTROLLER_ANALOG_RIGHT_THUMBSTICK_Y:
		return (m_ControllerStates[controllerIndex].State.Gamepad.sThumbRY / (float)SHRT_MAX);
		break;
	case CONTROLLER_ANALOG_LEFT_TRIGGER:
		return (m_ControllerStates[controllerIndex].State.Gamepad.bLeftTrigger / 255.0f);
		break;
	case CONTROLLER_ANALOG_RIGHT_TRIGGER:
		return (m_ControllerStates[controllerIndex].State.Gamepad.bRightTrigger / 255.0f);
		break;
	default:
		printf("Trying to access an unrecognised analog stick.\n");
		break;
	}

	return 0.0f;
}

void InputListener::OnIMGUIRender()
{
	ImGui::Begin("Input Debug\n");

	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	if (ImGui::CollapsingHeader("Controller Debug Details"))
	{

		if (ImGui::Checkbox("Enable Controller Support\n", &m_ControllersEnabled))
		{
			if (m_ControllersEnabled)
			{
				EnableControllerSupport(false);
			}
			else
			{
				DisableControllerSupport();
			}
		}

		if (m_ControllersEnabled)
		{
			if (ImGui::Checkbox("Enabled Deadzone\n", &m_DeadZoneEnabled))
			{
				SetDeadZoneEnabled(m_DeadZoneEnabled);
			}

			if (m_DeadZoneEnabled)
			{
				if (ImGui::InputFloat("Dead zone percentage (0.0 -> 1.0)\n", &m_DeadZonePercentageNormalised))
				{
					SetDeadZonePercentage(m_DeadZonePercentageNormalised);
				}
			}
		}

		for (int i = 0; i < MAX_CONTROLLERS; i++)
		{
			if (m_ControllerStates[i].IsConnected)
			{
				std::string str = "Controller - " + std::to_string(i);
				if (ImGui::CollapsingHeader(str.c_str()))
				{
					ImGui::PushID(str.c_str());

					ImGui::SeparatorText("Button Presses");
					if (ImGui::BeginTable("Button Presses Table", 2, tableFlags))
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("A Button\n");
						ImGui::TableSetColumnIndex(1); ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_A) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("B Button\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_B) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("X Button\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_X) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Y Button\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_Y) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("DPAD Up\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_DPAD_UP) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("DPAD Down\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_DPAD_DOWN) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("DPAD Left\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_DPAD_LEFT) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("DPAD Right\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_DPAD_RIGHT) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Start button\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_START) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Back button\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_BACK) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Left Thumbstick Press\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_LEFT_THUMBSTICK_PRESS) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Right Thumbstick Press\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_RIGHT_THUMBSTICK_PRESS) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Left Shoulder\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_LEFT_SHOULDER) ? "True\n" : "False\n");

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Right Shoulder");
						ImGui::TableSetColumnIndex(1); ImGui::Text(GetControllerButtonDown(i, CONTROLLER_BUTTON_RIGHT_SHOULDER) ? "True\n" : "False\n");

						ImGui::EndTable();
					}

					ImGui::SeparatorText("Analog Values");
					if (ImGui::BeginTable("Analog Values Table", 2, tableFlags))
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Left Trigger\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_LEFT_TRIGGER));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Right Trigger\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_RIGHT_TRIGGER));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Left Thumbstick X\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_LEFT_THUMBSTICK_X));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Left Thumbstick Y\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_LEFT_THUMBSTICK_Y));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Right Thumbstick X\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_RIGHT_THUMBSTICK_X));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("Right Thumbstick Y\n");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%f\n", GetControllerAnalogValue(i, CONTROLLER_ANALOG_STICK::CONTROLLER_ANALOG_RIGHT_THUMBSTICK_Y));

						ImGui::EndTable();
					}

					ImGui::PopID();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Keyboard Input Details"))
	{
		if (ImGui::BeginTable("Keyboard Input Details", 3, tableFlags))
		{
			for (auto& keyState : m_KeyStates)
			{	
				char key = (char)MapVirtualKey(keyState.second->KeyCode, MAPVK_VK_TO_CHAR);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::Text("%c Key\n", key);
				ImGui::TableSetColumnIndex(1); ImGui::Text("%s\n", keyState.second->IsDown() ? "True" : "False");
				ImGui::TableSetColumnIndex(2); ImGui::Text("%s\n", keyState.second->IsRepeating() ? "True" : "False");
			}

			ImGui::EndTable();
		}
	}
	
	if (ImGui::CollapsingHeader("Mouse Input Details"))
	{
		ImGui::Text("Mouse Position - X %f Y %f\n", m_MouseState.m_Position.x, m_MouseState.m_Position.y);
		ImGui::Text("Mouse Delta - X %f Y %f\n", m_MouseState.m_FrameDelta.x, m_MouseState.m_FrameDelta.y);

		if (ImGui::BeginTable("Mouse Input Details", 2, tableFlags))
		{
			for (size_t i = 0; i < MOUSE_BUTTON::MOUSE_BUTTON_COUNT; i++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::Text("Mouse Button %i\n", (int)i);

				if (m_MouseState.m_ButtonStates[i] != nullptr)
				{
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s\n", m_MouseState.m_ButtonStates[i]->IsDown() ? "True" : "False");
				}
				else
				{
					ImGui::TableSetColumnIndex(1); ImGui::Text("Pointer to object is invalid.\n");
				}
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

MouseState::MouseState()
{
	m_FrameDelta = Vector2(0.0f, 0.0f);
	m_LastPolledPosition = Vector2(0.0f, 0.0f);
	m_Position = Vector2(0.0f, 0.0f);
	m_LastPosition = Vector2(0.0f, 0.0f);

	for (size_t i = 0; i < MOUSE_BUTTON::MOUSE_BUTTON_COUNT; i++)
	{
		m_ButtonStates[i] = new MouseButtonState((MOUSE_BUTTON)i);
	}
}

MouseState::~MouseState()
{
	for (size_t i = 0; i < MOUSE_BUTTON::MOUSE_BUTTON_COUNT; i++)
	{
		if (m_ButtonStates[i] != nullptr)
		{
			delete m_ButtonStates[i];
			m_ButtonStates[i] = nullptr;
		}
	}
}

MouseButtonState& MouseState::GetButtonState(const MOUSE_BUTTON& buttonID)
{
	return *m_ButtonStates[(int)buttonID];
}

const Vector2& MouseState::GetMouseDelta() const
{
	return m_FrameDelta;
}

const Vector2& MouseState::GetPosition() const
{
	return m_Position;
}

MouseButtonState::MouseButtonState(const MOUSE_BUTTON& buttonID, const bool& down)
	: ButtonID(buttonID)
{
	m_IsDown = down;
}

MouseButtonState::~MouseButtonState()
{
	m_IsDown = false;
}

const bool& MouseButtonState::IsDown() const
{
	return m_IsDown;
}