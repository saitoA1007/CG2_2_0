#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"EngineSource/Core/DXC.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector2.h"

#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

    class GaussianBlurPSO {
    public:

        struct VertexData {
            Vector4 position;
            Vector2 texcoord;
        };

        struct ConstBuffer {
            float highLumMask;  // 明るさの範囲
            float sigma;  // ぼかしの強さ
            int32_t  bloomIteration;
            float intensity;
        };

    public:
        GaussianBlurPSO() = default;
        ~GaussianBlurPSO() = default;

        void Initialize(ID3D12Device* device, DXC* dxc, LogManager* logManager);
        void Set(ID3D12GraphicsCommandList* commandList);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
        ID3D12PipelineState* GetBrightPipelineState() const { return pipelineState_.Get(); }

        // ブルームを調整するためのパラメーター
        ID3D12Resource* GetBloomParameterResource()const { return blurParameterResource_.Get(); }

        ConstBuffer* constBuffer_ = nullptr;
    private:
        GaussianBlurPSO(const GaussianBlurPSO&) = delete;
        GaussianBlurPSO& operator=(const GaussianBlurPSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_; // 明るい部分を抽出する

        // 頂点バッファビューを作成する
        D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
        // Sprite用の頂点リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_;
        VertexData* vertexDataSprite_ = nullptr;

        // ブルーム処理で調整するためのパラメーターのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> blurParameterResource_;
    };
}