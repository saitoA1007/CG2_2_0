#include"DirectXCommon.h"
#include <cassert>
#include <string>
#include <format>
#include"Externals/DirectXTex/d3dx12.h"

#include"EngineSource/2D/ImGuiManager.h"

using namespace Microsoft::WRL;
using namespace GameEngine;

void DirectXCommon::Initialize(HWND hwnd, uint32_t width, uint32_t height, SrvManager* srvManager)
{

    // 初期化を開始するログ
    LogManager::GetInstance().Log("DirectXCommon Class start Initialize\n");
    
    // デバイスを生成
    device_ = std::make_unique<DXDevice>();
    device_->Initialize();

#ifdef _DEBUG
    DebugLayer(device_->GetDevice());
#endif

    command_ = std::make_unique<DXCommand>();
    command_->Initialize(device_->GetDevice());
    // スワップチェーンの作成
    swapChain_ = std::make_unique<DXSwapChain>();
    swapChain_->Initialize(hwnd, width, height, device_->GetFactory(), command_->GetQueue());

    // SRVを生成する
    srvManager->Initialize(device_->GetDevice());

    // RTV、DSVの生成
    renderTarget_ = std::make_unique<DXRenderTarget>();
    renderTarget_->Initialize(device_->GetDevice(), swapChain_->GetSwapChain());

    // ポストエフェクトの初期化
    postEffectManager_ = std::make_unique<PostEffectManager>();
    postEffectManager_->Initialize(device_->GetDevice(), clearColor_, width, height, renderTarget_->GetRTVDescriptorSize(), srvManager);

    depthStencil_ = std::make_unique<DXDepthStencil>();
    depthStencil_->Initialize(device_->GetDevice(), renderTarget_->GetDSVHeap(), width, height, srvManager);
    // フェンスの生成
    fence_ = std::make_unique<DXFence>();
    fence_->Initialize(device_->GetDevice());

   
    // クライアント領域のサイズと一緒にして画面全体に表示
    viewport_.Width = static_cast<float>(width);
    viewport_.Height = static_cast<float>(height);
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;

    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect_.left = 0;
    scissorRect_.right = static_cast<int>(width);
    scissorRect_.top = 0;
    scissorRect_.bottom = static_cast<int>(height);

    // 初期化を終了するログ
    LogManager::GetInstance().Log("DirectXCommon Class end Initialize\n");
}

void DirectXCommon::PreDraw()
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = depthStencil_->GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // SRV として使った後
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;           // 深度書き込みに戻す
    command_->GetCommandList()->ResourceBarrier(1, &barrier);

    postEffectManager_->PreDraw(command_->GetCommandList(), viewport_, scissorRect_, clearColor_, renderTarget_->GetDSVHeap());
}

void DirectXCommon::PostDraw(ImGuiManager* imGuiManager)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = depthStencil_->GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    command_->GetCommandList()->ResourceBarrier(1, &barrier);

    postEffectManager_->PostDraw(command_->GetCommandList(), viewport_, scissorRect_, depthStencil_->GetSRVHandle());

    // バリア: バックバッファをレンダーターゲットに
    uint32_t backBufferIndex = swapChain_->GetBackBufferIndex();
    D3D12_RESOURCE_BARRIER SwapchainBarrier{};
    SwapchainBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    SwapchainBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    SwapchainBarrier.Transition.pResource = renderTarget_->GetSwapChainResource(backBufferIndex);
    SwapchainBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    SwapchainBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    command_->GetCommandList()->ResourceBarrier(1, &SwapchainBarrier);

    // バックバッファを描画先に
    auto rtvHandle = renderTarget_->GetSwapChainRTVHandle(backBufferIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = renderTarget_->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    command_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
    //float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 青っぽい色、RGBAの順
    //commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex_], clearColor, 0, nullptr);

    // ビューポート/シザー
    command_->GetCommandList()->RSSetViewports(1, &viewport_);
    command_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

#ifdef _DEBUG
    // ImGuiを描画
    imGuiManager->Draw();
#else
    // ポストプロセスを行った最終結果を描画する
    copyPSO_->Draw(commandList_.Get(), postEffectManager_->GetSRVHandle());

#endif

    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    // バリア: バックバッファをPresentに
    SwapchainBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    SwapchainBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    command_->GetCommandList()->ResourceBarrier(1, &SwapchainBarrier);

    // コマンドリストの内容を確定させる。すべてのコマンドを積んでからcloseにすること
    command_->Close();
    // GPUにコマンドリストの実行を行わせる
    command_->Execute();

    // GPUとOSに画面の交換を行うように通知する
    swapChain_->Present();

    // GPUを待つ
    fence_->WaitForGPU(command_->GetQueue());

    // 次のフレーム用にコマンドリストを準備
    command_->Reset();
}

#ifdef _DEBUG
void DirectXCommon::DebugLayer(ID3D12Device* device) {

    // InfoQueueによるエラーチェックの設定
    ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        // ヤバいエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        // エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

        // 警告時に止まる
        // infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        // 抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            // Window11でのDXGIデバッカーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
//			// https://stackoverflow.com/questions/69885245/directx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        // 指定したメッセージの表示を抑制する
        infoQueue->PushStorageFilter(&filter);
    }
}
#endif