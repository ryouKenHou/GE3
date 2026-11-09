#include "AL3GamePlayScene.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "SceneManager.h"

void AL3GamePlayScene::Initialize() {
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
	b_->SetTexture("resources/texture/Font/tutorial.png");
	transformSprite_.scale = { 0.5f,0.2f,1.0f };
	transformSprite_.translation = { 951.f, 637.f, 0.f };

	gameOverSprite_ = std::make_unique<Sprite>();
	gameOverSprite_->Initialize(engineCommon_);
	gameOverSprite_->SetTexture("resources/texture/gameover.png");
	transformSpriteGameOver_.scale = { 2.f,2.f,2.0f };
	transformSpriteGameOver_.translation = { 0.f, 0.f, 0.f };

	scoreUiSprite_ = std::make_unique<Sprite>();
	scoreUiSprite_->Initialize(engineCommon_);
	scoreUiSprite_->SetTexture("resources/texture/Font/score.png");
	transformScoreUiSprite_.scale = { 0.2f,0.2f,1.0f };
	transformScoreUiSprite_.translation = defaultScoreSpriteTranslation_ - Vector3{ 0.f, 70.f, 0.f };

	scoreSprite_ = std::make_unique<Sprite>();
	scoreSprite_->Initialize(engineCommon_);
	for (int i = 0; i < kScoreDigitCount; ++i) {
		scoreDigitSprites_[i] = std::make_unique<Sprite>();
		scoreDigitSprites_[i]->Initialize(engineCommon_);
		scoreDigitSprites_[i]->SetTexture("resources/texture/Font/nums.png");
	}
	scoreSprite_->SetTexture("resources/texture/Font/nums.png");
	transformScoreSprite_.scale = { 0.2f,0.2f,1.0f };
	transformScoreSprite_.translation = defaultScoreSpriteTranslation_;
	uvTransformScoreSprite_.scale = { 0.1f, 0.99f, 1.f };
	uvTransformScoreSprite_.translation = { 0.0f, 0.0f,0.f };

	countdownDigitSprites_ = std::make_unique<Sprite>();
	countdownDigitSprites_->Initialize(engineCommon_);
	countdownDigitSprites_->SetTexture("resources/texture/Font/nums.png");
	countdownDigitTransforms_.scale = { 0.2f,0.2f, 1.0f };
	countdownDigitTransforms_.translation = countdownBaseTranslation_;
	countdownDigitUvTransforms_.scale = { 0.1f, 0.99f, 1.f };
	countdownDigitUvTransforms_.translation = { 0.0f, 0.0f, 0.0f };

	ball_ = std::make_unique<Model>();
	ball_->Initialize(engineCommon_);
	ball_->UsingTemplateModel(1);
	ball_->SetTexture("resources/circle.png");

	player_.Initialize(engineCommon_);

	tileMap_.Initialize(engineCommon_, 10, 8, 1.0f);
	tileMap_.LoadFromJson("resources/maps/room1.json");

	blockModel_ = std::make_unique<Model>();
	blockModel_->Initialize(engineCommon_);
	blockModel_->LoadModel("resources/model/block", "block.obj");
	block_.Initialize(engineCommon_, blockModel_.get());

	EnemyModel_ = std::make_unique<Model>();
	EnemyModel_->Initialize(engineCommon_);
	EnemyModel_->LoadModel("resources/model/enemy", "enemy.obj");

	SkyDomeModel_ = std::make_unique<Model>();
	SkyDomeModel_->Initialize(engineCommon_);
	SkyDomeModel_->LoadModel("resources/model/skyDome", "skyDome.obj");
	SkyDomeModel_->SetEnableLighting(false); // Disable lighting for the skyDome
	skyDome_.Initialize(engineCommon_, SkyDomeModel_.get());

	ParticleManager::GetInstance().Initialize(engineCommon_, 1000);
	ParticleManager::GetInstance().SetTexture("resources/circle.png");

	lastFrameTime_ = std::chrono::steady_clock::now();

	engineCommon_->LoadAudio("hit", "resources/sound/hit.mp3");
	engineCommon_->LoadAudio("shoot", "resources/sound/shoot.mp3");
	engineCommon_->LoadAudio("stun", "resources/sound/stun.mp3");
	engineCommon_->LoadAudio("dead", "resources/sound/dead.mp3");
}

