#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include<array>
#include <wrl.h>
#include"DXC.h"

#include"BlendMode.h"

namespace GameEngine {

    class TrianglePSO {
    public:

        TrianglePSO() = default;
        ~TrianglePSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc);

        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode].Get(); }

        ID3D12PipelineState* GetDrawModePipelineState(DrawModel drawMode) { return frameGraphicsPipelineState_[drawMode].Get(); }

    private:
        TrianglePSO(const TrianglePSO&) = delete;
        TrianglePSO& operator=(const TrianglePSO&) = delete;

        ID3D12Device* device_ = nullptr;

        std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, BlendMode::kCountOfBlendMode> graphicsPipelineState_;

        std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, DrawModel::kCountOfDrawMode> frameGraphicsPipelineState_;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        D3D12_BLEND_DESC blendDesc_[BlendMode::kCountOfBlendMode];
    };
}
