#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"EngineSource/Core/DXC.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Vector2.h"

#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

    class OutLinePSO {
    public:

        struct alignas(16) OutLineData {
            Vector2 centerPos; // 中心点
            int32_t numSamles; // サンプリング数。大きい程滑らか
            float blurWidth; // ぼかしの幅
        };

    public:
        OutLinePSO() = default;
        ~OutLinePSO() = default;

        void Initialize(ID3D12Device* device, DXC* dxc, LogManager* logManager);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

        // GPUに送るデータ
        OutLineData* outLineData_ = nullptr;

    private:
        OutLinePSO(const OutLinePSO&) = delete;
        OutLinePSO& operator=(const OutLinePSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

        // 整するためのパラメーターのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> parameterResource_;
    };
}
