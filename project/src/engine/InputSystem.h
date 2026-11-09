#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class InputSystem {
	public:
		InputSystem();
		~InputSystem();
		void Initialize(HINSTANCE hInstance, HWND hwnd);
		void Update();
		bool IsKeyReleased(int key);
		bool IsKeyHeld(int key);
		bool IsKeyTriggered(int key);

		BYTE* getKeyboardState() { return keyboardState; }
		BYTE* getPrevKeyboardState() { return prevKeyboardState; }

		// ================= Mouse =================
		// button: 0 = left, 1 = right, 2 = middle
		bool IsMouseButtonTriggered(int button);
		bool IsMouseButtonHeld(int button);
		bool IsMouseButtonReleased(int button);

		// Relative movement since last frame (from DirectInput)
		long GetMouseMoveX() const { return mouseState.lX; }
		long GetMouseMoveY() const { return mouseState.lY; }
		long GetMouseWheel() const { return mouseState.lZ; }

		// Absolute cursor position in client coordinates (from Win32)
		int GetMouseX() const { return mousePosition.x; }
		int GetMouseY() const { return mousePosition.y; }
		POINT GetMousePosition() const { return mousePosition; }

	private:
		IDirectInput8* directInput;
		IDirectInputDevice8* keyboard;
		BYTE keyboardState[256];
		BYTE prevKeyboardState[256];

		IDirectInputDevice8* mouse;
		DIMOUSESTATE mouseState;
		DIMOUSESTATE prevMouseState;

		HWND hwnd_;
		POINT mousePosition;
};