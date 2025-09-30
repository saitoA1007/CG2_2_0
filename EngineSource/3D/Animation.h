#pragma once
#include <d3d12.h>
#include <wrl.h>

#include"VertexData.h"

// 影響を受けるJointの数
const uint32_t kNumMaxInfluence = 4;
struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix; // 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
};

struct SkinCluster {
	std::vector<Matrix4x4> inverseBindPoseMatrices;

	Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;

	Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
};

namespace GameEngine {

	class Animation final {
	public:
		Animation() = default;
		~Animation() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		static void StaticInitialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap,const uint32_t& descriptorSizeSRV);



	private:

		// デバイス
		static ID3D12Device* device_;
		static ID3D12DescriptorHeap* srvHeap_;
		static uint32_t descriptorSizeSRV_;

	};
}