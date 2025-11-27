#include"RendererManager.h"
#include "LogManager.h"
#include"ImGuiManager.h"

using namespace GameEngine;

void RendererManager::Initialize(GraphicsDevice* graphicsDevice, uint32_t width, uint32_t height, SrvManager* srvManager) {
    LogManager::GetInstance().Log("RendererManager start Initialize");

    graphicsDevice_ = graphicsDevice;

    // ポストエフェクトマネージャーの初期化
    postEffectManager_ = std::make_unique<PostEffectManager>();
    postEffectManager_->Initialize(graphicsDevice_->GetDevice(),clearColor_,width,height,graphicsDevice_->GetRTVDescriptorSize(),srvManager);

    LogManager::GetInstance().Log("RendererManager end Initialize\n");
}

void RendererManager::BeginFrame() {

    // 深度バッファをSRVから深度書き込みに遷移
    TransitionDepthToWrite();

    // ポストエフェクト用レンダーターゲットへの描画開始
    postEffectManager_->PreDraw(graphicsDevice_->GetCommandList(),graphicsDevice_->GetViewport(),graphicsDevice_->GetScissorRect(),clearColor_,graphicsDevice_->GetDSVHeap());
}

void RendererManager::EndFrame(ImGuiManager* imGuiManager) {
    // 深度バッファを深度書き込みからSRVに遷移
    TransitionDepthToRead();

    // ポストエフェクトの描画
    postEffectManager_->PostDraw(graphicsDevice_->GetCommandList(),graphicsDevice_->GetViewport(),graphicsDevice_->GetScissorRect(),graphicsDevice_->GetDepthStencilSRVHandle());

    // バックバッファをレンダーターゲットに遷移
    TransitionBackBuffer(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // バックバッファを描画先に設定
    uint32_t backBufferIndex = graphicsDevice_->GetBackBufferIndex();
    auto rtvHandle = graphicsDevice_->GetSwapChainRTVHandle(backBufferIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = graphicsDevice_->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    graphicsDevice_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
    graphicsDevice_->GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);

    // ビューポート、シザーを設定
    graphicsDevice_->GetCommandList()->RSSetViewports(1, &graphicsDevice_->GetViewport());
    graphicsDevice_->GetCommandList()->RSSetScissorRects(1, &graphicsDevice_->GetScissorRect());

#ifdef _DEBUG
    // ImGuiを描画
    imGuiManager->Draw();
#else
    // ポストプロセス結果を描画
    copyPSO_->Draw(graphicsDevice_->GetCommandList(), postEffectManager_->GetSRVHandle());
#endif

    // バックバッファをPresentに遷移
    TransitionBackBuffer(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void RendererManager::TransitionDepthToWrite() {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = graphicsDevice_->GetDepthStencilResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    graphicsDevice_->GetCommandList()->ResourceBarrier(1, &barrier);
}

void RendererManager::TransitionDepthToRead() {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = graphicsDevice_->GetDepthStencilResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    graphicsDevice_->GetCommandList()->ResourceBarrier(1, &barrier);
}

void RendererManager::TransitionBackBuffer(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) {
    uint32_t backBufferIndex = graphicsDevice_->GetBackBufferIndex();
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = graphicsDevice_->GetSwapChainResource(backBufferIndex);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter = stateAfter;
    graphicsDevice_->GetCommandList()->ResourceBarrier(1, &barrier);
}