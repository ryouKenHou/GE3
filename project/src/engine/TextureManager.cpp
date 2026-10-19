#include "TextureManager.h"

DescriptorHandle TextureManager::Load(const std::string& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator& srvAllocator) {
	auto it = textures_.find(filePath);
	if (it != textures_.end()) {
		return it->second.srvHandle;
	}

	DirectX::ScratchImage mipImages = LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	textures_[filePath] = TextureData{};
	textures_[filePath].resource = CreateTextureResource(device, metadata);
	textures_[filePath].intermediateResource = UploadTextureData(textures_[filePath].resource.Get(), mipImages, device, cmdList);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textures_[filePath].srvHandle = srvAllocator.Allocate();
	device->CreateShaderResourceView(textures_[filePath].resource.Get(), &srvDesc, textures_[filePath].srvHandle.cpuHandle);
	return textures_[filePath].srvHandle;
}