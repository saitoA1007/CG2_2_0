#include"TextureManager.h"
#include"ConvertString.h"
#include"CreateBufferResource.h"
#include"DescriptorHandle.h"
#include"Log.h"

TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize() {
	GetInstance()->dxCommon_ = DirectXCommon::GetInstance();
}

void TextureManager::Finalize() {

	//for (int i = 0; i < GetInstance()->intermediateResources_.size(); ++i) {
	//	GetInstance()->textures_.at(i).textureResource.Reset();
	//}

	for (auto& tex : textures_) {
		tex.textureResource.Reset();
	}

	for (auto& inter : intermediateResources_) {
		inter.Reset();
	}

	GetInstance()->intermediateResources_.clear();	
}

uint32_t TextureManager::Load(const std::string& fileName, std::ofstream& logStream) {

	TextureManager* instance = GetInstance();

	// もし同じテクスチャを読み込んだのであれば、すでに格納されている配列番号を返す。
	for (int i = 0; i < instance->textures_.size(); ++i) {
		if (instance->textures_.at(i).fileName == fileName) {
			return i;
		}
	}

	// テクスチャ読み込みの最大数より小さければ追加する処理
	if (instance->index_ < kTextureNum_) {
		instance->index_++;
	} else {
		return -1;
	}

	instance->textures_.at(instance->index_).fileName = fileName;

		// テクスチャを読み込む
	instance->textures_.at(instance->index_).mipImage = instance->LoadTexture(fileName);
	if (!instance->textures_.at(instance->index_).mipImage.GetImages()) {
		Log(logStream, "Failed to load texture: " + fileName);
		assert(false);
	}
	instance->metadata_ = &instance->textures_.at(instance->index_).mipImage.GetMetadata();
	// テクスチャリソースを作成
	instance->textures_.at(instance->index_).textureResource = instance->CreateTextureResource(instance->dxCommon_->GetDevice(), *instance->metadata_);
	if (!instance->textures_.at(instance->index_).textureResource) {
		Log(logStream, "Failed to create texture resource for: " + fileName);
		assert(false);
	}
	// テクスチャデータをアップロード
	instance->intermediateResources_.push_back(instance->UploadTextureData(instance->textures_.at(instance->index_).textureResource.Get(), instance->textures_.at(instance->index_).mipImage, instance->dxCommon_->GetDevice(), instance->dxCommon_->GetCommandList()));
	if (!instance->intermediateResources_.at(instance->index_)) {
		Log(logStream, "Failed to upload texture data for: " + fileName);
		assert(false);
	}

	// metaDataを基にSRVの設定
	instance->srvDesc_.Format = instance->metadata_->format;
	instance->srvDesc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instance->srvDesc_.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;// 2Dテクスチャ
	instance->srvDesc_.Texture2D.MipLevels = UINT(instance->metadata_->mipLevels);

	// SRVを作成するDescriptorHeapの場所を決める。先頭はImGuiが使っているのでその次を使う
	instance->textures_.at(instance->index_).textureSrvHandleCPU = GetCPUDescriptorHandle(instance->dxCommon_->GetSRVHeap(), instance->dxCommon_->GetSRVDescriptorSize(), instance->index_ + 1);
	instance->textures_.at(instance->index_).textureSrvHandleGPU = GetGPUDescriptorHandle(instance->dxCommon_->GetSRVHeap(), instance->dxCommon_->GetSRVDescriptorSize(), instance->index_ + 1);
	Log(logStream, std::format("CPU Handle: {}, GPU Handle: {}", instance->textures_.at(instance->index_).textureSrvHandleCPU.ptr, instance->textures_.at(instance->index_).textureSrvHandleGPU.ptr));
	// SRVを作成
	instance->dxCommon_->GetDevice()->CreateShaderResourceView(instance->textures_.at(instance->index_).textureResource.Get(), &instance->srvDesc_, instance->textures_.at(instance->index_).textureSrvHandleCPU);

	// 読み込んだ画像が格納されている配列番号を返す
	return instance->index_;
}

D3D12_GPU_DESCRIPTOR_HANDLE& TextureManager::GetTextureSrvHandlesGPU(const uint32_t& textureHandle) {
	return GetInstance()->textures_.at(textureHandle).textureSrvHandleGPU;
}

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