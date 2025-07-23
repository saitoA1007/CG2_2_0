#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include<array>
#include <wrl.h>
#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Core/DXC.h"

#include"BlendMode.h"

namespace GameEngine {

    class SpritePSO final {
    public:

        SpritePSO() = default;
        ~SpritePSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc, LogManager* logManager);

        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode].Get(); }

    private:
        SpritePSO(const SpritePSO&) = delete;
        SpritePSO& operator=(const SpritePSO&) = delete;

        ID3D12Device* device_ = nullptr;

        std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, BlendMode::kCountOfBlendMode> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        // ログ
        LogManager* logManager_;

        D3D12_BLEND_DESC blendDesc_[BlendMode::kCountOfBlendMode];
    };
}