void AL3GamePlayScene::Finalize() {
	a_.reset();
	model_.reset();
	b_.reset();
	ball_.reset();
	debugCamera_.reset();
	spriteCommon_.reset();
}

void AL3GamePlayScene::ResetGame() {
    score_ = 0;
    gameOver_ = false;
    gameOverTimer_ = 0.0f;

    enemySpawnTimer_ = 0.0f;
    enemySpawnInterval_ = kEnemySpawnIntervalDefault;
    enemyMoveSpeed_ = kEnemyMoveSpeedDefault;

    currentMapIndex_ = 0;
    mapTimer_ = 0.0f;
    tileMap_.LoadFromJson(mapPaths_[currentMapIndex_]);

    enemies_.clear();
    bullets_.clear();

    player_.ResetForNewGame();

	transformScoreUiSprite_.translation = defaultScoreSpriteTranslation_ - Vector3{ 0.f, 70.f, 0.f };
	transformScoreSprite_.translation = defaultScoreSpriteTranslation_;
}

void AL3GamePlayScene::SwitchToNextMap() {
    // Emit particles at every currently-placed block's position before the
    // old map's blocks are destroyed/replaced by LoadFromJson.
    for (int y = 1; y < tileMap_.GetHeight()-1; ++y) {
        for (int x = 1; x < tileMap_.GetWidth()-1; ++x) {
            if (tileMap_.IsSolid(x, y)) {
                ParticleManager::GetInstance().Emit(tileMap_.GridToWorld(x, y), 5);
            }
        }
    }

    currentMapIndex_ = (currentMapIndex_ + 1) % static_cast<int>(mapPaths_.size());
    tileMap_.LoadFromJson(mapPaths_[currentMapIndex_]);
    score_ += 5;

	for (int y = 1; y < tileMap_.GetHeight() - 1; ++y) {
		for (int x = 1; x < tileMap_.GetWidth() - 1; ++x) {
			if (tileMap_.IsSolid(x, y)) {
				ParticleManager::GetInstance().Emit(tileMap_.GridToWorld(x, y), 5);
			}
		}
	}

    // Re-activate the player's shield on every map change.
    player_.SetShieldActive(true);
	ParticleManager::GetInstance().Emit(player_.GetPosition(), 3);

    // Increase difficulty: enemies spawn faster (lower interval, clamped) and
    // move faster.
    enemySpawnInterval_ *= kEnemySpawnIntervalMultiplier;
    if (enemySpawnInterval_ < kEnemySpawnIntervalMin) {
        enemySpawnInterval_ = kEnemySpawnIntervalMin;
    }
    enemyMoveSpeed_ *= kEnemyMoveSpeedMultiplier;

    // Apply the new move speed to enemies already on the map too, not just
    // future spawns.
    for (auto& e : enemies_) {
        e->SetMoveSpeed(enemyMoveSpeed_);
    }
}

