#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace GameEngine {

	class RtvManager {
	public:
		RtvManager() = default;
		~RtvManager() = default;

		void Initialize(ID3D12Device* device);


	private:
		RtvManager(const RtvManager&) = delete;
		RtvManager& operator=(const RtvManager&) = delete;

		// rtvヒープ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;

		uint32_t descriptorSizeRTV_ = 0;
	};

}