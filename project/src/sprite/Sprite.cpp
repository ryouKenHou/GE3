#include "Sprite.h"
#include "EngineCommon.h"

void Sprite::Initialize(EngineCommon* engineCommon, SpriteCommon* spriteCommon) {
	engineCommon_ = engineCommon;
	spriteCommon_ = spriteCommon;

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

	//load sencond texture for sprite
	DescriptorHandle texHandle = engineCommon_->GetTextureManager().Load(
		"resources/05_02/uvChecker.png",
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator() 
	);
	textureSrvGPUHandle = texHandle.gpuHandle;
}

void Sprite::SetTexture(const std::string& textureFilePath) {
	// Load the new texture and get its descriptor handle
	DescriptorHandle texHandle = engineCommon_->GetTextureManager().Load(
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
	engineCommon_->GetCommandList()->IASetIndexBuffer(&spriteCommon_->GetIndexBufferView());	
	engineCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &spriteCommon_->GetVertexBufferView());
	engineCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, engineCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvGPUHandle);
	engineCommon_->GetCommandList()->DrawIndexedInstanced(spriteCommon_->GetIndexCount(), 1, 0, 0, 0);
}
