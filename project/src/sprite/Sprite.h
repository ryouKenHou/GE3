#pragma once

#include "SpriteCommon.h"

class Sprite {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;	

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvGPUHandle{};	

	EngineCommon* engineCommon_ = nullptr;	
	SpriteCommon* spriteCommon_ = nullptr;

	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;
	Transform transform_ { { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };
	Transform uvTransform_ { { 1.0f, 1.0f, 1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };
public:
	void Initialize(EngineCommon* engineCommon, SpriteCommon* spriteCommon);
	void Draw();
	void SetTexture(const std::string& textureFilePath);

	// Lets multiple Sprites share one already-loaded texture (e.g. a digit
	// sprite sheet used by several score-digit sprites) without reloading it.
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() const { return textureSrvGPUHandle; }
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvGPUHandle = handle; }


	Transform& GetTransform() { return transform_; }
	Transform& GetUVTransform() { return uvTransform_; }

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetUVTransform(const Transform& uvTransform) { uvTransform_ = uvTransform; }

	void UpdateTransforms(Matrix4x4 orthoProjection) {
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
		Matrix4x4 wvpMatrix = worldMatrix * orthoProjection;
		*wvpData = { wvpMatrix, worldMatrix };
		materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
	}

	Material& GetMaterialData() { return *materialData; }
};