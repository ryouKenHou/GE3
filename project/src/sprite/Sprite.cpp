#include "Sprite.h"

void Sprite::Initialize(EngineCommon* engineCommon) {
	engineCommon_ = engineCommon;

	vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * 4);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0].position = { 0.0f, 360.f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[2].position = { 640.f, 360.f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[3].position = { 640.f, 0.0f, 0.0f, 1.0f };
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

	wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpData = nullptr;
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = Matrix4x4::Identity();

	indexCount = 6;

	//load sencond texture for sprite
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		"resources/05_02/uvChecker.png",
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator() // You will need to add this getter to EngineCommon
	);
	textureSrvGPUHandle = texHandle.gpuHandle;
}

void Sprite::SetTexture(const std::string& textureFilePath) {
	// Load the new texture and get its descriptor handle
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		textureFilePath,
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator()
	);
	// Update the GPU handle for the new texture
	textureSrvGPUHandle = texHandle.gpuHandle;
}

void Sprite::Draw() {
	auto commandList = engineCommon_->GetCommandList();
	engineCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);	
	engineCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	engineCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, engineCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvGPUHandle);
	engineCommon_->GetCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}
