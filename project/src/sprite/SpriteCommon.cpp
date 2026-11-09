#include "SpriteCommon.h"

SpriteCommon::SpriteCommon() : rootSigiture_(nullptr), pso_(nullptr), engineCommon_(nullptr) {}

void SpriteCommon::Initialize(EngineCommon* engineCommon) {
	engineCommon_ = engineCommon;
	CreateRootSignature();
	CreatePipelineState();
	CreateVertexAndIndexBuffers();
}

void SpriteCommon::PreDraw() {
	engineCommon_->GetCommandList()->SetGraphicsRootSignature(rootSigiture_);
	engineCommon_->GetCommandList()->SetPipelineState(pso_);
}

void SpriteCommon::CreateRootSignature() {
	rootSigiture_ = engineCommon_->GetRootSignature();
}

void SpriteCommon::CreatePipelineState() {	
	pso_ = engineCommon_->GetPSOManager().GetPSO(PSOType::Transparent3D);
}

void SpriteCommon::CreateVertexAndIndexBuffers() {
	vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * 4);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f, 1.f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[2].position = { 1.f,1.f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[3].position = { 1.f, 0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	indexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(uint32_t) * 6);
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	indexCount = 6;
}