void AL3GamePlayScene::Update() {
	Input::Update();

	BYTE* keyboardState = EngineCommon::GetInstance().GetInputSystem().getKeyboardState();

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

	Matrix4x4 worldMatrixSpriteGameOver = Matrix4x4::MakeAffineMatrix(transformSpriteGameOver_.scale, transformSpriteGameOver_.rotation, transformSpriteGameOver_.translation);
	Matrix4x4 wvpMatrixSpriteGameOver = worldMatrixSpriteGameOver * orthoProjection;
	*gameOverSprite_->wvpData = { wvpMatrixSpriteGameOver, worldMatrixSpriteGameOver };
	gameOverSprite_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformSprite_.scale, uvTransformSprite_.rotation, uvTransformSprite_.translation);

	Matrix4x4 worldMatrixSpriteUiScore = Matrix4x4::MakeAffineMatrix(transformScoreUiSprite_.scale, transformScoreUiSprite_.rotation, transformScoreUiSprite_.translation);
	Matrix4x4 wvpMatrixSpriteUiScore = worldMatrixSpriteUiScore * orthoProjection;
	*scoreUiSprite_->wvpData = { wvpMatrixSpriteUiScore, worldMatrixSpriteUiScore };
	scoreUiSprite_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformScoreUiSprite_.scale, uvTransformScoreUiSprite_.rotation, uvTransformScoreUiSprite_.translation);

	Matrix4x4 worldMatrixSpriteCountD = Matrix4x4::MakeAffineMatrix(countdownDigitTransforms_.scale, countdownDigitTransforms_.rotation, countdownDigitTransforms_.translation);
	Matrix4x4 wvpMatrixSpriteCountD = worldMatrixSpriteCountD * orthoProjection;
	*countdownDigitSprites_->wvpData = { wvpMatrixSpriteCountD, worldMatrixSpriteCountD };
	countdownDigitSprites_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(countdownDigitUvTransforms_.scale, countdownDigitUvTransforms_.rotation, countdownDigitUvTransforms_.translation + Vector3{ int(kMapSwitchInterval - mapTimer_) * 0.1f, 0.0f, 0.0f });

	Matrix4x4 worldMatrixSpriteScore = Matrix4x4::MakeAffineMatrix(transformScoreSprite_.scale, transformScoreSprite_.rotation, transformScoreSprite_.translation);
	Matrix4x4 wvpMatrixSpriteScore = worldMatrixSpriteScore * orthoProjection;
	*scoreSprite_->wvpData = { wvpMatrixSpriteScore, worldMatrixSpriteScore };
	scoreSprite_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformScoreSprite_.scale, uvTransformScoreSprite_.rotation, uvTransformScoreSprite_.translation);
	{
		// ---- 4-digit score display ----
		// Extracts each decimal digit of score_ (clamped to 0-9999) and positions
		// 4 digit sprites side-by-side, reusing the same digit sheet convention as
		// scoreSprite_: uvTransform.translation.x = digitValue * 0.1f selects which
		// digit glyph to show, and scale {0.1f, 0.99f, 1.f} crops to one glyph.
		int clampedScore = score_;
		if (clampedScore < 0) clampedScore = 0;
		if (clampedScore > 9999) clampedScore = 9999;

		int digits[kScoreDigitCount];
		int remaining = clampedScore;
		for (int i = kScoreDigitCount - 1; i >= 0; --i) {
			digits[i] = remaining % 10;
			remaining /= 10;
		}

		// Horizontal distance (in screen pixels, matching transformScoreSprite_'s
		// space) between each digit. Tune this if digits overlap or have gaps.
		const float kDigitSpacing = 80.0f;

		for (int i = 0; i < kScoreDigitCount; ++i) {
			scoreDigitTransforms_[i] = transformScoreSprite_;
			scoreDigitTransforms_[i].translation.x = transformScoreSprite_.translation.x + static_cast<float>(i) * kDigitSpacing;

			scoreDigitUvTransforms_[i] = uvTransformScoreSprite_;
			scoreDigitUvTransforms_[i].translation.x = static_cast<float>(digits[i]) * 0.1f;

			Matrix4x4 worldMatrixDigit = Matrix4x4::MakeAffineMatrix(
				scoreDigitTransforms_[i].scale, scoreDigitTransforms_[i].rotation, scoreDigitTransforms_[i].translation);
			Matrix4x4 wvpMatrixSprite1 = worldMatrixDigit * orthoProjection;
			*scoreDigitSprites_[i]->wvpData = { wvpMatrixSprite1, worldMatrixDigit };
			scoreDigitSprites_[i]->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(
				scoreDigitUvTransforms_[i].scale, scoreDigitUvTransforms_[i].rotation, scoreDigitUvTransforms_[i].translation);
		}
	}
	//*scoreDigitSprites_[0]->wvpData = { wvpMatrixSpriteScore, worldMatrixSpriteScore };
	//scoreDigitSprites_[0]->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformScoreSprite_.scale, uvTransformScoreSprite_.rotation, uvTransformScoreSprite_.translation + Vector3{ 0.1f,0.f,0.f });

	// ball
	Matrix4x4 worldMatrixBall = Matrix4x4::MakeAffineMatrix(transformball_.scale, transformball_.rotation, transformball_.translation);
	Matrix4x4 wvpMatrixBall = worldMatrixBall * debugCamera_->getViewProjectionMatrix();
	*ball_->wvpData = { wvpMatrixBall, worldMatrixBall };
	ball_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransformBall_.scale, uvTransformBall_.rotation, uvTransformBall_.translation);

	// player
	player_.Update(deltaTime_, debugCamera_.get(), &tileMap_);

	// ---- Game Over detection ----
	if (!gameOver_ && player_.GetHp() <= 0) {
		gameOver_ = true;
		gameOverTimer_ = 0.0f;
		transformScoreSprite_.translation = gameOverSpriteTranslation_;
		transformScoreUiSprite_.translation = gameOverSpriteTranslation_ - Vector3{ 0.f, 70.f, 0.f };
	}

	// While game over, left-click resets the run; skip all other gameplay
	// updates (shooting, map rotation, enemy/bullet logic) until reset.
	if (gameOver_) {
		gameOverTimer_ += deltaTime_;
		InputSystem& input = EngineCommon::GetInstance().GetInputSystem();
		if (gameOverTimer_ >= kGameOverInputDelay && input.IsMouseButtonTriggered(0)) {
			ResetGame();
		}
		return;
	}


	// ---- Player shoot request -> spawn bullet, apply camera shake ----
	{
		Vector3 shootDir{}, spawnPos{};
		if (player_.ConsumeShootRequest(shootDir, spawnPos)) {
			auto bullet = std::make_unique<Bullet>();
			Vector3 bulletVelocity = shootDir * player_.GetBulletSpeed();
			bullet->Initialize(engineCommon_, ball_.get(), spawnPos, bulletVelocity);
			bullets_.push_back(std::move(bullet));

			engineCommon_->PlayAudio("shoot");
			ParticleManager::GetInstance().Emit(spawnPos, 3);
			if (debugCamera_) {
				debugCamera_->Shake(0.15f, 0.15f);
			}
		}
	}

	// ---- Map rotation: switch map every kMapSwitchInterval seconds ----
	mapTimer_ += deltaTime_;
	if (mapTimer_ >= kMapSwitchInterval) {
		mapTimer_ = 0.0f;
		SwitchToNextMap();
	}

	// ---- Enemy spawning ----
	enemySpawnTimer_ += deltaTime_;
	if (enemySpawnTimer_ >= enemySpawnInterval_) {
		enemySpawnTimer_ = 0.0f;

		float angle = static_cast<float>(std::rand() % 360) * 3.14159265f / 180.0f;
		Vector3 playerPos = player_.GetPosition();
		Vector3 spawnPos{
			playerPos.x + std::cosf(angle) * kEnemySpawnRadius,
			playerPos.y + std::sinf(angle) * kEnemySpawnRadius,
			0.0f
		};

		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize(engineCommon_, EnemyModel_.get());
		newEnemy->SetPosition(spawnPos);
		newEnemy->SetMoveSpeed(enemyMoveSpeed_);
		enemies_.push_back(std::move(newEnemy));
	}

	// ---- Enemy AI / collision with player ----
	for (auto& e : enemies_) {
		if (!e->IsAlive()) continue;
		e->UpdateAI(deltaTime_, player_.GetPosition());
		e->Update(deltaTime_, debugCamera_.get());

		float dx = e->GetPosition().x - player_.GetPosition().x;
		float dy = e->GetPosition().y - player_.GetPosition().y;
		float distSq = dx * dx + dy * dy;
		float sumR = e->GetCollisionRadius() + player_.GetCollisionRadius();
		if (distSq <= sumR * sumR) {
			e->TakeDamage(2);
			player_.TakeDamage(2);
			engineCommon_->PlayAudio("hit");
		}
	}
	// Remove dead enemies.
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
		[](const std::unique_ptr<Enemy>& e) { return !e->IsAlive(); }),
		enemies_.end());

	// ---- Bullets: move/rebound, then check hits vs player/enemies ----
	for (auto& b : bullets_) {
		if (!b->IsAlive()) continue;
		b->Update(deltaTime_, debugCamera_.get(), &tileMap_);

		if (b->Overlaps(player_.GetPosition(), player_.GetCollisionRadius())) {
			player_.TakeDamage(1);
			ParticleManager::GetInstance().Emit(b->GetPosition(), 12);
			b->Kill();
			engineCommon_->PlayAudio("hit");
			continue;
		}
		for (auto& e : enemies_) {
			if (!e->IsAlive()) continue;
			if (b->Overlaps(e->GetPosition(), e->GetCollisionRadius())) {
				e->TakeDamage(1);
				ParticleManager::GetInstance().Emit(b->GetPosition(), 12);
				b->Kill();
				if (!e->IsAlive()) {
					score_ += 5;
				}
				engineCommon_->PlayAudio("stun");
				break;
			}
		}
	}
	// Remove dead bullets.
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
		[](const std::unique_ptr<Bullet>& b) { return !b->IsAlive(); }),
		bullets_.end());
	tileMap_.Update(deltaTime_, debugCamera_.get());

	debugCamera_->SetTranslation(player_.GetTransform()->translation + Vector3{ 0.0f, 0.5f, -18.0f }); // Camera follows the player)

	// block
	block_.Update(deltaTime_, debugCamera_.get());

	// skyDome
	skyDome_.Update(deltaTime_, debugCamera_.get());

}

