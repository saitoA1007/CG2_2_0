#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include<array>
#include <wrl.h>
#include"DXC.h"

#include"BlendMode.h"

namespace GameEngine {

    class ParticlePSO {
    public:

        ParticlePSO() = default;
        ~ParticlePSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc);

        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode].Get(); }

    private:
        ParticlePSO(const ParticlePSO&) = delete;
        ParticlePSO& operator=(const ParticlePSO&) = delete;

        ID3D12Device* device_ = nullptr;

        std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, BlendMode::kCountOfBlendMode> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

        // Shaderをコンパイルする
        Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
        Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

        D3D12_BLEND_DESC blendDesc_[BlendMode::kCountOfBlendMode];
    };
}
