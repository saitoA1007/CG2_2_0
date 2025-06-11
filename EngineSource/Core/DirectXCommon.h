#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <fstream>
#include"EngineSource/Common/LogManager.h"
#include"externals/DirectXTex/d3dx12.h"

#include"PSO/BloomPSO.h"

namespace GameEngine {

    class DirectXCommon {
    public:
        DirectXCommon() = default;
        ~DirectXCommon() = default;

        // 初期化
        void Initialize(HWND hwnd, uint32_t width, uint32_t height, LogManager* logManager);
        // 描画前処理
        void PreDraw();
        // 描画後処理
        void PostDraw();

    public:
        ID3D12Device* GetDevice() const { return device_.Get(); }
        ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
        IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }

        ID3D12DescriptorHeap* GetRTVHeap() const { return rtvHeap_.Get(); }
        ID3D12DescriptorHeap* GetSRVHeap() const { return srvHeap_.Get(); }
        ID3D12DescriptorHeap* GetDSVHeap() const { return dsvHeap_.Get(); }

        uint32_t GetBackBufferIndex() const { return backBufferIndex_; }

        uint32_t GetSRVDescriptorSize() const { return descriptorSizeSRV_; }
        uint32_t GetRTVDescriptorSize() const { return descriptorSizeRTV_; }
        uint32_t GetDSVDescriptorSize() const { return descriptorSizeDSV_; }

        DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }

        D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return rtvDesc_; }

        /// <summary>
        /// ブルームのPSOを取得
        /// </summary>
        /// <param name="pso"></param>
        void SetBloomPSO(BloomPSO* pso) { bloomPSO_ = pso; }

    private:
        DirectXCommon(const DirectXCommon&) = delete;
        DirectXCommon& operator=(const DirectXCommon&) = delete;

        // DirectXデバイスの生成
        void CreateDevice();
        // 描画命令を記録・実行するためのオブジェクト群を生成
        void CreateCommandObjects();
        // 画面表示のための「表裏バッファ」セット（スワップチェーン）を作成
        void CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height);
        // バックバッファ（SwapChain のリソース）に対して描画可能にするビューを作成
        void CreateRenderTargetViews();
        // Z値（深さ）やステンシル情報を書き込むためのバッファとビューを作成
        void CreateDepthStencilView(uint32_t width, uint32_t height);
        // CPU と GPU 間の同期を取るための Fence オブジェクトを作成
        void CreateFence();
        // GPUを待つ処理 
        void WaitForGPU();

        /// <summary>
        /// ブルーム用のRTV,SRVを作成
        /// </summary>
        /// <param name="width">テクスチャの幅</param>
        /// <param name="height">テクスチャの高さ</param>
        void CreateBloomRenderTargets(uint32_t width, uint32_t height);

        /// <summary>
        /// ブルームエフェクトの描画
        /// </summary>
        void DrawBloomEffect();

#ifdef _DEBUG
        void DebugLayer();
#endif

    private:
        static const uint32_t kSwapChainBufferCount = 2;

        Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

        Microsoft::WRL::ComPtr<ID3D12Device> device_;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kSwapChainBufferCount> swapChainResources_;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
        Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

        Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
        uint64_t fenceValue_ = 0;
        HANDLE fenceEvent_ = nullptr;

        uint32_t backBufferIndex_ = 0;
        uint32_t descriptorSizeRTV_ = 0;
        uint32_t descriptorSizeSRV_ = 0;
        uint32_t descriptorSizeDSV_ = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2] = {};

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};

        // ビューポート
        D3D12_VIEWPORT viewport_{};
        // シザー矩形
        D3D12_RECT scissorRect_{};

        // ログ
        LogManager* logManager_;

        // ブルーム用レンダリングターゲット
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBrightResource_;     // 明るい部分抽出用
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomBlurShrinkResource_; // 縮小させながらブラーをする
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomResultResource_;     // 最終敵なもの
        Microsoft::WRL::ComPtr<ID3D12Resource> bloomCompositeResource_;  // 合成用

        // ブラーを掛ける回数
        const uint32_t kBloomIteration = 3;

        // ブルーム用RTV
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> bloomRTVHeap_;
        D3D12_CPU_DESCRIPTOR_HANDLE bloomRTVHandle_[4]{};

        // ブルーム用SRV
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> bloomSRVHeap_;
        // SRVハンドル
        CD3DX12_GPU_DESCRIPTOR_HANDLE bloomSRVHandle_[4];

        // ブルーム用PSO
        BloomPSO* bloomPSO_ = nullptr;

        // ポストエフェクトを適応するかのフラグ
        bool isEnablePostEffect_ = false;
    };
}