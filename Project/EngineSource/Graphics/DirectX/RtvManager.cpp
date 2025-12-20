#include"RtvManager.h"
#include"LogManager.h"
#include"DescriptorHeap.h"
#include"DescriptorHandle.h"
using namespace GameEngine;

void RtvManager::Initialize(ID3D12Device* device) {

    // RTVの設定
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // RTV用のヒープ
    rtvHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128, false);

    descriptorSizeRTV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

}