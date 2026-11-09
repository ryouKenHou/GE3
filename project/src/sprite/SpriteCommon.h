#pragma once
#include <string>
#include <d3d12.h>
#include <dxcapi.h>
#include "EngineCommon.h"

class SpriteCommon {
private:
	ID3D12RootSignature* rootSigiture_;
	ID3D12PipelineState* pso_ = nullptr;
	EngineCommon* engineCommon_ = nullptr;

	VertexData* vertexData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	uint32_t indexCount = 0;

	void CreateVertexAndIndexBuffers();
	void CreateRootSignature();
	void CreatePipelineState();
public:
	SpriteCommon();
	~SpriteCommon() = default;
	ID3D12RootSignature* GetRootSignature() const { return rootSigiture_; }
	ID3D12PipelineState* GetPipelineState() const { return pso_; }
	EngineCommon* GetEngineCommon() const { return engineCommon_; }

	D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() { return vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return indexBufferView; }
	uint32_t GetIndexCount() const { return indexCount; }

	void Initialize(EngineCommon* engineCommon);
	void PreDraw();
};