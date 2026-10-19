#include "SpriteCommon.h"

SpriteCommon::SpriteCommon() : rootSigiture_(nullptr), pso_(nullptr), engineCommon_(nullptr) {}
SpriteCommon::~SpriteCommon() {
	if (rootSigiture_) {
		rootSigiture_->Release();
		rootSigiture_ = nullptr;
	}
}

void SpriteCommon::Initialize(EngineCommon* engineCommon) {
	engineCommon_ = engineCommon;
	CreateRootSignature();
	CreatePipelineState();
}

void SpriteCommon::PreDraw() {
	engineCommon_->GetCommandList()->SetGraphicsRootSignature(rootSigiture_);
	engineCommon_->GetCommandList()->SetPipelineState(pso_);
}

void SpriteCommon::CreateRootSignature() {
}

void SpriteCommon::CreatePipelineState() {	
	pso_ = engineCommon_->GetPSOManager().GetPSO(PSOType::Transparent3D);
}

