#pragma once
#include"GraphicsDevice.h"

#include"PostProcess/CopyPSO.h"
#include"PostProcess/PostEffectManager.h"

namespace GameEngine {

    // 前方宣言
    class ImGuiManager;

	class RendererManager {
	public:
		RendererManager() = default;
		~RendererManager() = default;

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="graphicsDevice"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="srvManager"></param>
		void Initialize(GraphicsDevice* graphicsDevice, uint32_t width, uint32_t height, SrvManager* srvManager);

        /// <summary>
        /// 描画前処理
        /// </summary>
        void BeginFrame();

        /// <summary>
        /// 描画後処理
        /// </summary>
        /// <param name="imGuiManager"></param>
        void EndFrame(ImGuiManager* imGuiManager);

        /// <summary>
        /// 深度バッファのリソースバリアを設定。SRVから深度書き込みへ遷移
        /// </summary>
        void TransitionDepthToWrite();

        /// <summary>
        /// 深度バッファのリソースバリアを設定。深度書き込みからSRVへ遷移
        /// </summary>
        void TransitionDepthToRead();

        /// <summary>
        /// バックバッファのリソースバリアを設定
        /// </summary>
        /// <param name="stateBefore">遷移前の状態</param>
        /// <param name="stateAfter">遷移後の状態</param>
        void TransitionBackBuffer(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

    public:
        /// <summary>
        /// PostEffect用のPSOを設定
        /// </summary>
        /// <param name="copyPSO"></param>
        void SetCopyPSO(CopyPSO* copyPSO) { copyPSO_ = copyPSO; }

        /// <summary>
        /// 最終的な描画先のsrvHandleを取得
        /// </summary>
        /// <returns></returns>
        CD3DX12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle() { return postEffectManager_->GetSRVHandle(); }

    private:
        RendererManager(const RendererManager&) = delete;
        RendererManager& operator=(const RendererManager&) = delete;

        // DirectXのコア機能を取得
        GraphicsDevice* graphicsDevice_ = nullptr;

        // ポストエフェクト
        std::unique_ptr<PostEffectManager> postEffectManager_;

        // コピー用PSO
        CopyPSO* copyPSO_ = nullptr;

        // 画面クリアの色
        float clearColor_[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        //float clearColor_[4] = { 1.0f,0.0f,1.0f,1.0f };
	};
}
