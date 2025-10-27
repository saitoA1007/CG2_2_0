#pragma once
#include <d3d12.h>
#include<iostream>
#include<vector>
#include <unordered_map>
#include <wrl.h>
#include"Externals/DirectXTex/DirectXTex.h"
#include"Externals/DirectXTex/d3dx12.h"

#include"DirectXCommon.h"
#include<format>

#include"SrvManager.h"

namespace GameEngine {

	class TextureManager final {
	public:
		// テクスチャ
		struct Texture {
			// テクスチャリソース
			Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
			// テクスチャアップロードするための変数
			Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources_;

			DirectX::ScratchImage mipImage{};
			// シェーダリソースビューのハンドル(CPU)
			CD3DX12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
			// シェーダリソースビューのハンドル(CPU)
			CD3DX12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;
			// 名前
			std::string fileName;
		};

	public:
		TextureManager() = default;
		~TextureManager() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="dxCommon"></param>
		void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

		// 解放処理
		void Finalize();

		/// <summary>
		/// 登録する関数
		/// </summary>
		/// <param name="registerName"></param>
		/// <param name="fileName"></param>
		void RegisterTexture(const std::string& registerName,const std::string& fileName);

		/// <summary>
		/// 名前からハンドルを取得
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		uint32_t GetHandleByName(const std::string& name) const;

		/// <summary>
		/// テクスチャを読み込んで転送する処理
		/// </summary>
		/// <param name="fileName">ファイル名</param>
		/// <param name="logStream">ログファイル</param>
		/// <returns></returns>
		uint32_t Load(const std::string& fileName);


		D3D12_GPU_DESCRIPTOR_HANDLE& GetTextureSrvHandlesGPU(const uint32_t& textureHandle);

	private:
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;

		// DirectXCommon
		DirectXCommon* dxCommon_ = nullptr;

		// テクスチャデータを管理する変数
		std::vector<Texture> textures_;

		const DirectX::TexMetadata* metadata_{};

		// metaDataを基にSRVの設定の変数
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_{};

		// テクスチャパス
		std::string texturePath_;

		SrvManager* srvManager_ = nullptr;

		// テクスチャのハンドルを保存する
		std::unordered_map<std::string, uint32_t> nameToHandles_;

	private:

		[[nodiscard]]
		DirectX::ScratchImage LoadTexture(const std::string& filePath);

		[[nodiscard]]
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
			ID3D12GraphicsCommandList* commandList);

		std::string GetFileName(const std::string& fullPath);
	};
}
