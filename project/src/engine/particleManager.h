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

	// maxParticles pre-allocates a pool of constant buffers so Emit() never creates GPU resources at runtime
	void Initialize(EngineCommon* engineCommon, uint32_t maxParticles = 256);

	void SetTexture(const std::string& textureFilePath);

	// Spawns up to `count` new particles at `position` (ignored if the pool is full)
	void Emit(const Vector3& position, uint32_t count = 1);

	// Advances simulation and refreshes each alive particle's WVP constant buffer.
	// viewProjection should be camera view * projection (billboard rotation uses billboardMatrix).
	void Update(float deltaTime, const Matrix4x4& billboardMatrix, const Matrix4x4& viewProjection);

	void Draw();

private:
	ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	struct GPUSlot {
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
		TransformationMatrix* wvpData = nullptr;
	};

	EngineCommon* engineCommon_ = nullptr;

	// Shared geometry: a single camera-facing quad reused by every particle
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvGPUHandle_{};

	std::vector<ParticleData> particles_;
	std::vector<GPUSlot> gpuSlots_; // pool, size == maxParticles_
	uint32_t maxParticles_ = 0;

	std::mt19937 randomEngine_{ std::random_device{}() };

	void CreateQuadMesh();
};