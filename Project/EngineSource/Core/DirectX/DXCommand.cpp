#include"DXCommand.h"
#include<cassert>
#include"LogManager.h"

using namespace GameEngine;

void DXCommand::Initialize(ID3D12Device* device) {

    LogManager::GetInstance().Log("Start　Create CommandList\n");

    // コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    // コマンドキューの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータを生成する
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    // コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    // コマンドリストを生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    LogManager::GetInstance().Log("End　Create CommandList\n");
}

void DXCommand::Close() {
    // コマンドリストの内容を確定させる。すべてのコマンドを積んでからcloseにすること
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));
}

void DXCommand::Execute() {
    // GPUにコマンドリストの実行を行わせる
    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
}

void DXCommand::Reset() {
    // 次のフレーム用にコマンドリストを準備
    HRESULT hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));
}
