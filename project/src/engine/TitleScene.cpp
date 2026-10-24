#include "TitleScene.h"
#include "SceneManager.h"
#include "GamePlayScene.h"
#include <memory>

void TitleScene::Initialize() {
    // Initialize title resources if needed
}

void TitleScene::Finalize() {
    // Clean up title resources
}

void TitleScene::Update() {
    Input::Update();

    // Press Space or Enter to transition to the Gameplay Scene state
    if (Input::IsKeyTriggered(DIK_SPACE) || Input::IsKeyTriggered(DIK_RETURN)) {
        SceneManager::GetInstance().ChangeScene(std::make_unique<GamePlayScene>());
    }
}

void TitleScene::Draw() {
    // Render title screen graphics / UI here
}