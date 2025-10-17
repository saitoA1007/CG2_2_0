#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include <wrl.h>
#include"DXC.h"

namespace GameEngine {

    class GridPSO {
    public:

        GridPSO() = default;
        ~GridPSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc);

        ID3D12PipelineState* GetPipelineState() { return graphicsPipelineState_.Get(); }
        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

    private:
        GridPSO(const GridPSO&) = delete;
        GridPSO& operator=(const GridPSO&) = delete;

        ID3D12Device* device_ = nullptr;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    };
}