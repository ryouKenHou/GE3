#include "EngineCommon.h"

void EngineCommon::Initialize(int32_t Width, int32_t Height, const wchar_t* title) {

	windowManager.Initialize(SW_SHOW, Width, Height, title);
	// Initialize the renderer with the specified width and height
	renderer_.Initialize(&windowManager);
	audioSystem_.Initialize();
	inputSystem_.Initialize(windowManager.getHinstance(), windowManager.getHwnd());
	spriteCommon_.Initialize(this);
	particleManager_.Initialize(this);
}

void EngineCommon::Finalize() {
	renderer_.Finalize();
	audioSystem_.Shutdown();
	windowManager.Finalize();
}