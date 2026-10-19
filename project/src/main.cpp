#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include <memory>

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	EngineCommon* engineCommon = &EngineCommon::GetInstance();
	SpriteCommon* spriteCommon = new SpriteCommon();



	int32_t kClientWidth = WindowManager::kClientWidth;
	int32_t kClientHeight = WindowManager::kClientHeight;

	engineCommon->Initialize(kClientWidth, kClientHeight);

	spriteCommon->Initialize(engineCommon);

	Transform cameraTransform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-5.0f} };
	std::unique_ptr<DebugCamera> debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize(kClientWidth, kClientHeight, cameraTransform);

	Transform transform{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transformball{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform TransformSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform uvTransform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransformBall{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	bool showPlane = true;
	bool showSprite = true;
	bool showBall = false;

	int frameCount = 0;
	bool useMonsterBall = 0;

	// Dynamically allocated with new/delete to satisfy the requirement explicitly.
	Model* model = new Model();
	model->Initialize(engineCommon);
	model->LoadModel("resources/05_02", "plane.obj");

	Object3D* a = new Object3D();
	a->Initialize(engineCommon);
	a->SetModel(model);

	std::unique_ptr<Sprite> b = std::make_unique<Sprite>();
	b->Initialize(engineCommon);
	b->SetTexture("resources/monsterBall.png");

	std::unique_ptr<Model> ball = std::make_unique<Model>();
	ball->Initialize(engineCommon);
	ball->UsingTemplateModel(1);

	while (true) {
		if (engineCommon->GetWindowManager().ProcessMessages()) {
			break;
		}
		else {

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

			debugCamera->Update(EngineCommon::GetInstance().GetInputSystem().getKeyboardState());

			frameCount++;


			// plane
			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotation, transform.translation);
			Matrix4x4 wvpMatrix = worldMatrix * debugCamera->getViewProjectionMatrix();
			*a->wvpData = { wvpMatrix, worldMatrix };
			a->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform.scale, uvTransform.rotation, uvTransform.translation);

			// sprite
			Matrix4x4 orthoProjection = Matrix4x4::MakeOrthographicMatrix(
				0.0f, 0.0f, static_cast<float>(kClientWidth), static_cast<float>(kClientHeight), 0.0f, 100.0f
			);

			Matrix4x4 worldMatrixSprite = Matrix4x4::MakeAffineMatrix(TransformSprite.scale, TransformSprite.rotation, TransformSprite.translation);
			Matrix4x4 wvpMatrixSprite = worldMatrixSprite * orthoProjection;
			*b->wvpData = { wvpMatrixSprite, worldMatrixSprite };
			b->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite.scale, uvTransformSprite.rotation, uvTransformSprite.translation);

			// ball
			Matrix4x4 worldMatrixBall = Matrix4x4::MakeAffineMatrix(transformball.scale, transformball.rotation, transformball.translation);
			Matrix4x4 wvpMatrixBall = worldMatrixBall * debugCamera->getViewProjectionMatrix();
			*ball->wvpData = { wvpMatrixBall, worldMatrixBall };
			ball->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformBall.scale, uvTransformBall.rotation, uvTransformBall.translation);

			engineCommon->PreDraw();
			

			DirectionalLight* directionalLightData = engineCommon->GetDirectionalLightData();

#ifdef _DEBUG
			ImGui::Begin("window");

			ImGui::Text(" ");
			ImGui::Text("------- plane -----------------");
			ImGui::Checkbox("Show Plane", &showPlane);
			ImGui::DragFloat3("plane Transform", &transform.translation.x, 0.1f);
			ImGui::DragFloat3("plane Rotation", &transform.rotation.x, 0.1f);
			ImGui::DragFloat3("plane Scale", &transform.scale.x, 0.1f);
			ImGui::DragFloat2("plane uv transform", &uvTransform.translation.x, 0.01f);
			ImGui::DragFloat2("plane uv scale", &uvTransform.scale.x, 0.01f);
			ImGui::DragFloat("plane uv rotation", &uvTransform.rotation.z, 0.01f);

			ImGui::Text(" ");
			ImGui::Text("------- sprite -----------------");
			ImGui::Checkbox("Show Sprite", &showSprite);
			ImGui::DragFloat3("sprite transform", &TransformSprite.translation.x, 0.1f);
			ImGui::DragFloat3("sprite rotation", &TransformSprite.rotation.x, 0.1f);
			ImGui::DragFloat3("sprite scale", &TransformSprite.scale.x, 0.1f);
			ImGui::DragFloat2("sprite uv transform", &uvTransformSprite.translation.x, 0.01f);
			ImGui::DragFloat2("sprite uv scale", &uvTransformSprite.scale.x, 0.01f);
			ImGui::DragFloat("sprite uv rotation", &uvTransformSprite.rotation.z, 0.01f);

			ImGui::Text(" ");
			ImGui::Text("------- ball -----------------");
			ImGui::Checkbox("Show Ball", &showBall);
			ImGui::DragFloat3("ball transform", &transformball.translation.x, 0.1f);
			ImGui::DragFloat3("ball rotation", &transformball.rotation.x, 0.1f);
			ImGui::DragFloat3("ball scale", &transformball.scale.x, 0.1f);
			ImGui::DragFloat2("ball uv transform", &uvTransformBall.translation.x, 0.01f);
			ImGui::DragFloat2("ball uv scale", &uvTransformBall.scale.x, 0.01f);
			ImGui::DragFloat("ball uv rotation", &uvTransformBall.rotation.z, 0.01f);

			ImGui::Text(" ");
			ImGui::Text("------- lighting -----------------");
			ImGui::ColorEdit4("directionalLightColor", &directionalLightData->color.x);
			ImGui::DragFloat3("DirectionalLightDirection", &directionalLightData->direction.x, 0.1f);
			ImGui::DragFloat("DirectionalLightIntensity", &directionalLightData->intensity, 0.1f);

			ImGui::Text(" 0: no lighting");
			ImGui::Text(" 1: half lambert");
			ImGui::Text(" 2: lambert");
			ImGui::DragInt("EnableLighting", &ball->materialData->enableLighting, 0.1f, 0, 2);
			a->materialData->enableLighting = ball->materialData->enableLighting;

			ImGui::Text(" ");
			ImGui::Text("------- audio -----------------");
			if (ImGui::Button("Play Audio")) {
				engineCommon->PlayAudio("Test");
			}

			ImGui::End();

			ImGui::Render();
#endif
			if (showPlane) a->Draw();
			if (showSprite) b->Draw();
			if (showBall) ball->Draw();

			spriteCommon->PreDraw();
			engineCommon->PostDraw();

		}
	}

	// Explicit new/delete pair for `a` to demonstrate manual dynamic management.
	delete a;
	delete spriteCommon;
	delete model;

	engineCommon->Finalize();

	return 0;
}