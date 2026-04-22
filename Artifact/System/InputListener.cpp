#include "pch.h"
#include "InputListener.h"

std::vector<InputListener*> InputListener::m_Instances = std::vector<InputListener*>();
InputListener::InputMap InputListener::m_KeyStates = InputListener::InputMap();
ControllerState InputListener::m_ControllerStates[MAX_CONTROLLERS] = { ControllerState() };
float InputListener::m_DeadZonePercentage = DEFAULT_CONTROLLER_DEADZONE;
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

	return *validInsertion.first->second;
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

	m_DeadZonePercentage = 0.0f;

	if (deadZoneEnabled)
	{
		m_DeadZonePercentage = DEFAULT_CONTROLLER_DEADZONE;
	}
}

void InputListener::DisableControllerSupport()
{
	m_ControllersEnabled = false;
}

void InputListener::SetDeadZonePercentage(const float& deadZonePercentage)
{
	m_DeadZonePercentage = deadZonePercentage;
	m_DeadZonePercentage = min(m_DeadZonePercentage, 100.0f);
	m_DeadZonePercentage = max(m_DeadZonePercentage, 0.0f);
}

void InputListener::SetDeadZoneEnabled(const bool& deadZoneEnabled)
{
	m_DeadZoneEnabled = deadZoneEnabled;
}

void InputListener::WndProcCallback(const int& msg, LPARAM lParam, WPARAM wParam)
{
	switch (msg)
	{
	//case WM_LBUTTONDOWN: { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_LEFT, true, false); } break;
	//case WM_LBUTTONUP:	 { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_LEFT, false, false); }	break;
	//case WM_MBUTTONDOWN: { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_MIDDLE, true, false); } break;
	//case WM_MBUTTONUP:	 { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_MIDDLE, false, false); } break;
	//case WM_RBUTTONDOWN: { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_RIGHT, true, false); } break;
	//case WM_RBUTTONUP:   { InputMonitor::KeyboardInputCallback(BENNETT_MOUSE_RIGHT, false, false); } break;

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
		//MouseMovementInputCallback(pos);
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

void InputListener::UpdateControllerStates()
{
	if (m_ControllersEnabled == false)
	{
		return;
	}

	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		DWORD dwResult = XInputGetState(i, &m_ControllerStates[i].m_State);
		m_ControllerStates[i].m_IsConnected = (dwResult == ERROR_SUCCESS);
	}
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

#include <Rendering/IMGUIIncludes.h>
void InputListener::DebugRender()
{

	ImGui::Begin("Controllers");

	if (ImGui::Checkbox("Enable Controllers", &m_ControllersEnabled))
	{
		if(m_ControllersEnabled)
		{
			EnableControllerSupport(false);
		}
		else
		{
			DisableControllerSupport();
		}
	}

	//Use this value when consulting the axis'.
	const float scaledDeadzoneValue = m_DeadZonePercentage * float(0x7FFF);

	for (size_t i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (m_ControllerStates[i].m_IsConnected)
		{
			std::string str = "Controller - " + i;
			if (ImGui::CollapsingHeader(str.c_str()))
			{
				bool pressed = m_ControllerStates[i].m_State.Gamepad.wButtons & XINPUT_GAMEPAD_A;
				ImGui::Checkbox("Button Test", &pressed);
			}
		}
	}

	ImGui::End();
}
