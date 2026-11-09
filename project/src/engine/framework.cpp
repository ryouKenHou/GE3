#include "Framework.h"
#include "EngineCommon.h"
#include "SceneManager.h"

Framework::~Framework() = default;

void Framework::Initialize() {
    EngineCommon::GetInstance().Initialize(WindowManager::kClientWidth, WindowManager::kClientHeight);

    // Start with whatever scene the concrete game's Framework subclass provides.
    SceneManager::GetInstance().ChangeScene(CreateInitialScene());
}

void Framework::Finalize() {
    SceneManager::GetInstance().Finalize();
    EngineCommon::GetInstance().Finalize();
}

void Framework::Update() {
    SceneManager::GetInstance().Update();
}

void Framework::Draw() {
    SceneManager::GetInstance().Draw();
}

bool Framework::IsEndRequested() {
    return EngineCommon::GetInstance().GetWindowManager().ProcessMessages();
}

void Framework::Run() {
    Initialize();

    while (true) {
        if (IsEndRequested()) {
            break;
        }

        Update();
        Draw();
    }

    Finalize();
}