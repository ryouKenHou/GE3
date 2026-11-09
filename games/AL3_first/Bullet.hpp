#pragma once
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "TileMap.hpp"
#include <cmath>

// A single projectile fired by the player.
// - Uses a shared Model (the scene's ball_ model) so no per-bullet model load is needed.
// - Moves in a straight line, rebounds off solid TileMap cells (walls/floor), and
//   is marked dead when it hits something it should be destroyed by (player/enemy).
class Bullet {
private:
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transform_{ {0.1f,0.1f,0.1f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScale_{ 0.1f, 0.1f, 0.1f };

	Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
	float radius_ = 0.1f; // collision radius against player/enemies
	bool alive_ = true;

	std::unique_ptr<Object3D> bulletObject_ = nullptr;

public:
	Bullet() = default;
	~Bullet() = default;

	void Initialize(EngineCommon* engineCommon, Model* sharedModel, const Vector3& startPosition, const Vector3& velocity) {
		bulletObject_ = std::make_unique<Object3D>();
		bulletObject_->Initialize(engineCommon);
		bulletObject_->SetModel(sharedModel);
		transform_.scale = defaultScale_;
		transform_.translation = startPosition;
		velocity_ = velocity;
	}

	// Moves the bullet and rebounds it off solid TileMap cells (walls/floor).
	void Update(float deltaTime, DebugCamera* debugCamera, TileMap* tileMap) {
		if (!alive_) return;

		// Move each axis separately so we can rebound (reflect velocity) independently
		// on X or Y when a wall is hit, instead of just stopping.
		MoveAndRebound(tileMap, 0, velocity_.x * deltaTime);
		MoveAndRebound(tileMap, 1, velocity_.y * deltaTime);

		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
		Matrix4x4 wvpMatrix = worldMatrix * debugCamera->getViewProjectionMatrix();
		*bulletObject_->wvpData = { wvpMatrix, worldMatrix };
		bulletObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	void Draw() {
		if (alive_ && bulletObject_) {
			bulletObject_->Draw();
		}
	}

	bool IsAlive() const { return alive_; }
	void Kill() { alive_ = false; }

	Vector3 GetPosition() const { return transform_.translation; }
	float GetRadius() const { return radius_; }

	// Simple circle-vs-circle overlap test in the X/Y plane (Z ignored, 2D action game).
	bool Overlaps(const Vector3& otherPos, float otherRadius) const {
		float dx = transform_.translation.x - otherPos.x;
		float dy = transform_.translation.y - otherPos.y;
		float distSq = dx * dx + dy * dy;
		float sumR = radius_ + otherRadius;
		return distSq <= (sumR * sumR);
	}

private:
	void MoveAndRebound(TileMap* tileMap, int axis, float delta) {
		if (axis == 0) transform_.translation.x += delta;
		else transform_.translation.y += delta;

		if (!tileMap || delta == 0.0f) return;

		float tileSize = tileMap->GetTileSize();
		float tileHalf = tileSize * 0.5f;

		int col = tileMap->WorldToCol(transform_.translation.x);
		int row = tileMap->WorldToRow(transform_.translation.y);

		if (!tileMap->IsSolid(col, row)) return;

		Vector3 tileCenter = tileMap->GridToWorld(col, row);

		if (axis == 0) {
			if (delta > 0.0f) {
				transform_.translation.x = tileCenter.x - tileHalf - radius_;
			} else {
				transform_.translation.x = tileCenter.x + tileHalf + radius_;
			}
			velocity_.x = -velocity_.x; // rebound
		} else {
			if (delta > 0.0f) {
				transform_.translation.y = tileCenter.y - tileHalf - radius_;
			} else {
				transform_.translation.y = tileCenter.y + tileHalf + radius_;
			}
			velocity_.y = -velocity_.y; // rebound
		}
	}
};
