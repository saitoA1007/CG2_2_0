#include"Animation.h"

using namespace GameEngine;

ID3D12Device* Animation::device_ = nullptr;
ID3D12DescriptorHeap* Animation::srvHeap_ = nullptr;
uint32_t Animation::descriptorSizeSRV_ = 0;

void Animation::StaticInitialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, const uint32_t& descriptorSizeSRV) {
	device_ = device;
	srvHeap_ = srvHeap;
	descriptorSizeSRV_ = descriptorSizeSRV;
}

