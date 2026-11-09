#pragma once

#include "BaseScene.h"
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include <memory>
#include <chrono>
#include <vector>
#include <string>

#include "player.hpp"
#include "block.hpp"
#include "TileMap.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"

class AL3GamePlayScene : public BaseScene {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

private:
    EngineCommon* engineCommon_ = nullptr;
    std::unique_ptr<SpriteCommon> spriteCommon_;

    int32_t kClientWidth_ = 0;
    int32_t kClientHeight_ = 0;

    std::unique_ptr<DebugCamera> debugCamera_;

    // transform for test object
    Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };    
    Transform transformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transformSpriteGameOver_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };    
    Transform uvTransformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	// sprite for score
	Transform transformScoreSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransformScoreSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform transformScoreUiSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransformScoreUiSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScoreSpriteTranslation_{ 0.0f, 640.0f, 0.0f };
	Vector3 gameOverSpriteTranslation_{ 400.0f, 180.0f, 0.0f };

	// 4-digit score display. Each digit is a separate Sprite instance using the
	// same digit sprite sheet as scoreSprite_; only the UV transform's
	// translation.x differs (0.1f per digit, as in uvTransformScoreSprite_).
	static constexpr int kScoreDigitCount = 4;
	int score_ = 0;
	std::unique_ptr<Sprite> scoreDigitSprites_[kScoreDigitCount];
	Transform scoreDigitTransforms_[kScoreDigitCount];
	Transform scoreDigitUvTransforms_[kScoreDigitCount];

	// Countdown showing seconds remaining before the map auto-switches.
	// Reuses the exact same digit-sheet sprite approach as the score digits:
	// each digit is its own Sprite sharing scoreSprite_'s texture, with
	// uvTransform.translation.x = digitValue * 0.1f selecting the glyph.
	std::unique_ptr<Sprite> countdownDigitSprites_;
	Transform countdownDigitTransforms_;
	Transform countdownDigitUvTransforms_;
	Vector3 countdownBaseTranslation_{ 620.0f, 10.0f, 0.0f }; // tune position on screen

    // transforms
    Transform transformball_{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform uvTransformBall_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    bool showPlane_ = false;
    bool showSprite_ = true;
    bool showBall_ = false;
	bool gameOver_ = false;
	float gameOverTimer_ = 0.0f;
	static constexpr float kGameOverInputDelay = 0.5f; // seconds before reset click is accepted

    int frameCount_ = 0;

    // test object
    std::unique_ptr<Model> model_;
    std::unique_ptr<Object3D> a_;
    std::unique_ptr<Sprite> b_;
	std::unique_ptr<Sprite> gameOverSprite_;
	std::unique_ptr<Sprite> scoreSprite_;
	std::unique_ptr<Sprite> scoreUiSprite_;

    std::unique_ptr<Model> ball_;
    std::unique_ptr<Model> blockModel_;
	std::unique_ptr<Model> EnemyModel_;
	std::unique_ptr<Model> SkyDomeModel_;

	Player player_; // Player class instance
	Block block_; // Block class instance
	TileMap tileMap_; // 2D tile map used to build rooms out of Blocks
	SkyDome skyDome_; // SkyDome class instance

	// ---------------- Bullets ----------------
	std::vector<std::unique_ptr<Bullet>> bullets_;

	// ---------------- Enemies ----------------
	std::vector<std::unique_ptr<Enemy>> enemies_;
	float enemySpawnTimer_ = 0.0f;
	static constexpr float kEnemySpawnIntervalDefault = 1.5f;
	static constexpr float kEnemyMoveSpeedDefault = 1.2f;
	float enemySpawnInterval_ = kEnemySpawnIntervalDefault; // was: static constexpr kEnemySpawnInterval; now mutable for difficulty scaling
	float enemyMoveSpeed_ = kEnemyMoveSpeedDefault;         // applied to newly spawned enemies; scales up each map change
	static constexpr float kEnemySpawnRadius = 5.5f;

	// ---------------- Map rotation / difficulty ----------------
	std::vector<std::string> mapPaths_ = {
		"resources/maps/room1.json",
		"resources/maps/room2.json",
		"resources/maps/room3.json"
	};
	int currentMapIndex_ = 0;
	float mapTimer_ = 0.0f;
	static constexpr float kMapSwitchInterval = 8.0f;
	static constexpr float kEnemySpawnIntervalMultiplier = 0.9f; // spawn faster each map change
	static constexpr float kEnemySpawnIntervalMin = 0.5f;
	static constexpr float kEnemyMoveSpeedMultiplier = 1.2f;     // move faster each map change

	void SwitchToNextMap();
	void ResetGame();

	std::chrono::steady_clock::time_point lastFrameTime_;
	float deltaTime_ = 0.0f;
};