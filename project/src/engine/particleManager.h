#pragma once
#include <vector>
#include <random>
#include "EngineCommon.h"

struct ParticleData {
	Vector3 position;
	Vector3 velocity;
	Vector4 color;
	float lifeTime = 0.0f;
	float currentTime = 0.0f;
	float scale = 1.0f;
};

class ParticleManager {
public:
	static ParticleManager& GetInstance() {
		static ParticleManager instance;
		return instance;
	}

	void Initialize(EngineCommon* engineCommon, uint32_t maxParticles = 256);

	void SetTexture(const std::string& textureFilePath);

	// Spawns up to `count` new particles at `position` using the current editor parameters
	void Emit(const Vector3& position, uint32_t count = 1);

	void Update(float deltaTime, const Matrix4x4& billboardMatrix, const Matrix4x4& viewProjection);

	void Draw();

	// Simple ImGui panel to tweak emission parameters live. Call once per frame between
	// ImGui::NewFrame() and ImGui::Render().
	void DrawImGui();

private:
	ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	struct GPUSlot {
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
		TransformationMatrix* wvpData = nullptr;
	};

	EngineCommon* engineCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvGPUHandle_{};

	std::vector<ParticleData> particles_;
	std::vector<GPUSlot> gpuSlots_;
	uint32_t maxParticles_ = 0;

	std::mt19937 randomEngine_{ std::random_device{}() };

	void CreateQuadMesh();

	// ---------- Editor-controlled emission parameters ----------
	Vector3 spawnPosition_{ 0.0f, 0.0f, 0.0f };
	Vector3 velocityMin_{ -1.0f, -1.0f, -1.0f };
	Vector3 velocityMax_{ 1.0f, 1.0f, 1.0f };
	float lifeTimeMin_ = 1.0f;
	float lifeTimeMax_ = 3.0f;
	float scaleMin_ = 0.5f;
	float scaleMax_ = 1.0f;
	Vector4 startColor_{ 1.0f, 1.0f, 1.0f, 1.0f };

	bool continuousEmit_ = false;
	float emitRate_ = 10.0f;       // particles per second, used when continuousEmit_ is on
	float emitAccumulator_ = 0.0f;
	int emitCountManual_ = 10;     // used by the "Emit Now" button

	char textureFilePathBuf_[256] = "resources/05_02/circle.png";
};