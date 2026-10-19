#include "ParticleManager.h"

void ParticleManager::Initialize(EngineCommon* engineCommon, uint32_t maxParticles) {
	engineCommon_ = engineCommon;
	maxParticles_ = maxParticles;

	CreateQuadMesh();

	// material (shared by all particles)
	materialResource_ = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	*materialData_ = { {1.0f, 1.0f, 1.0f, 1.0f}, 0 }; // lighting off for particles by default
	materialData_->uvTransform = Matrix4x4::Identity();

	// pre-allocate the WVP constant buffer pool
	gpuSlots_.resize(maxParticles_);
	for (auto& slot : gpuSlots_) {
		slot.wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		slot.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&slot.wvpData));
		*slot.wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };
	}

	particles_.reserve(maxParticles_);
}

void ParticleManager::CreateQuadMesh() {
	// Simple unit quad centered on origin, facing +Z (billboarded via the world matrix at draw time)
	VertexData quad[6] = {
		{ {-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ { 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },

		{ {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
		{ { 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ { 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
	};

	vertexResource_ = CreateBufferResource(engineCommon_->GetDevice(), sizeof(quad));

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(quad);
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* mapped = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	std::memcpy(mapped, quad, sizeof(quad));
	vertexResource_->Unmap(0, nullptr);
}

void ParticleManager::SetTexture(const std::string& textureFilePath) {
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		textureFilePath,
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator()
	);
	textureSrvGPUHandle_ = texHandle.gpuHandle;
}

void ParticleManager::Emit(const Vector3& position, uint32_t count) {
	std::uniform_real_distribution<float> distVel(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distLife(1.0f, 3.0f);

	for (uint32_t i = 0; i < count; ++i) {
		if (particles_.size() >= maxParticles_) {
			break; // pool full, drop the spawn request
		}

		ParticleData p{};
		p.position = position;
		p.velocity = { distVel(randomEngine_), distVel(randomEngine_), distVel(randomEngine_) };
		p.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		p.lifeTime = distLife(randomEngine_);
		p.currentTime = 0.0f;
		p.scale = 1.0f;

		particles_.push_back(p);
	}
}

void ParticleManager::Update(float deltaTime, const Matrix4x4& billboardMatrix, const Matrix4x4& viewProjection) {
	for (size_t i = 0; i < particles_.size(); /* no auto-increment */) {
		ParticleData& p = particles_[i];
		p.currentTime += deltaTime;

		if (p.currentTime >= p.lifeTime) {
			// swap-and-pop dead particle
			p = particles_.back();
			particles_.pop_back();
			continue;
		}

		p.position.x += p.velocity.x * deltaTime;
		p.position.y += p.velocity.y * deltaTime;
		p.position.z += p.velocity.z * deltaTime;

		// fade out over lifetime
		float alpha = 1.0f - (p.currentTime / p.lifeTime);
		p.color.w = alpha;

		Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix({ p.scale, p.scale, p.scale });
		Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(p.position);
		Matrix4x4 world = scaleMatrix * billboardMatrix * translateMatrix;
		Matrix4x4 wvp = world * viewProjection;

		gpuSlots_[i].wvpData->World = world;
		gpuSlots_[i].wvpData->WVP = wvp;

		++i;
	}
}

void ParticleManager::Draw() {
	if (particles_.empty()) {
		return;
	}

	auto* cmdList = engineCommon_->GetCommandList();
	cmdList->SetGraphicsRootSignature(engineCommon_->GetRootSignature());
	cmdList->SetPipelineState(engineCommon_->GetPSOManager().GetPSO(PSOType::Transparent3D));

	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, textureSrvGPUHandle_);
	cmdList->SetGraphicsRootConstantBufferView(3, engineCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());

	for (size_t i = 0; i < particles_.size(); ++i) {
		cmdList->SetGraphicsRootConstantBufferView(1, gpuSlots_[i].wvpResource->GetGPUVirtualAddress());
		cmdList->DrawInstanced(6, 1, 0, 0);
	}
}