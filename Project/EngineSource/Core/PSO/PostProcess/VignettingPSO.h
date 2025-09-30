#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"DXC.h"
#include"Vector4.h"
#include"Vector3.h"
#include"Vector2.h"

#include"LogManager.h"

namespace GameEngine {

    class VignettingPSO {
    public:

        struct alignas(16) VignettingData {
            float intensity; // ぼかさない円の範囲
            float time; // ぼかしぐわい
            float padding[2];
        };

    public:
        VignettingPSO() = default;
        ~VignettingPSO() = default;

        void Initialize(ID3D12Device* device, DXC* dxc, LogManager* logManager);

        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

        // GPUに送るデータ
        VignettingData* vignettingData_ = nullptr;

    private:
        VignettingPSO(const VignettingPSO&) = delete;
        VignettingPSO& operator=(const VignettingPSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

        // 整するためのパラメーターのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> parameterResource_;
    };
}