#include"TextureManager.h"
#include"ConvertString.h"
#include"CreateBufferResource.h"
#include"DescriptorHandle.h"
#include<format>
#include <filesystem>
#include"LogManager.h"
using namespace GameEngine;

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

void TextureManager::Finalize() {

	for (auto& tex : textures_) {
		tex.textureResource.Reset();
		tex.intermediateResources_.Reset();
	}
	textures_.clear();
}

void TextureManager::RegisterTexture(const std::string& registerName, const std::string& fileName) {
	// 同名のモデルが登録されている場合は早期リターン
	auto getName = nameToHandles_.find(registerName);
	if (getName != nameToHandles_.end()) {
		return;
	}

	// ロードする
	uint32_t handle = Load(fileName);

	// 登録する
	nameToHandles_[registerName] = handle;
}

uint32_t TextureManager::GetHandleByName(const std::string& name) const {
	auto getHandle = nameToHandles_.find(name);
	if (getHandle == nameToHandles_.end()) {
		return 0;
	}
	return getHandle->second;
}

uint32_t TextureManager::Load(const std::string& fileName) {

	// テクスチャーの読み込みを開始するログ
	LogManager::GetInstance().Log("Start LoadTexture : " + fileName);

	// もし同じテクスチャを読み込んだのであれば、すでに格納されている配列番号を返す。
	for (int i = 0; i < textures_.size(); ++i) {
		if (textures_.at(i).fileName == GetFileName(fileName)) {
			// 終了したこと、同じテクスチャを読み込んでいることを伝える
			LogManager::GetInstance().Log("End LoadTexture : " + fileName + ". This texture data already loaded");
			return i;
		}
	}

	Texture texture;
	// テクスチャ名を記録
	texture.fileName = GetFileName(fileName);

	// テクスチャを読み込む
	texture.mipImage = LoadTexture(fileName);
	if (!texture.mipImage.GetImages()) {
		LogManager::GetInstance().Log("Failed to load texture: " + fileName);
		assert(false);
	}
	metadata_ = &texture.mipImage.GetMetadata();
	// テクスチャリソースを作成
	texture.textureResource = CreateTextureResource(dxCommon_->GetDevice(), *metadata_);
	if (!texture.textureResource) {
		LogManager::GetInstance().Log("Failed to create textureResource for: " + fileName);
		assert(false);
	}
	// テクスチャデータをアップロード
	texture.intermediateResources_ = UploadTextureData(texture.textureResource.Get(), texture.mipImage, dxCommon_->GetDevice(), dxCommon_->GetCommandList());
	if (!texture.intermediateResources_) {
		LogManager::GetInstance().Log("Failed to upload texture data for: " + fileName);
		assert(false);
	}

	// srvインデックスを取得
	uint32_t index = srvManager_->AllocateSrvIndex();

	// metaDataを基にSRVの設定
	srvDesc_.Format = metadata_->format;
	srvDesc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc_.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	srvDesc_.Texture2D.MipLevels = UINT(metadata_->mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める。先頭はImGuiが使っているのでその次を使う
	texture.textureSrvHandleCPU = srvManager_->GetCPUHandle(index);
	texture.textureSrvHandleGPU = srvManager_->GetGPUHandle(index);
	LogManager::GetInstance().Log(std::format("CPU Handle: {}, GPU Handle: {}", texture.textureSrvHandleCPU.ptr, texture.textureSrvHandleGPU.ptr));
	// SRVを作成
	dxCommon_->GetDevice()->CreateShaderResourceView(texture.textureResource.Get(), &srvDesc_, texture.textureSrvHandleCPU);

	// 登録
	textures_.push_back(std::move(texture));

	// テクスチャーの読み込みを完了するログ
	LogManager::GetInstance().Log("End LoadTexture : " + fileName + "\n");

	// 読み込んだ画像が格納されている配列番号を返す
	return static_cast<uint32_t>(textures_.size() - 1);
}

D3D12_GPU_DESCRIPTOR_HANDLE& TextureManager::GetTextureSrvHandlesGPU(const uint32_t& textureHandle) {
	return textures_.at(textureHandle).textureSrvHandleGPU;
}

[[nodiscard]]
DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath)
{
	// テクスチャファイルを読み込んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返す
	return mipImages;
}

[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行 or 配列Textureの配列数 
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // ダンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。

	// 利用するHeapの設定。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし。
		&resourceDesc, // Resourceの設定。
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定
		nullptr, // Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	// Textureへ転送後は利用出来るよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

std::string TextureManager::GetFileName(const std::string& fullPath) {
	return std::filesystem::path(fullPath).filename().string();
}