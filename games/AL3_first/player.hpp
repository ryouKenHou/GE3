#pragma once

#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include "WindowManager.h"
#include "TileMap.hpp"
#include <cmath>
#include <cstdlib>

// Projects a world-space point through a matrix (view*projection) and returns
// clip-space coordinates (x, y, z, w). Caller is responsible for the
// perspective divide (dividing x/y/z by w) to get NDC coordinates.
inline Vector4 TransformToClipSpace(const Vector3& v, const Matrix4x4& m) {
	float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	return Vector4(x, y, z, w);
}

class Player {
private:
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transform_{ {3.0f,3.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScale_{ 0.5f,  0.5f,  0.5f };

	Transform WandTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform WandUVTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 WandDefaultScale_{ 0.5f, 0.5f, 0.5f };
	Vector3 DefaultWandToPlayerOffset_{ 0.0f,0.1f, -0.5f }; // Default offset of the wand relative to the player

	std::unique_ptr<Model> playerModel = nullptr;
	std::unique_ptr<Object3D> playerObject_ = nullptr;

	std::unique_ptr<Model> wandModel_ = nullptr;
	std::unique_ptr<Object3D> wandObject_ = nullptr;

	std::unique_ptr<Model> shieldModel_ = nullptr;
	std::unique_ptr<Object3D> shieldObject_ = nullptr;

	// ---------------- Physics / collision ----------------
	Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
	bool isGrounded_ = false;

	// Half-extents of the player's collision AABB in world units (X/Y plane;
	// Z is ignored for this 2D action game). Tuned to roughly match
	// defaultScale_ so the box visually matches the model.
	Vector3 collisionHalfExtents_{ 0.35f, 0.35f, 0.35f };

	static constexpr float kGravity = -5.0f;       // acceleration, units/s^2
	static constexpr float kMaxFallSpeed = -10.0f;  // terminal velocity (negative = downward)

	// ---------------- Combat ----------------
	int hp_ = 1;
	float shootCooldown_ = 0.0f;
	bool shieldActive_ = true;
	static constexpr float kShootCooldownDuration = 0.2f;
	static constexpr float kBulletSpeed = 8.0f;
	static constexpr float kRecoilForce = 6.0f;

	// Set by Update() when the player fires this frame; the scene polls this
	// each frame to know whether/where to spawn a Bullet (Player doesn't own
	// the bullet list / shared ball model, so it can't spawn Bullets itself).
	bool wantsToShoot_ = false;
	Vector3 pendingShootDirection_{ 0.0f, 0.0f, 0.0f };

	// ---------------- Damage shake ----------------
	float shakeTimer_ = 0.0f;
	static constexpr float kShakeDuration = 0.2f;
	static constexpr float kShakeMagnitude = 0.08f;
	Vector3 shakeOffset_{ 0.0f, 0.0f, 0.0f };


	// Resolves movement/collision for a single axis against solid TileMap cells.
	// axis: 0 = X, 1 = Y. 'delta' is the attempted movement along that axis.
	void MoveAndCollide(TileMap* tileMap, int axis, float delta) {
		if (!tileMap || delta == 0.0f) {
			if (axis == 0) transform_.translation.x += delta;
			else transform_.translation.y += delta;
			return;
		}

		float tileSize = tileMap->GetTileSize();
		float halfX = collisionHalfExtents_.x;
		float halfY = collisionHalfExtents_.y;

		if (axis == 0) {
			transform_.translation.x += delta;
		} else {
			transform_.translation.y += delta;
		}

		float minX = transform_.translation.x - halfX;
		float maxX = transform_.translation.x + halfX;
		float minY = transform_.translation.y - halfY;
		float maxY = transform_.translation.y + halfY;

		int colStart = tileMap->WorldToCol(minX);
		int colEnd = tileMap->WorldToCol(maxX);
		int rowStart = tileMap->WorldToRow(maxY); // higher world Y -> smaller row
		int rowEnd = tileMap->WorldToRow(minY);

		for (int row = rowStart; row <= rowEnd; ++row) {
			for (int col = colStart; col <= colEnd; ++col) {
				if (!tileMap->IsSolid(col, row)) continue;

				Vector3 tileCenter = tileMap->GridToWorld(col, row);
				float tileHalf = tileSize * 0.5f;
				float tMinX = tileCenter.x - tileHalf;
				float tMaxX = tileCenter.x + tileHalf;
				float tMinY = tileCenter.y - tileHalf;
				float tMaxY = tileCenter.y + tileHalf;

				// Re-evaluate current AABB each iteration since we may have already
				// been pushed out by a previous overlapping tile.
				minX = transform_.translation.x - halfX;
				maxX = transform_.translation.x + halfX;
				minY = transform_.translation.y - halfY;
				maxY = transform_.translation.y + halfY;

				bool overlap = (minX < tMaxX) && (maxX > tMinX) && (minY < tMaxY) && (maxY > tMinY);
				if (!overlap) continue;

				if (axis == 0) {
					// Push out horizontally and stop horizontal velocity.
					if (delta > 0.0f) {
						transform_.translation.x = tMinX - halfX;
					} else if (delta < 0.0f) {
						transform_.translation.x = tMaxX + halfX;
					}
					velocity_.x = 0.0f;
				} else {
					// Push out vertically and stop vertical velocity.
					if (delta > 0.0f) {
						// Moving up, hit the underside of a tile.
						transform_.translation.y = tMinY - halfY;
					} else if (delta < 0.0f) {
						// Moving down, landed on top of a tile.
						transform_.translation.y = tMaxY + halfY;
						isGrounded_ = true;
					}
					velocity_.y = 0.0f;
				}
			}
		}
	}

	// Applies gravity and resolves collision against the given TileMap.
	// Call once per frame before computing the render matrices.
	void UpdatePhysics(float deltaTime, TileMap* tileMap) {
		// Assume airborne; MoveAndCollide sets this back to true on landing.
		isGrounded_ = false;

		velocity_.y += kGravity * deltaTime;
		if (velocity_.y < kMaxFallSpeed) velocity_.y = kMaxFallSpeed;

		MoveAndCollide(tileMap, 0, velocity_.x * deltaTime);
		MoveAndCollide(tileMap, 1, velocity_.y * deltaTime);
	}

public:
	Player() = default;
	~Player() = default;

	bool isShieldActive() const { return shieldActive_; }

	void Initialize(EngineCommon* engineCommon) {
		// Initialize player model and object
		playerModel = std::make_unique<Model>();
		playerModel->Initialize(engineCommon);
		playerModel->LoadModel("resources/model/player", "player.obj");
		playerObject_ = std::make_unique<Object3D>();
		playerObject_->Initialize(engineCommon);
		playerObject_->SetModel(playerModel.get());
		transform_.scale = defaultScale_;
		transform_.translation = { 2.0f, 2.0f, 0.0f }; // Start player at origin

		// Initialize wand model and object
		wandModel_ = std::make_unique<Model>();
		wandModel_->Initialize(engineCommon);
		wandModel_->LoadModel("resources/model/wand", "wand.obj");
		wandObject_ = std::make_unique<Object3D>();
		wandObject_->Initialize(engineCommon);
		wandObject_->SetModel(wandModel_.get());
		WandTransform_.scale = WandDefaultScale_;

		// Initialize shield model and object
		shieldModel_ = std::make_unique<Model>();
		shieldModel_->Initialize(engineCommon);
		shieldModel_->LoadModel("resources/model/shield", "shield.obj");
		shieldObject_ = std::make_unique<Object3D>();
		shieldObject_->Initialize(engineCommon);
		shieldObject_->SetModel(shieldModel_.get());
	}

	void Update(float deltaTime, DebugCamera* debugCamera, TileMap* tileMap = nullptr) {
		// Gravity + collision against the tile map (floors/walls), resolved
		// before building the render matrices so the visuals match this frame's
		// corrected position.
		UpdatePhysics(deltaTime, tileMap);

		if (shootCooldown_ > 0.0f) {
			shootCooldown_ -= deltaTime;
		}
		wantsToShoot_ = false;

		if (shakeTimer_ > 0.0f) {
			shakeTimer_ -= deltaTime;
			float t = shakeTimer_ > 0.0f ? (shakeTimer_ / kShakeDuration) : 0.0f;
			float magnitude = kShakeMagnitude * t;
			shakeOffset_.x = ((std::rand() % 200) / 100.0f - 1.0f) * magnitude;
			shakeOffset_.y = ((std::rand() % 200) / 100.0f - 1.0f) * magnitude;
		} else {
			shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		}

		// Update player and wand transformations based on input or game logic
		Transform renderTransform = transform_;
		renderTransform.translation.y -= 0.5f; // Raise the player model slightly above the collision box
		renderTransform.translation = renderTransform.translation + shakeOffset_;
		Matrix4x4 worldMatrixPlayer = Matrix4x4::MakeAffineMatrix(renderTransform.scale, renderTransform.rotation, renderTransform.translation);
		Matrix4x4 wvpMatrixPlayer = worldMatrixPlayer * debugCamera->getViewProjectionMatrix();
		*playerObject_->wvpData = { wvpMatrixPlayer, worldMatrixPlayer };
		playerObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
		*shieldObject_->wvpData = { wvpMatrixPlayer, worldMatrixPlayer };
		shieldObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);

		// Update wand transformations based on input or game logic
		WandTransform_.translation = transform_.translation + DefaultWandToPlayerOffset_;

		// ---- Rotate the wand's Z axis to point toward the mouse cursor ----
		{
			// Project the wand's world position into clip space, then to screen space,
			// so we can compare it against the mouse's screen-space position.
			Matrix4x4 viewProjection = debugCamera->getViewProjectionMatrix();
			Vector4 clipPos = TransformToClipSpace(WandTransform_.translation, viewProjection);

			if (clipPos.w > 0.0001f) { // avoid divide-by-zero / behind-camera cases
				float ndcX = clipPos.x / clipPos.w;
				float ndcY = clipPos.y / clipPos.w;

				float screenX = (ndcX + 1.0f) * 0.5f * static_cast<float>(WindowManager::GetClientWidth());
				float screenY = (1.0f - ndcY) * 0.5f * static_cast<float>(WindowManager::GetClientHeight());

				InputSystem& input = EngineCommon::GetInstance().GetInputSystem();
				float mouseX = static_cast<float>(input.GetMouseX());
				float mouseY = static_cast<float>(input.GetMouseY());

				float dx = mouseX - screenX;
				float dy = mouseY - screenY;

				// atan2f gives the angle (in radians) from the wand's screen position to the mouse.
				// Negate dy because screen Y grows downward while a standard math angle expects Y to grow upward.
				float aimAngle = std::atan2f(-dy, dx);
				WandTransform_.rotation.z = aimAngle - 1.57f;

				// ---- Shooting ----
				// Shoot direction in world space: aimAngle is measured in screen space
				// (X right, Y up after the -dy negation), which matches the world X/Y
				// plane used for this 2D action game (Z fixed at 0).
				if (input.IsMouseButtonTriggered(0) && shootCooldown_ <= 0.0f) {
					Vector3 shootDir{ std::cosf(aimAngle), std::sinf(aimAngle), 0.0f };
					wantsToShoot_ = true;
					pendingShootDirection_ = shootDir;

					// Recoil: push the player in the reverse direction of the shot.
					velocity_.x -= shootDir.x * kRecoilForce;
					velocity_.y -= shootDir.y * kRecoilForce;

					shootCooldown_ = kShootCooldownDuration;
				}
			}
		}

		Matrix4x4 worldMatrixWand = Matrix4x4::MakeAffineMatrix(WandTransform_.scale, WandTransform_.rotation, WandTransform_.translation);
		Matrix4x4 wvpMatrixWand = worldMatrixWand * debugCamera->getViewProjectionMatrix();
		*wandObject_->wvpData = { wvpMatrixWand, worldMatrixWand };
		wandObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(WandUVTransform_.scale, WandUVTransform_.rotation, WandUVTransform_.translation);
	}

	void Draw() {
		playerObject_->Draw();
		wandObject_->Draw();
		if (shieldActive_) {
			shieldObject_->Draw();
		}
	}

	Transform* GetTransform() {
		return &transform_;
	}

	void SetTransform(const Transform& newTransform) {
		transform_ = newTransform;
	}

	// ---- Physics accessors (useful for horizontal movement input, jumping, etc.) ----
	Vector3& GetVelocity() { return velocity_; }
	void SetVelocity(const Vector3& v) { velocity_ = v; }
	bool IsGrounded() const { return isGrounded_; }

	// ---- Combat / HP ----
	int GetHp() const { return hp_; }
	bool IsAlive() const { return hp_ > 0; }

	// Resets combat/physics state for a fresh run (e.g. after Game Over).
	void ResetForNewGame() {
		hp_ = 1; // matches the initial value of hp_'s in-class initializer
		shieldActive_ = true;
		shootCooldown_ = 0.0f;
		velocity_ = { 0.0f, 0.0f, 0.0f };
		shakeTimer_ = 0.0f;
		shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		transform_.scale = defaultScale_;
		transform_.translation = { 2.0f, 2.0f, 0.0f }; // Start player at origin
	}

	// ---------------- Shield ----------------
	void SetShieldActive(bool active) { shieldActive_ = active; }
	bool IsShieldActive() const { return shieldActive_; }

	void TakeDamage(int amount) {
		if (shieldActive_) {
			// Shield absorbs the damage, deactivate it
			shieldActive_ = false;
			// Optionally, you can add some visual/audio feedback for shield break here
			return;
		}
		hp_ -= amount;
		if (hp_ < 0) hp_ = 0;
		shakeTimer_ = kShakeDuration;
	}

	// The scene polls these once per frame after calling Update() to know
	// whether to spawn a Bullet (Player doesn't own the shared ball model or
	// bullet list).
	bool ConsumeShootRequest(Vector3& outDirection, Vector3& outSpawnPosition) {
		if (!wantsToShoot_) return false;
		outDirection = pendingShootDirection_;
		outSpawnPosition = WandTransform_.translation + pendingShootDirection_ * 1.0f;
		wantsToShoot_ = false;
		return true;
	}

	float GetShootCooldownDuration() const { return kShootCooldownDuration; }
	float GetBulletSpeed() const { return kBulletSpeed; }

	// Collision helper for bullets/enemies (circle in the X/Y plane).
	Vector3 GetPosition() const { return transform_.translation; }
	float GetCollisionRadius() const { return collisionHalfExtents_.x; }
};