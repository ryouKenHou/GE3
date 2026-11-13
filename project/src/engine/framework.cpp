#include "Framework.h"
#include "EngineCommon.h"
#include "SceneManager.h"

Framework::~Framework() = default;

void Framework::Initialize(int32_t Width, int32_t Height, const wchar_t* title) {
	engineCommon_.Initialize(Width, Height, title);

    // Start with whatever scene the concrete game's Framework subclass provides.
	sceneManager_.SetEngineCommon(&engineCommon_);
    sceneManager_.ChangeScene(CreateInitialScene());
}

void Framework::Finalize() {
    sceneManager_.Finalize();
    engineCommon_.Finalize();
}

void Framework::Update() {
    sceneManager_.Update();
}

void Framework::Draw() {
    sceneManager_.Draw();
}

bool Framework::IsEndRequested() {
    return engineCommon_.GetWindowManager().ProcessMessages();
}

void Framework::Run(int32_t Width, int32_t Height, const wchar_t* title) {
    Initialize(Width, Height, title);

    while (true) {
        if (IsEndRequested()) {
            break;
        }

        Update();
        Draw();
    }

    Finalize();
}