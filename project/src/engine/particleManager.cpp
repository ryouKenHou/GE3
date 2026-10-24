#include "ParticleManager.h"
#include "../../externals/imgui/imgui/imgui.h"

void ParticleManager::Initialize(EngineCommon* engineCommon, uint32_t maxParticles) {
	engineCommon_ = engineCommon;
	maxParticles_ = maxParticles;

	CreateQuadMesh();

	materialResource_ = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	*materialData_ = { {1.0f, 1.0f, 1.0f, 1.0f}, 0 };
	materialData_->uvTransform = Matrix4x4::Identity();

	gpuSlots_.resize(maxParticles_);
	for (auto& slot : gpuSlots_) {
		slot.wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		slot.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&slot.wvpData));
		*slot.wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };
	}

	particles_.reserve(maxParticles_);
}

void ParticleManager::CreateQuadMesh() {
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
	std::uniform_real_distribution<float> distVelX(velocityMin_.x, velocityMax_.x);
	std::uniform_real_distribution<float> distVelY(velocityMin_.y, velocityMax_.y);
	std::uniform_real_distribution<float> distVelZ(velocityMin_.z, velocityMax_.z);
	std::uniform_real_distribution<float> distLife(lifeTimeMin_, lifeTimeMax_);
	std::uniform_real_distribution<float> distScale(scaleMin_, scaleMax_);

	for (uint32_t i = 0; i < count; ++i) {
		if (particles_.size() >= maxParticles_) {
			break;
		}

		ParticleData p{};
		p.position = position;
		p.velocity = { distVelX(randomEngine_), distVelY(randomEngine_), distVelZ(randomEngine_) };
		p.color = startColor_;
		p.lifeTime = distLife(randomEngine_);
		p.currentTime = 0.0f;
		p.scale = distScale(randomEngine_);

		particles_.push_back(p);
	}
}

void ParticleManager::Update(float deltaTime, const Matrix4x4& billboardMatrix, const Matrix4x4& viewProjection) {
	// Continuous emission driven by the editor's emitRate_
	if (continuousEmit_) {
		emitAccumulator_ += emitRate_ * deltaTime;
		uint32_t toEmit = static_cast<uint32_t>(emitAccumulator_);
		if (toEmit > 0) {
			Emit(spawnPosition_, toEmit);
			emitAccumulator_ -= static_cast<float>(toEmit);
		}
	}

	for (size_t i = 0; i < particles_.size(); /* no auto-increment */) {
		ParticleData& p = particles_[i];
		p.currentTime += deltaTime;

		if (p.currentTime >= p.lifeTime) {
			p = particles_.back();
			particles_.pop_back();
			continue;
		}

		p.position.x += p.velocity.x * deltaTime;
		p.position.y += p.velocity.y * deltaTime;
		p.position.z += p.velocity.z * deltaTime;

		float alpha = 1.0f - (p.currentTime / p.lifeTime);
		p.color.w = startColor_.w * alpha;

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

void ParticleManager::DrawImGui() {
	ImGui::Begin("Particle Editor");

	ImGui::Text("Alive: %zu / %u", particles_.size(), maxParticles_);
	ImGui::Separator();

	ImGui::DragFloat3("Spawn Position", &spawnPosition_.x, 0.1f);

	ImGui::Text("Velocity Range");
	ImGui::DragFloat3("Velocity Min", &velocityMin_.x, 0.05f);
	ImGui::DragFloat3("Velocity Max", &velocityMax_.x, 0.05f);

	ImGui::DragFloat("Life Time Min", &lifeTimeMin_, 0.05f, 0.01f, 60.0f);
	ImGui::DragFloat("Life Time Max", &lifeTimeMax_, 0.05f, lifeTimeMin_, 60.0f);

	ImGui::DragFloat("Scale Min", &scaleMin_, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("Scale Max", &scaleMax_, 0.01f, scaleMin_, 10.0f);

	ImGui::ColorEdit4("Start Color", &startColor_.x);

	ImGui::Separator();
	ImGui::Checkbox("Continuous Emit", &continuousEmit_);
	ImGui::DragFloat("Emit Rate (per sec)", &emitRate_, 0.5f, 0.0f, 1000.0f);

	ImGui::Separator();
	ImGui::SliderInt("Manual Emit Count", &emitCountManual_, 1, 100);
	if (ImGui::Button("Emit Now")) {
		Emit(spawnPosition_, static_cast<uint32_t>(emitCountManual_));
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear All")) {
		particles_.clear();
	}

	ImGui::Separator();
	ImGui::InputText("Texture Path", textureFilePathBuf_, sizeof(textureFilePathBuf_));
	if (ImGui::Button("Load Texture")) {
		SetTexture(textureFilePathBuf_);
	}

	ImGui::End();
}