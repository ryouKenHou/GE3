#pragma once
#include "Model.h"

class Object3D {
private:
	Model* model_ = nullptr;
	EngineCommon* engineCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

public:
	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;

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
};