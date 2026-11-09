#include "InputSystem.h"
#include <assert.h>

InputSystem::InputSystem() {
	directInput = nullptr;
	keyboard = nullptr;
	mouse = nullptr;
	hwnd_ = nullptr;
	memset(keyboardState, 0, sizeof(keyboardState));
	memset(prevKeyboardState, 0, sizeof(prevKeyboardState));
	memset(&mouseState, 0, sizeof(mouseState));
	memset(&prevMouseState, 0, sizeof(prevMouseState));
	mousePosition = { 0, 0 };
};

InputSystem::~InputSystem() {
	if (keyboard) {
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}
	if (mouse) {
		mouse->Unacquire();
		mouse->Release();
		mouse = nullptr;
	}
	if (directInput) {
		directInput->Release();
		directInput = nullptr;
	}
}

void InputSystem::Initialize(HINSTANCE hInstance, HWND hwnd) {
	hwnd_ = hwnd;

	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, nullptr);
	assert(SUCCEEDED(hr));
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	hr = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));

	hr = directInput->CreateDevice(GUID_SysMouse, &mouse, nullptr);
	assert(SUCCEEDED(hr));
	hr = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));
	hr = mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));
	mouse->Acquire();
}

void InputSystem::Update() {
	memcpy(prevKeyboardState, keyboardState, sizeof(keyboardState));
	HRESULT hr = keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
	if (FAILED(hr)) {
		hr = keyboard->Acquire();
		if (SUCCEEDED(hr)) {
			keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
		}
	}

	prevMouseState = mouseState;
	hr = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
	if (FAILED(hr)) {
		hr = mouse->Acquire();
		if (SUCCEEDED(hr)) {
			mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
		}
	}

	// Absolute cursor position (client coordinates), via Win32 since DirectInput only reports deltas
	if (hwnd_) {
		POINT p;
		if (GetCursorPos(&p)) {
			ScreenToClient(hwnd_, &p);
			mousePosition = p;
		}
	}
}

bool InputSystem::IsKeyTriggered(int key) {
	return keyboardState[key] & 0x80 && !(prevKeyboardState[key] & 0x80);
}

bool InputSystem::IsKeyReleased(int key) {
	return !(keyboardState[key] & 0x80) && prevKeyboardState[key] & 0x80;
}

bool InputSystem::IsKeyHeld(int key) {
	return keyboardState[key] & 0x80;
}

bool InputSystem::IsMouseButtonTriggered(int button) {
	return (mouseState.rgbButtons[button] & 0x80) && !(prevMouseState.rgbButtons[button] & 0x80);
}

bool InputSystem::IsMouseButtonReleased(int button) {
	return !(mouseState.rgbButtons[button] & 0x80) && (prevMouseState.rgbButtons[button] & 0x80);
}

bool InputSystem::IsMouseButtonHeld(int button) {
	return mouseState.rgbButtons[button] & 0x80;
}