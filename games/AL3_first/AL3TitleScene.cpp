#include "AL3TitleScene.h"
#include "SceneManager.h"
#include "AL3GamePlayScene.h"
#include "EngineCommon.h"
#include <memory>

void AL3TitleScene::Initialize() {
    // Initialize title resources if needed
    engineCommon_ = &EngineCommon::GetInstance();
    spriteCommon_ = std::make_unique<SpriteCommon>();
    spriteCommon_->Initialize(engineCommon_);

    b_ = std::make_unique<Sprite>();
    b_->Initialize(engineCommon_);
    b_->SetTexture("resources/texture/title.png");

    transformSprite_.scale = { 2.0f,2.0f,1.0f };
    transformSprite_.translation = { 0.f, 0.f, 0.f };

	engineCommon_->LoadAudio( "ui", "resources/sound/ui.mp3");
	engineCommon_->LoadAudio("bgm", "resources/sound/bgm.mp3");

	engineCommon_->PlayAudio("bgm", true, 0.5f);
}

void AL3TitleScene::Finalize() {
    // Clean up title resources
}

void AL3TitleScene::Update() {
    Input::Update();

    // Press Space or Enter to transition to AL3_first's own Gameplay Scene
    if (Input::IsKeyTriggered(DIK_SPACE) || Input::IsKeyTriggered(DIK_RETURN)|| EngineCommon::GetInstance().GetInputSystem().IsMouseButtonTriggered(0)) {
        engineCommon_->PlayAudio("ui");
        SceneManager::GetInstance().ChangeScene(std::make_unique<AL3GamePlayScene>());
    }

    Matrix4x4 orthoProjection = Matrix4x4::MakeOrthographicMatrix(
        0.0f, 0.0f, static_cast<float>(kClientWidth_), static_cast<float>(kClientHeight_), 0.0f, 100.0f);
    Matrix4x4 worldMatrixSprite = Matrix4x4::MakeAffineMatrix(transformSprite_.scale, transformSprite_.rotation, transformSprite_.translation);
    Matrix4x4 wvpMatrixSprite = worldMatrixSprite * orthoProjection;
    *b_->wvpData = { wvpMatrixSprite, worldMatrixSprite };
    b_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite_.scale, uvTransformSprite_.rotation, uvTransformSprite_.translation);
}

void AL3TitleScene::Draw() {
    // Render title screen graphics / UI here
    engineCommon_->PreDraw();
    spriteCommon_->PreDraw();
	b_->Draw();

#ifdef _DEBUG
    ImGui::Render();
#endif

    engineCommon_->PostDraw();
}