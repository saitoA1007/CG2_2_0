#pragma once
#include"RtvManager.h"
#include"SrvManager.h"

namespace GameEngine {

	// レンダーターゲットを作成する土台
	struct RenderTextureContext {
		uint32_t rtvIndex = 0;
		uint32_t srvIndex = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		ID3D12Resource* resource = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = {};
	};

	class RenderTexture {
	public:
		RenderTexture() = default;
		~RenderTexture() = default;

		void Initialize(RenderTextureContext& context);

		// レンダーターゲットに変更
		void TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList);
		
		// シェーダーリソースに変更
		void TransitionToShaderResource(ID3D12GraphicsCommandList* commandList);

		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const { return srvGpuHandle_; }

		uint32_t GetSrvIndex() const { return srvIndex_; }

		uint32_t GetRtvIndex() const { return rtvIndex_; }

		ID3D12Resource* GetResource() const { return resource_; }

	private:

		// インデックス
		uint32_t rtvIndex_ = 0;
		uint32_t srvIndex_ = 0;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		// リソース
		ID3D12Resource* resource_ = nullptr;

		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_;

		// 現在の状態
		D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	};

}