#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include"EngineSource/Core/DXC.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector2.h"

#include"EngineSource/Common/LogManager.h"

namespace GameEngine {

    class PostProcessPSO {
    public:

        struct VertexData {
            Vector4 position;
            Vector2 texcoord;
        };

        struct MaterialData {
            int32_t isEnablePostEffect;
            float padding[3];
        };

    public:
        PostProcessPSO() = default;
        ~PostProcessPSO() = default;

        void Initialize(ID3D12Device* device, const std::wstring& vsPath, const std::wstring& psPath, DXC* dxc, LogManager* logManager);
        void Set(ID3D12GraphicsCommandList* commandList);
        void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
        ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

        /// <summary>
        /// ポストエフェクトの有効化
        /// </summary>
        /// <param name="isEnable"></param>
        void SetIsEnablePostEffect(bool isEnable) { materialData_->isEnablePostEffect = isEnable; }

    private:
        PostProcessPSO(const PostProcessPSO&) = delete;
        PostProcessPSO& operator=(const PostProcessPSO&) = delete;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

        // 頂点バッファビューを作成する
        D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
        // Sprite用の頂点リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_;
        VertexData* vertexDataSprite_ = nullptr;

        // Sprite用のマテリアルリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
        // マテリアルにデータを書き込む
        MaterialData* materialData_ = nullptr;
    };
}
