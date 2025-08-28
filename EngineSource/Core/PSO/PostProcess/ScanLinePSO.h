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

    class ScanLinePSO {
    public:

        struct alignas(16) ScanLineData {
            float interval; // 間隔
            float time; // 時間
            float speed; // 速度
            float pad;
            Vector3 lineColor; // 線の色
            float pad2;
        };

    public:
        ScanLinePSO() = default;
        ~ScanLinePSO() = default;

        void Initialize(ID3D12Device* device, DXC* dxc, LogManager* logManager);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

        ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

        // GPUに送るデータ
        ScanLineData* scanLineData_ = nullptr;

    private:
        ScanLinePSO(const ScanLinePSO&) = delete;
        ScanLinePSO& operator=(const ScanLinePSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

        // 整するためのパラメーターのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> parameterResource_;
    };
}