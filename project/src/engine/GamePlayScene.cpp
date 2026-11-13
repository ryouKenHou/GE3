#include "GamePlayScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

void GamePlayScene::Initialize(EngineCommon* engineCommon, SceneManager* sceneManager) {
    engineCommon_ = engineCommon;
    sceneManager_ = sceneManager;
    spriteCommon_ = &engineCommon_->GetSpriteCommon();

    kClientWidth_ = WindowManager::kClientWidth;
    kClientHeight_ = WindowManager::kClientHeight;

    // NOTE: EngineCommon is already initialized once in Framework::Initialize() — do not call Initialize() again here.
    spriteCommon_->Initialize(engineCommon_);

    Transform cameraTransform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-5.0f} };
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize(kClientWidth_, kClientHeight_, cameraTransform);

    model_ = std::make_unique<Model>();
    model_->Initialize(engineCommon_);
    model_->LoadModel("resources/05_02", "plane.obj");

    a_ = std::make_unique<Object3D>();
    a_->Initialize(engineCommon_);
    a_->SetModel(model_.get());

    a2_ = std::make_unique<Object3D>();
    a2_->Initialize(engineCommon_);
    a2_->SetModel(model_.get());

    b_ = std::make_unique<Sprite>();
    b_->Initialize(engineCommon_, spriteCommon_);
    b_->SetTexture("resources/monsterBall.png");

    ball_ = std::make_unique<Model>();
    ball_->Initialize(engineCommon_);
    ball_->UsingTemplateModel(1);

    engineCommon_->GetParticleManager().SetTexture("resources/circle.png");

    lastFrameTime_ = std::chrono::steady_clock::now();
}

void GamePlayScene::Finalize() {
    // unique_ptr members clean themselves up automatically; explicit reset()
    // here just makes teardown order explicit/deterministic.
    a_.reset();
    model_.reset();
    b_.reset();
    ball_.reset();
    debugCamera_.reset();
    spriteCommon_ = nullptr;
}

void GamePlayScene::Update() {
    engineCommon_->GetInputSystem().Update();

    BYTE* keyboardState = engineCommon_->GetInputSystem().getKeyboardState();

    if (keyboardState[DIK_CAPSLOCK]) {
        OutputDebugStringA("Hit 0.\n");
    }

    if (engineCommon_->GetInputSystem().IsKeyTriggered(DIK_CAPSLOCK)) {
        OutputDebugStringA("Hit 1.\n");
    }

    if (engineCommon_->GetInputSystem().IsKeyHeld(DIK_CAPSLOCK)) {
        OutputDebugStringA("Hit 2.\n");
    }

    debugCamera_->Update(engineCommon_->GetInputSystem().getKeyboardState());
    debugCamera_->SetIsControlEnable(true);

    frameCount_++;

    auto currentFrameTime = std::chrono::steady_clock::now();
    deltaTime_ = std::chrono::duration<float>(currentFrameTime - lastFrameTime_).count();
    lastFrameTime_ = currentFrameTime;

    // plane
	a_->UpdateTransforms(debugCamera_->getViewProjectionMatrix());

    // plane2
	a2_->UpdateTransforms(debugCamera_->getViewProjectionMatrix());

    // sprite
    Matrix4x4 orthoProjection = Matrix4x4::MakeOrthographicMatrix(
        0.0f, 0.0f, static_cast<float>(kClientWidth_), static_cast<float>(kClientHeight_), 0.0f, 100.0f
    );

	b_->UpdateTransforms(orthoProjection);

    // ball
    Matrix4x4 worldMatrixBall = Matrix4x4::MakeAffineMatrix(transformball_.scale, transformball_.rotation, transformball_.translation);
    Matrix4x4 wvpMatrixBall = worldMatrixBall * debugCamera_->getViewProjectionMatrix();
    *ball_->wvpData = { wvpMatrixBall, worldMatrixBall };
    ball_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformBall_.scale, uvTransformBall_.rotation, uvTransformBall_.translation);
}

