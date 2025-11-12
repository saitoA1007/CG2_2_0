#pragma once
#include "GraphicsDevice.h"
#include "RendererManager.h"
#include "FrameRateController.h"
#include "SrvManager.h"
#include <Windows.h>

namespace GameEngine {

    // 前方宣言
    class ImGuiManager;
    class CopyPSO;

    /// <summary>
    /// レンダリングパイプライン全体を総合管理するクラス
    /// </summary>
    class RenderPipeline final {
    public:
        RenderPipeline() = default;
        ~RenderPipeline() = default;

        /// <summary>
        /// レンダリングパイプラインの初期化
        /// </summary>
        /// <param name="hwnd">ウィンドウハンドル</param>
        /// <param name="width">画面幅</param>
        /// <param name="height">画面高さ</param>
        /// <param name="srvManager">SRVマネージャー</param>
        void Initialize(uint32_t width, uint32_t height, SrvManager* srvManager, GraphicsDevice* graphicsDevice);

        /// <summary>
        /// 描画開始処理
        /// </summary>
        void BeginFrame();

        /// <summary>
        /// 描画終了処理
        /// </summary>
        /// <param name="imGuiManager"></param>
        void EndFrame(ImGuiManager* imGuiManager);

    public:
        RendererManager* GetRendererManager() const { return rendererManager_.get(); }

        void SetCopyPSO(CopyPSO* copyPSO) { rendererManager_->SetCopyPSO(copyPSO); }

        CD3DX12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle() { return rendererManager_->GetSRVHandle(); }

    private:
        RenderPipeline(const RenderPipeline&) = delete;
        RenderPipeline& operator=(const RenderPipeline&) = delete;

        GraphicsDevice* graphicsDevice_ = nullptr;
        std::unique_ptr<RendererManager> rendererManager_;
        std::unique_ptr<FrameRateController> frameRateController_;
    };
}