void AL3GamePlayScene::Draw() {
	engineCommon_->PreDraw();

	DirectionalLight* directionalLightData = engineCommon_->GetDirectionalLightData();

#ifdef _DEBUG
	ImGui::Begin("window");

	ImGui::Text(" ");
	ImGui::Text("------- camera -----------------");
	ImGui::DragFloat3("camera transform", &debugCamera_->GetTranslation()->x, 0.1f);

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
	ImGui::DragFloat3("sprite transform", &transformScoreSprite_.translation.x, 0.1f);
	ImGui::DragFloat3("sprite rotation", &transformScoreSprite_.rotation.x, 0.1f);
	ImGui::DragFloat3("sprite scale", &transformScoreSprite_.scale.x, 0.1f);
	ImGui::DragFloat2("sprite uv transform", &uvTransformScoreSprite_.translation.x, 0.01f);
	ImGui::DragFloat2("sprite uv scale", &uvTransformScoreSprite_.scale.x, 0.01f);
	ImGui::DragFloat("sprite uv rotation", &uvTransformScoreSprite_.rotation.z, 0.01f);

	ImGui::Text(" ");
	ImGui::Text("------- sprite2 -----------------");
	ImGui::Checkbox("Show Sprite", &showSprite_);
	ImGui::DragFloat3("sprite transform2", &scoreDigitTransforms_[0].translation.x, 0.1f);
	ImGui::DragFloat3("sprite rotation2", &scoreDigitTransforms_[0].rotation.x, 0.1f);
	ImGui::DragFloat3("sprite scale2", &scoreDigitTransforms_[0].scale.x, 0.1f);
	ImGui::DragFloat2("sprite uv transform2", &scoreDigitUvTransforms_[0].translation.x, 0.01f);
	ImGui::DragFloat2("sprite uv scale2", &scoreDigitUvTransforms_[0].scale.x, 0.01f);
	ImGui::DragFloat("sprite uv rotation2", &scoreDigitUvTransforms_[0].rotation.z, 0.01f);

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
	ImGui::Text("------- player -----------------");
	ImGui::DragFloat3("player transform", &player_.GetTransform()->translation.x, 0.1f);
	ImGui::DragFloat3("player rotation", &player_.GetTransform()->rotation.x, 0.1f);
	ImGui::DragFloat3("player scale", &player_.GetTransform()->scale.x, 0.1f);

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
	if (showBall_) ball_->Draw();
	player_.Draw();
	tileMap_.Draw();
	for (auto& b : bullets_) {
		b->Draw();
	}
	for (auto& e : enemies_) {
		e->Draw();
	}
	block_.Draw();
	skyDome_.Draw();

	spriteCommon_->PreDraw();
	if (showSprite_) b_->Draw();
	if (gameOver_) gameOverSprite_->Draw();
	else countdownDigitSprites_->Draw();
	//for (int i = 0; i < kScoreDigitCount; ++i) {
	//	scoreDigitSprites_[i]->SetTextureHandle(scoreSprite_->GetTextureHandle());
	//}
	//scoreSprite_->Draw();
	scoreUiSprite_->Draw();
	for (int i = 0; i < kScoreDigitCount; ++i) {
		scoreDigitSprites_[i]->Draw();
	}

	ParticleManager::GetInstance().Update(
		deltaTime_,
		debugCamera_->GetBillboardMatrix(),
		debugCamera_->getViewProjectionMatrix()
	);
	ParticleManager::GetInstance().Draw();

#ifdef _DEBUG
	ParticleManager::GetInstance().DrawImGui();
	tileMap_.DrawImGuiEditor();
	ImGui::Render();
#endif


	engineCommon_->PostDraw();
}