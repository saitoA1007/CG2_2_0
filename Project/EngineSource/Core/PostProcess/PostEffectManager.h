#pragma once
#include<vector>
#include <wrl.h>
#include <d3d12.h>

#include"externals/DirectXTex/d3dx12.h"

#include"LogManager.h"
#include"PostProcess/BloomPSO.h"
#include"PostProcess/ScanLinePSO.h"
#include"PostProcess/VignettingPSO.h"
#include"PostProcess/RadialBlurPSO.h"
#include"PostProcess/OutLinePSO.h"
#include"ResourceCounter.h"

#include"SrvManager.h"

namespace GameEngine {

    class PostEffectManager {
    public:

        // ポストエフェクトの描画モード
        enum class DrawMode {
            Default, // 通常の描画(ブルームとヴィネット)
            ScanLine, // ラインの描画
            RadialBlur, // 中心に集中するぼかし
        };

    public:

        /// <summary>
        /// 静的初期化
        /// </summary>
        /// <param name="bloomPSO"></param>
        /// <param name="logManager"></param>
        static void StaticInitialize(BloomPSO* bloomPSO,ScanLinePSO* scanLinePSO, VignettingPSO* vignettingPSO, RadialBlurPSO* radialBlurPSO, OutLinePSO* outLinePSO, LogManager* logManager);

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
            uint32_t descriptorSizeRTV, SrvManager* srvManager);

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
        void PostDraw(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

        /// <summary>
        /// SRVを取得
        /// </summary>
        /// <returns></returns>
        CD3DX12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle();

        /// <summary>
        /// 描画モードを設定する
        /// </summary>
        /// <param name="drawMode"></param>
        void SetDrawMode(DrawMode drawMode) { drawMode_ = drawMode; }

        /// <summary>
        /// 現在の描画形態を取得
        /// </summary>
        /// <returns></returns>
        DrawMode GetDrawMode() { return drawMode_; }

    public:

        // ブルーム用PSO
        static BloomPSO* bloomPSO_;

        // ヴィネット用のPSO;
        static VignettingPSO* vignettingPSO_;

        // ラジアルブラー用のPSO
        static RadialBlurPSO* radialBlurPSO_;

        // ライン用のPSO;
        static ScanLinePSO* scanLinePSO_;

        // アウトライン用のPSO
        static OutLinePSO* outLinePSO_;

    private:

        // ログ
        static LogManager* logManager_;

        // デバイス
        ID3D12Device* device_ = nullptr;

        // ポストエフェクト用RTVの総数
        static inline const uint32_t kRTVNum = 16;

        // Clearの最適値
        D3D12_CLEAR_VALUE clearValue_{};

        // ポストエフェクトのためのRTVを計測
        uint32_t rtvIndex_ = 0;

        // コピー描画用リソース
        Microsoft::WRL::ComPtr<ID3D12Resource> DrawObjectResource_;

        // ポストエフェクト用のRTVヒープ
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> postProcessRTVHeap_;

        // オブジェクトを描画する用のRTV
        D3D12_CPU_DESCRIPTOR_HANDLE drawObjectRTVHandle_;

        // オブジェクトを描画する用のSRV
        CD3DX12_GPU_DESCRIPTOR_HANDLE drawObjectSRVHandle_;

        // 描画するモード
        DrawMode drawMode_ = DrawMode::Default;

        // srvを管理する
        SrvManager* srvManager_;

    private:

        // ブルーム用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE bloomRTVHandle_[4]{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE bloomSRVHandle_[4];

        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBrightResource_;     // 明るい部分抽出用
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBlurShrinkResource_; // 縮小させながらブラーをする
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomResultResource_;     // 最終敵なもの
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomCompositeResource_;  // 合成用

    private:

         // ライン用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE scanLineRTVHandle_{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE scanLineSRVHandle_;

        // 線を描画するためのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> scanLineResource_;

    private:

        // ヴィネット用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE vignettingRTVHandle_{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE vignettingSRVHandle_;

        // ヴィネットを描画するためのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> vignettingResource_;

    private:

        // ラジアルブラー用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE radialBlurRTVHandle_{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE radialBlurSRVHandle_;

        // ラジアルブラーを描画するためのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> radialBlurResource_;

    private:

        // ラジアルブラー用のRTVハンドル
        D3D12_CPU_DESCRIPTOR_HANDLE outLineRTVHandle_{};

        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE outLineSRVHandle_;

        // ラジアルブラーを描画するためのリソース
        Microsoft::WRL::ComPtr<ID3D12Resource> outLineResource_;

    private:

        /// <summary>
        /// ブルーム処理をするためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeBloom(uint32_t width, uint32_t height, uint32_t descriptorSizeRTV);

        /// <summary>
        /// ブルームの描画処理
        /// </summary>
        /// <param name="commandList"></param>
        /// <param name="baseViewport"></param>
        /// <param name="baseScissorRect"></param>
        void DrawBloom(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& baseViewport, const D3D12_RECT& baseScissorRect);

        /// <summary>
        /// ラインの描画するためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeScanLine(uint32_t width, uint32_t height, uint32_t descriptorSizeRTV);

        /// <summary>
        /// ラインの描画処理
        /// </summary>
        /// <param name="commandList"></param>
        void DrawScanLine(ID3D12GraphicsCommandList* commandList);

        /// <summary>
        /// ヴィネットの描画するためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeVignetting(uint32_t width, uint32_t height, uint32_t descriptorSizeRTV);

        /// <summary>
        /// ヴィネットの描画処理
        /// </summary>
        /// <param name="commandList"></param>
        void DrawVignetting(ID3D12GraphicsCommandList* commandList);

        /// <summary>
        /// ラジアルブルーの描画するためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeRadialBlur(uint32_t width, uint32_t height, uint32_t descriptorSizeRTV);

        /// <summary>
        /// ラジアルブルーの描画処理
        /// </summary>
        /// <param name="commandList"></param>
        void DrawRadialBlur(ID3D12GraphicsCommandList* commandList);

        /// <summary>
        /// アウトラインの描画するためのRTVを設定
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="descriptorSizeSRV"></param>
        /// <param name="descriptorSizeRTV"></param>
        void InitializeOutLine(uint32_t width, uint32_t height, uint32_t descriptorSizeRTV);

        /// <summary>
        /// アウトラインの描画
        /// </summary>
        /// <param name="commandList"></param>
        /// <param name="depthSRV"></param>
        void DrawOutLine(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);
    };
}