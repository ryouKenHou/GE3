#include "Model.h"

void Model::Initialize(EngineCommon* engineCommon) {
	assert(engineCommon != nullptr);
	engineCommon_ = engineCommon;
}

void Model::LoadModel(const std::string& directoryPath, const std::string& filename) {
	modelData = ModelManager::GetInstance().Load(directoryPath, filename);

	// == vertex resource for model ==
	vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(*vertexData) * modelData.vertices.size());

	// material resource
	materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
	materialData->uvTransform = Matrix4x4::Identity();

	// WVP resource
	wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	//load sencond texture for sprite
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		modelData.material.textureFilePath,
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator()
	);

	// Store ONLY the GPU handle needed for drawing
	textureSrvGPUHandle = texHandle.gpuHandle;
}

void Model::UsingTemplateModel(int type) {
	switch (type) {
	case 1: {
		// == vertex resource for sphere ==
		const uint32_t kSubdivisoin = 20;
		const uint32_t kVertexCount = kSubdivisoin * kSubdivisoin * 4;
		const uint32_t kIndexCount = kSubdivisoin * kSubdivisoin * 6;

		vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * kVertexCount);

		// vertex buffer view
		vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeof(VertexData) * kVertexCount;
		vertexBufferView.StrideInBytes = sizeof(VertexData);

		// index buffer view
		indexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(uint32_t) * kIndexCount);
		indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = sizeof(uint32_t) * kIndexCount;
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData = nullptr;
		indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

		// copy vertex data
		vertexData = nullptr;
		vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

		const float pi = 3.14159265358979323846f;
		const float kLonEvery = (pi * 2 / kSubdivisoin);
		const float kLatEvery = (pi / kSubdivisoin);

		for (uint32_t latIndex = 0; latIndex < kSubdivisoin; ++latIndex) {
			float lat = -pi / 2.0f + latIndex * kLatEvery;
			for (uint32_t lonIndex = 0; lonIndex < kSubdivisoin; ++lonIndex) {
				float lon = lonIndex * kLonEvery;
				uint32_t start = (latIndex * kSubdivisoin + lonIndex) * 4;
				uint32_t indexStart = (latIndex * kSubdivisoin + lonIndex) * 6;

				vertexData[start + 0].position = { cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon), 1.0f };
				vertexData[start + 0].texcoord = { lon / (2 * pi), 1.0f - (lat + pi / 2) / pi };
				vertexData[start + 0].normal = { vertexData[start + 0].position.x, vertexData[start + 0].position.y, vertexData[start + 0].position.z };

				vertexData[start + 1].position = { cosf(lat + kLatEvery) * cosf(lon), sinf(lat + kLatEvery), cosf(lat + kLatEvery) * sinf(lon), 1.0f };
				vertexData[start + 1].texcoord = { lon / (2 * pi), 1.0f - (lat + kLatEvery + pi / 2) / pi };
				vertexData[start + 1].normal = { vertexData[start + 1].position.x, vertexData[start + 1].position.y, vertexData[start + 1].position.z };

				vertexData[start + 2].position = { cosf(lat) * cosf(lon + kLonEvery), sinf(lat), cosf(lat) * sinf(lon + kLonEvery), 1.0f };
				vertexData[start + 2].texcoord = { (lon + kLonEvery) / (2 * pi), 1.0f - (lat + pi / 2) / pi };
				vertexData[start + 2].normal = { vertexData[start + 2].position.x, vertexData[start + 2].position.y, vertexData[start + 2].position.z };

				vertexData[start + 3].position = { cosf(lat + kLatEvery) * cosf(lon + kLonEvery), sinf(lat + kLatEvery), cosf(lat + kLatEvery) * sinf(lon + kLonEvery), 1.0f };
				vertexData[start + 3].texcoord = { (lon + kLonEvery) / (2 * pi), 1.0f - (lat + kLatEvery + pi / 2) / pi };
				vertexData[start + 3].normal = { vertexData[start + 3].position.x, vertexData[start + 3].position.y, vertexData[start + 3].position.z };

				indexData[indexStart + 0] = start + 0; indexData[indexStart + 1] = start + 1; indexData[indexStart + 2] = start + 2;
				indexData[indexStart + 3] = start + 1; indexData[indexStart + 4] = start + 3; indexData[indexStart + 5] = start + 2;
			}
		}

		// material resource
		materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
		materialData = nullptr;
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
		materialData->uvTransform = Matrix4x4::Identity();

		// WVP resource
		wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		wvpData = nullptr;
		wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
		*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

		indexCount = kIndexCount;
		break;
	}
	default:
		assert(false && "Invalid model type");
		break;
	}
	//load sencond texture for sprite
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		"resources/05_02/uvChecker.png",
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator() // You will need to add this getter to EngineCommon
	);

	// Store ONLY the GPU handle needed for drawing
	textureSrvGPUHandle = texHandle.gpuHandle;
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
	engineCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvGPUHandle);
	if (indexResource) {
		engineCommon_->GetCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}
	else {
		engineCommon_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
	}
	
}