void GamePlayScene::Draw() {
    engineCommon_->PreDraw();

    DirectionalLight* directionalLightData = engineCommon_->GetDirectionalLightData();

#ifdef _DEBUG
    ImGui::Begin("window");

    ImGui::Text(" ");
    ImGui::Text("------- plane -----------------");
    ImGui::Checkbox("Show Plane", &showPlane_);
	Transform transform = a_->GetTransform();
    ImGui::DragFloat3("plane Transform", &transform.translation.x, 0.1f);
    ImGui::DragFloat3("plane Rotation", &transform.rotation.x, 0.1f);
    ImGui::DragFloat3("plane Scale", &transform.scale.x, 0.1f);
	a_->SetTransform(transform);
    Transform uvTransform = a_->GetUVTransform();
    ImGui::DragFloat2("plane uv transform", &uvTransform.translation.x, 0.01f);
    ImGui::DragFloat2("plane uv scale", &uvTransform.scale.x, 0.01f);
    ImGui::DragFloat("plane uv rotation", &uvTransform.rotation.z, 0.01f);
	a_->SetUVTransform(uvTransform);

    ImGui::Text(" ");
    ImGui::Text("------- plane2 -----------------");
    ImGui::Checkbox("Show Plane2", &showPlane2_);
	Transform transform2 = a2_->GetTransform();
    ImGui::DragFloat3("plane2 Transform", &transform2.translation.x, 0.1f);
    ImGui::DragFloat3("plane2 Rotation", &transform2.rotation.x, 0.1f);
    ImGui::DragFloat3("plane2 Scale", &transform2.scale.x, 0.1f);
	a2_->SetTransform(transform2);
    Transform uvTransform2 = a2_->GetUVTransform();
    ImGui::DragFloat2("plane2 uv transform", &uvTransform2.translation.x, 0.01f);
    ImGui::DragFloat2("plane2 uv scale", &uvTransform2.scale.x, 0.01f);
    ImGui::DragFloat("plane2 uv rotation", &uvTransform2.rotation.z, 0.01f);
	a2_->SetUVTransform(uvTransform2);
	Material materialData2 = a2_->GetMaterialData();
	ImGui::DragFloat4("plane2 material color", &materialData2.color.x, 0.01f);
	a2_->SetMaterialData(materialData2);

    ImGui::Text(" ");
    ImGui::Text("------- sprite -----------------");
    ImGui::Checkbox("Show Sprite", &showSprite_);
	Transform spriteTransform = b_->GetTransform();
    ImGui::DragFloat3("sprite transform", &spriteTransform.translation.x, 0.1f);
    ImGui::DragFloat3("sprite rotation", &spriteTransform.rotation.x, 0.1f);
    ImGui::DragFloat3("sprite scale", &spriteTransform.scale.x, 0.1f);
	b_->SetTransform(spriteTransform);
    Transform spriteUVTransform = b_->GetUVTransform();
    ImGui::DragFloat2("sprite uv transform", &spriteUVTransform.translation.x, 0.01f);
    ImGui::DragFloat2("sprite uv scale", &spriteUVTransform.scale.x, 0.01f);
    ImGui::DragFloat("sprite uv rotation", &spriteUVTransform.rotation.z, 0.01f);
	b_->SetUVTransform(spriteUVTransform);

    ImGui::Text(" ");
    ImGui::Text("------- ball -----------------");
    ImGui::Checkbox("Show Ball", &showBall_);
    ImGui::DragFloat3("ball transform", &transformball_.translation.x, 0.1f);
    ImGui::DragFloat3("ball rotation", &transformball_.rotation.x, 0.1f);
    ImGui::DragFloat3("ball scale", &transformball_.scale.x, 0.1f);
    ImGui::DragFloat2("ball uv transform", &uvTransformBall_.translation.x, 0.01f);
    ImGui::DragFloat2("ball uv scale", &uvTransformBall_.scale.x, 0.01f);
    ImGui::DragFloat("ball uv rotation", &uvTransformBall_.rotation.z, 0.01f);

    ImGui::Text(" ");
    ImGui::Text("------- lighting -----------------");
    ImGui::ColorEdit4("directionalLightColor", &directionalLightData->color.x);
    ImGui::DragFloat3("DirectionalLightDirection", &directionalLightData->direction.x, 0.1f);
    ImGui::DragFloat("DirectionalLightIntensity", &directionalLightData->intensity, 0.1f);

    ImGui::Text(" 0: no lighting");
    ImGui::Text(" 1: half lambert");
    ImGui::Text(" 2: lambert");
    ImGui::DragInt("EnableLighting", &ball_->materialData->enableLighting, 0.1f, 0, 2);
    a_->SetEnableLighting(ball_->materialData->enableLighting);

    ImGui::Text(" ");
    ImGui::Text("------- audio -----------------");
    if (ImGui::Button("Play Audio")) {
        engineCommon_->PlayAudio("Test");
    }

    ImGui::End();
#endif

    if (showBall_) ball_->Draw();
    if (showPlane_) a_->Draw();
    if (showPlane2_) a2_->Draw();     

    engineCommon_->GetParticleManager().Update(
        deltaTime_,
        debugCamera_->GetBillboardMatrix(),
        debugCamera_->getViewProjectionMatrix()
    );
    engineCommon_->GetParticleManager().Draw();

#ifdef _DEBUG
    engineCommon_->GetParticleManager().DrawImGui();
    ImGui::Render();
#endif

    spriteCommon_->PreDraw();
    if (showSprite_) b_->Draw();
    engineCommon_->PostDraw();
}