#include "GamePlayScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

void GamePlayScene::Initialize() {
    engineCommon_ = &EngineCommon::GetInstance();
    spriteCommon_ = std::make_unique<SpriteCommon>();

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

    b_ = std::make_unique<Sprite>();
    b_->Initialize(engineCommon_);
    b_->SetTexture("resources/monsterBall.png");

    ball_ = std::make_unique<Model>();
    ball_->Initialize(engineCommon_);
    ball_->UsingTemplateModel(1);

    ParticleManager::GetInstance().Initialize(engineCommon_);
    ParticleManager::GetInstance().SetTexture("resources/circle.png");

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
    spriteCommon_.reset();
}

void GamePlayScene::Update() {
    Input::Update();

    BYTE* keyboardState = EngineCommon::GetInstance().GetInputSystem().getKeyboardState();

    if (keyboardState[DIK_CAPSLOCK]) {
        OutputDebugStringA("Hit 0.\n");
    }

    if (Input::IsKeyTriggered(DIK_CAPSLOCK)) {
        OutputDebugStringA("Hit 1.\n");
    }

    if (Input::IsKeyHeld(DIK_CAPSLOCK)) {
        OutputDebugStringA("Hit 2.\n");
    }

    debugCamera_->Update(EngineCommon::GetInstance().GetInputSystem().getKeyboardState());

    frameCount_++;

    auto currentFrameTime = std::chrono::steady_clock::now();
    deltaTime_ = std::chrono::duration<float>(currentFrameTime - lastFrameTime_).count();
    lastFrameTime_ = currentFrameTime;

    // plane
    Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
    Matrix4x4 wvpMatrix = worldMatrix * debugCamera_->getViewProjectionMatrix();
    *a_->wvpData = { wvpMatrix, worldMatrix };
    a_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);

    // sprite
    Matrix4x4 orthoProjection = Matrix4x4::MakeOrthographicMatrix(
        0.0f, 0.0f, static_cast<float>(kClientWidth_), static_cast<float>(kClientHeight_), 0.0f, 100.0f
    );

    Matrix4x4 worldMatrixSprite = Matrix4x4::MakeAffineMatrix(transformSprite_.scale, transformSprite_.rotation, transformSprite_.translation);
    Matrix4x4 wvpMatrixSprite = worldMatrixSprite * orthoProjection;
    *b_->wvpData = { wvpMatrixSprite, worldMatrixSprite };
    b_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite_.scale, uvTransformSprite_.rotation, uvTransformSprite_.translation);

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
    ImGui::DragFloat3("plane Transform", &transform_.translation.x, 0.1f);
    ImGui::DragFloat3("plane Rotation", &transform_.rotation.x, 0.1f);
    ImGui::DragFloat3("plane Scale", &transform_.scale.x, 0.1f);
    ImGui::DragFloat2("plane uv transform", &uvTransform_.translation.x, 0.01f);
    ImGui::DragFloat2("plane uv scale", &uvTransform_.scale.x, 0.01f);
    ImGui::DragFloat("plane uv rotation", &uvTransform_.rotation.z, 0.01f);

    ImGui::Text(" ");
    ImGui::Text("------- sprite -----------------");
    ImGui::Checkbox("Show Sprite", &showSprite_);
    ImGui::DragFloat3("sprite transform", &transformSprite_.translation.x, 0.1f);
    ImGui::DragFloat3("sprite rotation", &transformSprite_.rotation.x, 0.1f);
    ImGui::DragFloat3("sprite scale", &transformSprite_.scale.x, 0.1f);
    ImGui::DragFloat2("sprite uv transform", &uvTransformSprite_.translation.x, 0.01f);
    ImGui::DragFloat2("sprite uv scale", &uvTransformSprite_.scale.x, 0.01f);
    ImGui::DragFloat("sprite uv rotation", &uvTransformSprite_.rotation.z, 0.01f);

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
    a_->materialData->enableLighting = ball_->materialData->enableLighting;

    ImGui::Text(" ");
    ImGui::Text("------- audio -----------------");
    if (ImGui::Button("Play Audio")) {
        engineCommon_->PlayAudio("Test");
    }

    ImGui::End();
#endif

    if (showPlane_) a_->Draw();
    if (showSprite_) b_->Draw();
    if (showBall_) ball_->Draw();

    ParticleManager::GetInstance().Update(
        deltaTime_,
        debugCamera_->GetBillboardMatrix(),
        debugCamera_->getViewProjectionMatrix()
    );
    ParticleManager::GetInstance().Draw();

#ifdef _DEBUG
    ParticleManager::GetInstance().DrawImGui();
    ImGui::Render();
#endif

    spriteCommon_->PreDraw();
    engineCommon_->PostDraw();
}