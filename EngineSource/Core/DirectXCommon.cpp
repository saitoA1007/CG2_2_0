#include"DirectXCommon.h"
#include <cassert>
#include <string>
#include <format>
#include"externals/DirectXTex/d3dx12.h"

#include"EngineSource/Common/ConvertString.h"
#include"EngineSource/Common/DescriptorHeap.h"
#include"EngineSource/Common/DescriptorHandle.h"
#include"EngineSource/Common/DepthStencilTexture.h"

using namespace Microsoft::WRL;
using namespace GameEngine;

void DirectXCommon::Initialize(HWND hwnd, uint32_t width, uint32_t height, LogManager* logManager)
{
    // ログを所得
    logManager_ = logManager;

    // 初期化を開始するログ
    if (logManager_) {
        logManager_->Log("DirectXCommon Class start Initialize\n");
    }

    // デバイスとコマンドオブジェクトの生成
    CreateDevice();
#ifdef _DEBUG
    DebugLayer();
#endif
    CreateCommandObjects();
    // スワップチェーンの作成
    CreateSwapChain(hwnd, width, height);
    // RTV、DSV、SRVの生成
    CreateRenderTargetViews();
    // オフスクリーンレンダリング用
    CreateOffscreenRenderTarget(width, height);
    CreateDepthStencilView(width, height);
    // フェンスの生成
    CreateFence();

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
    if (logManager_) {
        logManager_->Log("DirectXCommon Class end Initialize\n");
    }
}

void DirectXCommon::PreDraw()
{
	// バリア: オフスクリーンをレンダーターゲットに
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = offscreenRenderTarget_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);

	// 描画先をオフスクリーンRTVに
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &offscreenRTVHandle_, false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 青っぽい色、RGBAの順
	commandList_->ClearRenderTargetView(offscreenRTVHandle_, clearColor, 0, nullptr);
	// 指定した深度で画面全体をクリアする
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList_->RSSetViewports(1, &viewport_); // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_); // Scirssorを設定
}

void DirectXCommon::PostDraw()
{

	// バリア: オフスクリーンをピクセルシェーダリソースに
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = offscreenRenderTarget_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &barrier);

	// バリア: バックバッファをレンダーターゲットに
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	D3D12_RESOURCE_BARRIER bbBarrier{};
	bbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bbBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	bbBarrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
	bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &bbBarrier);

	// バックバッファを描画先に
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex_], false, &dsvHandle);

	// ビューポート/シザー
	commandList_->RSSetViewports(1, &viewport_);
	commandList_->RSSetScissorRects(1, &scissorRect_);

	// ポストプロセス描画
	DrawPostProcess();

	// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	// バリア: バックバッファをPresentに
	bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList_->ResourceBarrier(1, &bbBarrier);

    // コマンドリストの内容を確定させる。すべてのコマンドを積んでからcloseにすること
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    // GPUにコマンドリストの実行を行わせる
    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
    // GPUとOSに画面の交換を行うように通知する
    swapChain_->Present(1, 0);

    // GPUを待つ
    WaitForGPU();

    // 次のフレーム用にコマンドリストを準備
    hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDevice()
{
    // DirectXのデバイスを作成を開始するログ
    if (logManager_) {
        logManager_->Log("Start　Create Device\n");
    }
#ifdef _DEBUG
    ComPtr<ID3D12Debug1> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    // DXGIファクトリーの生成
    // HRESULTはWindow系のエラーコードであり、
    // 関数が成功したかどうかをSUCCEEDEDマクロで判定出来る
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    // 使用するアダプタ用の変数。最初にnullptrを入れておく
    ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    // 良い順にアダプタを頼む
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
        DXGI_ERROR_NOT_FOUND; ++i) {
        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));   // 取得出来ないのが一大事
        // ソフトウェアアダプタでなければ採用
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            // 採用したアダプタの情報をログに出力。wstringの方なので注意
            logManager_->Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }
    assert(useAdapter != nullptr);

    // 機能レベルとログを出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelString[] = { "12.2","12.1","12.0" };
    // 高い順に生成できるかを試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        // 採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
        // 指定した機能レベルでデバイスが生成できたかを確認
        if (SUCCEEDED(hr)) {
            // 生成できたログを出力を行ってループを抜ける
            logManager_->Log(std::format("FeatureLevel : {}\n", featureLevelString[i]));
            break;
        }
    }
    // デバイスの生成がうまくいかなかったので軌道できない
    assert(device_ != nullptr);
    logManager_->Log("Complete create D3D12Device!!!\n");
}

