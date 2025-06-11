#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"EngineSource/Core/DXC.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector2.h"

#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

    class BloomPSO {
    public:

        struct VertexData {
            Vector4 position;
            Vector2 texcoord;
        };

    public:
        BloomPSO() = default;
        ~BloomPSO() = default;

        void Initialize(ID3D12Device* device, const std::wstring& vsPath, DXC* dxc, LogManager* logManager,
            const std::wstring brightPsPath, const std::wstring blurPsPath, const std::wstring resultPsPath, const std::wstring compositePsPath);
        void Set(ID3D12GraphicsCommandList* commandList);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

        ID3D12PipelineState* GetBrightPipelineState() const { return brightPipelineState_.Get(); }
        ID3D12PipelineState* GetBlurPipelineState() const { return blurPipelineState_.Get(); }
        ID3D12PipelineState* GetResultPipelineState() const { return blurResultPipelineState_.Get(); }
        ID3D12PipelineState* GetBlurCompositePipelineState() const { return blurCompositePipelineState_.Get(); }

    private:
        BloomPSO(const BloomPSO&) = delete;
        BloomPSO& operator=(const BloomPSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> brightPipelineState_; // 明るい部分を抽出する
        Microsoft::WRL::ComPtr<ID3D12PipelineState> blurPipelineState_; // 縮小しながらぼかす
        Microsoft::WRL::ComPtr<ID3D12PipelineState> blurResultPipelineState_; // ぼかした結果を描画する
        Microsoft::WRL::ComPtr<ID3D12PipelineState> blurCompositePipelineState_; // 最終的に合成したのを描画する

        // 頂点バッファビューを作成する
        D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
        // Sprite用の頂点リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_;
        VertexData* vertexDataSprite_ = nullptr;
    };
}
