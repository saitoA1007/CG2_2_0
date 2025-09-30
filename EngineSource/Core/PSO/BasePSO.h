#pragma once
#include <d3d12.h>
#include <wrl.h>
#include"DXC.h"
#include"Vector4.h"
#include"Vector2.h"

#include"LogManager.h"

class BasePSO {
public:

    virtual ~BasePSO() = default;

    // 初期化
    virtual void Initialize(ID3D12Device* device, GameEngine::DXC* dxc, LogManager* logManager)=0;

    virtual ID3D12RootSignature* GetRootSignature() const = 0;

    virtual ID3D12PipelineState* GetPipelineState() const = 0;

private:
   
    ID3D12Device* device_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    // ログ
    LogManager* logManager_;
};