void DirectXCommon::CreateCommandObjects()
{
    // コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    HRESULT hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    // コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータを生成する
    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    // コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // コマンドリストを生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateSwapChain(HWND hwnd, uint32_t width, uint32_t height)
{
    // スワップチェーンを作成を開始するログ
    if (logManager_) {
        logManager_->Log("Start　Create SwapChain\n");
    }
    // スワップチェーンを生成する
    swapChainDesc.Width = width;                       // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc.Height = height;                     // 画面の高さ。ウィンドウのクライアント領域同じものにしておく
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1;  // マルチサンプル
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 描画をターゲットとして利用する
    swapChainDesc.BufferCount = 2;  // ダブルバッファー
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;  // モニタに映ったら、中身を破棄

    // コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(hr));

    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
    // うまく取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
    assert(SUCCEEDED(hr));

    // スワップチェーンを作成を完了するログ
    if (logManager_) {
        logManager_->Log("End　Create SwapChain\n");
    }
}

void DirectXCommon::CreateRenderTargetViews()
{
    // ターゲットビューの作成を開始するログ
    if (logManager_) {
        logManager_->Log("Start　Create TargetView\n");
    }

    // RTVの設定
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
    rtvHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    // SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
    srvHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256, true);
    // DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
    dsvHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    // DescriptorSizeを取得しておく
    descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // RTVを2つ作るのでディスクリプタを2つ用意する(スワップチェーンに使用する画面2つ)
    // まず1つ目を作る。
    rtvHandles_[0] = GetCPUDescriptorHandle(rtvHeap_.Get(), descriptorSizeRTV_, 0);
    device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles_[0]);
    // 2つ目のディスクリプタハンドルを得る(自力で)
    rtvHandles_[1] = GetCPUDescriptorHandle(rtvHeap_.Get(), descriptorSizeRTV_, 1);
    // 2つ目を作る
    device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles_[1]);

    // ターゲットビューの作成を完了するログ
    if (logManager_) {
        logManager_->Log("End　Create TargetView\n");
    }
}

void DirectXCommon::CreateDepthStencilView(uint32_t width, uint32_t height)
{
    // ステンシル情報の作成を開始するログ
    if (logManager_) {
        logManager_->Log("Start　Create DepthStencilView\n");
    }

    // DepthStencilTextureをウィンドウのサイズで作成
    depthStencilResource_ = CreateDepthStencilTextureResource(device_.Get(), width, height);

    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    // DSVHeapの先頭にDSVを作る
    device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());

    // ステンシル情報の作成を開始するログ
    if (logManager_) {
        logManager_->Log("End　Create DepthStencilView\n");
    }
}

void DirectXCommon::CreateFence()
{
    // フェンスの作成を開始するログ
    if (logManager_) {
        logManager_->Log("Start　Create Fence\n");
    }

    // 初期値0でFenceを作る
    HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    // FenceのSignalを待つためのイベントを作成する
    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);

    // フェンスの作成を完了するログ
    if (logManager_) {
        logManager_->Log("End　Create Fence\n");
    }
}

void DirectXCommon::WaitForGPU()
{
    // GPUの完了を待つ
    // Fenceの値を更新
    fenceValue_++;
    // GPUがここまでたどり着いた時に、Fenceの値を指定した値を代入するようにSignalを送る
    commandQueue_->Signal(fence_.Get(), fenceValue_);

    // Fenceの値が指定したSignal値にたどり着いているかを確認する
    // GetCompletedValueの初期値はFence作成時に渡した初期値
    if (fence_->GetCompletedValue() < fenceValue_) {
        // 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
        fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        // イベントを待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void DirectXCommon::CreateOffscreenRenderTarget(uint32_t width, uint32_t height) {

    // オフスクリーンレンダリング用のターゲットビューを作成
    if (logManager_) {
        logManager_->Log("Start　Create OffscreenRenderTarget\n");
    }

    // テクスチャリソース作成
    D3D12_RESOURCE_DESC desc{};
    desc.Width = width;   // テクスチャの幅
    desc.Height = height; // テクスチャの高さ
    desc.MipLevels = 1;  // mipMapの数
    desc.DepthOrArraySize = 1; //  奥行 or 配列Textureの配列数
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // TextureのFormat
    desc.SampleDesc.Count = 1; // ダンプリングのカウント
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // Textureの次元数
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // Clearの最適値
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = desc.Format;
    clearValue.Color[0] = 0.0f; clearValue.Color[1] = 0.0f; clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    // Resourceの生成
    HRESULT hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし
        &desc, // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue, // Clearの最適値
        IID_PPV_ARGS(&offscreenRenderTarget_)); // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    // RTVヒープ
    offscreenRTVHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
    offscreenRTVHandle_ = offscreenRTVHeap_->GetCPUDescriptorHandleForHeapStart();
    device_->CreateRenderTargetView(offscreenRenderTarget_.Get(), nullptr, offscreenRTVHandle_);

    // SRVヒープ
    offscreenSRVHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    offscreenSRVHandle_ = offscreenSRVHeap_->GetGPUDescriptorHandleForHeapStart();
    device_->CreateShaderResourceView(offscreenRenderTarget_.Get(), &srvDesc, offscreenSRVHeap_->GetCPUDescriptorHandleForHeapStart());

    // オフスクリーンレンダリング用のターゲットビューを作成を完了するログ
    if (logManager_) {
        logManager_->Log("End　Create OffscreenRenderTarget\n");
    }
}

void DirectXCommon::DrawPostProcess() {
    if (!postProcessPSO_) { return; }
    // SRVヒープセット
    ID3D12DescriptorHeap* heaps[] = { offscreenSRVHeap_.Get() };
    commandList_->SetDescriptorHeaps(1, heaps);
    postProcessPSO_->Set(commandList_.Get());
    postProcessPSO_->Draw(commandList_.Get(), offscreenSRVHandle_);
}

void DirectXCommon::SetIsEnablePostEffect(const bool& isEnable) {
    // ポストエフェクトの有効化を設定
    postProcessPSO_->SetIsEnablePostEffect(isEnable);
}

#ifdef _DEBUG
void DirectXCommon::DebugLayer() {

    // InfoQueueによるエラーチェックの設定
    ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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