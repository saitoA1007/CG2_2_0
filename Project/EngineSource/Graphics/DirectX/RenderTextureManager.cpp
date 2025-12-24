#include"RenderTextureManager.h"

using namespace GameEngine;

void RenderTextureManager::Initialize(RtvManager* rtvManager, SrvManager* srvManager, ID3D12Device* device) {
	rtvManager_ = rtvManager;
	srvManager_ = srvManager;
    device_ = device;
}

void RenderTextureManager::Create(const std::string& name, RtvContext context) {

	// RTVを作成
	uint32_t rtvIndex = rtvManager_->CreateRenderTargetResource(context);
	ID3D12Resource* resource = rtvManager_->GetResource(rtvIndex);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = context.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // SRVハンドル取得
    uint32_t srvIndex = srvManager_->AllocateSrvIndex(SrvHeapType::System);
    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = srvManager_->GetCPUHandle(srvIndex);
    D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle = srvManager_->GetGPUHandle(srvIndex);

    // オブジェクト描画用SRV
    device_->CreateShaderResourceView(resource, &srvDesc, srvCPUHandle);

    // テクスチャ情報を作成
    RenderTextureContext renderTextureContext{};
    renderTextureContext.rtvIndex = rtvIndex;
    renderTextureContext.srvIndex = srvIndex;
    renderTextureContext.width = context.width;
    renderTextureContext.height = context.height;
    renderTextureContext.resource = resource;
    renderTextureContext.srvGpuHandle = srvGPUHandle;

    // RenderTextureを作成
    std::unique_ptr<RenderTexture> renderTexture = std::make_unique<RenderTexture>();
    renderTexture->Initialize(renderTextureContext);

    // 登録する
    renderTextures_[name] = std::move(renderTexture);
}

RenderTexture* RenderTextureManager::GetRenderTexture(const std::string& name) {
    auto it = renderTextures_.find(name);
    if (it != renderTextures_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void RenderTextureManager::Release(const std::string& name) {
    auto it = renderTextures_.find(name);
    if (it != renderTextures_.end()) {
        // RtvとSrvのインデックスを解放
        uint32_t rtvIndex = it->second->GetRtvIndex();
        uint32_t srvIndex = it->second->GetSrvIndex();

        rtvManager_->ReleseIndex(rtvIndex);
        srvManager_->ReleseIndex(srvIndex);

        // マップから削除
        renderTextures_.erase(it);
    }
}