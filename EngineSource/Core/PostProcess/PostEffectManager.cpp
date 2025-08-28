#include"PostEffectManager.h"
#include"EngineSource/Common/DescriptorHeap.h"
#include"EngineSource/Common/DescriptorHandle.h"

using namespace GameEngine;

BloomPSO* PostEffectManager::bloomPSO_ = nullptr;
LogManager* PostEffectManager::logManager_ = nullptr;
ScanLinePSO* PostEffectManager::scanLinePSO_ = nullptr;
VignettingPSO* PostEffectManager::vignettingPSO_ = nullptr;
RadialBlurPSO* PostEffectManager::radialBlurPSO_ = nullptr;

void PostEffectManager::StaticInitialize(BloomPSO* bloomPSO, ScanLinePSO* scanLinePSO, VignettingPSO* vignettingPSO, RadialBlurPSO* radialBlurPSO, LogManager* logManager) {
    bloomPSO_ = bloomPSO;
    scanLinePSO_ = scanLinePSO;
    vignettingPSO_ = vignettingPSO;
    radialBlurPSO_ = radialBlurPSO;

    // ログを取得
    logManager_ = logManager;
}

void PostEffectManager::Initialize(ID3D12Device* device, float clearColor_[4], uint32_t width, uint32_t height, uint32_t descriptorSizeRTV, uint32_t descriptorSizeSRV, ID3D12DescriptorHeap* srvHeap) {

    // SRVヒープを取得
    srvHeap_ = srvHeap;

    // デバイスを取得
    device_ = device;

    // ポストエフェクトのRTV用ヒープ作成
    postProcessRTVHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kRTVNum, false);

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
    clearValue_.Format = desc.Format;
    clearValue_.Color[0] = clearColor_[0]; clearValue_.Color[1] = clearColor_[1]; clearValue_.Color[2] = clearColor_[2]; clearValue_.Color[3] = clearColor_[3];

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr;

    // オブジェクトを描画する用のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE,  // Heapの特殊な設定。特になし
        &desc,  // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue_, // Clearの最適値
        IID_PPV_ARGS(&DrawObjectResource_));  // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // オブジェクト描画用RTVを作成
    drawObjectRTVHandle_ = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, rtvIndex_);
    device_->CreateRenderTargetView(DrawObjectResource_.Get(), &rtvDesc, drawObjectRTVHandle_);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // SRVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    drawObjectSRVHandle_ = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    // オブジェクト描画用SRV
    device_->CreateShaderResourceView(DrawObjectResource_.Get(), &srvDesc, srvCPUHandle);

    // ブルームの初期化
    InitializeBloom(width, height, descriptorSizeSRV, descriptorSizeRTV);

    // ヴィネットの初期化
    InitializeVignetting(width, height, descriptorSizeSRV, descriptorSizeRTV);

    // scanLineの初期化
    InitializeScanLine(width, height, descriptorSizeSRV, descriptorSizeRTV);

    // ラジアルブルーの初期化
    InitializeRadialBlur(width, height, descriptorSizeSRV, descriptorSizeRTV);
}

void PostEffectManager::PreDraw(ID3D12GraphicsCommandList* commandList,const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect, float clearColor[4], ID3D12DescriptorHeap* dsvHeap) {

    // バリア: オフスクリーンをレンダーターゲットに
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = DrawObjectResource_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // 描画先をオフスクリーンRTVに
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &drawObjectRTVHandle_, false, &dsvHandle);

    // 指定した色で画面全体をクリアする
    commandList->ClearRenderTargetView(drawObjectRTVHandle_, clearColor, 0, nullptr);
    // 指定した深度で画面全体をクリアする
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList->RSSetViewports(1, &viewport); // Viewportを設定
    commandList->RSSetScissorRects(1, &scissorRect); // Scirssorを設定
}

