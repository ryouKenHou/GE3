#pragma once

#include "EngineCommon.h"
#include "ModelManager.h"

class Model {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	VertexData* vertexData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	ModelData modelData;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvGPUHandle{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;

	EngineCommon* engineCommon_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	uint32_t indexCount = 0;

	// Shared draw path: binds the mesh (vertex/index buffer + texture) and issues the draw call,
	// using caller-supplied CBV addresses for material/WVP so each instance can have its own transform.
	void DrawInternal(D3D12_GPU_VIRTUAL_ADDRESS materialGPUAddress, D3D12_GPU_VIRTUAL_ADDRESS wvpGPUAddress);

public:
	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;

	void Initialize(EngineCommon* engineCommon);
	void LoadModel(const std::string& directoryPath, const std::string& filename);
	void UsingTemplateModel(int type);

	// Standalone draw: uses this Model's own internal material/WVP buffers (single-instance usage)
	void Draw();

	// Instanced draw: an Object3D (or any owner) supplies its own material/WVP CBV addresses,
	// so multiple instances can share this Model's mesh/texture while keeping separate transforms.
	void DrawWithTransform(D3D12_GPU_VIRTUAL_ADDRESS materialGPUAddress, D3D12_GPU_VIRTUAL_ADDRESS wvpGPUAddress) {
		DrawInternal(materialGPUAddress, wvpGPUAddress);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSRVHandle() const { return textureSrvGPUHandle; }
};