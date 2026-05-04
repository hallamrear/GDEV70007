#pragma once
#include <Rendering/IMGUIRenderable.h>

#define MAX_CONTROLLERS 4
#define DEFAULT_CONTROLLER_DEADZONE (0.2f)

#define VK_KEY_0 0x30 // 0 key
#define VK_KEY_1 0x31 // 1 key
#define VK_KEY_2 0x32 // 2 key
#define VK_KEY_3 0x33 // 3 key
#define VK_KEY_4 0x34 // 4 key
#define VK_KEY_5 0x35 // 5 key
#define VK_KEY_6 0x36 // 6 key
#define VK_KEY_7 0x37 // 7 key
#define VK_KEY_8 0x38 // 8 key
#define VK_KEY_9 0x39 // 9 key
#define VK_KEY_A 0x41 // A key
#define VK_KEY_B 0x42 // B key
#define VK_KEY_C 0x43 // C key
#define VK_KEY_D 0x44 // D key
#define VK_KEY_E 0x45 // E key
#define VK_KEY_F 0x46 // F key
#define VK_KEY_G 0x47 // G key
#define VK_KEY_H 0x48 // H key
#define VK_KEY_I 0x49 // I key
#define VK_KEY_J 0x4A // J key
#define VK_KEY_K 0x4B // K key
#define VK_KEY_L 0x4C // L key
#define VK_KEY_M 0x4D // M key
#define VK_KEY_N 0x4E // N key
#define VK_KEY_O 0x4F // O key
#define VK_KEY_P 0x50 // P key
#define VK_KEY_Q 0x51 // Q key
#define VK_KEY_R 0x52 // R key
#define VK_KEY_S 0x53 // S key
#define VK_KEY_T 0x54 // T key
#define VK_KEY_U 0x55 // U key
#define VK_KEY_V 0x56 // V key
#define VK_KEY_W 0x57 // W key
#define VK_KEY_X 0x58 // X key
#define VK_KEY_Y 0x59 // Y key
#define VK_KEY_Z 0x5A // Z key

enum MOUSE_BUTTON
{
	MOUSE_BUTTON_LEFT = 0x0,
	MOUSE_BUTTON_MIDDLE = 0x1,
	MOUSE_BUTTON_RIGHT = 0x2,
	MOUSE_BUTTON_4 = 0x3,
	MOUSE_BUTTON_5 = 0x4,
	MOUSE_BUTTON_COUNT = 0x5,
};

enum CONTROLLER_BUTTON
{
	CONTROLLER_BUTTON_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
	CONTROLLER_BUTTON_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	CONTROLLER_BUTTON_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	CONTROLLER_BUTTON_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
	CONTROLLER_BUTTON_START = XINPUT_GAMEPAD_START,
	CONTROLLER_BUTTON_BACK = XINPUT_GAMEPAD_BACK,
	CONTROLLER_BUTTON_LEFT_THUMBSTICK_PRESS = XINPUT_GAMEPAD_LEFT_THUMB,
	CONTROLLER_BUTTON_RIGHT_THUMBSTICK_PRESS = XINPUT_GAMEPAD_RIGHT_THUMB,
	CONTROLLER_BUTTON_LEFT_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
	CONTROLLER_BUTTON_RIGHT_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	CONTROLLER_BUTTON_A = XINPUT_GAMEPAD_A,
	CONTROLLER_BUTTON_B = XINPUT_GAMEPAD_B,
	CONTROLLER_BUTTON_X = XINPUT_GAMEPAD_X,
	CONTROLLER_BUTTON_Y = XINPUT_GAMEPAD_Y,
	CONTROLLER_BUTTON_COUNT = 15
};

enum CONTROLLER_ANALOG_STICK
{
	CONTROLLER_ANALOG_LEFT_THUMBSTICK_X,
	CONTROLLER_ANALOG_LEFT_THUMBSTICK_Y,
	CONTROLLER_ANALOG_RIGHT_THUMBSTICK_X,
	CONTROLLER_ANALOG_RIGHT_THUMBSTICK_Y,
	CONTROLLER_ANALOG_LEFT_TRIGGER,
	CONTROLLER_ANALOG_RIGHT_TRIGGER,
	CONTROLLER_ANALOG_STICK_COUNT = 6
};

struct KeyState
{
private:
	friend class InputListener;
	bool m_IsRepeating;
	bool m_IsDown;

	KeyState(const int& keycode, const bool& repeating = false, const bool& down = false);
	~KeyState();

public:
	const int KeyCode;
	const bool& IsRepeating() const;
	const bool& IsDown() const;
};

struct MouseButtonState
{
private:
	friend class InputListener;
	bool m_IsDown;

public:
	const MOUSE_BUTTON ButtonID;
	MouseButtonState(const MOUSE_BUTTON& buttonID, const bool& down = false);
	~MouseButtonState();
	const bool& IsDown() const;
};

struct MouseState
{
private:
	friend class InputListener;
	Vector2 m_Position;
	Vector2 m_LastPosition;
	Vector2 m_FrameDelta;
	Vector2 m_LastPolledPosition;
	MouseButtonState* m_ButtonStates[5];

public:
	MouseState();
	~MouseState();

	MouseButtonState& GetButtonState(const MOUSE_BUTTON& buttonID);

	const Vector2& GetPosition() const;
	const Vector2& GetMouseDelta() const;
};

namespace std
{
	template<> struct less<KeyState>
	{
		bool operator()(const KeyState& keyOne, const KeyState& keyTwo) const
		{
			return keyOne.KeyCode < keyTwo.KeyCode;
		};
	};
}

class InputListener : public IIMGUIRenderable
{
public:

private:
	struct ControllerState
	{
		XINPUT_STATE State;
		bool IsConnected;
	};

	typedef std::map<int, KeyState*> InputMap;
	static std::vector<InputListener*> m_Instances;
	static InputMap m_KeyStates;
	static InputListener::ControllerState m_ControllerStates[MAX_CONTROLLERS];
	static MouseState m_MouseState;
	static bool m_ControllersEnabled;
	static float m_DeadZonePercentageNormalised;
	static bool m_DeadZoneEnabled;

	static void KeyboardInputCallback(const int& keycode, const bool& isDown);
	static void MouseInputCallback(const MOUSE_BUTTON& buttonCode, const bool& isDown);
	static KeyState& GetKeyStateReference(const int& keycode);
	static MouseButtonState& GetMouseButtonStateReference(const MOUSE_BUTTON& buttonCode);

	static void UpdateControllerStates();
	static void UpdateMouseState();

public:
	InputListener();
	~InputListener();

	static void EnableControllerSupport(const bool& deadZoneEnabled);
	static const bool& IsControllerSupportEnabled();
	static void DisableControllerSupport();
	static void SetDeadZonePercentage(const float& deadZonePercentage);
	static void SetDeadZoneEnabled(const bool& deadZoneEnabled);

	static void WndProcCallback(const int& msg, LPARAM lParam, WPARAM wParam);
	static void UpdateInputStates();

	static const MouseState& GetMouseState();
	static const bool& GetKeyDown(const int& keycode);
	static const KeyState& GetKeyState(const int& keycode);
	static const bool& GetButtonDown(const MOUSE_BUTTON& buttonCode);
	static const MouseButtonState& GetButtonState(const MOUSE_BUTTON& buttonCode);

	static const bool GetControllerButtonDown(const int& controllerIndex, const CONTROLLER_BUTTON& controllerButton);
	static const float GetControllerAnalogValue(const int& controllerIndex, const CONTROLLER_ANALOG_STICK& controllerAnalogStick);

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};

