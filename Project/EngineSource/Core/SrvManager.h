#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace GameEngine {

	class SrvManager {
	public:

		SrvManager() = default;
		~SrvManager() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="device"></param>
		void Initialize(ID3D12Device* device);

		ID3D12DescriptorHeap* GetSRVHeap() const { return srvHeap_.Get(); }

		/// <summary>
		/// インデックスを取得
		/// </summary>
		/// <returns></returns>
		uint32_t AllocateSrvIndex();

		/// <summary>
		/// インデックスを削除
		/// </summary>
		/// <param name="index"></param>
		void ReleseIndex(const uint32_t& index);

		/// <summary>
		/// 指定インデックスのCPUディスクリプタハンドルを取得
		/// </summary>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;

		/// <summary>
		/// 指定インデックスのGPUディスクリプタハンドルを取得
		/// </summary>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

	private:
		SrvManager(const SrvManager&) = delete;
		SrvManager& operator=(const SrvManager&) = delete;

		static ID3D12Device* device_;

		static inline const uint32_t kMaxSrvIndex_ = 1024; // 最大のsrvインデックス

		std::queue<uint32_t> availableIndices_;  // 利用出来るインデックス
		std::unordered_set<uint32_t> usedIndices_; // 利用しているインデックスを保持
		uint32_t nextNewIndex_ = 1; // 次のsrvインデックス

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
		uint32_t descriptorSizeSRV_ = 0;
	};
}