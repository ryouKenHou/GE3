#pragma once
#include <vector>
#include <random>
#include "EngineHelper.h"

class EngineCommon;

struct ParticleData {
	Vector3 position;
	Vector3 velocity;
	Vector4 color;
	float lifeTime = 0.0f;
	float currentTime = 0.0f;
	float scale = 1.0f;
};

// ---------- Field system ----------
struct AABB {
	Vector3 min{ -1.0f, -1.0f, -1.0f };
	Vector3 max{ 1.0f, 1.0f, 1.0f };

	bool Contains(const Vector3& p) const {
		return (p.x >= min.x && p.x <= max.x) &&
			(p.y >= min.y && p.y <= max.y) &&
			(p.z >= min.z && p.z <= max.z);
	}
};

enum class FieldType {
	Gravity,      // Constant pull toward -direction (or a fixed vector), ignores particle direction
	Acceleration, // Adds acceleration vector * deltaTime to velocity every frame while inside the area
};

struct ParticleField {
	FieldType type = FieldType::Gravity;
	AABB area{};
	Vector3 direction{ 0.0f, -1.0f, 0.0f }; // normalized direction the force pushes toward
	float strength = 9.8f;                   // magnitude of the force / acceleration
	bool enabled = true;
};

class ParticleManager {
public:
	//static ParticleManager& GetInstance() {
	//	static ParticleManager instance;
	//	return instance;
	//}
	ParticleManager() = default;
	~ParticleManager() = default;

	void Initialize(EngineCommon* engineCommon, uint32_t maxParticles = 256);

	void SetTexture(const std::string& textureFilePath);

	// Spawns up to `count` new particles at `position` using the current editor parameters
	void Emit(const Vector3& position, uint32_t count = 1);

	void Update(float deltaTime, const Matrix4x4& billboardMatrix, const Matrix4x4& viewProjection);

	void Draw();

	// Simple ImGui panel to tweak emission parameters live. Call once per frame between
	// ImGui::NewFrame() and ImGui::Render().
	void DrawImGui();

	// ---------- Field management ----------
	// Adds a field and returns its index (use with RemoveField / GetField).
	size_t AddField(const ParticleField& field);
	void RemoveField(size_t index);
	void ClearFields();
	ParticleField* GetField(size_t index);
	size_t GetFieldCount() const { return fields_.size(); }

private:
	//ParticleManager() = default;
	//ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	EngineCommon* engineCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvGPUHandle_{};

	// ---------- Instancing GPU resources ----------
	// Per-instance TransformationMatrix (WVP/World), indexed by SV_InstanceID in the VS.
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingTransformResource_;
	TransformationMatrix* instancingTransformData_ = nullptr;

	// Per-instance color (fade alpha etc.), indexed by SV_InstanceID in the VS.
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingColorResource_;
	Vector4* instancingColorData_ = nullptr;

	// Shared material (uvTransform / enableLighting / base tint color), one CBV for all particles.
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	std::vector<ParticleData> particles_;
	uint32_t maxParticles_ = 0;

	std::vector<ParticleField> fields_;

	std::mt19937 randomEngine_{ std::random_device{}() };

	bool useBillboardMatrix_ = true;

	void CreateQuadMesh();
	void ApplyFields(ParticleData& p, float deltaTime);

	// ---------- Editor-controlled emission parameters ----------
	Vector3 spawnPosition_{ 0.0f, 0.0f, 0.0f };
	Vector3 velocityMin_{ -1.0f, -1.0f, -1.0f };
	Vector3 velocityMax_{ 1.0f, 1.0f, 1.0f };
	float lifeTimeMin_ = 1.0f;
	float lifeTimeMax_ = 3.0f;
	float scaleMin_ = 0.05f;
	float scaleMax_ = 0.2f;
	Vector4 startColor_{ 1.0f, 1.0f, 1.0f, 1.0f };

	bool continuousEmit_ = false;
	float emitRate_ = 10.0f;       // particles per second, used when continuousEmit_ is on
	float emitAccumulator_ = 0.0f;
	int emitCountManual_ = 20;     // used by the "Emit Now" button

	char textureFilePathBuf_[256] = "resources/05_02/circle.png";

	// ---------- Field editor state ----------
	int editingFieldIndex_ = -1;
};