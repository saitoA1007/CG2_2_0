#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include <wrl.h>
#include"LogManager.h"
#include"DXC.h"

namespace GameEngine {

    class LinePSO {
    public:

        LinePSO() = default;
        ~LinePSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc, LogManager* logManager);

        ID3D12PipelineState* GetPipelineState() { return graphicsPipelineState_.Get(); }
        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

    private:
        LinePSO(const LinePSO&) = delete;
        LinePSO& operator=(const LinePSO&) = delete;

        ID3D12Device* device_ = nullptr;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        // ログ
        LogManager* logManager_;
    };
}