#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<iostream>
#include <wrl.h>
#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Core/DXC.h"

namespace GameEngine {

    class TrianglePSO {
    public:

        TrianglePSO() = default;
        ~TrianglePSO() = default;

        // 初期化
        void Initialize(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device, DXC* dxc, LogManager* logManager);

        ID3D12PipelineState* GetPipelineState() { return graphicsPipelineState_.Get(); }
        ID3D12RootSignature* GetRootSignature() { return rootSignature_.Get(); }

    private:
        TrianglePSO(const TrianglePSO&) = delete;
        TrianglePSO& operator=(const TrianglePSO&) = delete;

        ID3D12Device* device_ = nullptr;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

        // Shaderをコンパイルする
        Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
        Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

        // ログ
        LogManager* logManager_;
    };
}
