#pragma once
#include<vector>
#include <wrl.h>
#include <d3d12.h>

#include"externals/DirectXTex/d3dx12.h"

#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Core/PSO/BloomPSO.h"
#include"EngineSource/Core/ResourceCounter.h"

namespace GameEngine {

    class PostEffectManager {
    public:

        /// <summary>
        /// 静的初期化
        /// </summary>
        /// <param name="bloomPSO"></param>
        /// <param name="logManager"></param>
        static void StaticInitialize(BloomPSO* bloomPSO, LogManager* logManager);

        /// <summary>
        /// 初期化
        /// </summary>
        /// <param name="device"></param>
        /// <param name="clearColor_"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeRTV"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="srvHeap_"></param>
        void Initialize(ID3D12Device* device, float clearColor_[4], uint32_t width, uint32_t height,
            uint32_t descriptorSizeRTV, uint32_t descriptorSizeSRV, ID3D12DescriptorHeap* srvHeap_);

        /// <summary>
        /// 描画前処理
        /// </summary>
        /// <param name="commandList"></param>
        /// <param name="viewport"></param>
        /// <param name="scissorRect"></param>
        /// <param name="clearColor"></param>
        /// <param name="dsvHeap"></param>
        void PreDraw(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect, float clearColor[4], ID3D12DescriptorHeap* dsvHeap);

        /// <summary>
        /// 描画後処理
        /// </summary>
        /// <param name="commandList"></param>
        /// <param name="viewport"></param>
        /// <param name="scissorRect"></param>
        void PostDraw(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect);

        /// <summary>
        /// SRVを取得
        /// </summary>
        /// <returns></returns>
        CD3DX12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle() { return bloomSRVHandle_[3]; }

    private:

        // ログ
        static LogManager* logManager_;

        // デバイス
        ID3D12Device* device_ = nullptr;

        // srvヒープ
        ID3D12DescriptorHeap* srvHeap_ = nullptr;

        // ポストエフェクト用RTVの総数
        static inline const uint32_t kRTVNum = 16;

        // Clearの最適値
        D3D12_CLEAR_VALUE clearValue_{};

        // ポストエフェクトのためのRTVを計測
        uint32_t rtvIndex_ = 0;
        // ポストエフェクトのためのSRVを計測
        uint32_t srvIndex_ = static_cast<uint32_t>(ResourceCount::kStartOffScreenCount);

        // コピー描画用リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> DrawObjectResource_;

        // ポストエフェクト用のRTVヒープ
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> postProcessRTVHeap_;

        // オブジェクトを描画する用のRTV
        D3D12_CPU_DESCRIPTOR_HANDLE drawObjectRTVHandle_;

        // オブジェクトを描画する用のSRV
        CD3DX12_GPU_DESCRIPTOR_HANDLE drawObjectSRVHandle_;

    private:

        // ブルーム用PSO
        static BloomPSO* bloomPSO_;

        // ブルーム用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE bloomRTVHandle_[4]{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE bloomSRVHandle_[4];

        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBrightResource_;     // 明るい部分抽出用
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBlurShrinkResource_; // 縮小させながらブラーをする
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomResultResource_;     // 最終敵なもの
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomCompositeResource_;  // 合成用

    private:

        /// <summary>
        /// ブルーム処理をするためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeBloom(uint32_t width, uint32_t height, uint32_t descriptorSizeSRV, uint32_t descriptorSizeRTV);

        /// <summary>
        /// ブルームの描画処理
        /// </summary>
        /// <param name="commandList"></param>
        /// <param name="baseViewport"></param>
        /// <param name="baseScissorRect"></param>
        void DrawBloom(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& baseViewport, const D3D12_RECT& baseScissorRect);
    };
}