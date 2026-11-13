#include "PSOManager.h"

void PSOManager::Initialize(ID3D12Device* device) {
	CreateRootSignatures(device);
    CreateParticleRootSignature(device);

    D3D12_INPUT_LAYOUT_DESC inputLayout = CreateDefaultInputLayout();

    IDxcUtils* dxcUtils = nullptr;
    IDxcCompiler3* dxcCompiler = nullptr;
    IDxcIncludeHandler* includeHandler = nullptr;
    HREFTYPE hr;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));    
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));

    IDxcBlob* vertexShaderBlob = LoadOrGetShader(L"shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);
    IDxcBlob* pixelShaderBlob = LoadOrGetShader(L"shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob != nullptr);


    // 1. Create Default 3D Opaque PSO
    enumPipelines_[PSOType::Opaque3D] = PSOBuilder()
        .SetRootSignature(rootSignatures_[rootSignatureType::Default].Get())
        .SetShaders(vertexShaderBlob, pixelShaderBlob) // Pass compiled shader blobs
        .SetInputLayout(inputLayout)
        .Build(device);

    // 2. Create Wireframe PSO (Reuses same shaders/root signature, just tweaks rasterizer)
    enumPipelines_[PSOType::Wireframe] = PSOBuilder()
        .SetRootSignature(rootSignatures_[rootSignatureType::Default].Get())
        .SetShaders(vertexShaderBlob, pixelShaderBlob)
        .SetInputLayout(inputLayout)
        .SetWireframe()
        .Build(device);

    // 3. Create Transparent 3D PSO
    enumPipelines_[PSOType::Transparent3D] = PSOBuilder()
        .SetRootSignature(rootSignatures_[rootSignatureType::Default].Get())
        .SetShaders(vertexShaderBlob, pixelShaderBlob)
        .SetInputLayout(inputLayout)
        .EnableAlphaBlending()
        //.DisableDepthWrite()
        .Build(device);

    IDxcBlob* particleVertexShaderBlob = LoadOrGetShader(L"shader/Particle.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(particleVertexShaderBlob != nullptr);
    IDxcBlob* particlePixelShaderBlob = LoadOrGetShader(L"shader/Particle.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(particlePixelShaderBlob != nullptr);

    enumPipelines_[PSOType::Particle] = PSOBuilder()
        .SetRootSignature(rootSignatures_[rootSignatureType::Particle].Get())
        .SetShaders(particleVertexShaderBlob, particlePixelShaderBlob)
        .SetInputLayout(inputLayout) // position/texcoord/normal only; SV_InstanceID needs no vertex-buffer slot
        .EnableAlphaBlending()
        .DisableDepthWrite()
        .Build(device);
}

ID3D12PipelineState* PSOManager::GetPSO(PSOType type) const {
    auto it = enumPipelines_.find(type);
    return (it != enumPipelines_.end()) ? it->second.Get() : nullptr;
}

ID3D12PipelineState* PSOManager::GetPSO(const std::string& customName) const {
    auto it = customPipelines_.find(customName);
    return (it != customPipelines_.end()) ? it->second.Get() : nullptr;
}

ID3D12RootSignature* PSOManager::GetRootSignature(rootSignatureType type) const {
    auto it = rootSignatures_.find(type);
    return (it != rootSignatures_.end()) ? it->second.Get() : nullptr;
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
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
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

    Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
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
    return shaderBlob;
}

ID3D12PipelineState* PSOManager::CreatePSO(const std::string& name, ID3D12Device* device, ID3D12RootSignature* rootSig, IDxcBlob* vsBlob, IDxcBlob* psBlob, const D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT inputLayoutCount,
    bool enableAlphaBlending = false, bool disableDepthWrite = false) {
	PSOBuilder builder;
	builder.SetRootSignature(rootSig)
		.SetShaders(vsBlob, psBlob)
		.SetInputLayout(inputLayout, inputLayoutCount);
    if (enableAlphaBlending) builder.EnableAlphaBlending();
    if (disableDepthWrite) builder.DisableDepthWrite();

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = builder.Build(device);
	if (pso) {
		customPipelines_[name] = pso;
	}
	return pso.Get();
}

ID3D12PipelineState* PSOManager::CreatePSO(PSOType type, ID3D12Device* device, ID3D12RootSignature* rootSig, IDxcBlob* vsBlob, IDxcBlob* psBlob, const D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT inputLayoutCount,
    bool enableAlphaBlending = false, bool disableDepthWrite = false) {
	PSOBuilder builder;
	builder.SetRootSignature(rootSig)
		.SetShaders(vsBlob, psBlob)
		.SetInputLayout(inputLayout, inputLayoutCount);

    if (enableAlphaBlending) builder.EnableAlphaBlending();
    if (disableDepthWrite) builder.DisableDepthWrite();

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = builder.Build(device);
	if (pso) {
		enumPipelines_[type] = pso;
	}
	return pso.Get();
}

void PSOManager::CreateRootSignatures(ID3D12Device* device) {
    HREFTYPE hr;

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;
    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Log::LogMessage(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[rootSignatureType::Default]));
    assert(SUCCEEDED(hr));
}

D3D12_INPUT_LAYOUT_DESC PSOManager::CreateDefaultInputLayout() {
    static D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {}; // the "static" is temp fix! FIND SOLUTION FOR THIS!!!!
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
    return inputLayoutDesc;
}

IDxcBlob* PSOManager::LoadOrGetShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* utils, IDxcCompiler3* compiler, IDxcIncludeHandler* handler) {
    std::string pathKey = ConvertString(filePath);

    // If already compiled, return it instantly
    if (shaderBlobs_.find(pathKey) != shaderBlobs_.end()) {
        return shaderBlobs_[pathKey].Get();
    }

    // Otherwise, compile and store it in the map
    shaderBlobs_[pathKey] = CompileShader(filePath, profile, utils, compiler, handler);
    return shaderBlobs_[pathKey].Get();
}

void PSOManager::CreateParticleRootSignature(ID3D12Device* device) {
    HREFTYPE hr;

    // t0 (PS) = particle texture
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParameters[5] = {};

    // 0: StructuredBuffer<TransformationMatrix> t0, VS
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // 1: StructuredBuffer<float4> (per-instance color) t1, VS
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 1;

    // 2: Material CBV b0, PS
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].Descriptor.ShaderRegister = 0;

    // 3: Texture SRV table t0, PS
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    // 4: DirectionalLight CBV b1, PS
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[4].Descriptor.ShaderRegister = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Log::LogMessage(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatures_[rootSignatureType::Particle]));
    assert(SUCCEEDED(hr));
}