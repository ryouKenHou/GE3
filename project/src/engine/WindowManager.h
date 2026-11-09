#pragma once

#include <Windows.h>
#include <cstdint>

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class WindowManager {
public:
	// Class-scoped constants instead of globals defined in main.cpp
	static constexpr int32_t kClientWidth = 1280;
	static constexpr int32_t kClientHeight = 720;
private:
	WNDCLASS windowClass_;
	HWND hwnd_;
	int32_t clientWidth_ = kClientWidth;
	int32_t clientHeight_ = kClientHeight;

	// Lets the global WndProc update the current window's tracked size on WM_SIZE.
	static WindowManager* instance_;
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	// Inline getters can stay in the header for performance
	WNDCLASS getWindowClass() const { return windowClass_; }
	HWND getHwnd() const { return hwnd_; }
	HINSTANCE getHinstance() const { return windowClass_.hInstance; }

	// Live window client-area size, kept up to date whenever the window is resized.
	//int32_t GetClientWidth() const { return clientWidth_; }
	//int32_t GetClientHeight() const { return clientHeight_; }

	// Only declarations go here
	void Initialize(int nCmdShow, int width, int height, const wchar_t* windowTitle);
	void Finalize();
	bool ProcessMessages();

	static int32_t GetClientWidth() { return instance_ ? instance_->clientWidth_ : kClientWidth; }
	static int32_t GetClientHeight() { return instance_ ? instance_->clientHeight_ : kClientHeight; }
};