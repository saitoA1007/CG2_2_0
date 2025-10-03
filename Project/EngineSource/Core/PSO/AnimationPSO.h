#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include<array>
#include <wrl.h>
#include"LogManager.h"
#include"DXC.h"

#include"BlendMode.h"

namespace GameEngine {

    class AnimationPSO {
    public:

        AnimationPSO() = default;
        ~AnimationPSO() = default;

        // 初期化
        void Initialize(ID3D12Device* device, DXC* dxc, LogManager* logManager);

        ID3D12PipelineState* GetPipelineState() { return graphicsPipelineState_.Get(); }
        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

    private:
        AnimationPSO(const AnimationPSO&) = delete;
        AnimationPSO& operator=(const AnimationPSO&) = delete;

        ID3D12Device* device_ = nullptr;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        // ログ
        LogManager* logManager_;
    };
}
