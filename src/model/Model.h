#pragma once

#include "EngineCommon.h"



class Model {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;	
	VertexData* vertexData = nullptr;
	
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	ModelData modelData;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;

	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	EngineCommon* engineCommon_ = &EngineCommon::GetInstance();

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
public:
	TransformationMatrix* wvpData = nullptr;
	Material* materialData = nullptr;

	void LoadModel(const std::string& directoryPath, const std::string& filename);
	void UsingTemplateModel(int type);

	void Draw();
};