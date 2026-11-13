#include "TitleScene.h"
#include "SceneManager.h"
#include "GamePlayScene.h"
#include <memory>

void TitleScene::Initialize(EngineCommon* engineCommon, SceneManager* sceneManager) {
	engineCommon_ = engineCommon;
	sceneManager_ = sceneManager;
}

void TitleScene::Finalize() {
	// Clean up title resources
}

void TitleScene::Update() {
	engineCommon_->GetInputSystem().Update();

	// Press Space or Enter to transition to the Gameplay Scene state
	if (engineCommon_->GetInputSystem().IsKeyTriggered(DIK_SPACE) || engineCommon_->GetInputSystem().IsKeyTriggered(DIK_RETURN)) {
		sceneManager_->ChangeScene(std::make_unique<GamePlayScene>());
	}
}

void TitleScene::Draw() {
	// Render title screen graphics / UI here
}