#include "PSOManager.h"

void PSOManager::Initialize(ID3D12Device* device, ID3D12RootSignature* defaultRootSig) {
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    IDxcUtils* dxcUtils = nullptr;
    IDxcCompiler3* dxcCompiler = nullptr;
    HREFTYPE hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    IDxcIncludeHandler* includeHandler = nullptr;
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));
    vertexShaderBlob = CompileShader(L"shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);

    pixelShaderBlob = CompileShader(L"shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob != nullptr);


    // 1. Create Default 3D Opaque PSO
    enumPipelines_[PSOType::Opaque3D] = PSOBuilder()
        .SetRootSignature(defaultRootSig)
        .SetShaders(vertexShaderBlob, pixelShaderBlob) // Pass compiled shader blobs
        .SetInputLayout(inputElementDescs, _countof(inputElementDescs))
        .Build(device);

    // 2. Create Wireframe PSO (Reuses same shaders/root signature, just tweaks rasterizer)
    enumPipelines_[PSOType::Wireframe] = PSOBuilder()
        .SetRootSignature(defaultRootSig)
        .SetShaders(vertexShaderBlob, pixelShaderBlob)
        .SetInputLayout(inputElementDescs, _countof(inputElementDescs))
        .SetWireframe()
        .Build(device);

    // 3. Create Transparent 3D PSO
    enumPipelines_[PSOType::Transparent3D] = PSOBuilder()
        .SetRootSignature(defaultRootSig)
        .SetShaders(vertexShaderBlob, pixelShaderBlob)
        .SetInputLayout(inputElementDescs, _countof(inputElementDescs))
        .EnableAlphaBlending()
        .Build(device);
}

ID3D12PipelineState* PSOManager::GetPSO(PSOType type) const {
    auto it = enumPipelines_.find(type);
    return (it != enumPipelines_.end()) ? it->second.Get() : nullptr;
}

IDxcBlob* PSOManager::CompileShader(
    const std::wstring& filePath,
    const wchar_t* profile,
    IDxcUtils* dxcUtils,
    IDxcCompiler3* dxcCompiler,
    IDxcIncludeHandler* includeHandler
) {
    // read file
    Log::LogMessage(ConvertString(std::format(L"Begin CompileShader: path:{}, profile:{}.\n", filePath, profile)));
    IDxcBlobEncoding* shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    assert(SUCCEEDED(hr));

    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;

    // compile
    LPCWSTR arguments[] = {
        filePath.c_str(),
        L"-E", L"main",
        L"-T", profile,
        L"-Zi", L"-Qembed_debug",
        L"-Od",
        L"-Zpr",
    };

    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler,
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    // check error
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() > 0) {
        Log::LogMessage(shaderError->GetStringPointer());
        assert(false);
    }

    // get result
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));

    Log::LogMessage(ConvertString(std::format(L"Compile Succeeded: path:{}, profile:{}.\n", filePath, profile)));
    shaderSource->Release();
    shaderResult->Release();
    return shaderBlob;
}