#pragma once
#include <cstdint>

#include <format>
#include <vector>
#include <string>
#include <sstream>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "ConvertString.hpp"
#include "Matrix4x4.hpp"
#include "DebugCamera.h"
#include "EngineHelper.h"

#ifdef _DEBUG
#include "../../externals/imgui/imgui/imgui.h"
#include "../../externals/imgui/imgui/imgui_impl_win32.h"
#include "../../externals/imgui/imgui/imgui_impl_dx12.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#endif

#include "LogSystem.h"
#include "PsoManager.h"
#include "DescriptorAllocator.h"
#include "TextureManager.h"

#include <chrono>
#include <thread>

class WindowManager;

class Renderer {
public:
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	Renderer() = default;
	~Renderer() = default;

	void CreateDefaultPSO();

	void Initialize(WindowManager* windowManager);
	void InitializeFixFPS();
	void UpdateFixFPS();


	void PreDraw();
	void PostDraw();
	void Finalize();

	void InitializeDevice();
	void InitializeCommand();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateDepthBuffer();
	void InitializeRenderTargetViews();
	void InitializeDepthStencilView();
	void InitializeFence();
	void InitializeViewport();
	void InitializeScissorRect();
	void InitializeLight();
#ifdef _DEBUG
	void InitializeImGui();
#endif

	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

	DescriptorHandle LoadTexture(const std::string& filePath);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	ID3D12Device* GetDevice() { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
	DescriptorAllocator& GetSRVAllocator() { return srvAllocator_; }
	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	ID3D12RootSignature* GetRootSignature(rootSignatureType type = rootSignatureType::Default) { return psoManager_.GetRootSignature(type); }
	ID3D12Resource* GetDirectionalLightResource() { return directionalLightResource.Get(); }
	PSOManager& GetPSOManager() { return psoManager_; }
	TextureManager& GetTextureManager() { return textureManager_; }
	DirectionalLight* GetDirectionalLightData() { return directionalLightData; }

private:
	std::chrono::steady_clock::time_point reference_;
	DirectionalLight* directionalLightData;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	
	DescriptorAllocator rtvAllocator_; // 1. RTV Allocator (Non-Shader Visible, ~32 slots for swapchain & render targets)
	DescriptorAllocator dsvAllocator_; // 2. DSV Allocator (Non-Shader Visible, ~16 slots for depth buffers & shadow maps)
	DescriptorAllocator srvAllocator_; // 3. SRV/CBV/UAV Allocator (Shader Visible, ~2048+ slots for textures, materials, etc.)
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainTargets[2];

	D3D12_RESOURCE_BARRIER barrier;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;
	HANDLE fenceEvent;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;	
	
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	WindowManager* windowManager_ = nullptr;
	uint32_t clientWidth;
	uint32_t clientHeight;

	TextureManager textureManager_;
	PSOManager psoManager_;
};