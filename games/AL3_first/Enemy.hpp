#pragma once
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include "WindowManager.h"
#include <cmath>
#include <cstdlib>

class Enemy {
private:
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScale_{ 0.5f,0.5f, 0.5f };

	Model* EnemyModel = nullptr;
	std::unique_ptr<Object3D> EnemyObject_ = nullptr;

	int hp_ = 2;
	float moveSpeed_ = 2.0f;
	float collisionRadius_ = 0.25f;

	float shakeTimer_ = 0.0f;
	static constexpr float kShakeDuration = 0.2f;
	static constexpr float kShakeMagnitude = 0.08f;
	Vector3 shakeOffset_{ 0.0f, 0.0f, 0.0f };

	bool alive_ = true;

	EngineCommon* engineCommon_ = nullptr;

public:
	Enemy() = default;
	~Enemy() = default;

	// Lightweight path for tile maps / many instances: pass in a Model that is
	// already loaded and shared (e.g. owned by a TileMap). This Block does NOT
	// own the model and will not reload it.
	void Initialize(EngineCommon* engineCommon, Model* sharedModel) {
		EnemyObject_ = std::make_unique<Object3D>();
		EnemyObject_->Initialize(engineCommon);
		EnemyObject_->SetModel(sharedModel);
		transform_.scale = defaultScale_;

		engineCommon_ = engineCommon;
	}

	void Update(float deltaTime, DebugCamera* debugCamera) {
		Transform renderTransform = transform_;
		renderTransform.translation.x += shakeOffset_.x;
		renderTransform.translation.y += shakeOffset_.y;
		Matrix4x4 worldMatrixEnemy = Matrix4x4::MakeAffineMatrix(renderTransform.scale, renderTransform.rotation, renderTransform.translation);
		Matrix4x4 wvpMatrixEnemy = worldMatrixEnemy * debugCamera->getViewProjectionMatrix();
		*EnemyObject_->wvpData = { wvpMatrixEnemy, worldMatrixEnemy };
		EnemyObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	// Moves the enemy slowly toward the given target (the player) and rotates it
	// to face that direction. Call once per frame before Update()/Draw().
	void UpdateAI(float deltaTime, const Vector3& targetPosition) {
		if (!alive_) return;

		float dx = targetPosition.x - transform_.translation.x;
		float dy = targetPosition.y - transform_.translation.y;
		float dist = std::sqrtf(dx * dx + dy * dy);

		if (dist > 0.0001f) {
			float invDist = 1.0f / dist;
			float dirX = dx * invDist;
			float dirY = dy * invDist;

			transform_.translation.x += dirX * moveSpeed_ * deltaTime;
			transform_.translation.y += dirY * moveSpeed_ * deltaTime;

			// Rotate to face the player. Uses the same screen-space style aiming
			// convention as the player's wand (atan2f(y, x) around Z).
			transform_.rotation.z = std::atan2f(dirY, dirX);
		}

		if (shakeTimer_ > 0.0f) {
			shakeTimer_ -= deltaTime;
			float t = shakeTimer_ > 0.0f ? (shakeTimer_ / kShakeDuration) : 0.0f;
			float magnitude = kShakeMagnitude * t;
			shakeOffset_.x = ((std::rand() % 200) / 100.0f - 1.0f) * magnitude;
			shakeOffset_.y = ((std::rand() % 200) / 100.0f - 1.0f) * magnitude;
		} else {
			shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		}
	}

	void Draw() {
		if (EnemyObject_) {
			EnemyObject_->Draw();
		}
	}

	// ---- Transform accessors (needed so a TileMap can place each instance) ----
	Transform* GetTransform() { return &transform_; }
	void SetTransform(const Transform& newTransform) { transform_ = newTransform; }
	void SetPosition(const Vector3& position) { transform_.translation = position; }
	void SetRotation(const Vector3& rotation) { transform_.rotation = rotation; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }

	// ---- Combat / HP ----
	int GetHp() const { return hp_; }
	bool IsAlive() const { return alive_ && hp_ > 0; }

	void TakeDamage(int amount) {
		hp_ -= amount;
		shakeTimer_ = kShakeDuration;
		if (hp_ <= 0) {
			hp_ = 0;
			alive_ = false;
			engineCommon_->PlayAudio("dead");
		}
	}

	Vector3 GetPosition() const { return transform_.translation; }
	float GetCollisionRadius() const { return collisionRadius_; }

	// Allows the scene to scale enemy move speed up over time (e.g. each time
	// the map rotates / difficulty increases).
	void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
	float GetMoveSpeed() const { return moveSpeed_; }
};