void PostEffectManager::PostDraw(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect) {
    D3D12_RESOURCE_BARRIER PostProcessBarrier{};
    PostProcessBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    PostProcessBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    PostProcessBarrier.Transition.pResource = DrawObjectResource_.Get();
    PostProcessBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    PostProcessBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &PostProcessBarrier);

    /// ここにポストエフェクトの描画処理を記入===================

    // ブルームを描画
    DrawBloom(commandList, viewport, scissorRect);

    // ヴィネットの描画
    DrawVignetting(commandList);

    switch (drawMode_)
    {
    case GameEngine::PostEffectManager::DrawMode::Default:
        break;

    case GameEngine::PostEffectManager::DrawMode::ScanLine:
        // scanLineを描画
        DrawScanLine(commandList);
        break;

    case GameEngine::PostEffectManager::DrawMode::RadialBlur:
        // ラジアルブルーを描画
        DrawRadialBlur(commandList);
        break;
    }
}

CD3DX12_GPU_DESCRIPTOR_HANDLE& PostEffectManager::GetSRVHandle() { 

    // 描画形態に応じて返すSRVを変える
    switch (drawMode_)
    {
    case GameEngine::PostEffectManager::DrawMode::Default:
        return vignettingSRVHandle_;
        break;

    case GameEngine::PostEffectManager::DrawMode::ScanLine:
        return scanLineSRVHandle_;
        break;

    case GameEngine::PostEffectManager::DrawMode::RadialBlur:
        return radialBlurSRVHandle_;
        break;
    }

    return vignettingSRVHandle_;
}

void PostEffectManager::InitializeBloom(uint32_t width, uint32_t height, uint32_t descriptorSizeSRV, uint32_t descriptorSizeRTV) {
    if (logManager_) {
        logManager_->Log("Start Create BloomRenderTargets\n");
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

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    // 5つのブルーム用レンダーターゲットを作成
    HRESULT hr;

    // 明るい部分抽出用のリソースを作成 D3D12_RESOURCE_STATE_RENDER_TARGET
    hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE,  // Heapの特殊な設定。特になし
        &desc,  // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue_, // Clearの最適値
        IID_PPV_ARGS(&bloomBrightResource_));  // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    // 縮小しながらブラーする用のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue_, IID_PPV_ARGS(&bloomBlurShrinkResource_));
    assert(SUCCEEDED(hr));

    // ブラーした結果を描画のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue_, IID_PPV_ARGS(&bloomResultResource_));
    assert(SUCCEEDED(hr));

     // 合成したものを描画のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue_, IID_PPV_ARGS(&bloomCompositeResource_));
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // 2つ目
    bloomRTVHandle_[0] = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(bloomBrightResource_.Get(), &rtvDesc, bloomRTVHandle_[0]);
    // 3つ目
    bloomRTVHandle_[1] = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(bloomBlurShrinkResource_.Get(), &rtvDesc, bloomRTVHandle_[1]);
    // 4つ目
    bloomRTVHandle_[2] = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(bloomResultResource_.Get(), &rtvDesc, bloomRTVHandle_[2]);
    // 5つ目
    bloomRTVHandle_[3] = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(bloomCompositeResource_.Get(), &rtvDesc, bloomRTVHandle_[3]);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);

    bloomSRVHandle_[0] = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    device_->CreateShaderResourceView(bloomBrightResource_.Get(), &srvDesc, srvCPUHandle);

    // ブラー2回目用SRV
    srvCPUHandle.ptr += descriptorSizeSRV;
    bloomSRVHandle_[1] = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    device_->CreateShaderResourceView(bloomBlurShrinkResource_.Get(), &srvDesc, srvCPUHandle);

    // ブラー3回目用SRV
    srvCPUHandle.ptr += descriptorSizeSRV;
    bloomSRVHandle_[2] = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    device_->CreateShaderResourceView(bloomResultResource_.Get(), &srvDesc, srvCPUHandle);

    // ブラー4回目用SRV
    srvCPUHandle.ptr += descriptorSizeSRV;
    bloomSRVHandle_[3] = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    device_->CreateShaderResourceView(bloomCompositeResource_.Get(), &srvDesc, srvCPUHandle);

    if (logManager_) {
        logManager_->Log("End Create BloomRenderTargets\n");
    }
}

