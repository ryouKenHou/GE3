#pragma once
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include "WindowManager.h"
#include <cmath>

class Block {
private:
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScale_{ 0.5f,0.5f, 0.5f };

	Model* blockModel = nullptr;
	std::unique_ptr<Object3D> blockObject_ = nullptr;

public :
	Block() = default;
	~Block() = default;

	// Lightweight path for tile maps / many instances: pass in a Model that is
	// already loaded and shared (e.g. owned by a TileMap). This Block does NOT
	// own the model and will not reload it.
	void Initialize(EngineCommon* engineCommon, Model* sharedModel) {
		blockObject_ = std::make_unique<Object3D>();
		blockObject_->Initialize(engineCommon);
		blockObject_->SetModel(sharedModel);
		transform_.scale = defaultScale_;
	}

	void Update(float deltaTime, DebugCamera* debugCamera) {
		Matrix4x4 worldMatrixBlock = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
		Matrix4x4 wvpMatrixBlock = worldMatrixBlock * debugCamera->getViewProjectionMatrix();
		*blockObject_->wvpData = { wvpMatrixBlock, worldMatrixBlock };
		blockObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	void Draw() {
		if (blockObject_) {
			blockObject_->Draw();
		}
	}

	// ---- Transform accessors (needed so a TileMap can place each instance) ----
	Transform* GetTransform() { return &transform_; }
	void SetTransform(const Transform& newTransform) { transform_ = newTransform; }
	void SetPosition(const Vector3& position) { transform_.translation = position; }
	void SetRotation(const Vector3& rotation) { transform_.rotation = rotation; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
};

class SkyDome {
private:
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 defaultScale_{ 1.0f,1.0f, 1.0f };

	Model* blockModel = nullptr;
	std::unique_ptr<Object3D> blockObject_ = nullptr;

public:
	SkyDome() = default;
	~SkyDome() = default;

	// Lightweight path for tile maps / many instances: pass in a Model that is
	// already loaded and shared (e.g. owned by a TileMap). This SkyDome does NOT
	// own the model and will not reload it.
	void Initialize(EngineCommon* engineCommon, Model* sharedModel) {
		blockObject_ = std::make_unique<Object3D>();
		blockObject_->Initialize(engineCommon);
		blockObject_->SetModel(sharedModel);
		transform_.scale = defaultScale_;
	}

	void Update(float deltaTime, DebugCamera* debugCamera) {
		Matrix4x4 worldMatrixBlock = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
		Matrix4x4 wvpMatrixBlock = worldMatrixBlock * debugCamera->getViewProjectionMatrix();
		*blockObject_->wvpData = { wvpMatrixBlock, worldMatrixBlock };
		blockObject_->materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	void Draw() {
		if (blockObject_) {
			blockObject_->Draw();
		}
	}

	// ---- Transform accessors (needed so a TileMap can place each instance) ----
	Transform* GetTransform() { return &transform_; }
	void SetTransform(const Transform& newTransform) { transform_ = newTransform; }
	void SetPosition(const Vector3& position) { transform_.translation = position; }
	void SetRotation(const Vector3& rotation) { transform_.rotation = rotation; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
};