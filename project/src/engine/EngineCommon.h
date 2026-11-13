#pragma once

#include "WindowManager.h"
#include "Audio.h"
#include "InputSystem.h"
#include "SpriteCommon.h"
#include "particleManager.h"
#include "Renderer.h"



class EngineCommon {
public:
	EngineCommon() = default;
	~EngineCommon() = default;

	// ========================= Structs ========================
	struct D3DresourceLeakChecker {
		~D3DresourceLeakChecker() {
			Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		}
	};

	// ================= Function Declarations =================
	void Initialize(int32_t Width, int32_t Height, const wchar_t* title);

	void PreDraw() {renderer_.PreDraw();}
	void PostDraw() {renderer_.PostDraw();}
	void Finalize();

	// ================= Audio Function Declarations =================
	bool LoadAudio(const std::string& soundName, const std::string& filePath) { return audioSystem_.LoadAudio(soundName, filePath); }
	void SoundUnload(const std::string& soundName) { audioSystem_.UnloadSound(soundName); }
	void PlayAudio(const std::string& soundName, bool loop = false, float volume = 1.0f) { audioSystem_.PlayAudio(soundName, loop, volume); }

	// ================== getters ==================
	InputSystem& GetInputSystem() { return inputSystem_; }
	AudioSystem& GetAudioSystem() { return audioSystem_; }
	SpriteCommon& GetSpriteCommon() { return spriteCommon_; }
	TextureManager& GetTextureManager() { return GetRenderer().GetTextureManager(); }
	ParticleManager& GetParticleManager() { return particleManager_; }
	Renderer& GetRenderer() { return renderer_; }

	ID3D12Device* GetDevice() { return  GetRenderer().GetDevice(); }
	ID3D12GraphicsCommandList* GetCommandList() { return GetRenderer().GetCommandList(); }
	DescriptorAllocator& GetSRVAllocator() { return GetRenderer().GetSRVAllocator(); }
	uint32_t GetDescriptorSizeSRV() { return GetRenderer().GetDescriptorSizeSRV(); }
	ID3D12RootSignature* GetRootSignature() { return GetRenderer().GetRootSignature(); }
	ID3D12Resource* GetDirectionalLightResource() { return GetRenderer().GetDirectionalLightResource(); }
	PSOManager& GetPSOManager() { return GetRenderer().GetPSOManager(); }
	WindowManager& GetWindowManager() { return windowManager; }
	DirectionalLight* GetDirectionalLightData() { return GetRenderer().GetDirectionalLightData(); }

private:
	D3DresourceLeakChecker leakChecker;

	InputSystem inputSystem_;
	AudioSystem audioSystem_;
	SpriteCommon spriteCommon_;
	WindowManager windowManager;
	Renderer renderer_;
	ParticleManager particleManager_;
	
};

