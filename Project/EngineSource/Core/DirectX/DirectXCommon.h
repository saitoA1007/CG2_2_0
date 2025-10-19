#pragma once
#include<chrono>
#include<thread>
#include "DXDevice.h"
#include "DXCommand.h"
#include "DXSwapChain.h"
#include "DXRenderTarget.h"
#include "DXDepthStencil.h"
#include "DXFence.h"
#include <fstream>
#include"LogManager.h"
#include"Externals/DirectXTex/d3dx12.h"

#include"PostProcess/CopyPSO.h"

#include"PostProcess/PostEffectManager.h"

#include"SrvManager.h"

namespace GameEngine {

    // 前方宣言
    class ImGuiManager;

    class DirectXCommon final {
    public:
        DirectXCommon() = default;
        ~DirectXCommon() = default;

        // 初期化
        void Initialize(HWND hwnd, uint32_t width, uint32_t height,SrvManager* srvManager);
        // 描画前処理
        void PreDraw();
        // 描画後処理
        void PostDraw(ImGuiManager* imGuiManager);

    public:
        ID3D12Device* GetDevice() const { return device_->GetDevice(); }
        ID3D12GraphicsCommandList* GetCommandList() const { return command_->GetCommandList(); }
        IDXGISwapChain4* GetSwapChain() const { return swapChain_->GetSwapChain(); }

        ID3D12DescriptorHeap* GetRTVHeap() const { return renderTarget_->GetRTVHeap(); }
        ID3D12DescriptorHeap* GetDSVHeap() const { return renderTarget_->GetDSVHeap(); }

        uint32_t GetRTVDescriptorSize() const { return renderTarget_->GetRTVDescriptorSize(); }
        uint32_t GetDSVDescriptorSize() const { return renderTarget_->GetDSVDescriptorSize(); }

        DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChain_->GetSwapChainDesc(); }

        /// <summary>
        /// 画像コピー用のPSOを取得
        /// </summary>
        /// <param name="copyPSO"></param>
        void SetCopyPSO(CopyPSO* copyPSO) { copyPSO_ = copyPSO; }

        CD3DX12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle() { return postEffectManager_->GetSRVHandle(); }

    private:
        DirectXCommon(const DirectXCommon&) = delete;
        DirectXCommon& operator=(const DirectXCommon&) = delete;

#ifdef _DEBUG
        void DebugLayer(ID3D12Device* device);
#endif

    private:
        std::unique_ptr<DXDevice> device_;
        std::unique_ptr<DXCommand> command_;
        std::unique_ptr<DXSwapChain> swapChain_;
        std::unique_ptr<DXRenderTarget> renderTarget_;
        std::unique_ptr<DXDepthStencil> depthStencil_;
        std::unique_ptr<DXFence> fence_;

        // ビューポート
        D3D12_VIEWPORT viewport_{};
        // シザー矩形
        D3D12_RECT scissorRect_{};

        // コピー用PSO
        CopyPSO* copyPSO_ = nullptr;

        // 画面クリアの色
        float clearColor_[4] = { 0.2f,0.2f,0.2f,1.0f };
        //float clearColor_[4] = { 1.0f,0.0f,1.0f,1.0f };

        // ポストエフェクト
        std::unique_ptr<PostEffectManager> postEffectManager_;

        // 記録時間(FPS固定用)
        std::chrono::steady_clock::time_point reference_;

    private:

        // FPS固定初期化
        void InitializeFixFPS();

        // FPS固定更新
        void UpdateFixFPS();
    };
}