void PostEffectManager::DrawBloom(ID3D12GraphicsCommandList* commandList, const D3D12_VIEWPORT& baseViewport, const D3D12_RECT& baseScissorRect) {
    // SRVヒープをセット
    ID3D12DescriptorHeap* heaps[] = { srvHeap_ };
    commandList->SetDescriptorHeaps(1, heaps);

    // ルート
    commandList->SetGraphicsRootSignature(bloomPSO_->GetRootSignature());

    // 明るい部分を抽出
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース
    barrier.Transition.pResource = bloomBrightResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    commandList->SetPipelineState(bloomPSO_->GetBrightPipelineState());
    commandList->OMSetRenderTargets(1, &bloomRTVHandle_[0], false, nullptr);

    commandList->SetGraphicsRootConstantBufferView(1, bloomPSO_->GetBloomParameterResource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, drawObjectSRVHandle_);

    // 明るい部分抽出の描画
    bloomPSO_->Draw(commandList, bloomSRVHandle_[0]);

    // bloomBrightResourceをSRVに戻す
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移

    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);

    ///=============================

    // 縮小させながらブラー
    barrier.Transition.pResource = bloomBlurShrinkResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    auto desc = bloomBrightResource_->GetDesc();
    D3D12_VIEWPORT viewport = {};
    D3D12_RECT rect = {};
    // 描画サイズ
    viewport.Height = static_cast<FLOAT>(desc.Height / 2);
    viewport.Width = static_cast<FLOAT>(desc.Width / 2);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    // rect
    rect.left = 0;
    rect.right = static_cast<LONG>(viewport.Width);
    rect.top = 0;
    rect.bottom = static_cast<LONG>(viewport.Height);

    // レンダーターゲット設定
    commandList->SetPipelineState(bloomPSO_->GetBlurPipelineState());
    commandList->OMSetRenderTargets(1, &bloomRTVHandle_[1], false, nullptr);

    // 縮小させながらブラーをかけたものを描画
    for (uint32_t i = 0; i < bloomPSO_->GetBloomIteration(); ++i) {
        commandList->RSSetViewports(1, &viewport); // Viewportを設定
        commandList->RSSetScissorRects(1, &rect); // Scirssorを設定
        commandList->SetGraphicsRootDescriptorTable(2, drawObjectSRVHandle_);
        bloomPSO_->Draw(commandList, bloomSRVHandle_[0]);

        rect.top += static_cast<LONG>(viewport.Height);
        viewport.TopLeftX = 0;
        viewport.TopLeftY = static_cast<FLOAT>(rect.top);

        viewport.Width /= 2;
        viewport.Height /= 2;
        rect.bottom = rect.top + static_cast<LONG>(viewport.Height);
    }

    // bloomBlurShrinkResourceをSRVに戻す
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);

    //=================================

    // ブラーの最終結果
    barrier.Transition.pResource = bloomResultResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // ブラーの最終結果を描画
    commandList->SetPipelineState(bloomPSO_->GetResultPipelineState());
    commandList->OMSetRenderTargets(1, &bloomRTVHandle_[2], false, nullptr);
    // ビューポートを元に戻す
    commandList->RSSetViewports(1, &baseViewport);
    commandList->RSSetScissorRects(1, &baseScissorRect);

    commandList->SetGraphicsRootConstantBufferView(1, bloomPSO_->GetBloomParameterResource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, drawObjectSRVHandle_);
    bloomPSO_->Draw(commandList, bloomSRVHandle_[0]);

    // bloomResultResourceをSRVに戻す
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);

    //====================================-

    // 合成用のリソースがある場合はバリアを設定
    barrier.Transition.pResource = bloomCompositeResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // 合成したのを描画
    commandList->SetPipelineState(bloomPSO_->GetBlurCompositePipelineState());
    commandList->OMSetRenderTargets(1, &bloomRTVHandle_[3], false, nullptr);
    commandList->SetGraphicsRootConstantBufferView(1, bloomPSO_->GetBloomParameterResource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, drawObjectSRVHandle_);
    bloomPSO_->Draw(commandList, bloomSRVHandle_[0]);

    // 最終的にすべてのブルームリソースをSRVに戻す
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}

