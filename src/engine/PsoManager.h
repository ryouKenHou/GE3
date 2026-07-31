#pragma once
#include <unordered_map>
#include <string>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <assert.h>

#include "LogSystem.h"

class PSOBuilder {
public:
    PSOBuilder() {
        // Set standard 3D rendering defaults

        D3D12_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        desc_.BlendState = blendDesc;

        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        desc_.RasterizerState = rasterizerDesc;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        desc_.DepthStencilState = depthStencilDesc;

        desc_.SampleMask = UINT_MAX;
        desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc_.NumRenderTargets = 1;
        desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc_.SampleDesc.Count = 1;
    }

    PSOBuilder& SetRootSignature(ID3D12RootSignature* rootSig) {
        desc_.pRootSignature = rootSig;
        return *this;
    }

    PSOBuilder& SetShaders(IDxcBlob* vs, IDxcBlob* ps) {
        if (vs) desc_.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
        if (ps) desc_.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
        return *this;
    }

    PSOBuilder& SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* elements, UINT count) {
        desc_.InputLayout = { elements, count };
        return *this;
    }

    // Preset Helper: Enable Alpha Blending (for UI / Sprites / Transparency)
    PSOBuilder& EnableAlphaBlending() {
        D3D12_RENDER_TARGET_BLEND_DESC& rt = desc_.BlendState.RenderTarget[0];
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D12_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D12_BLEND_ONE;
        rt.DestBlendAlpha = D3D12_BLEND_ZERO;
        rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        return *this;
    }

    // Preset Helper: Wireframe Rendering
    PSOBuilder& SetWireframe() {
        desc_.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
        return *this;
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> Build(ID3D12Device* device) {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
        HRESULT hr = device->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso));
        return SUCCEEDED(hr) ? pso : nullptr;
    }

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
};



// Known pipeline types in your engine
enum class PSOType {
    Opaque3D,
    Transparent3D,
    Wireframe,
    Sprite2D,
    UI
};

class PSOManager {
public:
    PSOManager() = default;
    ~PSOManager() {
        pixelShaderBlob->Release();
        vertexShaderBlob->Release();
    }

    void Initialize(ID3D12Device* device, ID3D12RootSignature* defaultRootSignature);

    // Fetch a PSO for drawing
    ID3D12PipelineState* GetPSO(PSOType type) const;
    ID3D12PipelineState* GetPSO(const std::string& customName) const;

private:
    std::unordered_map<PSOType, Microsoft::WRL::ComPtr<ID3D12PipelineState>> enumPipelines_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> customPipelines_;

    IDxcBlob* CompileShader(
        const std::wstring& filePath,
        const wchar_t* profile,
        IDxcUtils* dxcUtils,
        IDxcCompiler3* dxcCompiler,
        IDxcIncludeHandler* includeHandler
    );


    IDxcBlob* vertexShaderBlob;
    IDxcBlob* pixelShaderBlob;
};