#pragma once
#include "Model.h"

class Object3D {
private:
	Model* model_ = nullptr;
	EngineCommon* engineCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;

	Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
public:
	void Initialize(EngineCommon* engineCommon) {
		engineCommon_ = engineCommon;

		// Each Object3D gets its own WVP constant buffer
		wvpResource_ = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
		*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

		// ...and its own material constant buffer
		materialResource_ = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
		materialData->uvTransform = Matrix4x4::Identity();
	}

	void SetModel(Model* model) {
		model_ = model; // just reference the shared mesh/texture; transform stays per-instance
	}

	void Draw() {
		if (model_) {
			model_->DrawWithTransform(materialResource_->GetGPUVirtualAddress(), wvpResource_->GetGPUVirtualAddress());
		}
	}

	Transform& GetTransform() { return transform_; }
	Transform& GetUVTransform() { return uvTransform_; }

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetUVTransform(const Transform& uvTransform) { uvTransform_ = uvTransform; }

	void UpdateTransforms(Matrix4x4 CameraViewProjectionMatrix) {
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
		Matrix4x4 wvpMatrix = worldMatrix * CameraViewProjectionMatrix;
		*wvpData = { wvpMatrix, worldMatrix };
		materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	Material& GetMaterialData() { return *materialData; }
};