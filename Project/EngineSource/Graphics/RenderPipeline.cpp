#include "RenderPipeline.h"
#include "ImGuiManager.h"
#include "LogManager.h"

using namespace GameEngine;

void RenderPipeline::Initialize(GraphicsDevice* graphicsDevice, PostEffectManager* postEffectManager) {
    LogManager::GetInstance().Log("RenderPipeline start Initialize");

    // DirectXのコア機能を取得
    graphicsDevice_ = graphicsDevice;

    // FPS固定初期化
    frameRateController_ = std::make_unique<FrameRateController>();
    frameRateController_->InitializeFixFPS();

    // レンダラーマネージャーの初期化
    rendererManager_ = std::make_unique<RendererManager>();
    rendererManager_->Initialize(graphicsDevice_, postEffectManager);

    LogManager::GetInstance().Log("RenderPipeline end Initialize\n");
}

void RenderPipeline::BeginFrame() {
    // レンダリング開始処理
    rendererManager_->BeginFrame();
}

void RenderPipeline::EndFrame(ImGuiManager* imGuiManager) {
    // レンダリング終了処理
    rendererManager_->EndFrame(imGuiManager);

    // コマンドリストの内容を確定させる。すべてのコマンドを積んでからcloseにすること
    graphicsDevice_->CloseCommandList();
    // GPUにコマンドリストの実行を行わせる
    graphicsDevice_->ExecuteCommand();

    // GPUとOSに画面の交換を行うように通知する
    graphicsDevice_->Present();

    // GPUを待つ
    graphicsDevice_->WaitForGPU();

    // 次のフレーム用にコマンドリストを準備
    graphicsDevice_->ResetCommandList();

    // FPS固定
    frameRateController_->UpdateFixFPS();
}