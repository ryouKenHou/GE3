#include "Model.h"

void Model::LoadModel(const std::string& directoryPath, const std::string& filename) {
	modelData = LoadObjFile(directoryPath, filename);

	// == vertex resource for model ==
	vertexResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(*vertexData) * modelData.vertices.size());

	// material resource
	materialResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
	materialData->uvTransform = Matrix4x4::Identity();

	// WVP resource
	wvpResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	//load sencond texture for sprite
	DirectX::ScratchImage mipImages2 = engineCommon_->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	textureResource2 = engineCommon_->CreateTextureResource(engineCommon_->GetDevice(), metadata2);
	intermediateResource2 = engineCommon_->UploadTextureData(textureResource2.Get(), mipImages2, engineCommon_->GetDevice(), engineCommon_->GetCommandList());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = engineCommon_->GetCPUDescriptorHandle(engineCommon_->GetSRVDescriptorHeap(), engineCommon_->GetDescriptorSizeSRV(), 2);
	textureSrvHandleGPU2 = engineCommon_->GetGPUDescriptorHandle(engineCommon_->GetSRVDescriptorHeap(), engineCommon_->GetDescriptorSizeSRV(), 2);
	engineCommon_->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

}

void Model::UsingTemplateModel(int type) {
	switch (type) {
	case 0:
		vertexResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * 4);
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
		vertexBufferViewSprite.BufferLocation = vertexResource->GetGPUVirtualAddress();
		vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
		vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

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

		indexResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(uint32_t) * 6);
		indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData = nullptr;
		indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
		indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

		wvpResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		wvpData = nullptr;
		wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
		*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

		materialResource = engineCommon_->CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
		materialData = nullptr;
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData->enableLighting = false;
		materialData->uvTransform = Matrix4x4::Identity();
		break;
	default:
		assert(false && "Invalid model type");
		break;
	}
}

void Model::Draw() {
	engineCommon_->GetCommandList()->SetGraphicsRootSignature(engineCommon_->GetRootSignature());
	engineCommon_->GetCommandList()->SetPipelineState(engineCommon_->GetPSOManager().GetPSO(PSOType::Opaque3D));
	if (indexResource) {
		engineCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	}
	engineCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	engineCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, engineCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
	if (indexResource) {
		engineCommon_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
	else {
		engineCommon_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
	}
	
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}

	return materialData;
}

ModelData  Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;

			texcoord.y = 1.0f - texcoord.y; // Invert the y-coordinate of the texture coordinate

			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				normal.x *= -1.0f;

				triangle[faceVertex] = { position, texcoord, normal };
			}



			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

