#pragma once
#include"RtvManager.h"
#include"SrvManager.h"

namespace GameEngine {

	// レンダーターゲットを作成する土台
	struct RenderTextureContext {
		uint32_t rtvIndex = 0;
		uint32_t srvIndex = 0;
		uint32_t dsvIndex = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		bool isDepth = false;
		uint32_t dsvHaveSrvIndex = 0;

		ID3D12Resource* resource = nullptr;
		ID3D12Resource* dsvResource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = {};
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

		D3D12_CPU_DESCRIPTOR_HANDLE& GetRtvHandle() { return rtvHandle_; }

		CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() { return srvGpuHandle_; }

		D3D12_CPU_DESCRIPTOR_HANDLE& GetDsvHandle() { return dsvHandle_; }

		uint32_t GetSrvIndex() const { return srvIndex_; }

		uint32_t GetRtvIndex() const { return rtvIndex_; }

		ID3D12Resource* GetResource() const { return resource_; }

		uint32_t GetWidth() const { return width_; }

		uint32_t GetHeight() const { return height_; }

	private:

		// インデックス
		uint32_t rtvIndex_ = 0;
		uint32_t srvIndex_ = 0;
		uint32_t dsvIndex_ = 0;
		uint32_t dsvHaveSrvIndex_ = 0;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		// リソース
		ID3D12Resource* resource_ = nullptr;

		// 深度リソース
		ID3D12Resource* depthResource_ = nullptr;

		// rtvハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;

		// dsvハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;

		// srvハンドル
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_;

		// 現在の状態
		D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		bool isDepth_ = false;
	};

}