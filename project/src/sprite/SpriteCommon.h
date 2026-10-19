#pragma once
#include <string>
#include <d3d12.h>
#include <dxcapi.h>
#include "EngineCommon.h"

class SpriteCommon {
private:
	ID3D12RootSignature* rootSigiture_;
	ID3D12PipelineState* pso_;
	EngineCommon* engineCommon_ = nullptr;

	void CreateRootSignature();
	void CreatePipelineState();
public:
	SpriteCommon();
	~SpriteCommon();
	ID3D12RootSignature* GetRootSignature() const { return rootSigiture_; }
	ID3D12PipelineState* GetPipelineState() const { return pso_; }
	EngineCommon* GetEngineCommon() const { return engineCommon_; }

	void Initialize(EngineCommon* engineCommon);
	void PreDraw();
};