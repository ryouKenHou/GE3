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

    Transform transform_{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    Transform uvTransform_{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    // --- DIRTY FLAGS ---
    bool isTransformDirty_ = true;
    bool isUVTransformDirty_ = true;

public:
    void Initialize(EngineCommon* engineCommon, SpriteCommon* spriteCommon);
    void Draw();
    void SetTexture(const std::string& textureFilePath);

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle() const { return textureSrvGPUHandle; }
    void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvGPUHandle = handle; }

    // --- READ-ONLY GETTERS ---
    const Transform& GetTransform() const { return transform_; }
    const Transform& GetUVTransform() const { return uvTransform_; }

    // --- SETTERS (Automatically trigger the dirty flag) ---
    void SetTransform(const Transform& transform) {
        transform_ = transform;
        isTransformDirty_ = true;
    }

    void SetPosition(const Vector3& pos) {
        transform_.translation = pos;
        isTransformDirty_ = true;
    }

    void SetRotation(const Vector3& rot) {
        transform_.rotation = rot;
        isTransformDirty_ = true;
    }

    void SetScale(const Vector3& scale) {
        transform_.scale = scale;
        isTransformDirty_ = true;
    }

    void SetUVTransform(const Transform& uvTransform) {
        uvTransform_ = uvTransform;
        isUVTransformDirty_ = true;
    }

    // --- SMART UPDATE METHOD ---
    void UpdateTransforms(Matrix4x4 orthoProjection) {
        // If neither transform nor UV transform changed, skip everything!
        if (!isTransformDirty_ && !isUVTransformDirty_) {
            return;
        }

        // Recalculate WVP only if transform changed
        if (isTransformDirty_) {
            Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translation);
            Matrix4x4 wvpMatrix = worldMatrix * orthoProjection;
            *wvpData = { wvpMatrix, worldMatrix };
            isTransformDirty_ = false; // Clear the flag
        }

        // Recalculate UV transform only if it changed
        if (isUVTransformDirty_) {
            materialData->uvTransform = Matrix4x4::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotation, uvTransform_.translation);
            isUVTransformDirty_ = false; // Clear the flag
        }
    }

    Material& GetMaterialData() { return *materialData; }
};