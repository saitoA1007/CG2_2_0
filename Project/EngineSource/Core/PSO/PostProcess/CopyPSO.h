#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"EngineSource/Core/DXC.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector2.h"

#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

    class CopyPSO {
    public:
        CopyPSO() = default;
        ~CopyPSO() = default;

        void Initialize(ID3D12Device* device, const std::wstring& vsPath, const std::wstring psPath, DXC* dxc, LogManager* logManager);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

        ID3D12PipelineState* GetBrightPipelineState() const { return pipelineState_.Get(); }

        ID3D12PipelineState* GetPipelineState() const {}

    private:
        CopyPSO(const CopyPSO&) = delete;
        CopyPSO& operator=(const CopyPSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    };
}