void PostEffectManager::InitializeScanLine(uint32_t width, uint32_t height, uint32_t descriptorSizeSRV, uint32_t descriptorSizeRTV) {
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

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr;

    // オブジェクトを描画する用のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE,  // Heapの特殊な設定。特になし
        &desc,  // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue_, // Clearの最適値
        IID_PPV_ARGS(&scanLineResource_));  // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // オブジェクト描画用RTVを作成
    scanLineRTVHandle_ = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(scanLineResource_.Get(), &rtvDesc, scanLineRTVHandle_);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // SRVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    scanLineSRVHandle_ = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    // オブジェクト描画用SRV
    device_->CreateShaderResourceView(scanLineResource_.Get(), &srvDesc, srvCPUHandle);
}

void PostEffectManager::DrawScanLine(ID3D12GraphicsCommandList* commandList) {
   
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース
    barrier.Transition.pResource = scanLineResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    commandList->SetGraphicsRootSignature(scanLinePSO_->GetRootSignature());
    commandList->SetPipelineState(scanLinePSO_->GetPipelineState());
    commandList->OMSetRenderTargets(1, &scanLineRTVHandle_, false, nullptr);

    // ラインを描画
    scanLinePSO_->Draw(commandList, vignettingSRVHandle_);

    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}

void PostEffectManager::InitializeVignetting(uint32_t width, uint32_t height, uint32_t descriptorSizeSRV, uint32_t descriptorSizeRTV) {
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

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr;

    // オブジェクトを描画する用のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE,  // Heapの特殊な設定。特になし
        &desc,  // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue_, // Clearの最適値
        IID_PPV_ARGS(&vignettingResource_));  // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // オブジェクト描画用RTVを作成
    vignettingRTVHandle_ = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(vignettingResource_.Get(), &rtvDesc, vignettingRTVHandle_);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // SRVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    vignettingSRVHandle_ = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    // オブジェクト描画用SRV
    device_->CreateShaderResourceView(vignettingResource_.Get(), &srvDesc, srvCPUHandle);
}

void PostEffectManager::DrawVignetting(ID3D12GraphicsCommandList* commandList) {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース
    barrier.Transition.pResource = vignettingResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    commandList->SetGraphicsRootSignature(vignettingPSO_->GetRootSignature());
    commandList->SetPipelineState(vignettingPSO_->GetPipelineState());
    commandList->OMSetRenderTargets(1, &vignettingRTVHandle_, false, nullptr);

    // ヴィネットを描画
    vignettingPSO_->Draw(commandList, bloomSRVHandle_[3]);

    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}

void PostEffectManager::InitializeRadialBlur(uint32_t width, uint32_t height, uint32_t descriptorSizeSRV, uint32_t descriptorSizeRTV) {
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

    // 利用するHeapの設定。
    CD3DX12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr;

    // オブジェクトを描画する用のリソースを作成
    hr = device_->CreateCommittedResource(
        &heapProps, // Heapの設定
        D3D12_HEAP_FLAG_NONE,  // Heapの特殊な設定。特になし
        &desc,  // Resourceの設定
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // データの転送される設定
        &clearValue_, // Clearの最適値
        IID_PPV_ARGS(&radialBlurResource_));  // 作成するResourceポインタへのポインタ
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // オブジェクト描画用RTVを作成
    radialBlurRTVHandle_ = GetCPUDescriptorHandle(postProcessRTVHeap_.Get(), descriptorSizeRTV, ++rtvIndex_);
    device_->CreateRenderTargetView(radialBlurResource_.Get(), &rtvDesc, radialBlurRTVHandle_);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // SRVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV, ++srvIndex_);
    radialBlurSRVHandle_ = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV, srvIndex_);
    // オブジェクト描画用SRV
    device_->CreateShaderResourceView(radialBlurResource_.Get(), &srvDesc, srvCPUHandle);
}

void PostEffectManager::DrawRadialBlur(ID3D12GraphicsCommandList* commandList) {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    // バリアを張る対象のリソース
    barrier.Transition.pResource = radialBlurResource_.Get();
    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    commandList->SetGraphicsRootSignature(radialBlurPSO_->GetRootSignature());
    commandList->SetPipelineState(radialBlurPSO_->GetPipelineState());
    commandList->OMSetRenderTargets(1, &radialBlurRTVHandle_, false, nullptr);

    // ラジアルブルーを描画
    radialBlurPSO_->Draw(commandList, vignettingSRVHandle_);

    // 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}