#pragma once
#include "Model.h"

class Object3D {
private:
	Model* model_ = nullptr;
	EngineCommon* engineCommon_ = nullptr;

public:
	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;

	void Initialize(EngineCommon* engineCommon) {
		engineCommon_ = engineCommon;

	}

	void SetModel(Model* model) {
		model_ = model;
		wvpData = model_->wvpData;
		materialData = model_->materialData;
	}

	void Draw() {
		if (model_) {
			model_->Draw();
		}
	}
};