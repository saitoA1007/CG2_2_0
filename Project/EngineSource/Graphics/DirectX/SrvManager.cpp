#include"SrvManager.h"
#include"DescriptorHandle.h"
#include"DescriptorHeap.h"
#include <cassert>

using namespace GameEngine;

ID3D12Device* SrvManager::device_ = nullptr;

void SrvManager::Initialize(ID3D12Device* device) {
	device_ = device;
	// SRV用のヒープを作成する
	srvHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSrvIndex_, true);
	// DescriptorSizeを取得しておく
	descriptorSizeSRV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::AllocateSrvIndex() {

    uint32_t index = 0;

	// 利用可能であれば使用する
	if (!availableIndices_.empty()) {
		index = availableIndices_.front();
		availableIndices_.pop();
	} else if (nextNewIndex_ < kMaxSrvIndex_) {
		index = nextNewIndex_++;
	} else {
		assert(0);
		return kMaxSrvIndex_;
	}

	// 使用中のインデックスをセット
	usedIndices_.insert(index);
	return index;
}

void SrvManager::ReleseIndex(const uint32_t& index) {
	// 使用中のインデックスを削除
	std::unordered_set<uint32_t>::iterator usedIndex = usedIndices_.find(index);
	// 要素が見つかれば削除
	if (usedIndex != usedIndices_.end()) {
		usedIndices_.erase(usedIndex);
		// 再利用を可能にする
		availableIndices_.push(index);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUHandle(uint32_t index) const {
	return GetCPUDescriptorHandle(srvHeap_.Get(), descriptorSizeSRV_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUHandle(uint32_t index) const {
	return GetGPUDescriptorHandle(srvHeap_.Get(), descriptorSizeSRV_, index);
}